#include <iostream>
#include <QApplication>
#include "MainWindow.h"
#include "RegisterAllCommands.h"
#include "RegisterAllTypes.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Ensure all object types are registered in MetaRegistry
    cadutils::RegisterAllTypes();

    // Ensure all commands are linked and registered
    cadutils::RegisterAllCommands();

    cadutils::MainWindow w;
    w.show();

    return app.exec();
}