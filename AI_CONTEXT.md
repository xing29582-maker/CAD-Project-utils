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
Common — 基础类型定义层。NameDefine.h 定义了 ObjectId、PropertyId、AnyValue（基于 string 的类型擦除值，支持算术类型、string、Point3d）、PropertyFlags 枚举等全局基础类型。还包含 Point3d（含 ToString/FromString 序列化）、Vector3d。

Data — 文档数据模型核心。包含 Document（含对象增删 add/remove/restore）、Object 体系、Property 系统、元数据注册。

Platform — 事务/Undo-Redo 层。包含 Transaction（纯数据记录类，支持属性变更+对象增删三种操作）和 TransactionManager（事务栈管理器）。

Geometry — 几何造型层，封装 OpenCASCADE。提供 IBody、GeoBuildUtils、MeshGenerator 等。

Render — 渲染层，封装 OpenSceneGraph + Qt。提供 IRenderView、IGraphicsNode。

Application — 应用层（静态库），包含 MainWindow（Qt 主窗口）、GraphicsScene（场景图管理）、RenderSystem（文档→渲染同步）。

Document / Object / Property 体系
Document — 文档容器，持有 unordered_map<ObjectId, shared_ptr<IObject>>，自增分配 ID。实现 IDirtySink 接口收集脏标记。持有一个 IPropertyChangeSink* m_changeSink 指针，由 TransactionManager 在 BeginTransaction 时设置为当前活跃的 Transaction。支持 add/remove/restore 三种对象管理操作，add 和 remove 在非回放状态下会通知 changeSink。

IObject — 纯虚接口，定义 GetObjectName()、GetObjectId()、buildShape()、SetParameters()、GetParameters()、OnPropertyChanging/Changed()、GetTypeMeta() 等。

Object — IObject 的基础实现，使用宏系统声明了三个属性：objName(string)、objId(uint64)、shapeBody(shared_ptr<IBody>)。持有 Document* 反向指针。属性变更时委托给 Document::OnPropertyChanging/Changed()。

SphereObject — Object 的具体子类，额外声明 center(Point3d) 和 radius(double) 两个属性，DirtyFlags::Geometry 标记。

Property<T> — 模板属性类，继承 PropertyBase。每个属性持有 PropertyId + DirtyFlags。set() 时先调 NotifyChanging()（通知 owner 的 OnPropertyChanging），再赋值，再调 NotifyChanged()。SetValueSilent() 跳过通知，用于 Undo/Redo 回放。通过 Bind() 绑定 owner 对象指针。Value() 方法对 AnyValueSupported 的类型返回 AnyValue(m_value)，不支持的类型返回空 AnyValue。

属性宏注册系统
PropertyRegistry.h 定义了一套编译期属性注册宏：

CAD_OBJECT_BEGIN(ClassName) — 声明类的属性基础设施，包括 _cad_init_properties()、_BindAllProps()、StaticTypeMeta()。
CAD_PROP(T, name, flags) — 声明一个 Property<T> m_##name 成员，同时通过 PropertyRegistry<ThisClass>::Registrar 静态变量在加载时自动注册到 EntryList()。属性 ID 通过 CAD_PROP_ID(name) = hash_combine(kTypeHash, fnv1a64("name")) 编译期计算。
CAD_DEFAULT_CTOR(ClassName) — 展开默认构造函数 + StaticTypeMeta() 实现，将所有注册的 Entry 转为 PropertyDescriptor 存入 TypeMeta，并注册到全局 MetaRegistry 单例。
TypeMeta 持有属性描述符列表，并建立 id/offset/name 三种索引 map，支持按 ID、偏移量、名称查找属性描述符。

PropertyDescriptor 包含 id、name、flags、offset（成员偏移）、applyAny 函数指针（用于 Undo 时静默写入值）。

Undo/Redo / Transaction 机制（TransactionManager 架构）

