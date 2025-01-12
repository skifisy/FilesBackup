#include <QDebug>
#include <QFileDialog>
#include "message.h"
#include "backupconfigdialog.h"
#include "ui_backupconfigdialog.h"
#include "input_dialog.h"

BackupConfigDialog::BackupConfigDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::BackupConfigDialog)
{
    ui->setupUi(this);
    // buttonGroup = new QButtonGroup(this);
    // buttonGroup->addButton(ui->noneRadioButton, RegularTimeType::none);
    // buttonGroup->addButton(ui->everydayRadioButton,
    // RegularTimeType::every_day); buttonGroup->addButton(
    //     ui->everyweekRadioButton, RegularTimeType::every_week);
    ui->descripText->setVisible(false);
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
    // packfiles
    QSharedPointer<BackupConfig> config(new BackupConfig);
    config->backPath = ui->backupFileDirectoryLineEdit->text();
    config->filename = ui->backupFilenameLineEdit->text() + ".bak";
    config->isEncrypt = ui->passwordCheckBox->isChecked();
    bool ok = true;
    config->unit =
        static_cast<backup::TimeUnit>(ui->timeSelection->currentIndex());
    if (config->unit != backup::TimeUnit::NONE)
        config->interval = ui->timeInterval->text().toInt(&ok);
    if (config->isEncrypt) { config->password = ui->passwordLineEdit->text(); }

    // validate
    if (config->filename.isEmpty()) {
        Message::warning(this, "备份文件名不能为空！");
        return;
    }
    if (config->backPath.isEmpty()) {
        Message::warning(this, "备份路径不能为空！");
        return;
    }
    if (config->isEncrypt && config->password.isEmpty()) {
        Message::warning(this, "文件密码不能为空!");
        return;
    }

    if (!ok) {
        Message::warning(this, "请输入正确的间隔时长");
        return;
    }
    // 获取选中的time check按钮
    // QAbstractButton *selectedButton = buttonGroup->checkedButton();
    // config->timetype =
    //     static_cast<RegularTimeType>(buttonGroup->id(selectedButton));
    emit backupFile(config);
    this->hide();
}

void BackupConfigDialog::on_timeSelection_currentIndexChanged(int index)
{
    ui->timeInterval->setDisabled(index == 0);
    ui->descripText->setVisible(index != 0);
    if (index == 0) { ui->timeInterval->clear(); }
}
