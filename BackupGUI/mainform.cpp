#include "backupconfigdialog.h"
#include "mainform.h"
#include "ui_mainform.h"
MainForm::MainForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainForm)
{
    ui->setupUi(this);
}

MainForm::~MainForm() { delete ui; }

void MainForm::on_startBackupButton_clicked()
{
    BackupConfigDialog *dialog = new BackupConfigDialog();
    dialog->exec();
}
