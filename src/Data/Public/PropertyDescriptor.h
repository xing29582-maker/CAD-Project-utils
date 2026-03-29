#pragma once

#include"NameDefine.h"


namespace cadutils
{
    class IObject;

    //struct PropertyDescriptor
    //{
    //    cadutils::PropertyId id;          // property稳定ID
    //    const char* name;       // UI/调试/序列化
    //    size_t offset;          // 成员偏移
    //    //ValueType type;         // 数据类型,todo
    //    //uint32_t flags;         // 可选：readonly / transient / animatable ,todo

    //    bool (*applyAny)(IObject& obj, const AnyValue& v) = nullptr;
    //    bool (*readAny)(const IObject& obj, AnyValue& out) = nullptr;
    //};

    struct PropertyDescriptor
    {
        PropertyId id = 0;
        const char* name = nullptr;
        uint32_t flags = 0;
        size_t offset = 0;
        bool (*applyAny)(IObject&, const AnyValue&) = nullptr;
        bool (*readAny)(const IObject& obj, AnyValue& out) = nullptr;

        PropertyDescriptor() = default;

        PropertyDescriptor(PropertyId inId,
            const char* inName,
            uint32_t inFlags,
            size_t inOffset,
            bool (*inApplyAny)(IObject&, const AnyValue&))
            : id(inId)
            , name(inName)
            , flags(inFlags)
            , offset(inOffset)
            , applyAny(inApplyAny)
        {
        }
    };
}