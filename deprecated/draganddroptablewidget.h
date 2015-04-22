#ifndef DRAGANDDROPTABLEWIDGET_H
#define DRAGANDDROPTABLEWIDGET_H

#include <QTableWidget>

namespace QtGui {

class DragAndDropTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit DragAndDropTableWidget(QWidget *parent = nullptr);
    
    void insertEmptyRowAtEndAutomatically(bool value);
    bool hasEmptyRowAtEnd() const;

protected:
    virtual bool dropMimeData(int row, int column, const QMimeData * data, Qt::DropAction action);
    virtual QStringList mimeTypes() const;
    virtual QMimeData *mimeData(const QList<QTableWidgetItem *> items) const;

private slots:
    void processCellChanged(int, int);

private:
    bool m_insertEmptyRowAtEnd;


};

}

#endif // DRAGANDDROPTABLEWIDGET_H
