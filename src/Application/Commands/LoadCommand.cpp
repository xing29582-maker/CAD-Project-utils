#include "LoadCommand.h"
#include "CommandHelper.h"
#include "CommandRegistry.h"
#include "Document.h"
#include "TransactionManager.h"
#include <QFileDialog>
#include <QMessageBox>

using namespace cadutils;

bool LoadCommand::CanExecute(const CommandContext& ctx) const
{
    return ctx.doc != nullptr;
}

void LoadCommand::Execute(CommandContext& ctx)
{
    if (!ctx.doc)
        return;

    QString path = QFileDialog::getOpenFileName(
        nullptr,
        "Load Document",
        "",
        "CAD Files (*.cad);;All Files (*.*)"
    );

    if (path.isEmpty())
        return;

    bool success = ctx.doc->LoadFromFile(path.toStdString());

    if (!success)
    {
        QMessageBox::critical(nullptr, "Error", "Failed to load document");
        return;
    }

    // Clear transaction stacks after loading
    if (ctx.txMgr)
    {
        ctx.txMgr->Clear();
    }

    // Trigger full refresh
    RefreshAfterCommand(ctx);
}

REGISTER_COMMAND(cadutils::LoadCommand, "cmd.file_load", LoadCommand)