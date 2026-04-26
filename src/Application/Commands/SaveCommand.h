#pragma once

#include "ICommand.h"

namespace cadutils
{
    class SaveCommand : public ICommand
    {
    public:
        std::string GetId() const override { return "cmd.file_save"; }
        std::string GetName() const override { return "Save"; }
        
        bool CanExecute(const CommandContext& ctx) const override;
        void Execute(CommandContext& ctx) override;
    };
}