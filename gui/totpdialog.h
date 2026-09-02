#ifndef TOTPDIALOG_H
#define TOTPDIALOG_H

#include <QDialog>
#include <QUrl>

QT_FORWARD_DECLARE_CLASS(QLineEdit)

namespace QtGui {

class TotpDialog : public QDialog {
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url)

public:
    explicit TotpDialog(const QString &issuer = QString(), QWidget *parent = nullptr);
    ~TotpDialog() override;

    QUrl url() const;

private:
    QLineEdit *m_secretEdit;
    QLineEdit *m_durationEdit;
    QLineEdit *m_digitsEdit;
    QLineEdit *m_issuerEdit;
};

} // namespace QtGui

#endif // TOTPDIALOG_H
