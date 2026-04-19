#pragma once

#include "PlatformExport.h"

#include <string>
#include <memory>

namespace cadutils
{
    class Document;
    class TransactionManager;
    class RenderSystem;

    // Command execution context, passed to every command
    struct CommandContext
    {
        std::shared_ptr<Document>           doc;
        std::shared_ptr<TransactionManager> txMgr;

        // Render system and main window are set by Application layer.
        // Using void* for mainWindow to avoid Qt dependency in Platform layer.
        void* renderSystem = nullptr;
        void* mainWindow   = nullptr;
    };

    // Abstract command interface
    class CADUTILS_PLATFORM_API ICommand
    {
    public:
        virtual ~ICommand() = default;

        // Unique command identifier, e.g. "cmd.undo"
        virtual std::string GetId() const = 0;

        // Display name shown in menus/toolbars
        virtual std::string GetName() const = 0;

        // Whether the command can currently execute
        virtual bool CanExecute(const CommandContext& ctx) const { (void)ctx; return true; }

        // Execute the command
        virtual void Execute(CommandContext& ctx) = 0;
    };

} // namespace cadutils