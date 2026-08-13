#pragma once

#include "IAction.h"

#include <memory>
#include <string>

namespace cadutils
{
    class ICommand;

    // 通用命令包装 Action：包装 CommandRegistry 中的一个命令。
    // 状态（enabled/visible/checked）委托给被包装的命令；
    // Trigger 委托给命令 Execute（内部先 CanExecute 复检）。
    class CommandAction : public IAction
    {
    public:
        CommandAction(std::string id,
                      std::string name,
                      std::shared_ptr<ICommand> command,
                      std::string icon = "",
                      std::string shortcut = "");

        std::string GetId() const override { return m_id; }
        std::string GetName() const override { return m_name; }
        std::string GetTooltip() const override { return m_tooltip.empty() ? m_name : m_tooltip; }
        std::string GetIcon() const override { return m_icon; }
        std::string GetShortcut() const override { return m_shortcut; }
        ICommand* GetCommand() const override { return m_command.get(); }

        void Trigger(CommandContext& ctx) override;

    private:
        std::string m_id;
        std::string m_name;
        std::string m_tooltip;
        std::string m_icon;
        std::string m_shortcut;
        std::shared_ptr<ICommand> m_command;
    };

} // namespace cadutils
