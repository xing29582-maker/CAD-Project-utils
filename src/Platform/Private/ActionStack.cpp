#include "ActionStack.h"

using namespace cadutils;

void ActionStack::Push(const std::shared_ptr<IAction>& action)
{
    if (action)
        m_stack.push_back(action);
}

std::shared_ptr<IAction> ActionStack::Pop()
{
    if (m_stack.empty())
        return nullptr;
    auto top = m_stack.back();
    m_stack.pop_back();
    return top;
}

std::shared_ptr<IAction> ActionStack::Peek() const
{
    if (m_stack.empty())
        return nullptr;
    return m_stack.back();
}
