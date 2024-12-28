#include <QDebug>
#include <QFileDialog>

#include "backupconfigdialog.h"
#include "ui_backupconfigdialog.h"

BackupConfigDialog::BackupConfigDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::BackupConfigDialog)
{
    ui->setupUi(this);
    buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(ui->noneRadioButton, RegularTimeType::none);
    buttonGroup->addButton(ui->everydayRadioButton, RegularTimeType::every_day);
    buttonGroup->addButton(
        ui->everyweekRadioButton, RegularTimeType::every_week);
}

BackupConfigDialog::~BackupConfigDialog() { delete ui; }

void BackupConfigDialog::on_browseButton_clicked()
{
    // 打开文件夹选择对话框
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        "选择文件夹",
        "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    // 如果选择了文件夹，更新标签显示路径
    if (!folderPath.isEmpty()) {
        ui->backupFileDirectoryLineEdit->setText(folderPath);
    }
}

void BackupConfigDialog::on_passwordCheckBox_clicked(bool checked)
{
    ui->passwordLineEdit->setEnabled(checked);
    ui->passwordLineEdit->clear();
    if (checked) {
        ui->passwordLineEdit->setPlaceholderText("请输入密码");
    } else {
        ui->passwordLineEdit->setPlaceholderText("");
    }
}

void BackupConfigDialog::on_cancelButton_clicked() { this->hide(); }

void BackupConfigDialog::on_startButton_clicked()
{
    // validate

    // packfiles
    QSharedPointer<BackupConfig> config(new BackupConfig);
    config->backPath = ui->backupFileDirectoryLineEdit->text();
    config->filename = ui->backupFilenameLineEdit->text();
    config->isEncrypt = ui->passwordCheckBox->isChecked();
    if (config->isEncrypt) { config->password = ui->passwordLineEdit->text(); }
    // 获取选中的按钮
    QAbstractButton *selectedButton = buttonGroup->checkedButton();
    config->timetype =
        static_cast<RegularTimeType>(buttonGroup->id(selectedButton));
    emit backupFile(config);
    this->hide();
}
