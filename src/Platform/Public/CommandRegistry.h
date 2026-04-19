#pragma once

#include "PlatformExport.h"
#include "ICommand.h"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace cadutils
{
    class CADUTILS_PLATFORM_API CommandRegistry
    {
    public:
        using FactoryFn = std::unique_ptr<ICommand>(*)();

        static CommandRegistry& Instance();

        // Register a command factory by id
        void Register(const std::string& id, FactoryFn factory);

        // Create a command instance by id. Returns nullptr if not found.
        std::unique_ptr<ICommand> Create(const std::string& id) const;

        // Get all registered command ids
        std::vector<std::string> GetAllIds() const;

        // Check if a command id is registered
        bool Contains(const std::string& id) const;

    private:
        CommandRegistry() = default;
        std::unordered_map<std::string, FactoryFn> m_factories;
    };

    // Helper struct for static auto-registration via REGISTER_COMMAND macro
    struct CommandRegistrar
    {
        CommandRegistrar(const char* id, CommandRegistry::FactoryFn fn)
        {
            CommandRegistry::Instance().Register(id, fn);
        }
    };

} // namespace cadutils

// Macro for auto-registering a command class.
// Place this in the command's .cpp file after #include, OUTSIDE any namespace.
// Usage: REGISTER_COMMAND(cadutils::MyCommand, "cmd.my_command")
// Note: Use a simple token for the second arg (TAG) to form a valid C++ identifier.
#define REGISTER_COMMAND(CmdClass, CmdId, Tag)                              \
    static ::cadutils::CommandRegistrar _cad_cmd_reg_##Tag(                 \
        CmdId,                                                              \
        []() -> std::unique_ptr<::cadutils::ICommand> {                     \
            return std::make_unique<CmdClass>();                             \
        }                                                                   \
    );