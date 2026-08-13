#include "CommandHelper.h"
#include "MainWindow.h"

namespace cadutils
{
    void RefreshAfterCommand(CommandContext& ctx)
    {
        auto* rs = GetRenderSystem(ctx);
        auto* mw = GetMainWindow(ctx);
        if (!rs || !ctx.doc) return;

        TessellationOptions opt;
        rs->FullSyncFromDocument(ctx.doc, opt);
        rs->Refresh(true);

        if (mw)
        {
            mw->RebuildAfterCommand();
            mw->RefreshActions();
        }
    }

} // namespace cadutils