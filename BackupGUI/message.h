#ifndef MESSAGE_H
#define MESSAGE_H
#include <QMessageBox>
class Message
{
  public:
    Message() = default;
    ~Message() = default;
    static void info(QWidget *parent, const QString &content)
    {
        QMessageBox::information(
            parent, "提示", content, QMessageBox::Yes, QMessageBox::Yes);
    }

    static void warning(QWidget *parent, const QString &content)
    {
        QMessageBox::warning(
            parent, "警告", content, QMessageBox::Yes, QMessageBox::Yes);
    }
};

#endif // MESSAGE_H