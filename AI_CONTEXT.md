cadutils_demo (exe)
  └─► cadutils_application (STATIC)
        ├─► cadutils_data (SHARED)
        ├─► cadutils_render (SHARED)
        ├─► cadutils_platform (SHARED)
        └─► cadutils_geometry (SHARED)
              ├─► cadutils_render
              ├─► cadutils_common (SHARED)
              └─► thirdparty::occt

cadutils_platform (SHARED)  ──► cadutils_data
cadutils_render             ──► thirdparty::osg, Qt5, cadutils_common
cadutils_common (SHARED)    ──  无外部依赖（最底层）


各模块职责
Common — 基础类型定义层。[`AnyValue`](src/Common/Public/NameDefine.h) 基于 string 的类型擦除值，支持算术类型、[`std::string`](src/Common/Public/NameDefine.h)、[`Point3d`](src/Common/Public/Point3d.h)。还包含 [`Point3d`](src/Common/Public/Point3d.h)（含 `ToString/FromString`）、[`Vector3d`](src/Common/Public/Vector3d.h)。

Data — 文档数据模型核心。包含 [`Document`](src/Data/Public/Document.h)（含对象增删 `add/remove/restore`）、[`Object`](src/Data/Private/Object.h) 体系、[`Property`](src/Data/Public/Property.h) 系统、元数据注册。

Platform — 事务/Undo-Redo + 命令基础设施层。包含 [`Transaction`](src/Platform/Public/Transaction.h)（纯数据记录类，支持属性变更+对象增删三种操作）、[`TransactionManager`](src/Platform/Public/TransactionManager.h)（事务栈管理器）、[`ICommand`](src/Platform/Public/ICommand.h)、[`CommandRegistry`](src/Platform/Public/CommandRegistry.h) 和 [`REGISTER_COMMAND`](src/Platform/Public/CommandRegistry.h:53) 宏。

Geometry — 几何造型层，封装 OpenCASCADE。提供 [`IBody`](src/Geometry/Public/IBody.h)、[`GeoBuildUtils`](src/Geometry/Public/GeoBuildUtils.h)、[`MeshGenerator`](src/Geometry/Public/MeshGenerator.h) 等。

Render — 渲染层，封装 OpenSceneGraph + Qt。提供 [`IRenderView`](src/Render/Public/IRenderView.h)、[`IGraphicsNode`](src/Render/Public/IGraphicsNode.h)。

Application — 应用层（静态库），包含 [`MainWindow`](src/Application/UI/MainWindow.h)（Qt 主窗口）、[`GraphicsScene`](src/Application/Scene/GraphicsScene.h)（场景图管理）、[`RenderSystem`](src/Application/App/RenderSystem.h)（文档→渲染同步）、命令实现、[`CommandUIBuilder`](src/Application/UI/CommandUIBuilder.h)（XML → QAction 构建）。

Document / Object / Property 体系
[`Document`](src/Data/Public/Document.h) — 文档容器，持有 `unordered_map<ObjectId, shared_ptr<IObject>>`，自增分配 ID。实现 [`IDirtySink`](src/Data/Public/IDirtySink.h) 接口收集脏标记。持有一个 [`IPropertyChangeSink`](src/Data/Public/IPropertyChangeSink.h)`* m_changeSink` 指针，由 [`TransactionManager::BeginTransaction()`](src/Platform/Public/TransactionManager.h:21) 设置为当前活跃的 [`Transaction`](src/Platform/Public/Transaction.h)。支持 `add/remove/restore` 三种对象管理操作，`add/remove` 在非回放状态下会通知 `changeSink`。

[`IObject`](src/Data/Public/IObject.h) — 纯虚接口，定义 `GetObjectName()`、`GetObjectId()`、`buildShape()`、`SetParameters()`、`GetParameters()`、`OnPropertyChanging/Changed()`、`GetTypeMeta()` 等。

[`Object`](src/Data/Private/Object.h) — [`IObject`](src/Data/Public/IObject.h) 的基础实现，使用宏系统声明了三个属性：`objName(string)`、`objId(uint64)`、`shapeBody(shared_ptr<IBody>)`。持有 [`Document`](src/Data/Public/Document.h)`*` 反向指针。属性变更时委托给 [`Document::OnPropertyChanging()`](src/Data/Public/Document.h:49) / [`Document::OnPropertyChanged()`](src/Data/Public/Document.h:50)。

[`SphereObject`](src/Data/Private/SphereObject.h) — [`Object`](src/Data/Private/Object.h) 的具体子类，额外声明 `center(Point3d)` 和 `radius(double)` 两个属性，带 [`DirtyFlags::Geometry`](src/Data/Public/DirtyFlags.h)。

