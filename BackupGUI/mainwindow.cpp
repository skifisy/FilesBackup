#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "mydialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>
#include <QProgressDialog>
#include <QTimer>
#include <QAction>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::close);
    connect(ui->action1, &QAction::triggered, this, [&]() {
        QMessageBox::information(this, "Triggered", "我是菜单项, 你不要调戏我...");
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_2_clicked()
{
    MyDialog dialog;
    connect(&dialog, &MyDialog::finished, this, [=](int res){
       qDebug() << "result: " << res;
    });
    int ret = dialog.exec();
    if(ret == QDialog::Accepted)
    {
        qDebug() << "accept button clicked...";
        // 显示主窗口
        MainWindow* w = new MainWindow;
        w->show();
    }
    else if(ret == QDialog::Rejected)
    {
        qDebug() << "reject button clicked...";
        // 不显示主窗口
    }
    else
    {
        // ret == 666
        qDebug() << "done button clicked...";
        // 根据需求进行逻辑处理
    }
}


void MainWindow::on_pushButton_3_clicked()
{
    QMessageBox::about(this, "about",  "这是一个简单的消息提示框!!!");
    QMessageBox::critical(this, "critical", "这是一个错误对话框-critical...");
    int ret = QMessageBox::question(this, "question",
             "你要保存修改的文件内容吗???",
              QMessageBox::Save|QMessageBox::Cancel,
              QMessageBox::Cancel);
    if(ret == QMessageBox::Save)
    {
        QMessageBox::information(this, "information", "恭喜你保存成功了, o(*￣︶￣*)o!!!");
    }
    else if(ret == QMessageBox::Cancel)
    {
        QMessageBox::warning(this, "warning", "你放弃了保存, ┭┮﹏┭┮ !!!");
    }
}


void MainWindow::on_pushButton_4_clicked()
{
    int ret = QInputDialog::getInt(this, "年龄", "您的当前年龄: ", 10, 1, 100, 2);
    QMessageBox::information(this, "年龄", "您的当前年龄: " + QString::number(ret));
}


void MainWindow::on_pushButton_5_clicked()
{
    // 1. 创建进度条对话框窗口对象
    QProgressDialog *progress = new QProgressDialog(
                "正在拷贝数据...", "取消拷贝", 0, 100, this);
    // 2. 初始化并显示进度条窗口
    progress->setWindowTitle("请稍后");
    progress->setWindowModality(Qt::WindowModal);
    progress->show();

    // 3. 更新进度条
    static int value = 0;
    QTimer *timer = new QTimer;
    connect(timer, &QTimer::timeout, this, [=]()
    {
         progress->setValue(value);
         value++;
         // 当value > 最大值的时候
         if(value > progress->maximum())
         {
             timer->stop();
             value = 0;
             delete progress;
             delete timer;
         }
    });

    connect(progress, &QProgressDialog::canceled, this, [=]()
    {
        timer->stop();
        value = 0;
        delete progress;
        delete timer;
    });

    timer->start(50);
}

