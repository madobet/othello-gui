#include "mainwindow.h"
#include <QApplication>

double total_time = 0.0;    // 当局game进行的总时间
int main(int argc, char *argv[])
{
    // Qt的标准操作
    QApplication app(argc, argv);
    MainWindow w;
    w.show();

    return app.exec();
}
