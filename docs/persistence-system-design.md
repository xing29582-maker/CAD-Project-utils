# 持久化系统设计文档

## 概述

本文档描述了 CAD-Project-utils 的持久化系统设计与实现，包括文档的保存和加载功能。

## 系统架构

### 核心组件

1. **JsonSerializer** - JSON 序列化/反序列化引擎
2. **Document** - 文档管理，提供保存/加载接口
3. **MetaRegistry** - 类型元数据注册表
4. **TypeMeta** - 类型元数据，包含属性描述和对象创建器
5. **RegisterAllTypes** - 类型注册初始化

### 文件格式

使用 JSON 格式存储文档数据，文件扩展名为 `.cad`。

```json
{
  "version": 1,
  "documentName": "Document",
  "nextObjectId": 4,
  "objects": [
    {
      "type": "SphereObject",
      "id": 1,
      "properties": {
        "objName": "Sphere1",
        "center": "0,0,0",
        "radius": "5.0"
      }
    }
  ]
}
```

## 序列化机制

### 属性序列化

通过 `PropertyDescriptor` 的 `serializable` 标志控制属性是否参与序列化：

- `CAD_PROP` - 可序列化属性
- `CAD_PROP_TRANSIENT` - 不可序列化属性（如运行时计算的数据）

### 对象序列化流程

1. 遍历文档中的所有对象
2. 获取对象的 TypeMeta
3. 遍历所有可序列化属性
4. 通过属性偏移量获取属性值
5. 将属性值转换为字符串并写入 JSON

```cpp
json JsonSerializer::SerializeObject(const std::shared_ptr<IObject>& obj)
{
    json jObj;
    const TypeMeta& meta = obj->GetTypeMeta();
    jObj["type"] = meta.typeName;
    jObj["id"] = obj->GetObjectId();

    json jProps;
    for (const auto& desc : meta.properties)
    {
        if (!desc.serializable)
            continue;

        auto* propBase = reinterpret_cast<const PropertyBase*>(
            reinterpret_cast<const char*>(obj.get()) + desc.offset);

        AnyValue val = propBase->Value();
        if (!val.text.empty())
        {
            jProps[desc.name] = val.text;
        }
    }

    jObj["properties"] = jProps;
    return jObj;
}
```

## 反序列化机制

### 类型注册

在程序启动时，必须注册所有对象类型到 MetaRegistry：

```cpp
void RegisterAllTypes()
{
    // 强制静态初始化所有对象类型
    (void)SphereObject::StaticTypeMeta();
    (void)BoxObject::StaticTypeMeta();
    (void)CylinderObject::StaticTypeMeta();
}
```

在 `main.cpp` 中调用：

```cpp
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    
    // 确保所有对象类型注册到 MetaRegistry
    cadutils::RegisterAllTypes();
    
    // ...
}
```

### 对象反序列化流程

1. 从 JSON 读取类型名和对象 ID
2. 通过 MetaRegistry 根据类型名创建对象实例
3. 遍历 JSON 中的属性
4. 通过 PropertyDescriptor 的 applyAny 函数设置属性值
5. 使用 `SetValueSilent` 避免触发属性变更通知

```cpp
std::shared_ptr<IObject> JsonSerializer::DeserializeObject(const json& j)
{
    std::string typeName = j["type"].get<std::string>();
    ObjectId id = j["id"].get<ObjectId>();

    // 通过 MetaRegistry 创建对象
    auto obj = MetaRegistry::Instance().CreateByTypeName(typeName.c_str());
    if (!obj)
        return nullptr;

    // 恢复属性
    if (j.contains("properties"))
    {
        const TypeMeta& meta = obj->GetTypeMeta();
        const json& jProps = j["properties"];

        for (auto it = jProps.begin(); it != jProps.end(); ++it)
        {
            const std::string& propName = it.key();
            const std::string& propValue = it.value().get<std::string>();

            const PropertyDescriptor* desc = meta.FindByName(propName);
            if (desc && desc->applyAny && desc->serializable)
            {
                desc->applyAny(*obj, AnyValue(propValue));
            }
        }
    }

    return obj;
}
```

### 文档加载流程

1. 清空当前文档
2. 从 JSON 反序列化所有对象
3. 使用 `addWithId` 恢复原始对象 ID
4. 恢复 nextObjectId 计数器
5. 清空事务栈
6. 全量同步渲染系统
7. 重建 UI 树

```cpp
void LoadCommand::Execute(CommandContext& ctx)
{
    QString path = QFileDialog::getOpenFileName(/*...*/);
    if (path.isEmpty())
        return;

    bool success = ctx.doc->LoadFromFile(path.toStdString());
    if (!success)
    {
        QMessageBox::critical(nullptr, "Error", "Failed to load document");
        return;
    }

    // 清空事务栈
    if (ctx.txMgr)
        ctx.txMgr->Clear();

    // 触发全量刷新
    RefreshAfterCommand(ctx);
}
```

## 渲染系统同步

### FullSyncFromDocument

加载文档后，使用 `FullSyncFromDocument` 进行全量同步：

1. 收集文档中所有对象 ID
2. 移除不再存在的图形节点
3. 为所有对象创建/更新图形节点
4. 调用 `buildShape()` 创建几何体
5. 进行网格化处理
6. 设置几何数据到图形节点

