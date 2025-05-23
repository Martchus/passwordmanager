#include "./passwordgeneratordialog.h"

#include "ui_passwordgeneratordialog.h"

#include <passwordfile/io/cryptoexception.h>

#include <qtutilities/misc/dialogutils.h>

#include <c++utilities/conversion/binaryconversion.h>

#include <QClipboard>
#include <QMessageBox>

#include <algorithm>
#include <random>
#include <sstream>
#include <string>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 7, 0))
#define CHECK_STATE_CHANGED_SIGNAL &QCheckBox::checkStateChanged
#else
#define CHECK_STATE_CHANGED_SIGNAL &QCheckBox::stateChanged
#endif

using namespace std;
using namespace Io;
using namespace Util;
using namespace QtUtilities;

namespace QtGui {

const char smallLetters[]
    = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };

const char capitalLetters[]
    = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };

const char digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

/*!
 * \class PasswordGeneratorDialog
 * \brief The PasswordGeneratorDialog class provides a password generation dialog.
 */

/*!
 * \brief Constructs a new password generator dialog.
 */
PasswordGeneratorDialog::PasswordGeneratorDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::PasswordGeneratorDialog)
{
    updateStyleSheet();
    m_ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    connect(m_ui->generatePassowordCommandLinkButton, &QCommandLinkButton::clicked, this, &PasswordGeneratorDialog::generateNewPassword);
    connect(m_ui->useCapitalLettersCheckBox, CHECK_STATE_CHANGED_SIGNAL, this, &PasswordGeneratorDialog::handleCheckedCategoriesChanged);
    connect(m_ui->useCapitalLettersCheckBox, CHECK_STATE_CHANGED_SIGNAL, this, &PasswordGeneratorDialog::handleCheckedCategoriesChanged);
    connect(m_ui->useSmallLettersCheckBox, CHECK_STATE_CHANGED_SIGNAL, this, &PasswordGeneratorDialog::handleCheckedCategoriesChanged);
    connect(m_ui->useDigitsCheckBox, CHECK_STATE_CHANGED_SIGNAL, this, &PasswordGeneratorDialog::handleCheckedCategoriesChanged);
    connect(m_ui->otherCharsLineEdit, &QLineEdit::textChanged, this, &PasswordGeneratorDialog::handleCheckedCategoriesChanged);
    connect(m_ui->passwordLineEdit, &QLineEdit::textChanged, this, &PasswordGeneratorDialog::handlePasswordChanged);
    connect(m_ui->closePushButton, &QPushButton::clicked, this, &PasswordGeneratorDialog::close);

#ifndef QT_NO_CLIPBOARD
    connect(m_ui->copyPasswordCommandLinkButton, &QCommandLinkButton::clicked, this, &PasswordGeneratorDialog::copyPassword);
#else
    m_ui->copyPasswordCommandLinkButton->setDisabled(true);
#endif

    handlePasswordChanged();
}

/*!
 * \brief Destroys the dialog.
 */
PasswordGeneratorDialog::~PasswordGeneratorDialog()
{
}

bool PasswordGeneratorDialog::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::PaletteChange:
        updateStyleSheet();
        break;
    default:;
    }
    return QDialog::event(event);
}

/*!
 * \brief Generates and shows a new password.
 */
void PasswordGeneratorDialog::generateNewPassword()
{
    // check length
    const auto length = m_ui->LengthSpinBox->value();
    if (length <= 0) {
        QMessageBox::warning(this, QApplication::applicationName(), tr("The length has to be at least one."));
        return;
    }

    // make list of characters to be used
    if (m_charset.empty()) {
        const auto useSmallLetters = m_ui->useSmallLettersCheckBox->isChecked();
        const auto useCapitalLetters = m_ui->useCapitalLettersCheckBox->isChecked();
        const auto useDigits = m_ui->useDigitsCheckBox->isChecked();
        const auto otherChars = m_ui->otherCharsLineEdit->text();
        const auto charsetSize = [&] {
            auto size = static_cast<size_t>(otherChars.size());
            if (useSmallLetters) {
                size += sizeof(smallLetters);
            }
            if (useCapitalLetters) {
                size += sizeof(capitalLetters);
            }
            if (useDigits) {
                size += sizeof(digits);
            }
            return size;
        }();

        m_charset.reserve(charsetSize);
        if (useSmallLetters) {
            m_charset.insert(m_charset.end(), std::begin(smallLetters), std::end(smallLetters));
        }
        if (useCapitalLetters) {
            m_charset.insert(m_charset.end(), std::begin(capitalLetters), std::end(capitalLetters));
        }
        if (useDigits) {
            m_charset.insert(m_charset.end(), std::begin(digits), std::end(digits));
        }
        char charval;
        for (const auto &qchar : otherChars) {
            charval = qchar.toLatin1();
            if (charval != '\x00' && charval != ' ' && std::find(m_charset.begin(), m_charset.end(), charval) == m_charset.end()) {
                m_charset.push_back(charval);
            }
        }
    }
    if (m_charset.empty()) {
        QMessageBox::warning(this, QApplication::applicationName(), tr("You have to select at least one checkbox."));
        return;
    }

    // create random string
    try {
        default_random_engine rng(m_random());
        uniform_int_distribution<size_t> dist(0, m_charset.size() - 1);
        const auto getRandomCharacter = [this, &dist, &rng]() { return m_charset[dist(rng)]; };
        string res(static_cast<size_t>(length), 0);
        generate_n(res.begin(), length, getRandomCharacter);
        m_ui->passwordLineEdit->setText(QString::fromLatin1(res.data(), length));
    } catch (const CryptoException &ex) {
        QMessageBox::warning(
            this, QApplication::applicationName(), tr("Failed to generate password.\nOpenSSL error: %1").arg(QString::fromLocal8Bit(ex.what())));
    }
}

/*!
 * \brief Handles when the user checked or unchecked a category.
 */
void PasswordGeneratorDialog::handleCheckedCategoriesChanged()
{
    m_ui->generatePassowordCommandLinkButton->setEnabled(m_ui->useCapitalLettersCheckBox->isChecked() || m_ui->useDigitsCheckBox->isChecked()
        || m_ui->useSmallLettersCheckBox->isChecked() || !m_ui->otherCharsLineEdit->text().isEmpty());
    m_charset.clear();
}

/*!
 * \brief Handles when the password changed.
 */
void PasswordGeneratorDialog::handlePasswordChanged()
{
    m_ui->copyPasswordCommandLinkButton->setEnabled(m_ui->passwordLineEdit->text().size() > 0);
}

/*!
 * \brief Updates the style sheet.
 */
void PasswordGeneratorDialog::updateStyleSheet()
{
#ifdef Q_OS_WINDOWS
    const auto p = palette();
    setStyleSheet(QStringLiteral("%1 QCommandLinkButton  { font-size: 12pt; color: %2; font-weight: normal; }")
            .arg(dialogStyleForPalette(p), instructionTextColorForPalette(p).name()));
#endif
}

#ifndef QT_NO_CLIPBOARD
/*!
 * \brief Copies the current password to the clipboard.
 */
void PasswordGeneratorDialog::copyPassword()
{
    QApplication::clipboard()->setText(m_ui->passwordLineEdit->text());
}
#endif

} // namespace QtGui
