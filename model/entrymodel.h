#ifndef ENTRYMODEL_H
#define ENTRYMODEL_H

#ifdef MODEL_UNDO_SUPPORT
#include "gui/stacksupport.h"
#endif

#include <c++utilities/application/global.h>

#include <QAbstractItemModel>

namespace Io {
class Entry;
class NodeEntry;
DECLARE_ENUM(EntryType, int)
}

namespace QtGui {

/*!
 * \brief The EntryModelRoles enum defines custom roles for the EntryModel class.
 */
enum EntryModelRoles
{
    SerializedRole = Qt::UserRole + 1, /**< the entry (including descendants) in serialized from (QByteArray) */
    DefaultExpandedRole = Qt::UserRole + 2 /**< whether the entry should be expanded by default */
};

class EntryModel : public QAbstractItemModel
#ifdef MODEL_UNDO_SUPPORT
        , public StackSupport
#endif
{
    Q_OBJECT
public:
    explicit EntryModel(QObject *parent = nullptr);
#ifdef MODEL_UNDO_SUPPORT
    explicit EntryModel(QUndoStack *undoStack, QObject *parent = nullptr);
#endif

    Io::NodeEntry *rootEntry();
    void setRootEntry(Io::NodeEntry *entry);
    Io::Entry *entry(const QModelIndex &index);
    QList<Io::Entry *> takeEntries(int row, int count, const QModelIndex &parent);
    bool insertEntries(int row, const QModelIndex &parent, const QList<Io::Entry *> &entries);
    Io::EntryType insertType() const;
    void setInsertType(Io::EntryType type);
    bool hidePasswords() const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex index(Io::Entry *entry) const;
    QModelIndex parent(const QModelIndex &child) const;
    bool hasChildren(const QModelIndex &parent) const;
    bool isNode(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QMap<int, QVariant> itemData(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    bool insertRows(int row, int count, const QModelIndex &parent);
    bool removeRows(int row, int count, const QModelIndex &parent);
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild);
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    Qt::DropActions supportedDropActions() const;

public Q_SLOTS:
    void reset();

private:
    Io::NodeEntry *m_rootEntry;
    Io::EntryType m_insertType;
};

/*!
 * \brief Returns the root entry.
 */
inline Io::NodeEntry *EntryModel::rootEntry()
{
    return m_rootEntry;
}

/*!
 * \brief Sets the root entry. Causes a model reset. The undo stack will be cleard if MODEL_UNDO_SUPPORT is defined.
 */
inline void EntryModel::setRootEntry(Io::NodeEntry *entry)
{
    if(m_rootEntry != entry) {
#ifdef MODEL_UNDO_SUPPORT
        clearUndoStack();
#endif
        beginResetModel();
        m_rootEntry = entry;
        endResetModel();
    }
}

/*!
 * \brief Resets the model. The root entry will be unset.
 */
inline void EntryModel::reset()
{
    setRootEntry(nullptr);
}

/*!
 * \brief Returns the entry type used when inserting new rows.
 */
inline Io::EntryType EntryModel::insertType() const
{
    return m_insertType;
}

/*!
 * \brief Sets the entry type used when inserting new rows.
 */
inline void EntryModel::setInsertType(Io::EntryType type)
{
    m_insertType = type;
}

}

#endif // ENTRYMODEL_H
