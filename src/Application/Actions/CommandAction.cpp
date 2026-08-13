#include "CommandAction.h"

using namespace cadutils;

CommandAction::CommandAction(std::string id,
                             std::string name,
                             std::shared_ptr<ICommand> command,
                             std::string icon,
                             std::string shortcut)
    : m_id(std::move(id))
    , m_name(std::move(name))
    , m_icon(std::move(icon))
    , m_shortcut(std::move(shortcut))
    , m_command(std::move(command))
{
    m_tooltip = m_name;
}

void CommandAction::Trigger(CommandContext& ctx)
{
    if (!m_command)
        return;

    if (!m_command->CanExecute(ctx))
        return;

    m_command->Execute(ctx);
    NotifyExecuted();
}
