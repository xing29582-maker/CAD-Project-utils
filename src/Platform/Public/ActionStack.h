#pragma once

#include "PlatformExport.h"
#include "IAction.h"

#include <memory>
#include <vector>

namespace cadutils
{
    // 父子 Action 执行栈：父 Action 挂起、子 Action 入栈，框架自动调度栈顶。
    class CADUTILS_PLATFORM_API ActionStack
    {
    public:
        ActionStack() = default;
        ~ActionStack() = default;

        // 子 Action 入栈（入栈后由 ActionManager 调用新栈顶 Trigger）
        void Push(const std::shared_ptr<IAction>& action);

        // 栈顶 Action 出栈（结果由 ActionManager 回传给新的栈顶父 Action）
        std::shared_ptr<IAction> Pop();

        // 当前最上层 Action（空 = 栈空）
        std::shared_ptr<IAction> Peek() const;

        bool IsEmpty() const noexcept { return m_stack.empty(); }
        std::size_t Size() const noexcept { return m_stack.size(); }

        // 清空栈（取消整个交互流程）
        void Clear() noexcept { m_stack.clear(); }

    private:
        std::vector<std::shared_ptr<IAction>> m_stack;
    };

} // namespace cadutils
