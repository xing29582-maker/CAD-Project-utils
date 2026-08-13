#pragma once

#include "PlatformExport.h"
#include "NameDefine.h"
#include "ICommand.h"

#include <string>
#include <vector>

namespace cadutils
{
    class IAction;

    // 状态监听接口：Action 状态变化 → 挂载点（QAction）同步。
    // 由 UI 层实现（如 QActionSink），Platform 层只定义契约。
    class CADUTILS_PLATFORM_API ActionSink
    {
    public:
        virtual ~ActionSink() = default;

        // Action 任一状态（enabled/visible/checked）变化时回调
        virtual void OnActionStateChanged(const std::string& actionId) = 0;

        // Action 执行完成回调（用于命令后刷新）
        virtual void OnActionExecuted(const std::string& actionId) = 0;
    };

    // Action 抽象：可复用的 UI 交互动作单元。
    // 所有 Action（含父子流程中的子 Action）都是同一个抽象；
    // 状态（enabled/visible/checked）委托给被包装的 Command，Action 本身不持有业务状态。
    class CADUTILS_PLATFORM_API IAction
    {
    public:
        virtual ~IAction() = default;

        // ---- 身份 / 元数据 ----
        virtual std::string GetId() const = 0;
        virtual std::string GetName() const = 0;
        virtual std::string GetTooltip() const { return GetName(); }
        virtual std::string GetIcon() const { return ""; }
        virtual std::string GetShortcut() const { return ""; }

        // 被包装的命令引用（可空：独立逻辑 Action）
        virtual ICommand* GetCommand() const { return nullptr; }

        // ---- 触发 ----
        // 由 ActionManager 压栈后调用（框架自动调度栈顶 Action）
        virtual void Trigger(CommandContext& ctx) = 0;

        // ---- 父子流程 ----
        // 子 Action 出栈时由框架调用（父 Action 实现此回调接收结果数据）
        virtual void OnSubActionFinished(CommandContext& ctx,
                                         const std::string& subActionId,
                                         const AnyValue& result)
        {
            (void)ctx; (void)subActionId; (void)result;
        }

        // 子 Action 出栈时框架读取其结果（空值 = 用户取消）
        virtual AnyValue GetResult() const { return AnyValue(); }

        // ---- 状态（委托给命令）----
        virtual bool IsEnabled(const CommandContext& ctx) const;
        virtual bool IsVisible(const CommandContext& ctx) const;
        virtual bool IsChecked(const CommandContext& ctx) const;

        // 重新求值状态，变化时通知 sink
        void Refresh(const CommandContext& ctx);

        // 缓存状态读取（sink 使用，无需 ctx）
        bool GetEnabled() const noexcept { return m_enabled; }
        bool GetVisible() const noexcept { return m_visible; }
        bool GetChecked() const noexcept { return m_checked; }

        // ---- sink 管理 ----
        void AttachSink(ActionSink* sink);
        void DetachSink(ActionSink* sink);
        void NotifyExecuted();

    protected:
        bool m_enabled = true;
        bool m_visible = true;
        bool m_checked = false;

    private:
        std::vector<ActionSink*> m_sinks;
    };

} // namespace cadutils
