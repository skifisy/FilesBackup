#ifndef INPUT_DIALOG_H
#define INPUT_DIALOG_H
#include <utility>
#include <QString>
#include <QMessageBox>
#include <QLineEdit>
#include <QInputDialog>
class InputDialog
{
  public:
    InputDialog(/* args */) = default;
    ~InputDialog() = default;
    /**
     * @return bool 用户是否点击确认；QString 用户填写的内容
     */
    static std::pair<bool, QString> getText(QWidget *parent, const QString &tip)
    {
        bool ok;
        QString text = QInputDialog::getText(
            parent, "提示", tip, QLineEdit::Normal, "", &ok);
        return {ok, text};
    }

    /**
     * @return bool 用户是否点击确认；QString 用户填写的内容
     */
    static std::pair<bool, QString>
    getPassword(QWidget *parent, const QString &tip)
    {
        bool ok;
        QString text = QInputDialog::getText(
            parent, "Password Input", tip, QLineEdit::Password, "", &ok);
        return {ok, text};
    }
};

#endif // INPUT_DIALOG_H