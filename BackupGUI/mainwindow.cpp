#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "mydialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>
#include <QProgressDialog>
#include <QTimer>
#include <QAction>
#include <QScreen>
#include "backupconfigdialog.h"
#include <QFileDialog>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 获取屏幕对象
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        // 获取屏幕的可用区域
        QRect screenGeometry = screen->availableGeometry();
        // 计算窗口居中的位置
        int x = (screenGeometry.width() - this->width()) / 2;
        int y = (screenGeometry.height() - this->height()) / 2;
        // 设置窗口位置
        this->move(x, y);
    }
    ui->backupFileList->setColumnWidth(0, 200);
    ui->backupFileList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    backupDialog = new BackupConfigDialog(this);
    connect(
        backupDialog,
        &BackupConfigDialog::backupFile,
        this,
        &MainWindow::on_backupFiles);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_startBackupButton_clicked()
{
    if (files_to_pack.empty()) {
        QMessageBox::information(
            this,
            "提示",
            "请选择需要备份的文件。",
            QMessageBox::Yes,
            QMessageBox::Yes);
        return;
    }
    backupDialog->show();
}

void MainWindow::on_addDirectoryButton_clicked()
{
    // 打开文件夹选择对话框
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        "选择文件夹",
        "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    // 如果选择了文件夹，更新标签显示路径
    if (!folderPath.isEmpty() && !files_to_pack.contains(folderPath)) {
        files_to_pack.insert(folderPath);
        QFileInfo fileInfo(folderPath);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(ui->backupFileList);
        rootItem->setText(0, fileInfo.fileName());
        rootItem->setText(1, folderPath);
        rootItem->setCheckState(0, Qt::Checked);
    }
}

void MainWindow::on_addFileButton_clicked()
{
    // 弹出文件对话框，允许多选文件
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "选择文件",
        "",
        "所有文件 (*);;文本文件 (*.txt);;图片文件 (*.png *.jpg)");

    if (!files.isEmpty()) {
        for (const QString &file : files) {
            if (!files_to_pack.contains(file)) {
                files_to_pack.insert(file);
                QFileInfo fileInfo(file);
                QTreeWidgetItem *rootItem =
                    new QTreeWidgetItem(ui->backupFileList);
                rootItem->setText(0, fileInfo.fileName());
                rootItem->setText(1, file);
                rootItem->setCheckState(0, Qt::Checked);
            } else {
                statusBar()->showMessage("文件" + file + "已存在", 5000);
            }
        }
    }
}

void MainWindow::on_clearFileButton_clicked()
{
    ui->backupFileList->clear();
    files_to_pack.clear();
}

void MainWindow::on_deleteFileButton_clicked()
{
    auto list = ui->backupFileList->selectedItems();
    for (QTreeWidgetItem *ptr : list) {
        files_to_pack.remove(ptr->text(1));
        delete ptr;
    }
}

void MainWindow::on_backupFiles(QSharedPointer<BackupConfig> config)
{
    qDebug() << config->backPath << "  " << config->filename;
    // TODO: backupLogic
    //
}
