#include "AddBoxCommand.h"
#include "CommandHelper.h"
#include "CommandRegistry.h"
#include "TransactionManager.h"
#include "Document.h"
#include "ObjectFactory.h"
#include "Point3d.h"
#include "MainWindow.h"

#include <QString>

REGISTER_COMMAND(cadutils::AddBoxCommand, "cmd.add_box", AddBoxCommand)

namespace cadutils
{
    void AddBoxCommand::Execute(CommandContext& ctx)
    {
        if (!ctx.txMgr || !ctx.doc) return;

        auto* mw = GetMainWindow(ctx);
        int counter = 1;
        if (mw)
            counter = mw->BoxCounter()++;

        QString name = QString("Box_%1").arg(counter);
        auto newObj = ObjectFactory::CreateBoxObject(
            name.toStdString(), Point3d(0, 0, 0), 80.0, 60.0, 40.0);

        ctx.txMgr->BeginTransaction();
        ctx.doc->add(newObj);
        ctx.txMgr->Commit();

        RefreshAfterCommand(ctx);
    }
}