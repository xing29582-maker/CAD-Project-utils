#pragma once

#include "ICommand.h"
#include "RenderSystem.h"
#include "TessellationOptions.h"

namespace cadutils
{
    // Forward declaration — MainWindow is in Application/UI
    class MainWindow;

    // Helper to extract typed pointers from CommandContext
    inline RenderSystem* GetRenderSystem(CommandContext& ctx)
    {
        return static_cast<RenderSystem*>(ctx.renderSystem);
    }

    inline MainWindow* GetMainWindow(CommandContext& ctx)
    {
        return static_cast<MainWindow*>(ctx.mainWindow);
    }

    // Common post-command refresh: full sync + rebuild tree
    void RefreshAfterCommand(CommandContext& ctx);

} // namespace cadutils