[`Property<T>`](src/Data/Public/Property.h) — 模板属性类，继承 [`PropertyBase`](src/Data/Public/Property.h)。每个属性持有 `PropertyId + DirtyFlags`。`set()` 时先调 `NotifyChanging()`（通知 owner 的 `OnPropertyChanging`），再赋值，再调 `NotifyChanged()`。`SetValueSilent()` 跳过通知，用于 Undo/Redo 回放。通过 `Bind()` 绑定 owner 对象指针。`Value()` 对 [`AnyValueSupported`](src/Common/Public/NameDefine.h) 的类型返回 [`AnyValue`](src/Common/Public/NameDefine.h)，不支持的类型返回空值。

属性宏注册系统
[`PropertyRegistry.h`](src/Data/Public/PropertyRegistry.h) 定义了一套编译期属性注册宏：

- [`CAD_OBJECT_BEGIN`](src/Data/Public/PropertyRegistry.h:88) — 声明类的属性基础设施，包括 `_cad_init_properties()`、`_BindAllProps()`、`StaticTypeMeta()`。
- [`CAD_PROP`](src/Data/Public/PropertyRegistry.h:116) — 声明一个 `Property<T> m_##name` 成员，同时通过 `PropertyRegistry<ThisClass>::Registrar` 静态变量在加载时自动注册到 `EntryList()`。属性 ID 通过 [`CAD_PROP_ID`](src/Data/Public/PropertyRegistry.h:86) 编译期计算。
- [`CAD_DEFAULT_CTOR`](src/Data/Public/PropertyRegistry.h:154) — 展开默认构造函数 + `StaticTypeMeta()` 实现，将所有注册的 `Entry` 转为 [`PropertyDescriptor`](src/Data/Public/PropertyDescriptor.h) 存入 [`TypeMeta`](src/Data/Public/TypeMeta.h)，并注册到全局 [`MetaRegistry`](src/Data/Public/MetaRegistry.h) 单例。

[`TypeMeta`](src/Data/Public/TypeMeta.h) 持有属性描述符列表，并建立 `id/offset/name` 三种索引 map，支持按 ID、偏移量、名称查找属性描述符。

[`PropertyDescriptor`](src/Data/Public/PropertyDescriptor.h) 包含 `id`、`name`、`flags`、`offset`（成员偏移）、`applyAny` 函数指针（用于 Undo 时静默写入值）。

Undo/Redo / Transaction 机制（TransactionManager 架构）
[`Transaction`](src/Platform/Public/Transaction.h) — 纯数据记录类，实现 [`IPropertyChangeSink`](src/Data/Public/IPropertyChangeSink.h) 接口，内部使用 `vector<ChangeEntry>` 保持操作顺序：
- `OnPropertyChanging()` 记录 `oldValue`（同一属性只记第一次）
- `OnPropertyChanged()` 记录 `newValue`
- `OnObjectAdded()` 记录对象创建（持有 `shared_ptr`）
- `OnObjectRemoved()` 记录对象删除（持有 `shared_ptr` 保活）
- `GetEntries()` 返回变更记录只读引用
- `IsEmpty()` 判断是否有变更

[`ChangeEntry`](src/Platform/Public/ChangeKey.h) 有三种类型（[`ChangeType`](src/Platform/Public/ChangeKey.h)）：`PropertyChange`、`ObjectAdd`、`ObjectRemove`。对象增删的 `ChangeEntry` 持有 `shared_ptr<IObject>`，保证被删除对象不会被析构。

[`TransactionManager`](src/Platform/Public/TransactionManager.h) — 事务栈管理器，位于 Platform 层：
- 持有 `weak_ptr<Document>`、`m_undoStack`、`m_redoStack`、`m_active`
- `BeginTransaction()`：创建 [`Transaction`](src/Platform/Public/Transaction.h)，设为 [`Document`](src/Data/Public/Document.h) 的 `changeSink`
- `Commit()`：将 active 入 undoStack，清空 redoStack（新操作打断 redo 链），清空 [`Document`](src/Data/Public/Document.h) 的 `changeSink`
- `RollBack()`：逆序恢复 active 中的操作，丢弃 active
- `Undo()`：弹出 undoStack 顶，逆序 `ApplyTransaction()`，入 redoStack
- `Redo()`：弹出 redoStack 顶，正序 `ApplyTransaction()`，入 undoStack
- [`ApplyTransaction()`](src/Platform/Public/TransactionManager.h:43) 内部方法：设置 [`ExecStateGuard`](src/Data/Public/Document.h:64)，遍历 entries 处理三种操作类型：
  - `PropertyChange`：`ApplyPropertySilent + 补写 DirtyFlags`
  - `ObjectAdd` 的 Undo：`Document::remove()`；Redo：`Document::restore()`
  - `ObjectRemove` 的 Undo：`Document::restore()`；Redo：`Document::remove()`

