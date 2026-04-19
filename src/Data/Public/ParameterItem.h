#pragma once

#include <string>

enum class ParamKey : uint32_t
{
    Unknow = 0,
    Id,
    Name,
    Radius,
    CenterX,
    CenterY,
    CenterZ,
    Length,
    Width,
    Height,
};

struct ParameterItem
{
    ParameterItem() = default;
    ParameterItem(ParamKey key, const std::string& name, const std::string& value, bool editable)
        :Key(key)
        , Name(name)
        , Value(value)
        , Editable(editable)
    {

    }
    ParamKey    Key;     // ���Ա�ʶ��ö�ٻ� int��
    std::string Name;       // UI ��ʾ����"Radius"
    std::string Value;      // �ı�ֵ��"50"
    bool        Editable;   // UI �Ƿ������༭
};