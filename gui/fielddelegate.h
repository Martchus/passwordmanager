#ifndef QTGUI_FIELDDELEGATE_H
#define QTGUI_FIELDDELEGATE_H

#include <QStyledItemDelegate>

namespace QtGui {

class FieldDelegate : public QStyledItemDelegate {
public:
    FieldDelegate(QObject *parent = nullptr);

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
};

} // namespace QtGui

#endif // QTGUI_FIELDDELEGATE_H
