#pragma once

#include "ICommand.h"

namespace cadutils
{
    class UndoCommand : public ICommand
    {
    public:
        std::string GetId() const override { return "cmd.undo"; }
        std::string GetName() const override { return "Undo"; }
        bool CanExecute(const CommandContext& ctx) const override;
        void Execute(CommandContext& ctx) override;
    };
}