```cpp
void RenderSystem::FullSyncFromDocument(const std::shared_ptr<Document>& doc, 
                                        const TessellationOptions& opt)
{
    // 收集当前文档对象 ID
    std::unordered_set<ObjectId> docIds;
    for (const auto& obj : doc->GetObjects())
    {
        docIds.insert(obj->GetObjectId());
    }

    // 移除不再存在的图形节点
    auto allNodes = m_gscene->GetAllNodesWithId();
    for (const auto& [id, node] : allNodes)
    {
        if (docIds.find(id) == docIds.end())
        {
            m_gscene->Remove(id);
        }
    }

    // 添加/更新所有文档对象
    for (const auto& obj : doc->GetObjects())
    {
        const auto id = obj->GetObjectId();
        std::shared_ptr<IGraphicsNode> gnode = m_gscene->GetOrCreate(id);
        std::shared_ptr<IBody> body = obj->buildShape();
        GeometryData geoData = m__mesher.Tessellate(body, opt);
        gnode->SetGeometryData(geoData);
    }
}
```

### 相机自动适配

在 `RenderView::refresh` 中，fullSync 模式下自动调整相机视角：

```cpp
void RenderView::refresh(const std::unordered_map<ObjectId, std::shared_ptr<IGraphicsNode>>& gRepNodes, 
                        bool fullSync)
{
    // ... 更新图形节点 ...

    // fullSync 或有新节点时自动适配相机
    if ((hasNewNodes || fullSync) && m_widget && m_widget->viewer())
    {
        m_widget->viewer()->home();
    }
}
```

## 关键设计决策

### 1. 使用 addWithId 而非 add

反序列化时使用 `addWithId` 保持原始对象 ID，确保引用关系正确：

```cpp
void Document::addWithId(const std::shared_ptr<IObject>& obj, ObjectId id)
{
    m_objects.emplace(id, obj);
    std::shared_ptr<Object> obj2 = std::dynamic_pointer_cast<Object>(obj);
    if (obj2)
    {
        obj2->m_objId.SetValueSilent(id);  // 静默设置，不触发通知
        obj2->SetOwnerDoc(this);
    }

    // 更新 nextId
    if (id >= m_nextId)
    {
        m_nextId = id + 1;
    }
}
```

### 2. 默认构造函数初始化

为对象提供合理的默认值，确保反序列化失败时也能正常工作：

```cpp
CAD_DEFAULT_CTOR(SphereObject)
{
    // 初始化默认值用于反序列化
    m_center.SetValueSilent(Point3d(0, 0, 0));
    m_radius.SetValueSilent(1.0);
}
```

### 3. 类型注册时机

在 main 函数开始时立即注册所有类型，确保反序列化时能找到类型：

```cpp
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    
    // 必须在任何反序列化操作之前调用
    cadutils::RegisterAllTypes();
    
    cadutils::RegisterAllCommands();
    
    cadutils::MainWindow w;
    w.show();
    
    return app.exec();
}
```

## 错误处理

### 序列化错误

- 文件打开失败
- JSON 写入失败
- 捕获所有异常并返回 false

### 反序列化错误

- 文件不存在或无法打开
- JSON 格式错误
- 版本不匹配
- 类型未注册
- 属性应用失败
- 捕获所有异常并返回 false

## 扩展性

### 添加新对象类型

1. 定义对象类，使用 `CAD_OBJECT_BEGIN/END` 宏
2. 使用 `CAD_PROP` 标记可序列化属性
3. 实现默认构造函数（使用 `CAD_DEFAULT_CTOR`）
4. 在 `RegisterAllTypes()` 中添加类型注册

```cpp
// 新对象类型
class NewObject : public Object
{
    CAD_OBJECT_BEGIN(NewObject);
        CAD_PROP(Point3d, position, DirtyFlags::Geometry)
        CAD_PROP(double, size, DirtyFlags::Geometry)
    CAD_OBJECT_END;
    
public:
    NewObject();
    // ...
};

// 注册
void RegisterAllTypes()
{
    (void)SphereObject::StaticTypeMeta();
    (void)BoxObject::StaticTypeMeta();
    (void)CylinderObject::StaticTypeMeta();
    (void)NewObject::StaticTypeMeta();  // 添加新类型
}
```

### 版本升级

在 JSON 中包含版本号，支持未来的格式升级：

```cpp
bool JsonSerializer::DeserializeDocument(Document& doc, const json& j)
{
    int version = j["version"].get<int>();
    
    if (version == 1)
    {
        // 版本 1 的反序列化逻辑
    }
    else if (version == 2)
    {
        // 版本 2 的反序列化逻辑（未来）
    }
    else
    {
        return false;  // 不支持的版本
    }
}
```

## 性能考虑

1. **属性访问优化**：通过偏移量直接访问属性，避免虚函数调用
2. **批量操作**：一次性序列化/反序列化所有对象
3. **延迟渲染**：加载完成后统一刷新渲染系统
4. **内存管理**：使用智能指针自动管理对象生命周期

## 测试建议

1. 保存空文档
2. 保存包含各种对象类型的文档
3. 加载不存在的文件
4. 加载损坏的 JSON 文件
5. 加载旧版本文件
6. 保存后立即加载，验证数据一致性
7. 多次保存加载循环测试

## 已知限制

1. 不支持对象间的引用关系序列化
2. 不支持自定义属性类型的自动序列化
3. 文件格式为纯文本，较大文档可能占用较多空间
4. 不支持增量保存/加载

## 未来改进方向

1. 支持二进制格式以减小文件大小
2. 支持增量保存（仅保存变更）
3. 支持对象引用关系
4. 支持自定义序列化器
5. 添加文件压缩
6. 支持自动备份
7. 添加文件加密选项