#pragma once

#include"NameDefine.h"


namespace cadutils
{
    class IObject;

    //struct PropertyDescriptor
    //{
    //    cadutils::PropertyId id;          // property�ȶ�ID
    //    const char* name;       // UI/����/���л�
    //    size_t offset;          // ��Աƫ��
    //    //ValueType type;         // ��������,todo
    //    //uint32_t flags;         // ��ѡ��readonly / transient / animatable ,todo

    //    bool (*applyAny)(IObject& obj, const AnyValue& v) = nullptr;
    //    bool (*readAny)(const IObject& obj, AnyValue& out) = nullptr;
    //};

    struct PropertyDescriptor
    {
        PropertyId id = 0;
        const char* name = nullptr;
        uint32_t flags = 0;
        size_t offset = 0;
        bool serializable = true;  // 新增：是否参与序列化，默认为 true
        bool (*applyAny)(IObject&, const AnyValue&) = nullptr;
        bool (*readAny)(const IObject& obj, AnyValue& out) = nullptr;

        PropertyDescriptor() = default;

        PropertyDescriptor(PropertyId inId,
            const char* inName,
            uint32_t inFlags,
            size_t inOffset,
            bool inSerializable,
            bool (*inApplyAny)(IObject&, const AnyValue&))
            : id(inId)
            , name(inName)
            , flags(inFlags)
            , offset(inOffset)
            , serializable(inSerializable)
            , applyAny(inApplyAny)
        {
        }
    };
}