#include "./passwordgeneratordialog.h"

#include "gui/ui_passwordgeneratordialog.h"

#include <passwordfile/io/cryptoexception.h>

#include <qtutilities/misc/dialogutils.h>

#include <c++utilities/conversion/binaryconversion.h>

#include <openssl/rand.h>

#include <QMessageBox>
#include <QClipboard>

#include <string>
#include <sstream>
#include <algorithm>
#include <random>

using namespace std;
using namespace Io;
using namespace Util;
using namespace Dialogs;

namespace QtGui {

const char smallLetters[] = {'a','b','c','d','e','f',
                             'g','h','i','j','k',
                             'l','m','n','o','p',
                             'q','r','s','t','u',
                             'v','w','x','y','z'};

const char capitalLetters[] = {'A','B','C','D','E','F',
                               'G','H','I','J','K',
                               'L','M','N','O','P',
                               'Q','R','S','T','U',
                               'V','W','X','Y','Z'};

const char digits[] = {'0','1','2','3','4',
                       '5','6','7','8','9'};

/*!
 * \class PasswordGeneratorDialog
 * \brief The PasswordGeneratorDialog class provides a password generation dialog.
 */

/*!
 * \brief Constructs a new password generator dialog.
 */
PasswordGeneratorDialog::PasswordGeneratorDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::PasswordGeneratorDialog)
{
    m_ui->setupUi(this);
#ifdef Q_OS_WIN32
    setStyleSheet(QStringLiteral("%1 QCommandLinkButton  { font-size: 12pt; color: %2; font-weight: normal; }").arg(dialogStyle(), instructionTextColor().name()));
#endif
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    connect(m_ui->copyPasswordCommandLinkButton, &QCommandLinkButton::clicked, this, &PasswordGeneratorDialog::copyPassword);
    connect(m_ui->generatePassowordCommandLinkButton, &QCommandLinkButton::clicked, this, &PasswordGeneratorDialog::generateNewPassword);
    connect(m_ui->useCapitalLettersCheckBox, &QCheckBox::stateChanged, this, &PasswordGeneratorDialog::handleCheckedCategoriesChanged);
    connect(m_ui->useCapitalLettersCheckBox, &QCheckBox::stateChanged, this, &PasswordGeneratorDialog::handleCheckedCategoriesChanged);
    connect(m_ui->useSmallLettersCheckBox, &QCheckBox::stateChanged, this, &PasswordGeneratorDialog::handleCheckedCategoriesChanged);
    connect(m_ui->useDigitsCheckBox, &QCheckBox::stateChanged, this, &PasswordGeneratorDialog::handleCheckedCategoriesChanged);
    connect(m_ui->otherCharsLineEdit, &QLineEdit::textChanged, this, &PasswordGeneratorDialog::handleCheckedCategoriesChanged);
    connect(m_ui->passwordLineEdit, &QLineEdit::textChanged, this, &PasswordGeneratorDialog::handlePasswordChanged);
    connect(m_ui->closePushButton, &QPushButton::clicked, this, &PasswordGeneratorDialog::close);

    handlePasswordChanged();
}

/*!
 * \brief Destroys the dialog.
 */
PasswordGeneratorDialog::~PasswordGeneratorDialog()
{
    delete m_ui;
}

/*!
 * \brief Generates and shows a new password.
 */
void PasswordGeneratorDialog::generateNewPassword()
{
    int length = m_ui->LengthSpinBox->value();
    if(length > 0) {
        if(m_charset.empty()) {
            bool useSmallLetters = m_ui->useSmallLettersCheckBox->isChecked();
            bool useCapitalLetters = m_ui->useCapitalLettersCheckBox->isChecked();
            bool useDigits = m_ui->useDigitsCheckBox->isChecked();
            QString otherChars = m_ui->otherCharsLineEdit->text();
            int charsetSize = otherChars.length();
            if(useSmallLetters) {
                charsetSize += sizeof(smallLetters);
            }
            if(useCapitalLetters) {
                charsetSize += sizeof(capitalLetters);
            }
            if(useDigits) {
                charsetSize += sizeof(digits);
            }
            m_charset.reserve(charsetSize);
            if(useSmallLetters) {
                m_charset.insert(m_charset.end(), std::begin(smallLetters), std::end(smallLetters));
            }
            if(useCapitalLetters) {
                m_charset.insert(m_charset.end(), std::begin(capitalLetters), std::end(capitalLetters));
            }
            if(useDigits) {
                m_charset.insert(m_charset.end(), std::begin(digits), std::end(digits));
            }
            char charval;
            foreach(QChar qchar, otherChars) {
                charval = qchar.toLatin1();
                if(charval != '\x00' && charval != ' ' && std::find(m_charset.begin(), m_charset.end(), charval) == m_charset.end()) {
                    m_charset.push_back(charval);
                }
            }
        }
        if(!m_charset.empty()) {
            try {
                default_random_engine rng(m_random());
                uniform_int_distribution<> dist(0, m_charset.size() - 1);
                auto randchar = [this, &dist, &rng]() {
                    return m_charset[dist(rng)];
                };
                string res(length, 0);
                generate_n(res.begin(), length, randchar);
                m_ui->passwordLineEdit->setText(QString::fromLatin1(res.c_str()));
            } catch(const CryptoException &ex) {
                QMessageBox::warning(this, QApplication::applicationName(), tr("Failed to generate password.\nOpenSSL error: %1").arg(QString::fromLocal8Bit(ex.what())));
            }
        } else {
            QMessageBox::warning(this, QApplication::applicationName(), tr("You have to select at least one checkbox."));
        }
    } else {
        QMessageBox::warning(this, QApplication::applicationName(), tr("The length has to be at least one."));
    }
}

/*!
 * \brief Handles when the user checked or unchecked a category.
 */
void PasswordGeneratorDialog::handleCheckedCategoriesChanged()
{
    m_ui->generatePassowordCommandLinkButton->setEnabled(m_ui->useCapitalLettersCheckBox->isChecked()
                                                        || m_ui->useDigitsCheckBox->isChecked()
                                                        || m_ui->useSmallLettersCheckBox->isChecked()
                                                        || !m_ui->otherCharsLineEdit->text().isEmpty());
    m_charset.clear();
}

/*!
 * \brief Handles when the password changed.
 */
void PasswordGeneratorDialog::handlePasswordChanged()
{
    m_ui->copyPasswordCommandLinkButton->setEnabled(m_ui->passwordLineEdit->text().count() > 0);
}

/*!
 * \brief Copies the current password to the clipboard.
 */
void PasswordGeneratorDialog::copyPassword()
{
    QClipboard *cb = QApplication::clipboard();
    cb->setText(m_ui->passwordLineEdit->text());
}

}