通知链流程：
属性 `set()` → `Property::NotifyChanging()` → `Object::OnPropertyChanging()` → `Document::OnPropertyChanging()` → `m_changeSink->OnPropertyChanging()` 记录 `oldValue`
属性赋值后 → `NotifyChanged()` → `Object::OnPropertyChanged()` → `Document::OnPropertyChanged()` → `OnObjectDirty()` 写入 dirty + `m_changeSink->OnPropertyChanged()` 记录 `newValue`

[`Document::ExecStateGuard`](src/Data/Public/Document.h:64) — RAII 切换 Undo/Redo 状态，防止回放期间重复录入历史。`ExecState` 和 `ExecStateGuard` 均为 `public`，供 [`TransactionManager`](src/Platform/Public/TransactionManager.h) 使用。

DirtyFlags 机制
[`DirtyFlags`](src/Data/Public/DirtyFlags.h) 是 `uint8_t` 位标志枚举：`None=0`、`Visual=1`、`Transform=2`、`Geometry=4`。

每个 [`PropertyBase`](src/Data/Public/Property.h) 在构造时绑定一个 `DirtyFlags`。属性变更后，[`Document::OnPropertyChanged()`](src/Data/Public/Document.h:50) 将该属性的 flags 写入 `m_dirty map`（按 `ObjectId` 聚合）。`ConsumeDirty()` 取出并清空脏列表，供 [`RenderSystem::SyncFromDocument()`](src/Application/App/RenderSystem.h:25) 消费，驱动几何重建和渲染更新。Undo/Redo 时 [`TransactionManager::ApplyTransaction()`](src/Platform/Public/TransactionManager.h:43) 也会补写 `DirtyFlags`。

命令系统（新增）
Platform 层提供命令抽象与注册基础设施：

- [`ICommand`](src/Platform/Public/ICommand.h) — 命令接口，定义 `GetId()`、`GetName()`、`CanExecute()`、`Execute()`
- [`CommandContext`](src/Platform/Public/ICommand.h) — 命令执行上下文，持有 `Document`、`TransactionManager`，以及以 `void*` 形式传递的 `RenderSystem/MainWindow`
- [`CommandRegistry`](src/Platform/Public/CommandRegistry.h) — 全局命令工厂注册表单例，支持 `Register()`、`Create()`、`Contains()`、`GetAllIds()`
- [`CommandRegistrar`](src/Platform/Public/CommandRegistry.h) — 静态注册辅助类型
- [`REGISTER_COMMAND`](src/Platform/Public/CommandRegistry.h:53) — 宏注册入口，命令 `.cpp` 文件通过静态变量自动注册工厂

Application 层提供具体命令与 UI 构建：

- [`UndoCommand`](src/Application/Commands/UndoCommand.h)
- [`RedoCommand`](src/Application/Commands/RedoCommand.h)
- [`AddSphereCommand`](src/Application/Commands/AddSphereCommand.h)
- [`DeleteSelectedCommand`](src/Application/Commands/DeleteSelectedCommand.h)
- [`CommandHelper`](src/Application/Commands/CommandHelper.h) / [`RefreshAfterCommand()`](src/Application/Commands/CommandHelper.cpp:6) — 将 `CommandContext` 中的 `void*` 转回具体类型，并执行命令后的统一刷新逻辑
- [`CommandUIBuilder`](src/Application/UI/CommandUIBuilder.h) — 读取 XML 配置，创建 [`QAction`](src/Application/UI/CommandUIBuilder.cpp)，构建菜单和工具栏
- [`RegisterAllCommands`](src/Application/Commands/RegisterAllCommands.h) — 启动时调用的链接保障入口

命令注册方式
每个命令在自己的 `.cpp` 中使用类似如下方式自动注册：

