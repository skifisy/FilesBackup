#ifndef BACKUPCONFIGDIALOG_H
#define BACKUPCONFIGDIALOG_H

#include <QButtonGroup>
#include <QDialog>
#include "file_meta.h"

enum RegularTimeType
{
    SECOND,
    MINUTE,
    HOUR,
    DAY,
    WEEK,
    MONTH,
    YEAR
};
struct BackupConfig
{
    QString filename;
    QString backPath;
    RegularTimeType timetype;
    bool isEncrypt;
    QString password;
};

namespace Ui {
class BackupConfigDialog;
}

class BackupConfigDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit BackupConfigDialog(QWidget *parent = nullptr);
    ~BackupConfigDialog();

  signals:
    void backupFile(QSharedPointer<BackupConfig> config);

  private slots:
    void on_browseButton_clicked();

    void on_passwordCheckBox_clicked(bool checked);

    void on_cancelButton_clicked();

    void on_startButton_clicked();

    void on_comboBox_currentIndexChanged(int index);

private:
    Ui::BackupConfigDialog *ui;
    // QButtonGroup *buttonGroup;
};

#endif // BACKUPCONFIGDIALOG_H
