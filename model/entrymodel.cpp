#include "./entrymodel.h"

#ifdef PASSWORD_MANAGER_UNDO_SUPPORT
#include "../gui/undocommands.h"
#endif

#include <passwordfile/io/entry.h>
#include <passwordfile/io/parsingexception.h>

#include <QBuffer>
#include <QDebug>
#include <QIcon>
#include <QMimeData>

#include <iostream>
#include <memory>
#include <sstream>

using namespace std;
using namespace Io;

namespace QtGui {
/*!
 * \class EntryModel
 * \brief The EntryModel class provides a model interface for a hierarchy of Entry instances.
 *
 * When building the Qt Widgets GUI, the model also supports Qt Widgets' undo framework.
 * \sa http://qt-project.org/doc/qt-5/qabstractitemmodel.html
 * \sa http://qt-project.org/doc/qt-5/qundo.html
 */

/*!
 * \brief Constructs a new entry model.
 */
EntryModel::EntryModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_rootEntry(nullptr)
    , m_insertType(EntryType::Node)
{
}

#ifdef PASSWORD_MANAGER_UNDO_SUPPORT
/*!
 * \brief Constructs a new entry model with the specified \a undoStack.
 *
 * This constructor is only available when PASSWORD_MANAGER_GUI_QTWIDGETS is defined.
 */
EntryModel::EntryModel(QUndoStack *undoStack, QObject *parent)
    : QAbstractItemModel(parent)
    , StackSupport(undoStack)
    , m_rootEntry(nullptr)
    , m_insertType(EntryType::Node)
{
}
#endif

QHash<int, QByteArray> EntryModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { Qt::DisplayRole, "name" },
        { EntryModelRoles::DefaultExpandedRole, "isDefaultExpanded" },
    };
    return roles;
}

/*!
 * \brief Returns the Entry for the specified \a index of nullptr if the \a index is invalid.
 *
 * Modifications should be done using the common methods defined by the QAbstractItemModel class.
 * This method is intended to be used only internally to implement the QAbstractItemModel interface
 * or when implementing a QUndoCommand to support Qt's undo framework.
 */
Entry *EntryModel::entry(const QModelIndex &index)
{
    return index.isValid() ? static_cast<Entry *>(index.internalPointer()) : nullptr;
}

/*!
 * \brief Removes \a count number of entries at the specified \a row within the specified \a parent.
 * \returns Returns the removed entries (rather then deleting them).
 *
 * Modifications should be done using the common methods defined by the QAbstractItemModel class.
 * This method is intended to be used only internally to implement the QAbstractItemModel interface
 * or when implementing a QUndoCommand to support Qt's undo framework.
 */
QList<Entry *> EntryModel::takeEntries(int row, int count, const QModelIndex &parent)
{
    Entry *const parentEntry = entry(parent);
    if (!parentEntry || parentEntry->type() != EntryType::Node) {
        return QList<Entry *>();
    }
    QList<Entry *> res;
    NodeEntry *const parentNodeEntry = static_cast<NodeEntry *>(parentEntry);
    int lastIndex = row + count - 1;
    const vector<Entry *> &children = parentNodeEntry->children();
    if (lastIndex < 0 || static_cast<size_t>(lastIndex) >= children.size()) {
        lastIndex = static_cast<int>(children.size() - 1);
    }
    beginRemoveRows(parent, row, lastIndex);
    for (int index = lastIndex; index >= row; --index) {
        Entry *const child = children[static_cast<size_t>(index)];
        child->setParent(nullptr);
        res << child;
    }
    endRemoveRows();
    return res;
}

/*!
 * \brief Inserts the specified \a entries before the specified \a row within the specified \a parent.
 *
 * Modifications should be done using the common methods defined by the QAbstractItemModel class.
 * This method is intended to be used only internally to implement the QAbstractItemModel interface
 * or when implementing a QUndoCommand to support Qt's undo framework.
 */
bool EntryModel::insertEntries(int row, const QModelIndex &parent, const QList<Entry *> &entries)
{
    if (entries.isEmpty()) {
        return true;
    }
    Entry *const parentEntry = entry(parent);
    if (!parentEntry || parentEntry->type() != EntryType::Node) {
        return false;
    }
    NodeEntry *const parentNodeEntry = static_cast<NodeEntry *>(parentEntry);
    const vector<Entry *> &children = parentNodeEntry->children();
    if (row < 0 || static_cast<size_t>(row) > children.size()) {
        row = static_cast<int>(children.size());
    }
    beginInsertRows(parent, row, row + static_cast<int>(entries.size()) - 1);
    for (Entry *const entry : entries) {
        entry->setParent(parentNodeEntry, row);
        ++row;
    }
    endInsertRows();
    return true;
}

