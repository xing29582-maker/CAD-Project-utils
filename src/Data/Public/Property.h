#pragma once

#include "NameDefine.h"
#include "DirtyFlags.h"
#include "IObject.h"
#include "IPropertyChangeSink.h"

#include <memory>

namespace cadutils
{
    class PropertyBase {
    public:
        virtual ~PropertyBase() = default;

        PropertyId   id()    const noexcept { return m_propId; }
        DirtyFlags  flags() const noexcept { return m_flags; }
        virtual AnyValue Value() const = 0;
    protected:
        PropertyBase(PropertyId pid, DirtyFlags flags)
            : m_propId(pid), m_flags(flags) {
        }

    private:
        PropertyId  m_propId;
        DirtyFlags m_flags;
    };

    //Property 的职责：只“发信号”，不“找对象”
    template<class T>
    class Property : public PropertyBase
    {
    public:
        Property() = default;

        // 不要在构造里塞 ObjectId / sink，这些通常是 OnAddedToDocument 才能确定
        Property(PropertyId pid, DirtyFlags flags)
            : PropertyBase(pid, flags)
        {
        }

        const T& get() const { return m_value; }

        void set(T nv) 
        {
            if (nv == m_value) return;

            NotifyChanging(); // 统一入口
            m_value = std::move(nv);
            NotifyChanged();          
        }

        void SetValueSilent(T nv)
        { 
            m_value = std::move(nv);
        }

        // 绑定
        void Bind(IObject *obj)
        {
            m_owner = obj;
        }

        virtual AnyValue Value() const override
        {
            return AnyValue();
        }
    protected:
        void NotifyChanging()
        {
            if (m_owner)
                m_owner->OnPropertyChanging(*this);
        }
        void NotifyChanged()
        {
            if (m_owner)
                m_owner->OnPropertyChanged(*this);
        }

    private:
        IObject *m_owner;         // owner object
        T        m_value;
    };

}