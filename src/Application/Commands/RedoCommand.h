#pragma once

#include "ICommand.h"

namespace cadutils
{
    class RedoCommand : public ICommand
    {
    public:
        std::string GetId() const override { return "cmd.redo"; }
        std::string GetName() const override { return "Redo"; }
        bool CanExecute(const CommandContext& ctx) const override;
        void Execute(CommandContext& ctx) override;
    };
}