#pragma once

#include "ICommand.h"

namespace cadutils
{
    class DeleteSelectedCommand : public ICommand
    {
    public:
        std::string GetId() const override { return "cmd.delete_selected"; }
        std::string GetName() const override { return "Delete"; }
        bool CanExecute(const CommandContext& ctx) const override;
        void Execute(CommandContext& ctx) override;
    };
}