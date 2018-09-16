#ifndef ENTRYFILTERMODEL_H
#define ENTRYFILTERMODEL_H

#include <QSortFilterProxyModel>

namespace QtGui {

class EntryModel;

class EntryFilterModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit EntryFilterModel(QObject *parent = nullptr);
    void setSourceModel(QAbstractItemModel *sourceModel) override;
    Q_INVOKABLE bool isNode(const QModelIndex &parent) const;
    Q_INVOKABLE void setInsertTypeToNode();
    Q_INVOKABLE void setInsertTypeToAccount();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    bool hasAcceptedChildren(const QModelIndex &index) const;

    EntryModel *m_sourceModel;
};

} // namespace QtGui

#endif // ENTRYFILTERMODEL_H
