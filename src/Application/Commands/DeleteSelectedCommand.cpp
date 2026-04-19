#include "DeleteSelectedCommand.h"
#include "CommandHelper.h"
#include "CommandRegistry.h"
#include "TransactionManager.h"
#include "Document.h"
#include "MainWindow.h"


REGISTER_COMMAND(cadutils::DeleteSelectedCommand, "cmd.delete_selected", DeleteSelectedCommand)

namespace cadutils
{
    bool DeleteSelectedCommand::CanExecute(const CommandContext& ctx) const
    {
        return ctx.doc && ctx.doc->GetSelected() != 0;
    }

    void DeleteSelectedCommand::Execute(CommandContext& ctx)
    {
        if (!ctx.txMgr || !ctx.doc) return;

        ObjectId selId = ctx.doc->GetSelected();
        if (selId == 0)
            return;

        ctx.txMgr->BeginTransaction();
        ctx.doc->remove(selId);
        ctx.txMgr->Commit();

        ctx.doc->SetSelected(0);

        RefreshAfterCommand(ctx);

        auto* mw = GetMainWindow(ctx);
        if (mw)
            mw->UpdatePropertiesById(0);
    }
}