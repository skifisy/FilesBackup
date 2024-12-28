#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QVBoxLayout>
#include <QPushButton>
#include "fileexplore.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // 创建窗口类对象
    MainWindow w;
    // 显示窗口
    w.show();
    //    FileExplorer fileExplorer;
    //    fileExplorer.show();
    return a.exec();
}
