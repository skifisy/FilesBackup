#ifndef MYDIALOG_H
#define MYDIALOG_H

#include <QDialog>

namespace Ui {
class MyDialog;
}

class MyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MyDialog(QWidget *parent = nullptr);
    ~MyDialog();

private slots:
    void on_acceptButton_clicked();

    void on_doneButton_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::MyDialog *ui;
};

#endif // MYDIALOG_H
