#include "IAction.h"

using namespace cadutils;

bool IAction::IsEnabled(const CommandContext& ctx) const
{
    if (auto* cmd = GetCommand())
        return cmd->CanExecute(ctx);
    return true;
}

bool IAction::IsVisible(const CommandContext& ctx) const
{
    if (auto* cmd = GetCommand())
        return cmd->IsVisible(ctx);
    return true;
}

bool IAction::IsChecked(const CommandContext& ctx) const
{
    if (auto* cmd = GetCommand())
        return cmd->IsChecked(ctx);
    return false;
}

void IAction::Refresh(const CommandContext& ctx)
{
    const bool en = IsEnabled(ctx);
    const bool vis = IsVisible(ctx);
    const bool chk = IsChecked(ctx);

    if (en != m_enabled || vis != m_visible || chk != m_checked)
    {
        m_enabled = en;
        m_visible = vis;
        m_checked = chk;

        const std::string id = GetId();
        for (auto* sink : m_sinks)
        {
            if (sink)
                sink->OnActionStateChanged(id);
        }
    }
}

void IAction::AttachSink(ActionSink* sink)
{
    if (!sink)
        return;
    for (auto* s : m_sinks)
    {
        if (s == sink)
            return; // 已挂载
    }
    m_sinks.push_back(sink);
}

void IAction::DetachSink(ActionSink* sink)
{
    for (auto it = m_sinks.begin(); it != m_sinks.end(); ++it)
    {
        if (*it == sink)
        {
            m_sinks.erase(it);
            return;
        }
    }
}

void IAction::NotifyExecuted()
{
    const std::string id = GetId();
    for (auto* sink : m_sinks)
    {
        if (sink)
            sink->OnActionExecuted(id);
    }
}
