#include "AddSphereCommand.h"
#include "CommandHelper.h"
#include "CommandRegistry.h"
#include "TransactionManager.h"
#include "Document.h"
#include "ObjectFactory.h"
#include "Point3d.h"
#include "MainWindow.h"

#include <QString>


REGISTER_COMMAND(cadutils::AddSphereCommand, "cmd.add_sphere", AddSphereCommand)

namespace cadutils
{
    void AddSphereCommand::Execute(CommandContext& ctx)
    {
        if (!ctx.txMgr || !ctx.doc) return;

        auto* mw = GetMainWindow(ctx);
        int counter = 1;
        if (mw)
            counter = mw->SphereCounter()++;

        QString name = QString("Sphere_%1").arg(counter);
        auto newObj = ObjectFactory::CreateSphereObject(
            name.toStdString(), Point3d(0, 0, 0), 50.0);

        ctx.txMgr->BeginTransaction();
        ctx.doc->add(newObj);
        ctx.txMgr->Commit();

        RefreshAfterCommand(ctx);
    }
}