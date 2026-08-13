#include "ActionManager.h"

using namespace cadutils;

bool ActionManager::Register(const std::shared_ptr<IAction>& action)
{
    if (!action)
        return false;
    const std::string id = action->GetId();
    if (id.empty() || m_actions.find(id) != m_actions.end())
        return false;
    m_actions[id] = action;
    return true;
}

std::shared_ptr<IAction> ActionManager::Find(const std::string& id) const
{
    auto it = m_actions.find(id);
    return it == m_actions.end() ? nullptr : it->second;
}

std::vector<std::shared_ptr<IAction>> ActionManager::GetAllActions() const
{
    std::vector<std::shared_ptr<IAction>> out;
    out.reserve(m_actions.size());
    for (const auto& [id, action] : m_actions)
        out.push_back(action);
    return out;
}

bool ActionManager::PushAction(const std::string& id, CommandContext& ctx)
{
    auto action = Find(id);
    if (!action)
        return false;

    m_stack.Push(action);
    // 框架自动调度：调用新的栈顶 Action
    action->Trigger(ctx);
    return true;
}

bool ActionManager::PopAction(CommandContext& ctx)
{
    if (m_stack.IsEmpty())
        return false;

    auto sub = m_stack.Pop();
    if (sub && !m_stack.IsEmpty())
    {
        // 框架把子 Action 的结果数据回传给新的栈顶（父 Action）
        auto parent = m_stack.Peek();
        if (parent)
            parent->OnSubActionFinished(ctx, sub->GetId(), sub->GetResult());
    }
    return true;
}

void ActionManager::RefreshAll(CommandContext& ctx)
{
    for (auto& [id, action] : m_actions)
        action->Refresh(ctx);
}
