#ifndef PASSWORDGENERATORDIALOG_H
#define PASSWORDGENERATORDIALOG_H

#include <passwordfile/util/opensslrandomdevice.h>

#include <QDialog>

#include <vector>

namespace QtGui {

namespace Ui {
class PasswordGeneratorDialog;
}

class PasswordGeneratorDialog : public QDialog {
    Q_OBJECT

public:
    explicit PasswordGeneratorDialog(QWidget *parent = 0);
    ~PasswordGeneratorDialog();

private Q_SLOTS:
    void generateNewPassword();
    void handleCheckedCategoriesChanged();
    void handlePasswordChanged();
    void copyPassword();

private:
    Ui::PasswordGeneratorDialog *m_ui;
    std::vector<char> m_charset;
    Util::OpenSslRandomDevice m_random;
};
} // namespace QtGui

#endif // PASSWORDGENERATORDIALOG_H
