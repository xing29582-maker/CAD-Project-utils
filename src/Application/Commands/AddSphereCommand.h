#pragma once

#include "ICommand.h"

namespace cadutils
{
    class AddSphereCommand : public ICommand
    {
    public:
        std::string GetId() const override { return "cmd.add_sphere"; }
        std::string GetName() const override { return "Add Sphere"; }
        void Execute(CommandContext& ctx) override;
    };
}