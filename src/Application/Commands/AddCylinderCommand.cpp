#include "AddCylinderCommand.h"
#include "CommandHelper.h"
#include "CommandRegistry.h"
#include "TransactionManager.h"
#include "Document.h"
#include "ObjectFactory.h"
#include "Point3d.h"
#include "MainWindow.h"

#include <QString>

REGISTER_COMMAND(cadutils::AddCylinderCommand, "cmd.add_cylinder", AddCylinderCommand)

namespace cadutils
{
    void AddCylinderCommand::Execute(CommandContext& ctx)
    {
        if (!ctx.txMgr || !ctx.doc) return;

        auto* mw = GetMainWindow(ctx);
        int counter = 1;
        if (mw)
            counter = mw->CylinderCounter()++;

        QString name = QString("Cylinder_%1").arg(counter);
        auto newObj = ObjectFactory::CreateCylinderObject(
            name.toStdString(), Point3d(0, 0, 0), 30.0, 80.0);

        ctx.txMgr->BeginTransaction();
        ctx.doc->add(newObj);
        ctx.txMgr->Commit();

        RefreshAfterCommand(ctx);
    }
}