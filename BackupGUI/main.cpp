#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QVBoxLayout>
#include <QPushButton>
#include "file_meta.h"
#include <fstream>
#include "filesystem"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // 创建窗口类对象
    MainWindow w;
    // 显示窗口
    w.show();
    backup::BackupFileHeader header;
    std::ofstream ofs("qt-test-ofs");
    header.Dump(ofs);
    qDebug() << backup::GetCurPath().ToString().c_str();
    return a.exec();
}
