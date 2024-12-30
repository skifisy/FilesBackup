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
#include <QFileDialog>
#include <QLineEdit>
#include "backupconfigdialog.h"
#include "backup_impl.h"
#include "message.h"
#include "input_dialog.h"
#include "util.h"

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
    ui->packFileList->setColumnWidth(0, 250);
    ui->packFileList->setSelectionMode(QAbstractItemView::ExtendedSelection);
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
        QTreeWidgetItem *rootItem = generateOneTreeItem(
            fileInfo.fileName(), GetTypeTag(folderPath), folderPath, "");
        ui->backupFileList->addTopLevelItem(rootItem);
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

                QTreeWidgetItem *rootItem = generateOneTreeItem(
                    fileInfo.fileName(),
                    GetTypeTag(file),
                    file,
                    "" // 顶层的pack_path为""
                );
                ui->backupFileList->addTopLevelItem(rootItem);
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
    QList<QTreeWidgetItem *> filelist = getCheckedItems(ui->backupFileList);
    std::vector<std::pair<std::string, std::string>> list;
    for (QTreeWidgetItem *item : filelist) {
        list.emplace_back(
            item->text(2).toStdString(),
            item->data(0, Qt::UserRole).toString().toStdString());
    }
    backup::BackUp *back = new backup::BackUpImpl();
    backup::BackupConfig backupConfig{
        config->filename.toStdString(),
        config->backPath.toStdString(),
        config->isEncrypt,
        config->password.toStdString()};
    backup::Status s = back->BackupBatch(backupConfig, list);
    if (s.code == backup::OK) {
        Message::info(this, "文件备份成功！");
    } else {
        Message::warning(this, s.msg.c_str());
    }
}

QString MainWindow::GetTypeTag(backup::FileType type)
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

QString MainWindow::GetTypeTag(const QString &file_path)
{
    backup::Path path = file_path.toStdString();
    backup::FileType type = path.GetFileType();
    return GetTypeTag(type);
}

void MainWindow::generateTreeItem(
    const backup::Path &dir,
    QTreeWidgetItem *parent)
{
    QString pack_path_parent = parent->data(0, Qt::UserRole).toString();
    QString pack_path;
    if (pack_path_parent.isEmpty()) {
        pack_path = parent->text(0);
    } else {
        pack_path = pack_path_parent + "/" + parent->text(0);
    }
    for (backup::Path &file : backup::GetFilesFromDir(dir)) {
        backup::Path fullPath = (dir / file).ToString().c_str();
        backup::FileType type = fullPath.GetFileType();

        QTreeWidgetItem *item = generateOneTreeItem(
            file.ToString().c_str(),
            GetTypeTag(type),
            fullPath.ToString().c_str(),
            pack_path);
        parent->addChild(item);
        if (type == backup::FileType::DIR) { generateTreeItem(fullPath, item); }
    }
}

QTreeWidgetItem *MainWindow::generateOneTreeItem(
    const QString &filename,
    const QString &typetag,
    const QString &fullpath,
    const QString &pack_path)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, filename);
    item->setText(1, typetag);
    item->setText(2, fullpath);
    item->setCheckState(0, Qt::Checked);
    item->setData(0, Qt::UserRole, pack_path);
    return item;
}

QList<QTreeWidgetItem *> MainWindow::getCheckedItems(QTreeWidget *tree)
{
    QList<QTreeWidgetItem *> list;
    // 1. 遍历所有顶级项
    for (int i = 0; i < tree->topLevelItemCount(); i++) {
        auto item = tree->topLevelItem(i);
        if (item->checkState(0) == Qt::Checked) { list.append(item); }
        // 2. 递归添加子项
        getCheckedItems(list, item);
    }
    return list;
}

void MainWindow::getCheckedItems(
    QList<QTreeWidgetItem *> &list,
    QTreeWidgetItem *root)
{
    // 1. 遍历root的所有子项
    for (int i = 0; i < root->childCount(); i++) {
        auto child = root->child(i);
        if (child->checkState(0) == Qt::Checked) { list.append(child); }
        // 2. 递归遍历
        getCheckedItems(list, child);
    }
}

QTreeWidgetItem *MainWindow::generateOneRecoverItem(
    const QString &filename,
    const QString &size,
    const QString &filetype,
    const QString &permission,
    const QString &mod_time,
    const QString &owner)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(static_cast<int>(RecoverEnum::FILE_NAME), filename);
    item->setText(static_cast<int>(RecoverEnum::SIZE), size);
    item->setText(static_cast<int>(RecoverEnum::FILE_TYPE), filetype);
    item->setText(static_cast<int>(RecoverEnum::PERMISSION), permission);
    item->setText(static_cast<int>(RecoverEnum::MOD_TIME), mod_time);
    item->setText(static_cast<int>(RecoverEnum::OWNER), owner);
    item->setCheckState(0, Qt::Checked);
    return item;
}

void MainWindow::on_browseLocalFile_clicked()
{
    auto tree = ui->packFileList;
    tree->clear();
    ui->localFileRestoreLineEdit->clear();
    password.clear();
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择备份文件", "", "备份文件 (*.bak);;所有文件 (*)");
    if (filePath.isEmpty()) return;
    backup::BackUp *back = new backup::BackUpImpl();
    auto [status, is_encryt] = back->isEncrypted(filePath.toStdString());
    if (status.code == backup::OK && is_encryt) {
        this->is_encrypted = is_encrypted;
        while (true) {
            auto [ok, pass] = InputDialog::getText(this, "请输入密码：");
            if (!ok) return;
            if (pass.isEmpty()) { Message::warning(this, "密码不能为空！"); }
            password = std::move(pass);
            break;
        }
    } else if (status.code != backup::OK) {
        Message::warning(this, status.msg.c_str());
        return;
    } else
        this->is_encrypted = false;
    ui->localFileRestoreLineEdit->setText(filePath);
    auto [s, rootnode] =
        back->GetFileList(filePath.toStdString(), password.toStdString());
    if (s.code != backup::OK) {
        Message::warning(this, s.msg.c_str());
        return;
    }
    // 构建filetree
    // 1. 添加第一层
    for (auto &[filename, node] : rootnode->children_) {
        auto &meta = node->meta_;
        QTreeWidgetItem *item = generateOneRecoverItem(
            filename.c_str(),
            FormatFileSize(meta.size),
            GetTypeTag(static_cast<backup::FileType>(meta.type)),
            FormatPermission(
                meta.permissions, static_cast<backup::FileType>(meta.type)),
            FormatTime(meta.mod_time),
            backup::UidToString(meta.uid).c_str());
        tree->addTopLevelItem(item);
        // 2. 递归添加
        generateRecoverTreeItem(node, item);
    }
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

void MainWindow::generateRecoverTreeItem(
    std::shared_ptr<backup::FileNode> root,
    QTreeWidgetItem *parent)
{
    for (auto &[filename, node] : root->children_) {
        auto &meta = node->meta_;
        QTreeWidgetItem *item = generateOneRecoverItem(
            filename.c_str(),
            FormatFileSize(meta.size),
            GetTypeTag(static_cast<backup::FileType>(meta.type)),
            FormatPermission(
                meta.permissions, static_cast<backup::FileType>(meta.type)),
            FormatTime(meta.mod_time),
            backup::UidToString(meta.uid).c_str());
        parent->addChild(item);
        generateRecoverTreeItem(node, item);
    }
}