QModelIndex EntryModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        if (m_rootEntry && row == 0) {
            return createIndex(row, column, m_rootEntry);
        }
        return QModelIndex();
    }
    const auto *const parentEntry = static_cast<const Entry *>(parent.internalPointer());
    if (!parentEntry) {
        return QModelIndex();
    }
    switch (parentEntry->type()) {
    case EntryType::Node: {
        const std::vector<Entry *> &children = static_cast<const NodeEntry *>(parentEntry)->children();
        if (row >= 0 && static_cast<size_t>(row) < children.size()) {
            return createIndex(row, column, children[static_cast<size_t>(row)]);
        }
        break;
    }
    case EntryType::Account:;
    }
    return QModelIndex();
}

/*!
 * \brief Returns the index of the specified \a entry.
 * \remarks It is up to the caller to ensure that \a entry is a child of the root element.
 */
QModelIndex EntryModel::index(Entry *entry) const
{
    if (entry->parent()) {
        return createIndex(entry->index(), 0, entry);
    } else {
        return createIndex(0, 0, m_rootEntry);
    }
}

QModelIndex EntryModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }
    const auto *const entry = static_cast<Entry *>(child.internalPointer());
    if (!entry) {
        return QModelIndex();
    }
    NodeEntry *const parent = entry->parent();
    if (parent && (child.row() >= 0 && static_cast<size_t>(child.row()) < parent->children().size())) {
        return createIndex(parent->index() > 0 ? parent->index() : 0, 0, parent);
    }
    return QModelIndex();
}

bool EntryModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return true;
    }
    const auto *const entry = static_cast<Entry *>(parent.internalPointer());
    return entry && entry->type() == EntryType::Node && !static_cast<const NodeEntry *>(entry)->children().empty();
}

/*!
 * \brief Returns an indication whether the specified \a parent might have children.
 * \remarks Only node entries might have childs.
 */
bool EntryModel::isNode(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return false;
    }
    const auto *const entry = static_cast<const Entry *>(parent.internalPointer());
    return entry && entry->type() == EntryType::Node;
}

QVariant EntryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    const auto *const entry = static_cast<const Entry *>(index.internalPointer());
    if (!entry) {
        return QVariant();
    }
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch (index.column()) {
        case 0:
            return QString::fromStdString(entry->label());
        default:;
        }
        break;
    case Qt::DecorationRole:
        if (index.column() == 0 && entry->type() == EntryType::Node) {
            static const QVariant folderIcon = QIcon::fromTheme(QStringLiteral("folder"));
            return folderIcon;
        }
        break;
    case SerializedRole: {
        stringstream ss(stringstream::in | stringstream::out | stringstream::binary);
        ss.exceptions(std::stringstream::failbit | std::stringstream::badbit);
        try {
            entry->make(ss);
            // FIXME: make conversion to QByteArray more efficient
            const auto str(ss.str());
            return QByteArray(str.data(), static_cast<QByteArray::size_type>(str.size()));
        } catch (const std::ios_base::failure &) {
            return false;
        }
    }
    case DefaultExpandedRole:
        return entry->type() == EntryType::Node && static_cast<const NodeEntry *>(entry)->isExpandedByDefault();
    default:;
    }
    return QVariant();
}

QMap<int, QVariant> EntryModel::itemData(const QModelIndex &index) const
{
    return QMap<int, QVariant>{
        { Qt::DisplayRole, data(index, Qt::DisplayRole) },
        { SerializedRole, data(index, SerializedRole) },
    };
}

bool EntryModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
#ifdef PASSWORD_MANAGER_UNDO_SUPPORT
    if (undoStack()) {
        return push(make_unique<EntryModelSetValueCommand>(this, index, value, role));
    }
