#pragma once

#include "ICommand.h"

namespace cadutils
{
    class AddCylinderCommand : public ICommand
    {
    public:
        std::string GetId() const override { return "cmd.add_cylinder"; }
        std::string GetName() const override { return "Add Cylinder"; }
        void Execute(CommandContext& ctx) override;
    };
}