#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <QSet>
#include "backupconfigdialog.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

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

    void on_backupFiles(QSharedPointer<BackupConfig> config);

  private:
    Ui::MainWindow *ui;
    // first: filename
    // second: filepath
    std::vector<std::pair<QString, QString>> files_to_save;
    // QString: full path to files
    QSet<QString> files_to_pack;
    BackupConfigDialog *backupDialog;
};
#endif // MAINWINDOW_H
