#include "SaveCommand.h"
#include "CommandHelper.h"
#include "CommandRegistry.h"
#include "Document.h"
#include <QFileDialog>
#include <QMessageBox>

using namespace cadutils;

bool SaveCommand::CanExecute(const CommandContext& ctx) const
{
    return ctx.doc != nullptr;
}

void SaveCommand::Execute(CommandContext& ctx)
{
    if (!ctx.doc)
        return;

    QString path = QFileDialog::getSaveFileName(
        nullptr,
        "Save Document",
        "",
        "CAD Files (*.cad);;All Files (*.*)"
    );

    if (path.isEmpty())
        return;

    // Ensure .cad extension
    if (!path.endsWith(".cad", Qt::CaseInsensitive))
    {
        path += ".cad";
    }

    bool success = ctx.doc->SaveToFile(path.toStdString());

    if (!success)
    {
        QMessageBox::critical(nullptr, "Error", "Failed to save document");
    }
}

REGISTER_COMMAND(cadutils::SaveCommand, "cmd.file_save", SaveCommand)