#pragma once

#include "NameDefine.h"
#include "Document.h"
#include "IObject.h"
#include "TypeMeta.h"
#include "MetaRegistry.h"

#include <vector>
#include <string>

template<class C>
struct PropertyRegistry
{
    using BindFn = void(*)(C&);
    using ApplyAnyFn = bool(*)(cadutils::IObject&, const cadutils::AnyValue&);

    struct Entry
    {
        const char* name;
        cadutils::PropertyId id;
        uint32_t flags;
        size_t offset;
        bool serializable;  // 新增：是否参与序列化
        BindFn bindFn;
        ApplyAnyFn applyAny;
    };

    static std::vector<BindFn>& BindFnList()
    {
        static std::vector<BindFn> v; // 函数内 static，避免静态初始化顺序问题
        return v;
    }

    static std::vector<Entry>& EntryList()
    {
        static std::vector<Entry> s_list;
        return s_list;
    }

    struct Registrar
    {
        Registrar(const char* inName,
            cadutils::PropertyId inId,
            uint32_t inFlags,
            size_t inOffset,
            bool inSerializable,  // 新增参数
            BindFn inBindFn,
            ApplyAnyFn inApplyAny)
        {
            BindFnList().push_back(inBindFn);
            EntryList().push_back(Entry{
                inName,
                inId,
                inFlags,
                inOffset,
                inSerializable,  // 新增字段
                inBindFn,
                inApplyAny
                });
        }
    };
};

constexpr cadutils::PropertyId fnv1a64(const char* s) noexcept
{
    cadutils::PropertyId h = 14695981039346656037ull;
    while (*s)
    {
        h ^= static_cast<unsigned char>(*s++);
        h *= 1099511628211ull;
    }
    return h;
}

constexpr cadutils::PropertyId hash_combine(cadutils::PropertyId a, cadutils::PropertyId b) noexcept
{
    // 这是一个常用的 combine，任意即可，只要稳定
    return a ^ (b + 0x9e3779b97f4a7c15ull + (a << 6) + (a >> 2));
}

#define CAD_DECLARE_TYPE(CLASS)                                      \
public:                                                              \
    static const ::cad::TypeMeta& staticMeta();                      \
    const ::cad::TypeMeta& meta() const override { return staticMeta(); } \
    static ::cad::TypeId staticTypeId() { return ::cad::makeTypeId(#CLASS); } \
    static const char* staticTypeName() { return #CLASS; }

#define CAD_PROP_ID(PropName) hash_combine(kTypeHash, fnv1a64(#PropName))

#define CAD_OBJECT_BEGIN(ClassName) \
private: \
    using ThisClass = ClassName; \
    void _cad_init_properties() { \
        OnInitProperty(*this); \
    } \
protected: \
    void OnInitProperty(ClassName& self) { \
        _BindAllProps(self); \
    } \
private: \
    void _BindAllProps(ClassName& self) { \
        for (auto fn : PropertyRegistry<ClassName>::BindFnList()) { \
            fn(self); \
        } \
    } \
public: \
    /* 强制用户必须实现默认构造 */ \
    ClassName(); \
    static constexpr const char* kClassName = #ClassName; \
    static constexpr cadutils::PropertyId kTypeHash = fnv1a64(kClassName); \
    static const cadutils::TypeMeta& StaticTypeMeta(); \
    virtual const cadutils::TypeMeta& GetTypeMeta() const { \
        return StaticTypeMeta(); \
    } \
private: \
    void InitFun();

#define CAD_PROP(T, name, flags) \
private: \
    Property<T> m_##name{ CAD_PROP_ID(name), (flags) }; \
    static void _cad_bind_##name(ThisClass& self) { \
        self.m_##name.Bind(&self); \
    } \
    static bool _cad_apply_##name(IObject& obj, const cadutils::AnyValue& v) { \
        auto* p = dynamic_cast<ThisClass*>(&obj); \
        if (!p) return false; \
        if constexpr (!AnyValueSupported<T>::value) { \
            return false; \
        } else { \
            if (!v.Is<T>()) return false; \
            p->m_##name.SetValueSilent(v.Get<T>()); \
            return true; \
        } \
    }\
    static constexpr size_t _cad_offset_##name() { \
        return offsetof(ThisClass, m_##name); \
    } \
    using _cad_reg_type_##name = typename PropertyRegistry<ThisClass>::Registrar; \
    inline static _cad_reg_type_##name _cad_reg_##name = _cad_reg_type_##name(\
        #name, \
        CAD_PROP_ID(name), \
        static_cast<uint32_t>(flags), \
        _cad_offset_##name(), \
        true, /* serializable = true */ \
        & _cad_bind_##name, \
        & _cad_apply_##name \
    );

#define CAD_PROP_TRANSIENT(T, name, flags) \
private: \
    Property<T> m_##name{ CAD_PROP_ID(name), (flags) }; \
    static void _cad_bind_##name(ThisClass& self) { \
        self.m_##name.Bind(&self); \
    } \
    static bool _cad_apply_##name(IObject& obj, const cadutils::AnyValue& v) { \
        auto* p = dynamic_cast<ThisClass*>(&obj); \
        if (!p) return false; \
        if constexpr (!AnyValueSupported<T>::value) { \
            return false; \
        } else { \
            if (!v.Is<T>()) return false; \
            p->m_##name.SetValueSilent(v.Get<T>()); \
            return true; \
        } \
    }\
    static constexpr size_t _cad_offset_##name() { \
        return offsetof(ThisClass, m_##name); \
    } \
    using _cad_reg_type_##name = typename PropertyRegistry<ThisClass>::Registrar; \
    inline static _cad_reg_type_##name _cad_reg_##name = _cad_reg_type_##name(\
        #name, \
        CAD_PROP_ID(name), \
        static_cast<uint32_t>(flags), \
        _cad_offset_##name(), \
        false, /* serializable = false */ \
        & _cad_bind_##name, \
        & _cad_apply_##name \
    );

#define CAD_OBJECT_END \
private: \
    struct _CadPropGuard { \
        _CadPropGuard(ThisClass* self) { self->_cad_init_properties(); } \
    }; \
    _CadPropGuard _cad_guard_{ this }; \
public:

#define CAD_DEFAULT_CTOR(ClassName) \
ClassName::ClassName() { \
    this->InitFun(); \
} \
const TypeMeta& ClassName::StaticTypeMeta() \
{ \
    static const TypeMeta s_meta = []() -> TypeMeta { \
        TypeMeta m; \
        m.typeId = ClassName::kTypeHash; \
        m.typeName = ClassName::kClassName; \
        m.creator = []() -> std::shared_ptr<IObject> { \
            return std::make_shared<ClassName>(); \
        }; \
        for (const auto& e : PropertyRegistry<ClassName>::EntryList()) { \
            m.properties.push_back(PropertyDescriptor( \
                e.id, e.name, e.flags, e.offset, e.serializable, e.applyAny \
            )); \
        } \
        m.BuildIndices(); \
        return m; \
    }(); \
    static const bool s_registered = []() { \
        MetaRegistry::Instance().Register(&s_meta); \
        return true; \
    }(); \
    (void)s_registered; \
    return s_meta; \
} \
void ClassName::InitFun()