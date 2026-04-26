#pragma once

#include "ICommand.h"

namespace cadutils
{
    class LoadCommand : public ICommand
    {
    public:
        std::string GetId() const override { return "cmd.file_load"; }
        std::string GetName() const override { return "Load"; }
        
        bool CanExecute(const CommandContext& ctx) const override;
        void Execute(CommandContext& ctx) override;
    };
}