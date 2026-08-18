#include "./totpdialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QUrl>
#include <QUrlQuery>
#include <QVBoxLayout>

namespace QtGui {

TotpDialog::TotpDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Insert TOTP field"));

    auto *const mainLayout = new QVBoxLayout(this);
    auto *const formLayout = new QFormLayout();

    m_secretEdit = new QLineEdit(this);
    m_secretEdit->setClearButtonEnabled(true);

    m_durationEdit = new QLineEdit(this);
    m_durationEdit->setPlaceholderText(QStringLiteral("30"));
    m_durationEdit->setClearButtonEnabled(true);

    m_digitsEdit = new QLineEdit(this);
    m_digitsEdit->setPlaceholderText(QStringLiteral("6"));
    m_digitsEdit->setClearButtonEnabled(true);

    m_issuerEdit = new QLineEdit(this);
    m_issuerEdit->setClearButtonEnabled(true);

    formLayout->addRow(tr("&Secret (mandatory):"), m_secretEdit);
    formLayout->addRow(tr("&Duration (seconds):"), m_durationEdit);
    formLayout->addRow(tr("&Number of digits:"), m_digitsEdit);
    formLayout->addRow(tr("&Issuer (informative):"), m_issuerEdit);

    mainLayout->addLayout(formLayout);

    auto *const buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox);

    auto *const okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setEnabled(false);

    connect(m_secretEdit, &QLineEdit::textChanged, this, [okButton](const QString &text) { okButton->setEnabled(!text.trimmed().isEmpty()); });
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

TotpDialog::~TotpDialog() = default;

QUrl TotpDialog::url() const
{
    const auto secret = m_secretEdit->text().trimmed();
    const auto duration = m_durationEdit->text().trimmed();
    const auto digits = m_digitsEdit->text().trimmed();
    const auto issuer = m_issuerEdit->text().trimmed();
    const auto label = issuer.isEmpty() ? QStringLiteral("TOTP") : issuer;

    auto query = QUrlQuery();
    query.addQueryItem(QStringLiteral("secret"), secret);
    query.addQueryItem(QStringLiteral("period"), duration.isEmpty() ? QStringLiteral("30") : duration);
    query.addQueryItem(QStringLiteral("digits"), digits.isEmpty() ? QStringLiteral("6") : digits);
    if (!issuer.isEmpty()) {
        query.addQueryItem(QStringLiteral("issuer"), issuer);
    }

    auto url = QUrl();
    url.setScheme(QStringLiteral("otpauth"));
    url.setHost(QStringLiteral("totp"));
    url.setPath(QLatin1Char('/') + label);
    url.setQuery(query);
    return url;
}

} // namespace QtGui