Transaction — 纯数据记录类，实现 IPropertyChangeSink 接口，内部使用 vector<ChangeEntry> 保持操作顺序：
- OnPropertyChanging() 记录 oldValue（同一属性只记第一次）
- OnPropertyChanged() 记录 newValue
- OnObjectAdded() 记录对象创建（持有 shared_ptr）
- OnObjectRemoved() 记录对象删除（持有 shared_ptr 保活）
- GetEntries() 返回变更记录的只读引用
- IsEmpty() 判断是否有变更

ChangeEntry 有三种类型（ChangeType）：PropertyChange、ObjectAdd、ObjectRemove。ObjectAdd/ObjectRemove 的 ChangeEntry 持有 shared_ptr<IObject>，保证被删除的对象不会被析构。

TransactionManager — 事务栈管理器，位于 Platform 层：
- 持有 weak_ptr<Document>、m_undoStack、m_redoStack、m_active
- BeginTransaction()：创建 Transaction，设为 Document 的 changeSink
- Commit()：将 active Transaction 入 undoStack，清空 redoStack（新操作打断 redo 链），清空 Document changeSink
- RollBack()：逆序恢复 active 中的操作，丢弃 active
- Undo()：弹出 undoStack 顶，逆序 ApplyTransaction，入 redoStack
- Redo()：弹出 redoStack 顶，正序 ApplyTransaction，入 undoStack
- ApplyTransaction() 内部方法：设置 ExecStateGuard，遍历 entries 处理三种操作类型：
  - PropertyChange：ApplyPropertySilent + 补写 DirtyFlags
  - ObjectAdd 的 Undo：Document::remove()；Redo：Document::restore()
  - ObjectRemove 的 Undo：Document::restore()；Redo：Document::remove()

通知链流程：
属性 set() → Property::NotifyChanging() → Object::OnPropertyChanging() → Document::OnPropertyChanging() → m_changeSink->OnPropertyChanging() 记录 oldValue
属性赋值后 → NotifyChanged() → Object::OnPropertyChanged() → Document::OnPropertyChanged() → OnObjectDirty() 写入 dirty + m_changeSink->OnPropertyChanged() 记录 newValue

Document::ExecStateGuard — RAII 切换 Undo/Redo 状态，防止回放期间重复录入历史。ExecState 和 ExecStateGuard 均为 public，供 TransactionManager 使用。

变更记录以 ChangeKey{objId, propId} 为 key，ChangeRec{oldValue, newValue} 为 value 存储在 unordered_map 中。

DirtyFlags 机制
DirtyFlags 是 uint8_t 位标志枚举：None=0、Visual=1、Transform=2、Geometry=4。

每个 PropertyBase 在构造时绑定一个 DirtyFlags。属性变更后，Document::OnPropertyChanged() 将该属性的 flags 写入 m_dirty map（按 ObjectId 聚合）。ConsumeDirty() 取出并清空脏列表，供 RenderSystem::SyncFromDocument() 消费，驱动几何重建和渲染更新。Undo/Redo 时 TransactionManager::ApplyTransaction() 也会补写 DirtyFlags。

UI 层
MainWindow 持有 shared_ptr<Document>、shared_ptr<RenderSystem>、shared_ptr<TransactionManager>。
- 属性编辑时通过 BeginTransaction/Commit 包裹修改
- Edit 菜单和工具栏提供 Undo (Ctrl+Z) / Redo (Ctrl+Y) / Add Sphere / Delete 操作
- 对象创建和删除也通过 BeginTransaction/Commit 包裹
- Undo/Redo 后调用 FullSyncFromDocument 全量同步渲染（处理对象增删），并重建文档树和属性面板

AnyValue 类型支持
AnyValue 基于 string 的类型擦除，支持以下类型的序列化/反序列化：
- 算术类型（int, uint, long, float, double 等）
- std::string
- Point3d（通过 ToString/FromString，格式 "x,y,z"）
不支持的类型（如 shared_ptr<IBody>）返回空 AnyValue，不参与 Undo/Redo。