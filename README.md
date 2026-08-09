# CAD-Project-utils

一个基于 C++17 的轻量级 CAD 应用程序框架，支持参数化几何建模、Undo/Redo、命令系统和文档持久化。

## 架构概览

项目采用分层模块化设计，依赖方向自上而下：

```
cadutils_demo (exe)
  └─► cadutils_application (SHARED)
        ├─► cadutils_data (SHARED)
        ├─► cadutils_render (SHARED)
        ├─► cadutils_platform (SHARED)
        └─► cadutils_geometry (SHARED)
              ├─► cadutils_render
              ├─► cadutils_common (SHARED)
              └─► thirdparty::occt
```

## 模块职责

| 模块 | 类型 | 职责 |
|------|------|------|
| **Common** | SHARED | 基础类型定义层，包含 `AnyValue` 类型擦除值、`Point3d`、`Vector3d`，无外部依赖 |
| **Data** | SHARED | 文档数据模型核心，包含 `Document`、`Object` 体系、`Property<T>` 属性系统、元数据注册 |
| **Platform** | SHARED | 事务/Undo-Redo + 命令基础设施层，包含 `Transaction`、`TransactionManager`、`ICommand`、`CommandRegistry` |
| **Geometry** | SHARED | 几何造型层，封装 OpenCASCADE (OCCT)，提供 `IBody`、`GeoBuildUtils`、`MeshGenerator` |
| **Render** | SHARED | 渲染层，封装 OpenSceneGraph + Qt5，提供 `IRenderView`、`IGraphicsNode` |
| **Application** | SHARED | 应用层，包含 `MainWindow`（Qt 主窗口）、`GraphicsScene`、`RenderSystem`、命令实现、`CommandUIBuilder` |

## 核心特性

### 1. 属性系统

编译期宏驱动的属性注册系统，通过 `CAD_OBJECT_BEGIN` / `CAD_PROP` / `CAD_DEFAULT_CTOR` 宏自动完成属性的声明、注册和元数据生成。

```cpp
class SphereObject : public Object {
    CAD_OBJECT_BEGIN(SphereObject)
        CAD_PROP(Point3d, center, DirtyFlags::Geometry)
        CAD_PROP(double, radius, DirtyFlags::Geometry)
    CAD_OBJECT_END;
};
```

- 编译期 FNV-1a 哈希生成全局唯一的 PropertyId
- 属性变更自动触发通知链（`IPropertyChangeSink`）
- `TypeMeta` 持有时属性描述符，支持按 ID/偏移/名称三种方式查找

### 2. 事务系统与 Undo/Redo

完整的事务栈管理器，支持三种操作类型：

| 操作类型 | Undo 行为 | Redo 行为 |
|----------|-----------|-----------|
| 属性修改 | 恢复旧值 | 重新应用新值 |
| 对象创建 | 从文档移除 | 恢复到文档 |
| 对象删除 | 恢复到文档 | 从文档移除 |

- `Transaction` 通过 `vector<ChangeEntry>` 保持操作顺序，Undo 时逆序回放
- 被删除对象通过 `shared_ptr` 保存在 `ChangeEntry` 中，确保不会被析构
- `ExecStateGuard` RAII 机制防止回放期间重复录入历史

### 3. 命令系统

UI 操作与业务逻辑分离的命令模式：

- `ICommand` 抽象接口，定义 `GetId()` / `GetName()` / `CanExecute()` / `Execute()`
- `REGISTER_COMMAND` 宏实现自动注册，程序启动时工厂函数自动注册到 `CommandRegistry` 单例
- UI 布局由 `config/ui_layout.xml` XML 配置文件驱动，`CommandUIBuilder` 运行时解析并构建菜单/工具栏
- 添加新命令只需：实现类 + `REGISTER_COMMAND` + XML 配置，无需修改其他文件

### 4. 文档持久化

基于 JSON 的文档保存/加载（`.cad` 文件）：

- `JsonSerializer` 通过 `PropertyDescriptor` 的偏移量直接访问属性值进行序列化
- 反序列化使用 `MetaRegistry` 根据类型名创建对象实例
- `SetValueSilent` 恢复属性值，避免触发变更通知链
- 使用 `addWithId` 保持原始对象 ID
- 文件包含 `version` 字段，支持未来格式升级

### 5. DirtyFlags 与渲染同步

| Flag | 含义 |
|------|------|
| `None` | 无变更 |
| `Visual` | 外观变化（颜色、材质） |
| `Transform` | 位置/变换变化 |
| `Geometry` | 几何形状变化（需重建 mesh） |

- 属性变更后自动聚合脏标记到 Document
- `RenderSystem::SyncFromDocument()` 增量处理脏对象
- `RenderSystem::FullSyncFromDocument()` 全量同步（用于 Undo/Redo 和文档加载）

### 6. AnyValue 类型擦除

基于 `std::string` 的值容器，作为事务系统和序列化的值传输载体：

| 支持类型 | 序列化格式 |
|----------|-----------|
| `int`, `uint`, `long`, `float`, `double` 等 | `std::to_string` |
| `bool` | `"true"` / `"false"` |
| `std::string` | 原样存储 |
| `Point3d` | `"x,y,z"` 格式 |

不支持的类型（如 `shared_ptr<IBody>`）返回空 `AnyValue`，不参与 Undo/Redo。

## 第三方依赖

- **Qt5** — UI 框架（MainWindow、QAction、属性面板、树视图）
- **OpenCASCADE (OCCT)** — 几何造型内核
- **OpenSceneGraph (OSG) 3.6.5** — 3D 渲染引擎

## 构建

项目使用 CMake 构建系统：

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

post-build 步骤会自动将 Qt/OSG 运行库复制到输出目录；`config/ui_layout.xml` 不再自动复制，`MainWindow` 启动时按相对路径（`applicationDirPath` 向上逐级）搜索，从仓库根目录运行时直接命中源 `config/`，发布时需将 `config/` 与可执行文件一并部署。

## 项目结构

```
CAD-Project-utils/
├── src/
│   ├── main.cpp                    # 程序入口
│   ├── CMakeLists.txt              # 顶层 CMake
│   ├── Common/Public/              # 基础类型（AnyValue, Point3d, Vector3d）
│   ├── Data/                       # 数据模型（Document, Object, Property）
│   │   ├── Public/                 #   公开头文件 + 模板
│   │   └── Private/                #   实现 + 具体类型（SphereObject 等）
│   ├── Platform/                   # 事务 + 命令基础设施
│   │   ├── Public/                 #   Transaction, TransactionManager, ICommand, CommandRegistry
│   │   └── Private/                #   实现
│   ├── Geometry/                   # 几何造型（封装 OCCT）
│   ├── Render/                     # 渲染（封装 OSG + Qt）
│   └── Application/                # 应用层
│       ├── UI/                     #   MainWindow, CommandUIBuilder
│       ├── App/                    #   RenderSystem
│       ├── Scene/                  #   GraphicsScene
│       └── Commands/               #   具体命令实现
├── config/
│   └── ui_layout.xml               # UI 布局配置
├── docs/                           # 设计文档
│   ├── property-system-design.md
│   ├── transaction-system-design.md
│   ├── command-system-design.md
│   └── persistence-system-design.md
├── third_party/                    # 第三方库
│   └── osg-3.6.5/
├── AI_CONTEXT.md                   # AI 开发上下文文档
└── README.md
```

## 设计文档

- [属性系统设计](docs/property-system-design.md)
- [事务系统设计](docs/transaction-system-design.md)
- [命令系统设计](docs/command-system-design.md)
- [持久化系统设计](docs/persistence-system-design.md)
