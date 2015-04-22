#include "draganddroptablewidget.h"

#include <QDropEvent>
#include <QMimeData>
#include <QMessageBox>

namespace QtGui {

DragAndDropTableWidget::DragAndDropTableWidget(QWidget *parent) :
    QTableWidget(parent),
    m_insertEmptyRowAtEnd(false)
{
    connect(this, &DragAndDropTableWidget::cellChanged,
            this, &DragAndDropTableWidget::processCellChanged);
}

void DragAndDropTableWidget::insertEmptyRowAtEndAutomatically(bool value)
{
    m_insertEmptyRowAtEnd = value;
    if(value) {
        processCellChanged(0,0);
    }
}

bool DragAndDropTableWidget::hasEmptyRowAtEnd() const
{
    bool hasEmptyRow = false;
    int rowCount = this->rowCount();
    if(rowCount > 0) {
        if(QTableWidgetItem *item = this->item(rowCount - 1, 0)) {
            hasEmptyRow = item->text().isEmpty();
        } else {
            hasEmptyRow = true;
        }
    }
    return hasEmptyRow;
}

bool DragAndDropTableWidget::dropMimeData(int row, int column, const QMimeData *data, Qt::DropAction action)
{
    if(data->hasText()) {
        if(QTableWidgetItem *item = this->item(row, column))
            item->setText(data->text());
        return false;
    } else {
        return QTableWidget::dropMimeData(row, column, data, action);
    }
}

QStringList DragAndDropTableWidget::mimeTypes() const
{
    return QTableWidget::mimeTypes() << QStringLiteral("text/plain");
}

QMimeData *DragAndDropTableWidget::mimeData(const QList<QTableWidgetItem *> items) const
{
    QString text;
    if(items.count() > 0) {
        QTableWidgetItem *lastItem = items.last();
        foreach(QTableWidgetItem *item, items) {
            if(!item->text().isEmpty()) {
                text.append(item->text());
                if(item != lastItem) {
                    text.append(QStringLiteral("\n"));
                }
            }
        }
    }
    QMimeData *data = QTableWidget::mimeData(items);
    data->setText(text);
    return data;
}

void DragAndDropTableWidget::processCellChanged(int, int)
{
    if(m_insertEmptyRowAtEnd && !hasEmptyRowAtEnd()) {
        insertRow(rowCount());
    }
}

}
