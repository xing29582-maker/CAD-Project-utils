# Property 系统技术文档

## 概述

Property 系统是 cadutils 数据模型的核心基础设施，提供：

- 类型安全的属性声明与访问
- 编译期宏驱动的自动注册
- 属性变更通知链（支撑 Undo/Redo 和脏标记）
- 基于 string 的类型擦除值（AnyValue），用于序列化和事务记录

整个系统分布在 Common 和 Data 两个模块中，不依赖任何上层模块。

---

## 核心类型

### AnyValue（[`NameDefine.h`](../src/Common/Public/NameDefine.h)）

基于 `std::string` 的类型擦除值容器。所有支持 Undo/Redo 的属性值都通过 AnyValue 进行序列化/反序列化。

```cpp
struct AnyValue {
    std::string text;

    // 构造：从 string / 算术类型 / Point3d 隐式转换
    AnyValue(const std::string& v);
    AnyValue(T v);                    // T 为算术类型
    AnyValue(const Point3d& pt);

    // 类型检查与提取
    template<typename T> bool Is() const;
    template<typename T> T Get() const;
};
```

支持的类型通过 `AnyValueSupported<T>` trait 声明：

| 类型 | 支持 |
|------|------|
| `std::string` | ✓ |
| `bool` | ✓ |
| `int` / `unsigned int` | ✓ |
| `long` / `unsigned long` | ✓ |
| `long long` / `unsigned long long` | ✓ |
| `float` / `double` | ✓ |
| `Point3d` | ✓ |
| `shared_ptr<IBody>` 等 | ✗（返回空 AnyValue，不参与 Undo） |

### DirtyFlags（[`DirtyFlags.h`](../src/Data/Public/DirtyFlags.h)）

`uint8_t` 位标志枚举，标识属性变更影响的范围：

```cpp
enum class DirtyFlags : uint8_t {
    None      = 0,
    Visual    = 1 << 0,   // 仅外观变化（颜色、材质）
    Transform = 1 << 1,   // 位置/变换变化
    Geometry  = 1 << 2,   // 几何形状变化（需重建 mesh）
};
```

每个 Property 在声明时绑定一个 DirtyFlags，属性变更后通过通知链传递到 Document 的脏标记系统。

---

## Property 模板类

### PropertyBase（[`Property.h`](../src/Data/Public/Property.h)）

属性基类，持有 `PropertyId` 和 `DirtyFlags`：

```cpp
class PropertyBase {
public:
    PropertyId  id()    const noexcept;
    DirtyFlags  flags() const noexcept;
    virtual AnyValue Value() const = 0;  // 类型擦除取值
};
```

### Property\<T\>（[`Property.h`](../src/Data/Public/Property.h)）

模板属性类，继承 PropertyBase：

```cpp
template<class T>
class Property : public PropertyBase {
public:
    Property(PropertyId pid, DirtyFlags flags);

    const T& get() const;           // 读取值
    void set(T nv);                 // 写入值（触发通知）
    void SetValueSilent(T nv);      // 静默写入（Undo/Redo 回放用）
    void Bind(IObject* obj);        // 绑定 owner 对象
    AnyValue Value() const override; // 类型擦除取值
};
```

关键行为：

- `set()` 内部流程：`NotifyChanging()` → 赋值 → `NotifyChanged()`
- `SetValueSilent()` 直接赋值，跳过所有通知，专用于 Undo/Redo 回放
- `Value()` 对 `AnyValueSupported<T>` 为 true 的类型返回有效 AnyValue，否则返回空值
- `Bind()` 设置 owner 指针，使通知能传递到 IObject

---

## 属性宏注册系统

### 宏定义（[`PropertyRegistry.h`](../src/Data/Public/PropertyRegistry.h)）

#### CAD_OBJECT_BEGIN(ClassName)

在类体内声明属性基础设施：

```cpp
class SphereObject : public Object {
    CAD_OBJECT_BEGIN(SphereObject)
        // ... CAD_PROP 声明 ...
    CAD_OBJECT_END;
};
```

展开后生成：
- `ThisClass` 类型别名
- `_cad_init_properties()` — 初始化所有属性绑定
- `_BindAllProps()` — 遍历 `PropertyRegistry<ThisClass>::BindFnList()` 调用每个属性的 Bind 函数
- `kClassName` / `kTypeHash` — 编译期类名和类型哈希
- `StaticTypeMeta()` — 静态方法，返回该类的 TypeMeta
- `GetTypeMeta()` — 虚方法，调用 StaticTypeMeta()
- 默认构造函数声明

#### CAD_PROP(T, name, flags)

声明一个属性成员并自动注册：

