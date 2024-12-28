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
#include <QLineEdit>
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

    if (!folderPath.isEmpty() && !files_to_pack.contains(folderPath)) {
        files_to_pack.insert(folderPath);
        QFileInfo fileInfo(folderPath);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(ui->backupFileList);
        rootItem->setText(0, fileInfo.fileName());
        rootItem->setText(1, getTypeTag(folderPath));
        rootItem->setText(2, folderPath);
        rootItem->setCheckState(0, Qt::Checked);
        generateTreeItem(folderPath.toStdString(), rootItem);
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
                rootItem->setText(2, file);
                rootItem->setCheckState(0, Qt::Checked);
                rootItem->setText(1, getTypeTag(file));
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

QString MainWindow::getTypeTag(backup::FileType type)
{
    switch (type) {
    case backup::FileType::REG:
        return "文件";
    case backup::FileType::DIR:
        return "文件夹";
    case backup::FileType::FIFO:
        return "管道";
    case backup::FileType::FLNK:
        return "软链接";
    case backup::FileType::SOCK:
        return "socket";
    case backup::FileType::CHR:
        return "chr";
    case backup::FileType::BLK:
        return "blk";
    default:
        return "unknown";
    }
}

QString MainWindow::getTypeTag(const QString &file_path)
{
    backup::Path path = file_path.toStdString();
    backup::FileType type = path.GetFileType();
    return getTypeTag(type);
}

void MainWindow::generateTreeItem(
    const backup::Path &dir,
    QTreeWidgetItem *parent)
{
    for (backup::Path &file : backup::GetFilesFromDir(dir)) {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, file.ToString().c_str());
        backup::Path fullPath = (dir / file).ToString().c_str();
        backup::FileType type = fullPath.GetFileType();
        item->setText(1, getTypeTag(type));            // type
        item->setText(2, fullPath.ToString().c_str()); // fullpath
        item->setCheckState(0, Qt::Checked);
        if (type == backup::FileType::DIR) { generateTreeItem(fullPath, item); }
    }
}

void MainWindow::on_browseLocalFile_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择打包文件", "", "所有文件 (*);;打包文件 (*.bak)");
    qDebug() << filePath;
    ui->localFileRestoreLineEdit->setText(filePath);
}

void MainWindow::on_browseRestoreDirectoryButton_clicked()
{
    // 打开文件夹选择对话框
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        "选择文件夹",
        "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!folderPath.isEmpty()) {
        ui->backupFileRestoreDirectoryLineEdit->setText(folderPath);
    }
}
