#ifndef ENTRYMODEL_H
#define ENTRYMODEL_H

#if defined(PASSWORD_MANAGER_GUI_QTWIDGETS) || defined(PASSWORD_MANAGER_ENABLE_UNDO_SUPPORT_FOR_QUICK_GUI)
#include "../gui/stacksupport.h"
#endif

#include <c++utilities/application/global.h>

#include <QAbstractItemModel>

namespace Io {
class Entry;
class NodeEntry;
enum class EntryType;
} // namespace Io

namespace QtGui {

/*!
 * \brief The EntryModelRoles enum defines custom roles for the EntryModel class.
 */
enum EntryModelRoles {
    SerializedRole = Qt::UserRole + 1, /**< the entry (including descendants) in serialized from (QByteArray) */
    DefaultExpandedRole, /**< whether the entry should be expanded by default */
};

class EntryModel : public QAbstractItemModel
#ifdef PASSWORD_MANAGER_UNDO_SUPPORT
    ,
                   public StackSupport
#endif
{
    Q_OBJECT
    Q_PROPERTY(Io::NodeEntry *rootEntry READ rootEntry WRITE setRootEntry)
    Q_PROPERTY(Io::EntryType insertType READ insertType WRITE setInsertType)

public:
    explicit EntryModel(QObject *parent = nullptr);
#ifdef PASSWORD_MANAGER_UNDO_SUPPORT
    explicit EntryModel(QUndoStack *undoStack, QObject *parent = nullptr);
#endif

    QHash<int, QByteArray> roleNames() const override;
    Io::NodeEntry *rootEntry();
    void setRootEntry(Io::NodeEntry *entry);
    Q_INVOKABLE Io::Entry *entry(const QModelIndex &index);
    Q_INVOKABLE QList<Io::Entry *> takeEntries(int row, int count, const QModelIndex &parent);
    Q_INVOKABLE bool insertEntries(int row, const QModelIndex &parent, const QList<Io::Entry *> &entries);
    Io::EntryType insertType() const;
    void setInsertType(Io::EntryType type);
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex index(Io::Entry *entry) const;
    QModelIndex parent(const QModelIndex &child) const override;
    bool hasChildren(const QModelIndex &parent) const override;
    Q_INVOKABLE bool isNode(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QMap<int, QVariant> itemData(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    Q_INVOKABLE bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    Q_INVOKABLE bool moveRows(
        const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    Qt::DropActions supportedDropActions() const override;
    Q_INVOKABLE void setInsertTypeToNode();
    Q_INVOKABLE void setInsertTypeToAccount();

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
 * \brief Sets the root entry. Causes a model reset. The undo stack for the Qt Widgets GUI will be cleared if building
 *        with Qt Widgets GUI support.
 */
inline void EntryModel::setRootEntry(Io::NodeEntry *entry)
{
    if (m_rootEntry != entry) {
#ifdef PASSWORD_MANAGER_UNDO_SUPPORT
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

} // namespace QtGui

#endif // ENTRYMODEL_H