```cpp
CAD_PROP(double, radius, DirtyFlags::Geometry)
```

展开后生成：
- `Property<double> m_radius{ CAD_PROP_ID(radius), DirtyFlags::Geometry }` — 属性成员
- `_cad_bind_radius()` — 静态绑定函数，调用 `self.m_radius.Bind(&self)`
- `_cad_apply_radius()` — 静态 ApplyAny 函数，用于 Undo/Redo 静默写入
- `_cad_offset_radius()` — 编译期计算成员偏移量
- `_cad_reg_radius` — 静态 Registrar 变量，在 DLL 加载时自动注册到 `PropertyRegistry<ThisClass>`

#### CAD_OBJECT_END

生成 `_CadPropGuard` 成员，在对象构造时自动调用 `_cad_init_properties()`，确保所有属性完成 Bind。

#### CAD_DEFAULT_CTOR(ClassName)

在 .cpp 文件中展开默认构造函数和 `StaticTypeMeta()` 实现：

```cpp
// SphereObject.cpp
CAD_DEFAULT_CTOR(SphereObject)
{
    // 用户自定义初始化代码（可为空）
}
```

展开后：
1. 默认构造函数调用 `InitFun()`
2. `StaticTypeMeta()` 构建 TypeMeta：遍历 `PropertyRegistry<ClassName>::EntryList()`，将每个 Entry 转为 PropertyDescriptor，调用 `BuildIndices()` 建立索引，注册到全局 MetaRegistry

### PropertyId 生成

属性 ID 通过编译期 FNV-1a 哈希计算：

```cpp
#define CAD_PROP_ID(PropName) hash_combine(kTypeHash, fnv1a64(#PropName))
```

即 `hash_combine(类名哈希, 属性名哈希)`，保证全局唯一。

---

## 注册表与元数据

### PropertyRegistry\<C\>（[`PropertyRegistry.h`](../src/Data/Public/PropertyRegistry.h)）

每个类有一个独立的 `PropertyRegistry<C>` 模板实例化，持有：

- `BindFnList()` — 所有属性的 Bind 函数列表
- `EntryList()` — 所有属性的注册信息列表

使用函数内 static 变量避免静态初始化顺序问题。

### PropertyDescriptor（[`PropertyDescriptor.h`](../src/Data/Public/PropertyDescriptor.h)）

属性描述符，运行时元数据：

```cpp
struct PropertyDescriptor {
    PropertyId id;                                    // 属性唯一 ID
    const char* name;                                 // 属性名
    uint32_t flags;                                   // DirtyFlags
    size_t offset;                                    // 成员偏移量
    bool (*applyAny)(IObject&, const AnyValue&);      // 静默写入函数指针
};
```

`applyAny` 是 Undo/Redo 的关键：`Document::ApplyPropertySilent()` 通过它将 AnyValue 写回属性，不触发通知链。

### TypeMeta（[`TypeMeta.h`](../src/Data/Public/TypeMeta.h)）

类型元数据，持有一个类的所有属性描述符：

```cpp
struct TypeMeta {
    ObjTypeId typeId;
    const char* typeName;
    vector<PropertyDescriptor> properties;

    // 三种索引 map
    unordered_map<PropertyId, const PropertyDescriptor*> idMap;
    unordered_map<size_t, const PropertyDescriptor*> offsetMap;
    unordered_map<string_view, const PropertyDescriptor*> nameMap;

    void BuildIndices();
    const PropertyDescriptor* FindById(PropertyId id) const;
    const PropertyDescriptor* FindByOffset(size_t offset) const;
    const PropertyDescriptor* FindByName(string_view name) const;
};
```

### MetaRegistry（[`MetaRegistry.h`](../src/Data/Public/MetaRegistry.h)）

全局类型注册表单例：

```cpp
class MetaRegistry {
public:
    static MetaRegistry& Instance();
    void Register(const TypeMeta* meta);
    const TypeMeta* Find(PropertyId typeId) const;
};
```

每个类的 `StaticTypeMeta()` 首次调用时自动注册到 MetaRegistry。

---

## 变更通知链

属性变更时的完整通知流程：

```
Property<T>::set(newValue)
  │
  ├─ NotifyChanging()
  │    └─ IObject::OnPropertyChanging(prop)
  │         └─ Object::OnPropertyChanging(prop)
  │              └─ Document::OnPropertyChanging(obj, prop)
  │                   └─ m_changeSink->OnPropertyChanging(objId, propId, oldValue)
  │                        └─ Transaction 记录 oldValue（同一属性只记第一次）
  │
  ├─ m_value = newValue  （实际赋值）
  │
  └─ NotifyChanged()
       └─ IObject::OnPropertyChanged(prop)
            └─ Object::OnPropertyChanged(prop)
                 └─ Document::OnPropertyChanged(obj, prop)
                      ├─ OnObjectDirty(objId, prop.flags())  → 写入 m_dirty map
                      └─ m_changeSink->OnPropertyChanged(objId, propId, newValue)
                           └─ Transaction 记录 newValue
```

