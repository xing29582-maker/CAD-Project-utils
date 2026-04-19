# 后续开发建议（基于当前代码状态）

## 1. 目的

本文档基于当前工程实现状态、已有设计文档以及最新编译结果，对项目接下来的开发方向进行整理，目标是：

- 明确当前系统已经完成到什么程度
- 判断下一阶段最值得投入的开发主线
- 给出推荐的执行顺序和拆解方案
- 降低后续实现过程中的返工风险

---

## 2. 当前项目状态判断

从 [`AI_CONTEXT.md`](../AI_CONTEXT.md) 和 [`plans/next-dev-plan.md`](../plans/next-dev-plan.md) 可以看出，项目已经完成了核心基础设施搭建，并进入“从 Demo 骨架走向可持续扩展原型”的阶段。

### 2.1 已经具备的能力

当前代码已经具备以下核心能力：

- [`Document`](../src/Data/Public/Document.h) / [`IObject`](../src/Data/Public/IObject.h) / [`Object`](../src/Data/Private/Object.h) 数据模型
- [`Property<T>`](../src/Data/Public/Property.h) + [`PropertyDescriptor`](../src/Data/Public/PropertyDescriptor.h) + [`TypeMeta`](../src/Data/Public/TypeMeta.h) 的属性与元数据系统
- [`Transaction`](../src/Platform/Public/Transaction.h) + [`TransactionManager`](../src/Platform/Public/TransactionManager.h) 的 Undo/Redo 事务回放机制
- [`ICommand`](../src/Platform/Public/ICommand.h) + [`CommandRegistry`](../src/Platform/Public/CommandRegistry.h) + [`REGISTER_COMMAND`](../src/Platform/Public/CommandRegistry.h:53) 的命令框架
- [`CommandUIBuilder`](../src/Application/UI/CommandUIBuilder.h) + [`config/ui_layout.xml`](../config/ui_layout.xml) 的 XML 驱动 UI
- [`RenderSystem`](../src/Application/App/RenderSystem.h) → [`GraphicsScene`](../src/Application/Scene/GraphicsScene.h) → [`IRenderView`](../src/Render/Public/IRenderView.h) 的渲染同步链
- Sphere / Box / Cylinder 三类基础几何对象扩展能力
- 3D 视图、树视图、属性面板的选中联动能力

### 2.2 当前阶段的真实结论

项目当前已经不是“缺基础框架”，而是“缺业务闭环”。

也就是说，现在最缺的不是再多加几个对象类型，而是让现有对象具备：

- 可保存
- 可加载
- 可恢复工作现场
- 可稳定驱动 UI 命令状态
- 可继续扩展更多对象而不破坏现有结构

---

## 3. 阶段三的当前结论

根据最近的构建结果，阶段三“对象类型扩展”已经达到以下状态：

- [`BoxObject`](../src/Data/Private/BoxObject.h) / [`CylinderObject`](../src/Data/Private/CylinderObject.h) 已加入工程
- [`GeoBuildUtils`](../src/Geometry/Public/GeoBuildUtils.h) 已扩展 Box / Cylinder 造型能力
- [`ObjectFactory`](../src/Data/Public/ObjectFactory.h) 已扩展对象创建入口
- 已新增对应命令与 UI 项
- 工程已成功通过 CMake 重新生成与 Release 编译

### 3.1 目前剩余的不是“能不能编译”，而是“要不要进入验收”
建议把阶段三视为“代码完成，等待功能验收”。

建议验收项：

- 新建 Box 是否成功显示
- 新建 Cylinder 是否成功显示
- 树视图对象名称是否正确
- 属性面板是否正确显示对象参数
- 修改参数后是否触发几何更新
- 删除、Undo、Redo 对新对象是否工作正常
- 选中联动是否对新对象同样成立

如果以上都正常，可以正式把阶段三标记为完成。

---

## 4. 接下来最优先的开发主线

## 4.1 第一优先级：文档持久化

这是当前最值得优先推进的方向。

### 原因

#### 原因一：当前元数据系统已经天然适合做通用序列化
现有的：

- [`TypeMeta`](../src/Data/Public/TypeMeta.h)
- [`PropertyDescriptor`](../src/Data/Public/PropertyDescriptor.h)
- [`Property<T>`](../src/Data/Public/Property.h)
- [`AnyValue`](../src/Common/Public/NameDefine.h)

已经提供了接近“反射式序列化”的基础能力。

这意味着你不需要为每个对象单独手写大量序列化代码。

