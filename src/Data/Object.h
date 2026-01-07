#pragma once

#include"DataExport.h"

#include <string>
#include <vector>
#include <memory>

namespace cadutils 
{        
    // 先做简单属性：vector<pair>
    using Property = std::pair<std::string, std::string>;

    class CADUTILS_DATA_API Object 
    {
    public:
        explicit Object(std::string name);
        virtual ~Object() = default;

        const std::string& name() const;
        const std::vector<Property>& properties() const;

    protected:
        void addProperty(std::string k, std::string v);

    private:
        std::string m_name;
        std::vector<Property> m_props;
    };

    class CADUTILS_DATA_API Box : public Object 
    {
    public:
        Box();
    };

    class CADUTILS_DATA_API Sphere : public Object 
    {
    public:
        Sphere();
    };

} // namespace cadutils