#endif
    if (!index.isValid()) {
        return false;
    }
    auto *const entry = static_cast<Entry *>(index.internalPointer());
    if (!entry) {
        return false;
    }
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch (index.column()) {
        case 0:
            entry->setLabel(value.toString().toStdString());
            emit dataChanged(index, index, QVector<int>({ Qt::DisplayRole, Qt::EditRole }));
            return true;
        default:;
        }
        break;
    case SerializedRole: {
        NodeEntry *const parent = entry->parent();
        const QModelIndex parentIndex = index.parent();
        if (!parent || !parentIndex.isValid()) {
            return false;
        }
        QByteArray array(value.toByteArray());
        if (array.isEmpty()) {
            return false;
        }
        try {
            stringstream ss(stringstream::in | stringstream::out | stringstream::binary);
            ss.exceptions(std::stringstream::failbit | std::stringstream::badbit);
#if defined(__GLIBCXX__) && !defined(_LIBCPP_VERSION)
            ss.rdbuf()->pubsetbuf(array.data(), array.size());
#else
            ss.write(array.data(), array.size());
#endif
            Entry *const newEntry = Entry::parse(ss);
            const int row = entry->index();
            beginRemoveRows(parentIndex, row, row);
            delete entry;
            endRemoveRows();
            beginInsertRows(parentIndex, row, row);
            newEntry->setParent(parent, row);
            endInsertRows();
            return true;
        } catch (const Io::ParsingException &parsingError) {
            cerr << "EntryModel::setData: parsing exception: " << parsingError.what() << endl;
            return false;
        } catch (const std::ios_base::failure &failure) {
            cerr << "EntryModel::setData: IO exception: " << failure.what() << endl;
            return false;
        }
    }
    case DefaultExpandedRole:
        switch (entry->type()) {
        case EntryType::Account:
            return false;
        case EntryType::Node:
            static_cast<NodeEntry *>(entry)->setExpandedByDefault(value.toBool());
            emit dataChanged(index, index, QVector<int>() << role);
            return true;
        }
        break;
    default:;
    }
    return false;
}

bool EntryModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    for (QMap<int, QVariant>::ConstIterator it = roles.constBegin(); it != roles.constEnd(); ++it) {
        setData(index, it.value(), it.key());
    }
    return true;
}

Qt::ItemFlags EntryModel::flags(const QModelIndex &index) const
{
    return isNode(index) ? QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled
                         : QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
}

QVariant EntryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (orientation) {
    case Qt::Horizontal:
        switch (role) {
        case Qt::DisplayRole:
            switch (section) {
            case 0:
                return tr("Name");
            default:;
            }
            break;
        default:;
        }
        break;
    default:;
    }
    return QVariant();
}

int EntryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        if (Entry *parentEntry = static_cast<Entry *>(parent.internalPointer())) {
            switch (parentEntry->type()) {
            case EntryType::Node:
                return static_cast<int>(static_cast<NodeEntry *>(parentEntry)->children().size());
            case EntryType::Account:;
            }
        }
    } else if (m_rootEntry) {
        return 1;
    }
    return 0;
}

int EntryModel::columnCount(const QModelIndex &) const
{
    return 1;
}

bool EntryModel::insertRows(int row, int count, const QModelIndex &parent)
{
#ifdef PASSWORD_MANAGER_UNDO_SUPPORT
    if (undoStack()) {
        return push(make_unique<EntryModelInsertRowsCommand>(this, row, count, parent));
    }
#endif
    if (!parent.isValid()) {
        return false;
    }
    auto *const parentEntry = static_cast<Entry *>(parent.internalPointer());
    if (!parentEntry || parentEntry->type() != EntryType::Node) {
        return false;
    }
    beginInsertRows(parent, row, row + count - 1);
    for (int end = row + count; row < end; ++row) {
        Entry *newEntry;
        switch (m_insertType) {
        case EntryType::Node:
            newEntry = new NodeEntry;
            break;
        case EntryType::Account:
            newEntry = new AccountEntry;
            break;
        default:
            return false; // should never be reached, just to suppress compiler warning
        }
        newEntry->setParent(static_cast<NodeEntry *>(parentEntry), row);
    }
    endInsertRows();
    return true;
}

bool EntryModel::removeRows(int row, int count, const QModelIndex &parent)
{
#ifdef PASSWORD_MANAGER_UNDO_SUPPORT
    if (undoStack()) {
        return push(make_unique<EntryModelRemoveRowsCommand>(this, row, count, parent));
    }
#endif
    if (!parent.isValid() || count <= 0) {
        return false;
    }
    auto *const parentEntry = static_cast<Entry *>(parent.internalPointer());
    if (!parentEntry || parentEntry->type() != EntryType::Node) {
        return false;
    }
    beginRemoveRows(parent, row, row + count - 1);
    static_cast<NodeEntry *>(parentEntry)->deleteChildren(row, row + count);
    endRemoveRows();
    return true;
}

bool EntryModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
#ifdef PASSWORD_MANAGER_UNDO_SUPPORT
    if (undoStack()) {
        return push(make_unique<EntryModelMoveRowsCommand>(this, sourceParent, sourceRow, count, destinationParent, destinationChild));
    }
#endif
#if CPP_UTILITIES_DEBUG_BUILD
    std::cout << "sourceRow: " << sourceRow << endl;
    std::cout << "destinationChild: " << destinationChild << endl;
