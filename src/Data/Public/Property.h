#pragma once

#include "NameDefine.h"
#include "DirtyFlags.h"
#include "IChangeSink.h"

namespace cadutils
{
    template<class T>
    class Property {
    public:
        Property() = default;

        Property(ObjectId id,  DirtyFlags flags, IChangeSink* sink, T init = {})
            : id_(id), flags_(flags), sink_(sink), v_(std::move(init)) {
        }

        const T& get() const { return v_; }

        void set(T nv) {
            if (nv == v_) return;
            v_ = std::move(nv);
            if (sink_) sink_->OnPropertyChanged(id_, flags_);
        }

        // 事务/Undo 之类可能需要“静默赋值”（不触发二次记录）——先留着
        void set_silent(T nv) { v_ = std::move(nv); }

        // 对象创建后 id/sink 可能才确定，提供绑定接口
        void Bind(ObjectId id, DirtyFlags flags, IChangeSink* sink) {
            id_ = id; flags_ = flags; sink_ = sink;
        }

    private:
        ObjectId id_{};
        DirtyFlags flags_{ DirtyFlags::None };
        IChangeSink* sink_{ nullptr };
        T v_{};
    };
}