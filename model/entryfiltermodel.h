#ifndef ENTRYFILTERMODEL_H
#define ENTRYFILTERMODEL_H

#include <QSortFilterProxyModel>

namespace QtGui {

class EntryFilterModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit EntryFilterModel(QObject *parent = nullptr);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    bool hasAcceptedChildren(const QModelIndex &index) const;
};
}

#endif // ENTRYFILTERMODEL_H
