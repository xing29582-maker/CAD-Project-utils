#include "UndoCommand.h"
#include "CommandHelper.h"
#include "CommandRegistry.h"
#include "TransactionManager.h"
#include "Document.h"
#include "MainWindow.h"


REGISTER_COMMAND(cadutils::UndoCommand, "cmd.undo", UndoCommand)

namespace cadutils
{
    bool UndoCommand::CanExecute(const CommandContext& ctx) const
    {
        return ctx.txMgr && ctx.txMgr->CanUndo();
    }

    void UndoCommand::Execute(CommandContext& ctx)
    {
        if (!ctx.txMgr) return;
        ctx.txMgr->Undo();

        RefreshAfterCommand(ctx);

        auto* mw = GetMainWindow(ctx);
        if (mw && ctx.doc)
        {
            ObjectId selId = ctx.doc->GetSelected();
            mw->UpdatePropertiesById(selId);
        }
    }
}