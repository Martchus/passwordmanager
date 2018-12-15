#ifndef PASSWORDGENERATORDIALOG_H
#define PASSWORDGENERATORDIALOG_H

#include <passwordfile/util/opensslrandomdevice.h>

#include <QDialog>

#include <memory>
#include <vector>

namespace QtGui {

namespace Ui {
class PasswordGeneratorDialog;
}

class PasswordGeneratorDialog : public QDialog {
    Q_OBJECT

public:
    explicit PasswordGeneratorDialog(QWidget *parent = nullptr);
    ~PasswordGeneratorDialog() override;

private Q_SLOTS:
    void generateNewPassword();
    void handleCheckedCategoriesChanged();
    void handlePasswordChanged();
#ifndef QT_NO_CLIPBOARD
    void copyPassword();
#endif

private:
    std::unique_ptr<Ui::PasswordGeneratorDialog> m_ui;
    std::vector<char> m_charset;
    Util::OpenSslRandomDevice m_random;
};
} // namespace QtGui

#endif // PASSWORDGENERATORDIALOG_H