#### 原因二：没有 Save/Load，当前对象扩展价值无法沉淀
现在即使有了 [`SphereObject`](../src/Data/Private/SphereObject.h)、[`BoxObject`](../src/Data/Private/BoxObject.h)、[`CylinderObject`](../src/Data/Private/CylinderObject.h)，使用后仍无法保存工作结果，测试效率和可用性都会很受限制。

#### 原因三：后续很多能力都依赖持久化
后续如果要做：

- 文件菜单
- 新建 / 打开 / 保存
- 对象模板
- 自动化测试数据
- 参数化建模缓存
- 项目恢复

都需要一个稳定的文档格式作为中间契约。

### 结论

当前最合理的主线应当是：

**优先完成基于 JSON 的文档持久化能力。**

---

## 5. 文档持久化的建议实现方案

## 5.1 目标范围

建议先做“最小可用版本”，不要一开始就追求复杂格式和兼容策略。

第一版建议只支持：

- 当前文档全量保存
- 当前文档全量加载
- Sphere / Box / Cylinder 三类对象
- 属性值按字符串形式持久化
- 加载后执行一次完整重建和渲染同步

不建议第一版就做：

- 局部导入
- 增量合并
- 历史版本兼容
- 局部对象引用修复
- 复杂类型二进制序列化

---

## 5.2 建议的 JSON 结构

建议定义为一个稳定、可版本化的结构，例如：

```json
{
  "version": 1,
  "documentName": "Untitled",
  "objects": [
    {
      "type": "SphereObject",
      "id": 1,
      "properties": {
        "objName": "Sphere_1",
        "center": "0,0,0",
        "radius": "50"
      }
    }
  ]
}
```

### 设计建议

- `version`：必须有，为后续格式升级保留空间
- `documentName`：对应 [`Document`](../src/Data/Public/Document.h) 的文档名
- `type`：必须使用稳定字符串，不能依赖运行时偶然类名
- `id`：保留对象 ID，方便恢复对象身份
- `properties`：第一版统一使用字符串，与 [`AnyValue`](../src/Common/Public/NameDefine.h) 当前机制保持一致

---

## 5.3 序列化策略

### 序列化时

建议流程：

1. 遍历 [`Document`](../src/Data/Public/Document.h) 中全部对象
2. 对每个对象获取 [`IObject::GetTypeMeta()`](../src/Data/Public/IObject.h)
3. 遍历其中的 [`PropertyDescriptor`](../src/Data/Public/PropertyDescriptor.h)
4. 读取每个属性当前值
5. 写入 JSON

### 需要特别注意

有些属性不应持久化，例如：

- [`shapeBody`](../src/Data/Private/Object.h)

因为这类属性是运行时缓存，不是业务数据。加载后应通过参数重新生成，而不是从文件直接恢复。

### 因此建议
应显式定义“哪些属性参与持久化”，而不是简单无差别遍历全部属性。

可选方案：

- 方案 A：按属性名排除，如排除 `shapeBody`
- 方案 B：后续给 [`PropertyDescriptor`](../src/Data/Public/PropertyDescriptor.h) 增加 `serializable` 标记
- 方案 C：在第一版手工维护一个忽略列表

建议第一版先使用简单可控的忽略列表，后续再升级为元数据标记。

---

## 5.4 反序列化策略

建议流程：

1. 读取 JSON 根对象
2. 创建新的 [`Document`](../src/Data/Public/Document.h)
3. 遍历 `objects`
4. 通过 `type` 调用对象工厂创建实例
5. 用 `properties` 回写属性
6. 恢复对象 ID
7. 文档完成后执行一次完整同步与渲染刷新

关键点：

- 不要在反序列化过程中频繁局部刷新
- 完成所有对象加载后，再统一调用完整同步
- 事务栈应在 Load 后重置，避免把“加载文件”与用户编辑历史混在一起

---

## 5.5 建议新增或修改的代码位置

推荐涉及以下文件：

- 新增 [`src/Data/Public/ISerializer.h`](../src/Data/Public/Document.h)
- 新增 `src/Data/Private/JsonSerializer.h`
- 新增 `src/Data/Private/JsonSerializer.cpp`
- 修改 [`src/Data/Public/Document.h`](../src/Data/Public/Document.h)
- 修改 `src/Data/Private/Document.cpp`
- 新增 `SaveCommand`
- 新增 `LoadCommand`
- 修改 [`config/ui_layout.xml`](../config/ui_layout.xml)

如果不想一开始抽象接口，也可以先直接在 [`Document`](../src/Data/Public/Document.h) 中落地：

- `bool SaveToFile(const std::string& path)`
- `bool LoadFromFile(const std::string& path)`

但从长期维护角度，建议仍保留 Serializer 抽象层。

---

