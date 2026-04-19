#pragma once

#include "ICommand.h"

#include <QString>
#include <memory>
#include <vector>

class QMainWindow;

namespace cadutils
{
    class CommandRegistry;

    // Holds a live command instance + its QAction, used by CommandUIBuilder
    struct CommandBinding
    {
        std::unique_ptr<ICommand> command;
        // QAction is owned by QMainWindow, no need to manage lifetime here
    };

    class CommandUIBuilder
    {
    public:
        // Build menus and toolbars from an XML layout file.
        // Commands are created from the registry and connected to the context.
        // Returns false if the file cannot be opened or parsed.
        static bool BuildFromXml(const QString& xmlPath,
                                 QMainWindow* window,
                                 CommandRegistry& registry,
                                 std::shared_ptr<CommandContext> ctx);

        // Fallback: build menus/toolbars from all registered commands
        // when XML config file is not found.
        static void BuildFallback(QMainWindow* window,
                                  CommandRegistry& registry,
                                  std::shared_ptr<CommandContext> ctx);
    };

} // namespace cadutils