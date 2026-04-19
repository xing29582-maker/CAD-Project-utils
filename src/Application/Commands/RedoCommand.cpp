#include "RedoCommand.h"
#include "CommandHelper.h"
#include "CommandRegistry.h"
#include "TransactionManager.h"
#include "Document.h"
#include "MainWindow.h"


REGISTER_COMMAND(cadutils::RedoCommand, "cmd.redo", RedoCommand)

namespace cadutils
{
    bool RedoCommand::CanExecute(const CommandContext& ctx) const
    {
        return ctx.txMgr && ctx.txMgr->CanRedo();
    }

    void RedoCommand::Execute(CommandContext& ctx)
    {
        if (!ctx.txMgr) return;
        ctx.txMgr->Redo();

        RefreshAfterCommand(ctx);

        auto* mw = GetMainWindow(ctx);
        if (mw && ctx.doc)
        {
            ObjectId selId = ctx.doc->GetSelected();
            mw->UpdatePropertiesById(selId);
        }
    }
}