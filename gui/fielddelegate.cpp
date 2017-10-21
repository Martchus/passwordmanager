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
    // fall back to default implementation if editor is not QLineEdit
    auto *const lineEdit = qobject_cast<QLineEdit *>(editor);
    if (!lineEdit) {
        QStyledItemDelegate::setEditorData(editor, index);
        return;
    }

    // set the text
    const QAbstractItemModel *const model = index.model();
    lineEdit->setText(model->data(index, Qt::EditRole).toString());

    // set the echo mode
    if (index.column() > 0) {
        const auto *const fieldModel = qobject_cast<const FieldModel *>(model);
        if (fieldModel && fieldModel->passwordVisibility() == PasswordVisibility::Never) {
            if (const Field *const field = fieldModel->field(static_cast<size_t>(index.row()))) {
                lineEdit->setEchoMode(field->type() != FieldType::Password ? QLineEdit::Normal : QLineEdit::Password);
                return;
            }
        }
    }
    lineEdit->setEchoMode(QLineEdit::Normal);
}

} // namespace QtGui