#endif
    // check validation of specified arguments: source and destination parent entries need to be node entries
    if (sourceRow < 0 || count <= 0) {
        return false;
    }
    const auto *const srcParentEntry = entry(sourceParent);
    const auto *const destParentEntry = entry(destinationParent);
    if (!srcParentEntry || !destParentEntry || srcParentEntry->type() != EntryType::Node || destParentEntry->type() != EntryType::Node) {
        return false;
    }
    // determine the source parent entry and dest parent entry as node entries
    auto *const srcParentNodeEntry = static_cast<NodeEntry *>(sourceParent.internalPointer());
    auto *const destParentNodeEntry = static_cast<NodeEntry *>(destinationParent.internalPointer());
    // source rows must be within the valid range
    if (static_cast<std::size_t>(sourceRow + count) > srcParentNodeEntry->children().size()) {
        return false;
    }
    // if source and destination parent are the same the destination child mustn't be in the source range
    if (srcParentNodeEntry == destParentNodeEntry) {
        if (destinationChild == sourceRow) {
            return true;
        }
        if (!(destinationChild < sourceRow || (sourceRow + count) < destinationChild)) {
            return false;
        }
    }
    // do not move a row to one of its own children! -> check before
    for (int index = 0; index < count; ++index) {
        Entry *const toMove = srcParentNodeEntry->children()[static_cast<std::size_t>(sourceRow + index)];
        if (toMove->type() != EntryType::Node) {
            continue;
        }
        if (toMove == destParentNodeEntry || destParentNodeEntry->isIndirectChildOf(static_cast<NodeEntry *>(toMove))) {
            return false;
        }
    }
    // actually perform the move operation
    if (!beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild)) {
        return false;
    }
    for (int index = 0; index < count; ++index) {
        Entry *toMove = srcParentNodeEntry->children()[static_cast<std::size_t>(sourceRow + index)];
        if (srcParentNodeEntry == destParentNodeEntry && sourceRow < destinationChild) {
            toMove->setParent(destParentNodeEntry, destinationChild + index - 1);
        } else {
            toMove->setParent(destParentNodeEntry, destinationChild + index);
        }
    }
    endMoveRows();
    return true;
}

QStringList EntryModel::mimeTypes() const
{
    return QStringList({ QStringLiteral("application/x-entrymodelpathlistmove"), QStringLiteral("text/plain") });
}

QMimeData *EntryModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.count() <= 0) {
        return nullptr;
    }
    const QStringList types = mimeTypes();
    if (types.isEmpty()) {
        return nullptr;
    }
    auto *const data = new QMimeData();
    QStringList plainTextParts;
    plainTextParts.reserve(indexes.size());
    QByteArray encoded;
    QDataStream dataStream(&encoded, QIODevice::WriteOnly);
    for (const QModelIndex &index : indexes) {
        if (!index.isValid()) {
            continue;
        }
        const auto *const entry = static_cast<const Entry *>(index.internalPointer());
        const auto path(entry->path());
        dataStream << static_cast<quint32>(path.size());
        for (const string &part : path) {
            dataStream << QString::fromStdString(part);
        }
        plainTextParts << QString::fromStdString(entry->label());
    }
    data->setData(types.at(0), encoded);
    data->setText(plainTextParts.join(QStringLiteral(", ")));
    return data;
}

bool EntryModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (!m_rootEntry || !data || action != Qt::MoveAction) {
        return false;
    }
    const QStringList types = mimeTypes();
    if (types.isEmpty()) {
        return false;
    }
    const QString format = types.at(0);
    if (!data->hasFormat(format)) {
        return false;
    }
    if (row > rowCount(parent) || row < 0) {
        row = rowCount(parent);
    }
    if (column > columnCount(parent) || column < 0) {
        column = 0;
    }
    // decode and insert
    QByteArray encoded(data->data(format));
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    int moved = 0;
    while (!stream.atEnd()) {
        quint32 size;
        stream >> size;
        list<string> path;
        for (quint32 i = 0; i < size; ++i) {
            QString part;
            stream >> part;
            path.push_back(part.toStdString());
        }
        auto *const entry = m_rootEntry->entryByPath(path, true);
        if (!entry) {
            continue;
        }
        auto *const srcParentEntry = entry->parent();
        if (srcParentEntry && moveRows(index(srcParentEntry), entry->index(), 1, parent, row)) {
            ++moved;
        }
    }
    return false;
}

Qt::DropActions EntryModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

/*!
 * \brief Sets the insert type to node.
 * \remarks Intended to prevent the offort of exposing EntryType to QML.
 */
void EntryModel::setInsertTypeToNode()
{
    setInsertType(Io::EntryType::Node);
}

/*!
 * \brief Sets the insert type to account.
 * \remarks Intended to prevent the offort of exposing EntryType to QML.
 */
void EntryModel::setInsertTypeToAccount()
{
    setInsertType(Io::EntryType::Account);
}

} // namespace QtGui
