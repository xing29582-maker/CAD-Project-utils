#pragma once

#include "NameDefine.h"
#include "IChangeSink.h"

#include <vector>
#include <string>

template<class C>
struct PropertyRegistry
{
    using BindFn = void(*)(C& self, cadutils::ObjectId , cadutils::IChangeSink *);

    static std::vector<BindFn>& List()
    {
        static std::vector<BindFn> v; // 函数内 static，避免静态初始化顺序问题
        return v;
    }

    struct Registrar
    {
        explicit Registrar(BindFn fn) { List().push_back(fn); }
    };
};

#define CAD_OBJECT_BEGIN(ClassName) \
private: \
    using ThisClass = ClassName;\
    void OnAddedToDocument(cadutils::Document& doc) override { \
        auto* sink = static_cast<cadutils::IChangeSink*>(&doc); \
        _BindAllProps(GetObjectId(), sink); \
    } \
    void _BindAllProps(cadutils::ObjectId id, cadutils::IChangeSink* sink) { \
        for (auto fn : PropertyRegistry<ThisClass>::List()) { \
            fn(*this, id, sink); \
        } \
    }

#define CAD_PROP(T, name, flags) \
private: \
    Property<T> m_##name; \
    static void _cad_bind_##name(ThisClass& self, cadutils::ObjectId id, cadutils::IChangeSink* sink) { \
        self.m_##name.Bind(id, flags, sink); \
    } \
    inline static PropertyRegistry<ThisClass>::Registrar _cad_reg_##name{ &_cad_bind_##name };



#define CAD_OBJECT_END \
public: