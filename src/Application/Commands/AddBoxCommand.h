#pragma once

#include "ICommand.h"

namespace cadutils
{
    class AddBoxCommand : public ICommand
    {
    public:
        std::string GetId() const override { return "cmd.add_box"; }
        std::string GetName() const override { return "Add Box"; }
        void Execute(CommandContext& ctx) override;
    };
}