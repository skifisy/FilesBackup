#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <QSet>
#include "backupconfigdialog.h"
#include "file_sys.h"
#include <QTreeWidgetItem>
#include "file_tree.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// 文件恢复时，文件树的列名
enum struct RecoverEnum
{
    FILE_NAME = 0,
    SIZE,
    FILE_TYPE,
    PERMISSION,
    MOD_TIME,
    OWNER
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
  private slots:
    void on_startBackupButton_clicked();

    void on_addDirectoryButton_clicked();

    void on_addFileButton_clicked();

    void on_clearFileButton_clicked();

    void on_deleteFileButton_clicked();
    // 开始打包文件
    void on_backupFiles(QSharedPointer<BackupConfig> config);
    // 备份文件解析
    void on_browseLocalFile_clicked();

    void on_browseRestoreDirectoryButton_clicked();
    // 设计双击进入界面
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);

  private:
    /// 获取文件类型
    QString GetTypeTag(backup::FileType type);
    QString GetTypeTag(const QString &file_path);
    /// 获取文件图标
    QString GetFileIcon(backup::FileType type);

    // 文件备份创建item
    void generateTreeItem(const backup::Path &dir, QTreeWidgetItem *parent);
    QTreeWidgetItem *generateOneTreeItem(
        const QString &filename,
        const QString &typetag,
        const QString &fullpath,
        const QString &pack_path);
    QList<QTreeWidgetItem *> getCheckedItems(QTreeWidget *tree);
    void getCheckedItems(QList<QTreeWidgetItem *> &list, QTreeWidgetItem *root);

    // 文件恢复创建item
    QTreeWidgetItem *generateOneRecoverItem(
        const QString &filename,
        const QString &size,
        backup::FileType filetype,
        const QString &permission,
        const QString &mod_time,
        const QString &owner,
        bool setCheckState = true);
    void generateRecoverTreeItem(
        std::shared_ptr<backup::FileNode> root,
        QTreeWidgetItem *parent);

    QString CurPathToString();
    Ui::MainWindow *ui;
    // QString: 文件的绝对路径
    // 只存储top_level（后面level不考虑）
    QSet<QString> files_to_pack;
    BackupConfigDialog *backupDialog;
    // 打包文件恢复时，首次输入密码后暂存
    QString password;
    bool is_encrypted;
    QList<QString> cur_path;
    std::shared_ptr<backup::FileNode> recover_tree;
};
#endif // MAINWINDOW_H