## 6. 建议在阶段四顺手完成的重构：ObjectFactory 通用化

## 6.1 为什么要一起做

文档加载一定会遇到这个问题：

- 读到 `"SphereObject"`，如何创建 [`SphereObject`](../src/Data/Private/SphereObject.h)
- 读到 `"BoxObject"`，如何创建 [`BoxObject`](../src/Data/Private/BoxObject.h)
- 读到 `"CylinderObject"`，如何创建 [`CylinderObject`](../src/Data/Private/CylinderObject.h)

如果 [`ObjectFactory`](../src/Data/Public/ObjectFactory.h) 继续保持纯硬编码静态函数形式，后续类型越多，分支越散，维护成本会逐渐升高。

---

## 6.2 推荐方案

建议把 [`ObjectFactory`](../src/Data/Public/ObjectFactory.h) 设计成“两层结构”。

### 第一层：保留现有便捷接口
例如继续保留：

- `CreateSphereObject()`
- `CreateBoxObject()`
- `CreateCylinderObject()`

这样不会破坏已有调用点。

### 第二层：增加注册式接口
新增类似能力：

```cpp
using ObjectCreator = std::function<std::shared_ptr<IObject>(const std::string& name)>;

static void Register(const std::string& typeName, ObjectCreator creator);
static std::shared_ptr<IObject> CreateByType(const std::string& typeName, const std::string& name);
```

这样在反序列化时，只需要：

- 从 JSON 读出 `type`
- 调用 `CreateByType(type, name)`

即可完成实例创建。

---

## 6.3 设计建议

- `typeName` 应与持久化格式中的 `type` 保持一致
- 每个对象类最好定义稳定的类型名常量
- 不建议在多个位置散落重复字符串字面量
- 后续如果对象越来越多，可以考虑类似 [`REGISTER_COMMAND`](../src/Platform/Public/CommandRegistry.h:53) 的对象注册宏，但当前阶段不是必须

---

## 7. 第二优先级：命令系统增强

当前 [`CommandUIBuilder`](../src/Application/UI/CommandUIBuilder.h) 已经可以把 XML 转为 QAction，但它还没有形成真正稳定的“命令状态管理”。

目前的主要问题是：

- QAction 的 enabled 状态没有与文档状态实时同步
- `CanExecute()` 更像是点击时的二次判断，而不是 UI 的驱动依据

这在对象数量、命令数量增加后会越来越明显。

---

## 7.1 先做命令可用状态实时刷新

### 目标

在以下场景变化时，相关 QAction 能自动更新 enabled：

- 选中对象变化
- 对象增加 / 删除
- Undo / Redo 后
- 文档 Load / New 后
- 命令执行完成后

### 典型收益

- [`DeleteSelectedCommand`](../src/Application/Commands/DeleteSelectedCommand.cpp) 没有选中对象时应禁用
- [`UndoCommand`](../src/Application/Commands/UndoCommand.cpp) 没有可撤销事务时应禁用
- [`RedoCommand`](../src/Application/Commands/RedoCommand.cpp) 没有可重做事务时应禁用

---

## 7.2 推荐增加 ActionManager

不建议把刷新逻辑继续堆在 [`CommandUIBuilder`](../src/Application/UI/CommandUIBuilder.h) 里，建议引入一个轻量的 Action 管理层。

比如新增一个 `ActionManager`，职责包括：

- 保存 `commandId -> QAction`
- 保存每个 QAction 对应的命令创建方式或刷新逻辑
- 提供统一 `RefreshAll()` 接口
- 在关键时机由 [`MainWindow`](../src/Application/UI/MainWindow.h) 触发刷新

这样命令 UI 的职责会更清晰：

- [`CommandUIBuilder`](../src/Application/UI/CommandUIBuilder.h)：负责“创建”
- `ActionManager`：负责“维护状态”
- [`ICommand`](../src/Platform/Public/ICommand.h)：负责“业务执行 + 可执行性判断”

---

## 7.3 命令元数据建议内聚到 ICommand

当前 XML 中已经能配置 shortcut，但从长期看，命令自身也应该有一定的元数据能力。

建议未来在 [`ICommand`](../src/Platform/Public/ICommand.h) 中增加可选接口，例如：

```cpp
virtual std::string GetIcon() const { return ""; }
virtual std::string GetTooltip() const { return GetName(); }
virtual std::string GetShortcut() const { return ""; }
```

这样可以形成更好的分工：

- 命令类负责声明默认元数据
- XML 负责布局与可选覆盖
- UI 构建层负责最终合并展示

---

## 8. 参数化建模准备：先做设计，不建议立刻编码

参数化建模是长期方向，但不适合作为当前直接实现任务。

