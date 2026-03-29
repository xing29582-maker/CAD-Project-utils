cadutils_demo (exe)
  └─► cadutils_application (STATIC)
        ├─► cadutils_data (SHARED)
        ├─► cadutils_render (SHARED)
        └─► cadutils_geometry (SHARED)
              ├─► cadutils_render
              ├─► cadutils_common (SHARED)
              └─► thirdparty::occt

cadutils_platform (SHARED)  ──► cadutils_data
cadutils_render             ──► thirdparty::osg, Qt5, cadutils_common
cadutils_common (SHARED)    ──  无外部依赖（最底层）


各模块职责
Common — 基础类型定义层。NameDefine.h 定义了 ObjectId、PropertyId、AnyValue（基于 string 的类型擦除值）、PropertyFlags 枚举等全局基础类型。还包含 Point3d、Vector3d。

Data — 文档数据模型核心。包含 Document、Object 体系、Property 系统、元数据注册。

Platform — 事务/Undo-Redo 层。目前只有 Transaction。

Geometry — 几何造型层，封装 OpenCASCADE。提供 IBody、GeoBuildUtils、MeshGenerator 等。

Render — 渲染层，封装 OpenSceneGraph + Qt。提供 IRenderView、IGraphicsNode。

Application — 应用层（静态库），包含 MainWindow（Qt 主窗口）、GraphicsScene（场景图管理）、RenderSystem（文档→渲染同步）。

Document / Object / Property 体系
Document — 文档容器，持有 unordered_map<ObjectId, shared_ptr<IObject>>，自增分配 ID。实现 IDirtySink 接口收集脏标记。持有一个 IPropertyChangeSink* 指针指向当前活跃的 Transaction。提供 Undo() / Redo()（目前 Redo 为空实现）。通过 ExecStateGuard RAII 切换 Undo/Redo 状态，防止回放期间重复录入历史。

IObject — 纯虚接口，定义 GetObjectName()、GetObjectId()、buildShape()、SetParameters()、GetParameters()、OnPropertyChanging/Changed()、GetTypeMeta() 等。

Object — IObject 的基础实现，使用宏系统声明了三个属性：objName(string)、objId(uint64)、shapeBody(shared_ptr<IBody>)。持有 Document* 反向指针。属性变更时委托给 Document::OnPropertyChanging/Changed()。

SphereObject — Object 的具体子类，额外声明 center(Point3d) 和 radius(double) 两个属性，DirtyFlags::Geometry 标记。

Property<T> — 模板属性类，继承 PropertyBase。每个属性持有 PropertyId + DirtyFlags。set() 时先调 NotifyChanging()（通知 owner 的 OnPropertyChanging），再赋值，再调 NotifyChanged()。SetValueSilent() 跳过通知，用于 Undo/Redo 回放。通过 Bind() 绑定 owner 对象指针。

属性宏注册系统
PropertyRegistry.h 定义了一套编译期属性注册宏：

CAD_OBJECT_BEGIN(ClassName) — 声明类的属性基础设施，包括 _cad_init_properties()、_BindAllProps()、StaticTypeMeta()。
CAD_PROP(T, name, flags) — 声明一个 Property<T> m_##name 成员，同时通过 PropertyRegistry<ThisClass>::Registrar 静态变量在加载时自动注册到 EntryList()。属性 ID 通过 CAD_PROP_ID(name) = hash_combine(kTypeHash, fnv1a64("name")) 编译期计算。
CAD_DEFAULT_CTOR(ClassName) — 展开默认构造函数 + StaticTypeMeta() 实现，将所有注册的 Entry 转为 PropertyDescriptor 存入 TypeMeta，并注册到全局 MetaRegistry 单例。
TypeMeta 持有属性描述符列表，并建立 id/offset/name 三种索引 map，支持按 ID、偏移量、名称查找属性描述符。

PropertyDescriptor 包含 id、name、flags、offset（成员偏移）、applyAny 函数指针（用于 Undo 时静默写入值）。

Undo/Redo / Transaction 机制
Transaction 实现 IPropertyChangeSink 接口：

构造时调 Start() 清空变更记录，并将自身设为 Document 的当前 Transaction。
属性 set() → Property::NotifyChanging() → Object::OnPropertyChanging() → Document::OnPropertyChanging() → Transaction::OnPropertyChanging() 记录 old value（同一属性只记第一次）。
属性赋值后 → NotifyChanged() → Document::OnPropertyChanged() → 写入 dirty map。（注意：Transaction::OnPropertyChanged 目前在 Document 层没有被调用，只有 dirty 标记被写入）
RollBack() — 设置 ExecState::Undo，遍历 m_changes 调用 Document::ApplyPropertySilent() 恢复旧值。ApplyPropertySilent 通过 TypeMeta 查找 PropertyDescriptor，调用其 applyAny 函数指针（即 _cad_apply_##name，内部调 SetValueSilent）。
析构时将 Document 的 Transaction 指针置空。
变更记录以 ChangeKey{objId, propId} 为 key，ChangeRec{oldValue, newValue} 为 value 存储在 unordered_map 中。

当前 Document::Undo() / Redo() 方法体基本为空，实际的 undo 逻辑在 Transaction::RollBack() 中。尚未实现 undo stack（历史事务栈）。

DirtyFlags 机制
DirtyFlags 是 uint8_t 位标志枚举：None=0、Visual=1、Transform=2、Geometry=4。

每个 PropertyBase 在构造时绑定一个 DirtyFlags。属性变更后，Document::OnPropertyChanged() 将该属性的 flags 写入 m_dirty map（按 ObjectId 聚合）。ConsumeDirty() 取出并清空脏列表，供 RenderSystem::SyncFromDocument() 消费，驱动几何重建和渲染更新。