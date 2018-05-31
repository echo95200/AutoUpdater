#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("AutoUpdater");
    w.showFullScreen();
    w.setWindowFlags(Qt::WindowStaysOnTopHint);

    w.show();

    return a.exec();
}
