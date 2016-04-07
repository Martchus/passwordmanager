#include "./entryfiltermodel.h"
#include "./entrymodel.h"

namespace QtGui {

/*!
 * \class EntryFilterModel
 * \brief The EntryFilterModel class provides a QSortFilterProxyModel specialization for the EntryModel class.
 *
 * \sa http://qt-project.org/doc/qt-5/qsortfilterproxymodel.html
 */

/*!
 * \brief Constructs a new filter entry model.
 */
EntryFilterModel::EntryFilterModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{}

bool EntryFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    // just use default implementation
    if(QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent)) {
        return true;
    }

    // also accept rows where the direct parent is accepted
    if(sourceParent.isValid() && QSortFilterProxyModel::filterAcceptsRow(sourceParent.row(), sourceParent.parent())) {
        return true;
    }

    // also accept rows which contain accepted childs
    return hasAcceptedChildren(sourceModel()->index(sourceRow, 0, sourceParent));
}

bool EntryFilterModel::hasAcceptedChildren(const QModelIndex &index) const
{
    for(int i = 0, rowCount = sourceModel()->rowCount(index); i < rowCount; ++i) {
        if(filterAcceptsRow(i, index)) {
            return true;
        }
    }
    return false;
}

}
