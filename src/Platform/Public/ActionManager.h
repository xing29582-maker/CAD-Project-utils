#pragma once

#include "PlatformExport.h"
#include "IAction.h"
#include "ActionStack.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace cadutils
{
    // Action 注册表 + 父子栈调度 + 状态刷新中枢。
    // 由应用层（MainWindow）持有，传入 CommandContext 驱动调度与刷新。
    class CADUTILS_PLATFORM_API ActionManager
    {
    public:
        ActionManager() = default;
        ~ActionManager() = default;

        // ---- 注册表 ----
        // 注册 Action（重复 id 返回 false）
        bool Register(const std::shared_ptr<IAction>& action);
        std::shared_ptr<IAction> Find(const std::string& id) const;
        std::vector<std::shared_ptr<IAction>> GetAllActions() const;

        // ---- 栈调度 ----
        // 父 Action 请求子流程：按 id 压栈并自动调用新栈顶 Action 的 Trigger
        bool PushAction(const std::string& id, CommandContext& ctx);

        // 栈顶 Action 结束出栈；结果数据经框架回传给新的栈顶（父 Action）
        bool PopAction(CommandContext& ctx);

        // 取消整个交互流程
        void ClearStack() { m_stack.Clear(); }
        bool IsStackEmpty() const noexcept { return m_stack.IsEmpty(); }
        std::shared_ptr<IAction> PeekAction() const { return m_stack.Peek(); }

        // ---- 状态刷新 ----
        // 遍历所有 Action 重新求值状态（委托命令），变化时通知 sink
        void RefreshAll(CommandContext& ctx);

        // 调试/测试：访问执行栈
        ActionStack& GetStack() noexcept { return m_stack; }

    private:
        std::unordered_map<std::string, std::shared_ptr<IAction>> m_actions;
        ActionStack m_stack;
    };

} // namespace cadutils
