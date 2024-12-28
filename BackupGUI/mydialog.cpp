#include "mydialog.h"
#include "ui_mydialog.h"
#include <QDebug>
MyDialog::MyDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MyDialog)
{
    ui->setupUi(this);
    connect(this, &MyDialog::accepted, this, [=]() { qDebug() << "accepted"; });
}

MyDialog::~MyDialog() { delete ui; }

void MyDialog::on_acceptButton_clicked()
{
    this->accept(); // exec()函数返回值为QDialog::Accepted
}

void MyDialog::on_doneButton_clicked()
{
    this->reject(); // exec()函数返回值为QDialog::Rejected
}

void MyDialog::on_pushButton_3_clicked()
{
    // exec()函数返回值为 done() 的参数, 并根据参数发射出对应的信号
    this->done(666);
}
