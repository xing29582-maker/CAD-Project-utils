#pragma once

#include <memory>
#include <string>
#include <stdexcept>

namespace cadutils
{
	using ObjectId = uint64_t;
	using PropertyId = uint64_t;
    using ObjTypeId = std::uint64_t;

    struct AnyValue
    {
        std::string text;

        AnyValue() = default;

        AnyValue(const char* v)
            : text(v)
        {
        }

        AnyValue(const std::string& v)
            : text(v)
        {
        }

        template<typename T,
            typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        AnyValue(T v)
            : text(std::to_string(v))
        {
        }

        template<typename T>
        bool Is() const
        {
            try
            {
                Get<T>();
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        template<typename T>
        T Get() const
        {
            if constexpr (std::is_same_v<T, std::string>)
            {
                return text;
            }
            else if constexpr (std::is_same_v<T, const char*>)
            {
                return text.c_str();
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                if (text == "true" || text == "1")
                    return true;
                if (text == "false" || text == "0")
                    return false;

                throw std::runtime_error("AnyValue bool convert failed");
            }
            else if constexpr (std::is_integral_v<T>)
            {
                long long v = std::stoll(text);
                return static_cast<T>(v);
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                double v = std::stod(text);
                return static_cast<T>(v);
            }
            else
            {
                static_assert(!sizeof(T), "AnyValue::Get<T>() unsupported type");
            }
        }
    };

    template<typename T>
    struct AnyValueSupported : std::false_type {};

    template<>
    struct AnyValueSupported<std::string> : std::true_type {};

    template<>
    struct AnyValueSupported<bool> : std::true_type {};

    template<>
    struct AnyValueSupported<int> : std::true_type {};

    template<>
    struct AnyValueSupported<unsigned int> : std::true_type {};

    template<>
    struct AnyValueSupported<long> : std::true_type {};

    template<>
    struct AnyValueSupported<unsigned long> : std::true_type {};

    template<>
    struct AnyValueSupported<long long> : std::true_type {};

    template<>
    struct AnyValueSupported<unsigned long long> : std::true_type {};

    template<>
    struct AnyValueSupported<float> : std::true_type {};

    template<>
    struct AnyValueSupported<double> : std::true_type {};

    enum class PropertyValueKind : std::uint32_t
    {
        Unknown = 0,
        Bool,
        Int,
        Double,
        String,
        Object,
        Custom
    };

    enum PropertyFlags : std::uint32_t
    {
        Property_None = 0,
        Property_ReadOnly = 1 << 0,
        Property_Transient = 1 << 1,
        Property_NoUndo = 1 << 2,
        Property_NoSerialize = 1 << 3
    };
    //using AnyValue = std::variant<
    //    std::monostate,
    //    bool,
    //    int,
    //    double,
    //    std::string,
    //    Point3d,
    //    Vector3d,
    //    Color,
    //    Transform
    //>;

    //class AnyValue {
    //public:
    //    AnyValue() = default;

    //    template<class T>
    //    AnyValue(T v) : value_(std::move(v)) {}

    //    template<class T>
    //    bool is() const {
    //        return std::holds_alternative<T>(value_);
    //    }

    //    template<class T>
    //    const T& get() const {
    //        return std::get<T>(value_);
    //    }

    //    template<class T>
    //    T& get() {
    //        return std::get<T>(value_);
    //    }

    //private:
    //    std::variant<
    //        std::monostate,
    //        bool,
    //        int,
    //        double,
    //        std::string,
    //        Point3d,
    //        Vector3d
    //    > value_;
    //};

}