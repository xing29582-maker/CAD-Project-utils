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
};

struct ParameterItem
{
    ParameterItem(ParamKey key, const std::string& name, const std::string& value, bool editable)
        :Key(key)
        , Name(name)
        , Value(value)
        , Editable(editable)
    {

    }
    ParamKey    Key;     // 属性标识（枚举或 int）
    std::string Name;       // UI 显示名："Radius"
    std::string Value;      // 文本值："50"
    bool        Editable;   // UI 是否允许编辑
};