### 关键接口

- [`IPropertyChangeSink`](../src/Data/Public/IPropertyChangeSink.h) — 变更通知接收接口，由 Transaction 实现
- [`IDirtySink`](../src/Data/Public/IDirtySink.h) — 脏标记接收接口，由 Document 实现

### Undo/Redo 回放路径

回放时使用 `SetValueSilent()` 跳过通知链：

```
TransactionManager::ApplyTransaction()
  └─ Document::ApplyPropertySilent(objId, propId, value)
       └─ TypeMeta::FindById(propId) → PropertyDescriptor
            └─ desc->applyAny(obj, value)
                 └─ Property<T>::SetValueSilent(value)  （不触发通知）
```

Document 在回放期间通过 `ExecStateGuard` 设置 `IsReplaying() == true`，即使有通知也会被 `OnPropertyChanging/Changed` 中的 guard 拦截。

---

## 使用示例

### 定义一个新的对象类型

```cpp
// MyObject.h
class MyObject : public Object {
    CAD_OBJECT_BEGIN(MyObject)
        CAD_PROP(double, width,  DirtyFlags::Geometry)
        CAD_PROP(double, height, DirtyFlags::Geometry)
        CAD_PROP(std::string, color, DirtyFlags::Visual)
    CAD_OBJECT_END;

public:
    explicit MyObject(const std::string& name, double w, double h);
    std::shared_ptr<IBody> buildShape() override;
};

// MyObject.cpp
CAD_DEFAULT_CTOR(MyObject)
{
    // 可选的默认初始化
}

MyObject::MyObject(const std::string& name, double w, double h)
    : Object(name)
{
    m_width.set(w);
    m_height.set(h);
    m_color.set("gray");
}
```

以上代码自动完成：
1. 三个属性的 PropertyId 编译期生成
2. 属性在 DLL 加载时注册到 `PropertyRegistry<MyObject>`
3. 构造时自动 Bind 所有属性到 owner
4. `StaticTypeMeta()` 首次调用时构建 TypeMeta 并注册到 MetaRegistry
5. 属性 `set()` 自动触发通知链，支持 Undo/Redo 录入

### 属性继承

子类通过 `CAD_OBJECT_BEGIN` 声明自己的属性，父类的属性由父类的 `PropertyRegistry` 管理。每个类有独立的 `PropertyRegistry<C>` 和 `TypeMeta`。

当前实现中，`Object` 基类声明了 `objName`、`objId`、`shapeBody` 三个属性；`SphereObject` 额外声明了 `center`、`radius`。子类的 `GetTypeMeta()` 返回子类自己的 TypeMeta（只包含子类声明的属性）。

---

## 文件清单

| 文件 | 职责 |
|------|------|
| [`NameDefine.h`](../src/Common/Public/NameDefine.h) | AnyValue、AnyValueSupported trait、基础类型别名 |
| [`DirtyFlags.h`](../src/Data/Public/DirtyFlags.h) | DirtyFlags 枚举 |
| [`Property.h`](../src/Data/Public/Property.h) | PropertyBase、Property\<T\> 模板类 |
| [`PropertyRegistry.h`](../src/Data/Public/PropertyRegistry.h) | PropertyRegistry 模板、宏定义（CAD_OBJECT_BEGIN/CAD_PROP/CAD_DEFAULT_CTOR） |
| [`PropertyDescriptor.h`](../src/Data/Public/PropertyDescriptor.h) | PropertyDescriptor 结构体 |
| [`TypeMeta.h`](../src/Data/Public/TypeMeta.h) | TypeMeta 结构体（属性描述符集合 + 索引） |
| [`MetaRegistry.h`](../src/Data/Public/MetaRegistry.h) | MetaRegistry 全局单例 |
| [`IPropertyChangeSink.h`](../src/Data/Public/IPropertyChangeSink.h) | 变更通知接口 |
| [`IDirtySink.h`](../src/Data/Public/IDirtySink.h) | 脏标记接口 |
| [`Object.h`](../src/Data/Private/Object.h) / [`Object.cpp`](../src/Data/Private/Object.cpp) | 基础对象实现，演示宏使用 |
| [`SphereObject.h`](../src/Data/Private/SphereObject.h) / [`SphereObject.cpp`](../src/Data/Private/SphereObject.cpp) | 具体对象子类，演示属性继承 |