- [`REGISTER_COMMAND(cadutils::UndoCommand, "cmd.undo", UndoCommand)`](src/Application/Commands/UndoCommand.cpp:8)
- [`REGISTER_COMMAND(cadutils::RedoCommand, "cmd.redo", RedoCommand)`](src/Application/Commands/RedoCommand.cpp:8)
- [`REGISTER_COMMAND(cadutils::AddSphereCommand, "cmd.add_sphere", AddSphereCommand)`](src/Application/Commands/AddSphereCommand.cpp:12)
- [`REGISTER_COMMAND(cadutils::DeleteSelectedCommand, "cmd.delete_selected", DeleteSelectedCommand)`](src/Application/Commands/DeleteSelectedCommand.cpp)

UI 配置方式
UI 布局由 [`config/ui_layout.xml`](config/ui_layout.xml) 驱动，而不是在 [`MainWindow::buildUi()`](src/Application/UI/MainWindow.cpp:73) 中硬编码 Action。

配置结构：
- `<MenuBar>` / `<Menu name="...">`
- `<ToolBars>` / `<ToolBar name="...">`
- `<Item command="cmd.xxx" shortcut="..."/>`
- `<Separator/>`

[`CommandUIBuilder::BuildFromXml()`](src/Application/UI/CommandUIBuilder.cpp:135) 的流程：
1. 打开 XML 文件
2. 遍历 `<Menu>` 和 `<ToolBar>`
3. 遇到 `<Item>` 时通过 [`CommandRegistry::Create()`](src/Platform/Public/CommandRegistry.h:25) 创建命令实例
4. 创建 [`QAction`](src/Application/UI/CommandUIBuilder.cpp)
5. 将 `triggered` 信号绑定到命令的 `CanExecute()` / `Execute()`
6. 加入菜单或工具栏

UI 层
[`MainWindow`](src/Application/UI/MainWindow.h) 现在持有 `shared_ptr<Document>`、`shared_ptr<RenderSystem>`、`shared_ptr<TransactionManager>`，同时对命令系统暴露了几个公共辅助接口：

- `RebuildAfterCommand()`
- `UpdatePropertiesById()`
- `SyncAndRefresh()`
- `GetDocument()`
- `GetTransactionManager()`
- `GetRenderSystem()`
- `SphereCounter()`

[`MainWindow`](src/Application/UI/MainWindow.cpp) 的变化：
- `buildUi()` 只负责 Dock / Tree / Property 面板基础搭建
- 菜单和工具栏不再硬编码创建 `QAction`
- 构造函数中创建 [`CommandContext`](src/Platform/Public/ICommand.h)，然后调用 [`CommandUIBuilder::BuildFromXml()`](src/Application/UI/CommandUIBuilder.cpp:135)
- 属性编辑仍旧通过 [`TransactionManager::BeginTransaction()`](src/Platform/Public/TransactionManager.h:21) / `Commit()` 包裹
- 对象创建/删除、Undo/Redo 改为由命令系统执行
- 命令执行后统一调用 [`RefreshAfterCommand()`](src/Application/Commands/CommandHelper.cpp:6)，内部完成：
  - [`RenderSystem::FullSyncFromDocument()`](src/Application/App/RenderSystem.h:29)
  - [`RenderSystem::Refresh()`](src/Application/App/RenderSystem.h:34)
  - [`MainWindow::RebuildAfterCommand()`](src/Application/UI/MainWindow.h:27)

程序启动与链接
[`main.cpp`](src/main.cpp) 现在额外包含 [`RegisterAllCommands.h`](src/Application/Commands/RegisterAllCommands.h) 并调用 `RegisterAllCommands()`，作为命令注册初始化的显式入口。

构建系统变化
[`src/Application/CMakeLists.txt`](src/Application/CMakeLists.txt) 新增了 `Application/Commands` include 路径。
[`src/CMakeLists.txt`](src/CMakeLists.txt) 新增：
- `Application/Commands` 给 exe 的 include 路径
- post-build 复制 [`config/ui_layout.xml`](config/ui_layout.xml) 到输出目录的 `config/` 下

AnyValue 类型支持
[`AnyValue`](src/Common/Public/NameDefine.h) 基于 string 的类型擦除，支持以下类型的序列化/反序列化：
- 算术类型（`int`, `uint`, `long`, `float`, `double` 等）
- [`std::string`](src/Common/Public/NameDefine.h)
- [`Point3d`](src/Common/Public/Point3d.h)（通过 `ToString/FromString`，格式 `"x,y,z"`）

不支持的类型（如 `shared_ptr<IBody>`）返回空 [`AnyValue`](src/Common/Public/NameDefine.h)，不参与 Undo/Redo。

文档
- 事务系统设计文档：[`docs/transaction-system-design.md`](docs/transaction-system-design.md)
- 命令系统设计文档：[`docs/command-system-design.md`](docs/command-system-design.md)