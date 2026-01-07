#include <iostream>
#include <QApplication>
#include "MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    cadutils::MainWindow w;
    w.show();

    return app.exec();
}