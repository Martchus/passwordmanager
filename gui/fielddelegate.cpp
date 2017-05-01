#include "fielddelegate.h"

#include "../model/fieldmodel.h"

#include <passwordfile/io/field.h>

#include <QLineEdit>

using namespace std;
using namespace Io;

namespace QtGui {

FieldDelegate::FieldDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void FieldDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (auto *lineEdit = qobject_cast<QLineEdit *>(editor)) {
        const auto *model = index.model();
        lineEdit->setText(model->data(index, Qt::EditRole).toString());
        if (const auto *fieldModel = qobject_cast<const FieldModel *>(model)) {
            if (fieldModel->passwordVisibility() == PasswordVisibility::Never) {
                lineEdit->setEchoMode(
                    fieldModel->field(static_cast<size_t>(index.row()))->type() != FieldType::Password ? QLineEdit::Normal : QLineEdit::Password);
            } else {
                lineEdit->setEchoMode(QLineEdit::Normal);
            }
        } else {
            lineEdit->setEchoMode(QLineEdit::Normal);
        }
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

} // namespace QtGui
