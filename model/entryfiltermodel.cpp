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
    if(QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent)) {
        return true;
    }
    QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    if(sourceModel()->hasChildren(sourceIndex)) {
        return true;
    }
    return false;
}

}
