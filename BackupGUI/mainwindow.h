#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <QSet>
#include "backupconfigdialog.h"
#include "file_sys.h"
#include <QTreeWidgetItem>
#include "file_tree.h"
#include "backup_check.h"
#include "schedule_task.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// 选择文件时，文件树的列名
enum class BackupEnum
{
    FILE_NAME = 0,
    SIZE,
    FILE_TYPE,
    PERMISSION,
    MOD_TIME,
    OWNER,
    FULL_PATH
};

// 文件恢复时，文件树的列名
enum class RecoverEnum
{
    FILE_NAME = 0,
    SIZE,
    FILE_TYPE,
    PERMISSION,
    MOD_TIME,
    OWNER
};

// 备份验证，tree widget列名
enum class CheckEnum
{
    DIFF_TYPE,   // 差异类型
    ORIGIN_PATH, // 源路径
    BACKUP_PATH, // 备份路径
    DETAILS,     // 详情
};

// 定时备份
enum class SchedulerEnum
{
    FILENAME,    // 文件名
    NEXTTIME,    // 下次备份时间
    FREQUENT,    // 备份频率
    ENCRYPT,     // 是否加密
    TASK_STATUS, // 任务状态
    LOCAL_PATH,  // 本地路径
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
    // 双击文件夹
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);
    // 开始恢复文件
    void on_startRestoreButton_clicked();
    void on_checkStateChange(QTreeWidgetItem *item, int column);

    void on_browseCheckFile_clicked();

    void on_checkResultList_itemClicked(QTreeWidgetItem *item, int column);

    void on_checkResultList_customContextMenuRequested(const QPoint &pos);

    void on_tabWidget_currentChanged(int index);

    void on_deleteTaskButton_clicked();

    void on_clearTaskButton_clicked();

private:
    /// 获取文件类型
    QString GetTypeTag(backup::FileType type);
    /// 获取文件图标
    QString GetFileIcon(backup::FileType type);
    QString GetCheckTypeTag(backup::CheckType type);

    // 文件备份创建item
    void generateTreeItem(const backup::Path &dir, QTreeWidgetItem *parent);
    QTreeWidgetItem *generateOneTreeItem(
        const QString &filename,
        backup::FileType filetype,
        const QString &fullpath,
        const QString &pack_path);

    QList<QTreeWidgetItem *>
    getCheckedItems(QTreeWidget *tree, bool isRecursive = true);

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

    void TreeUpdateParentCheckState(QTreeWidgetItem *childItem);

    void TreeItemSetCheckState(QTreeWidgetItem *item, Qt::CheckState state);

    void RenderTaskList();

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
    backup::TaskScheduler scheduler;
};
#endif // MAINWINDOW_H