### 原因

虽然当前已经有：

- [`Property<T>`](../src/Data/Public/Property.h)
- [`DirtyFlags`](../src/Data/Public/DirtyFlags.h)
- [`Transaction`](../src/Platform/Public/Transaction.h)

但还没有：

- 属性依赖图
- 自动重算顺序
- 循环依赖检测
- “源属性”和“派生属性”的边界定义
- 回放时的依赖重建规则

如果现在直接编码，容易让原本清晰的属性通知链和事务链变复杂。

---

## 8.1 更合理的推进方式

建议先补一份专门设计文档，例如：

- `docs/parametric-modeling-preparation.md`

至少先回答这些问题：

1. 依赖关系存放在哪里
2. 属性变化后何时触发重算
3. 如何避免循环依赖
4. Undo/Redo 时记录源属性还是派生结果
5. 多对象之间是否允许依赖

先形成清晰设计，再决定代码落地方案。

---

## 9. 推荐的执行顺序

基于当前代码状态，建议执行顺序调整为：

### 步骤一：阶段三功能验收
目标：把“编译通过”转为“功能闭环成立”。

### 步骤二：文档持久化最小可用版本
目标：支持当前文档 Save / Load。

### 步骤三：ObjectFactory 注册化
目标：为 Save / Load 和未来对象扩展提供统一创建入口。

### 步骤四：ActionManager + 命令状态刷新
目标：让 UI 可用状态和文档状态一致。

### 步骤五：补充设计文档
建议新增：

- `docs/persistence-design.md`
- `docs/object-factory-design.md`
- `docs/command-ui-refresh-design.md`
- `docs/parametric-modeling-preparation.md`

---

## 10. 风险点与注意事项

## 10.1 编码警告 C4819
当前多个文件存在编码警告，例如：

- [`src/Data/Public/Property.h`](../src/Data/Public/Property.h)
- [`src/Data/Public/PropertyRegistry.h`](../src/Data/Public/PropertyRegistry.h)
- [`src/Data/Public/Document.h`](../src/Data/Public/Document.h)
- [`src/Application/UI/MainWindow.cpp`](../src/Application/UI/MainWindow.cpp)

这虽然不阻塞编译，但后续如果继续增加中文注释或字符串，可能导致乱码或 diff 混乱。建议后续统一转为 UTF-8。

---

## 10.2 DLL 导出警告 C4251 / C4275
当前 [`Document`](../src/Data/Public/Document.h)、[`TransactionManager`](../src/Platform/Public/TransactionManager.h)、[`CommandRegistry`](../src/Platform/Public/CommandRegistry.h) 暴露 STL 成员，存在典型的 DLL 接口 warning。

短期内可以接受，但如果后续希望把这些模块长期稳定化，建议逐步考虑：

- pImpl
- 接口 / 实现分离
- 减少导出类中直接暴露 STL 成员

---

## 10.3 shapeBody 不应进入持久化
[`shapeBody`](../src/Data/Private/Object.h) 是运行时几何缓存，不是业务源数据，不应保存到 JSON。

正确方式应是：

- 持久化参数属性
- 加载后重新调用几何构建流程

---

## 10.4 对象 ID 恢复策略要提前定
Load 时是否保留对象 `id`，会影响：

- 树视图恢复
- 后续选中逻辑
- 下一次对象 ID 分配
- 事务与文档状态重置策略

建议第一版加载时保留文件中的对象 ID，并同步修正 [`Document`](../src/Data/Public/Document.h) 的下一个可分配 ID。

---

## 10.5 Load 后事务栈应清空
加载文件后，旧文档的 Undo/Redo 栈通常不应继续沿用。

因此建议：

- Load 成功后重建或重置 [`TransactionManager`](../src/Platform/Public/TransactionManager.h)
- 把加载行为视为“新文档状态建立”，而不是普通编辑事务

---

## 11. 最终结论

从当前代码状态看，接下来的开发重点不应再放在“继续添加更多几何体类型”，而应放在让现有系统形成真正的业务闭环。

### 建议主线

1. 完成阶段三功能验收
2. 进入“文档持久化”开发
3. 在阶段四中顺手完成 [`ObjectFactory`](../src/Data/Public/ObjectFactory.h) 的注册化重构
4. 然后实现命令 UI 状态实时刷新
5. 参数化建模先写设计，不急于编码

### 一句话概括

**当前最该做的是让已有对象“能保存、能加载、能稳定管理命令状态”，而不是继续单纯增加几何类型。**

这一步完成后，项目才真正从“技术演示原型”进入“可持续迭代的 CAD 原型框架”阶段。