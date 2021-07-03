#include "./fieldmodel.h"

#ifdef PASSWORD_MANAGER_UNDO_SUPPORT
#include "../gui/undocommands.h"
#endif

#include <passwordfile/io/field.h>

#include <QMimeData>
#include <QStringList>

#include <memory>

using namespace std;
using namespace Io;

namespace QtGui {

/*!
 * \class FieldModel
 * \brief The FieldModel class provides a model interface for the fields of an AccountEntry.
 *
 * When building the Qt Widgets GUI, the model also supports Qt Widgets' undo framework.
 * \sa http://qt-project.org/doc/qt-5/qabstracttablemodel.html
 * \sa http://qt-project.org/doc/qt-5/qundo.html
 */

/*!
 * \brief Constructs a new field model.
 */
FieldModel::FieldModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_accountEntry(nullptr)
    , m_fields(nullptr)
    , m_passwordVisibility(PasswordVisibility::OnlyWhenEditing)
{
}

#ifdef PASSWORD_MANAGER_UNDO_SUPPORT
/*!
 * \brief Constructs a new field model with the specified \a undoStack.
 *
 * This constructor is only available when PASSWORD_MANAGER_GUI_QTWIDGETS is defined.
 */
FieldModel::FieldModel(QUndoStack *undoStack, QObject *parent)
    : QAbstractTableModel(parent)
    , StackSupport(undoStack)
    , m_accountEntry(nullptr)
    , m_fields(nullptr)
{
}
#endif

QHash<int, QByteArray> FieldModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { FieldModelRoles::FieldTypeRole, "fieldType" },
        { FieldModelRoles::Key, "key" },
        { FieldModelRoles::Value, "value" },
        { FieldModelRoles::IsPassword, "isPassword" },
        { FieldModelRoles::AlwaysActualValue, "actualValue" },
        { FieldModelRoles::IsLastRow, "isLastRow" },
    };
    return roles;
}

/*!
 * \brief Sets the account entry. Causes a model reset.
 *
 * The \a entry might be nullptr.
 * The caller keeps the ownership and should not destroy the \a entry as long it is assigned.
 */
void FieldModel::setAccountEntry(AccountEntry *entry)
{
    if (entry == m_accountEntry) {
        return;
    }
    beginResetModel();
    if ((m_accountEntry = entry)) {
        m_fields = &entry->fields();
    } else {
        m_fields = nullptr;
    }
    endResetModel();
}

inline QVariant FieldModel::passwordValue(const QModelIndex &index, int role) const
{
    return (m_passwordVisibility == PasswordVisibility::Always || role == Qt::EditRole
               || (*m_fields)[static_cast<size_t>(index.row())].type() != FieldType::Password)
        ? QString::fromStdString((*m_fields)[static_cast<size_t>(index.row())].value())
        : QString(static_cast<QString::size_type>((*m_fields)[static_cast<size_t>(index.row())].value().size()), QChar(0x2022));
}

QVariant FieldModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_fields || index.row() < 0) {
        return QVariant();
    }
    // return data for existent field
    if (static_cast<size_t>(index.row()) < m_fields->size()) {
        switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            switch (index.column()) {
            case 0:
                return QString::fromStdString((*m_fields)[static_cast<size_t>(index.row())].name());
            case 1:
                return passwordValue(index, role);
            default:;
            }
            break;
        case FieldTypeRole:
            return static_cast<int>((*m_fields)[static_cast<size_t>(index.row())].type());
        case Key:
            return QString::fromStdString((*m_fields)[static_cast<size_t>(index.row())].name());
        case Value:
            return passwordValue(index, role);
        case AlwaysActualValue:
            return QString::fromStdString((*m_fields)[static_cast<size_t>(index.row())].value());
        case IsPassword:
            return (*m_fields)[static_cast<size_t>(index.row())].type() == FieldType::Password;
        case IsLastRow:
            return false;
        default:;
        }

    } else if (static_cast<size_t>(index.row()) == m_fields->size()) {
        // return data for empty field at the end which enables the user to append fields
        switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            switch (index.column()) {
            case 0:
                return QString();
            case 1:
                return QString();
            default:;
            }
            break;
        case Key:
        case Value:
        case AlwaysActualValue:
            return QString();
        case IsPassword:
            return false;
        case IsLastRow:
            return true;
        default:;
        }
    }
    return QVariant();
}

QMap<int, QVariant> FieldModel::itemData(const QModelIndex &index) const
{
    static const auto roleMap = [this, index] {
        QMap<int, QVariant> roleMap;
        for (const auto role : initializer_list<int>{ Qt::EditRole, FieldTypeRole }) {
            const auto variantData(data(index, role));
            if (variantData.isValid()) {
                roleMap.insert(role, variantData);
            }
        }
        return roleMap;
    }();
    return roleMap;
}

bool FieldModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
#ifdef PASSWORD_MANAGER_UNDO_SUPPORT
    if (undoStack()) {
        return push(make_unique<FieldModelSetValueCommand>(this, index, value, role));
    }
#endif
    if (!index.isValid() || !m_fields || index.row() < 0) {
        return false;
    }
    QVector<int> roles;
    if (static_cast<size_t>(index.row()) < m_fields->size()) {
        // set data for existing field
        switch (role) {
        case Qt::EditRole:
            switch (index.column()) {
            case 0:
                m_fields->at(static_cast<size_t>(index.row())).setName(value.toString().toStdString());
                roles << Qt::DisplayRole << Qt::EditRole << Key;
                break;
            case 1:
                m_fields->at(static_cast<size_t>(index.row())).setValue(value.toString().toStdString());
                roles << Qt::DisplayRole << Qt::EditRole << Value << AlwaysActualValue;
                break;
            default:;
            }
            break;
        case FieldTypeRole: {
            bool ok;
            int fieldType = value.toInt(&ok);
            if (ok && Field::isValidType(fieldType)) {
                m_fields->at(static_cast<size_t>(index.row())).setType(static_cast<FieldType>(fieldType));
                roles << Qt::DisplayRole << Value << FieldTypeRole << IsPassword;
            }
            break;
        }
        case Key:
            m_fields->at(static_cast<size_t>(index.row())).setName(value.toString().toStdString());
            roles << Qt::DisplayRole << Qt::EditRole << Key;
            break;
        case Value:
        case AlwaysActualValue:
            m_fields->at(static_cast<size_t>(index.row())).setValue(value.toString().toStdString());
            roles << Qt::DisplayRole << Qt::EditRole << Value << AlwaysActualValue;
            break;
        case IsPassword:
            m_fields->at(static_cast<size_t>(index.row())).setType(value.toBool() ? FieldType::Password : FieldType::Normal);
            roles << Qt::DisplayRole << Value << FieldTypeRole << IsPassword;
            break;
        default:;
        }
        // remove last field if empty, showing an empty field at the end to enabled appending new rows is provided by the data method
        if (!roles.isEmpty() && static_cast<size_t>(index.row()) == m_fields->size() - 1
            && m_fields->at(static_cast<size_t>(index.row())).isEmpty()) {
            beginRemoveRows(index.parent(), index.row(), index.row());
            m_fields->pop_back();
            endRemoveRows();
        }
    } else if (static_cast<size_t>(index.row()) == m_fields->size() && !value.toString().isEmpty()) {
        // set data for a new field emplaced at the end of the field list
        switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            switch (index.column()) {
            case 0:
                beginInsertRows(index.parent(), rowCount(), rowCount());
                m_fields->emplace_back(m_accountEntry);
                m_fields->back().setName(value.toString().toStdString());
                endInsertRows();
                roles << Qt::DisplayRole << Qt::EditRole << IsLastRow;
                break;
            case 1:
                beginInsertRows(index.parent(), rowCount(), rowCount());
                m_fields->emplace_back(m_accountEntry);
                m_fields->back().setValue(value.toString().toStdString());
                endInsertRows();
                roles << Qt::DisplayRole << Qt::EditRole << IsLastRow;
                break;
            default:;
            }
            break;
        default:;
        }
    }
    // return false if nothing could be changed
    if (roles.isEmpty()) {
        return false;
    }
    // emit data changed signal on success
    emit dataChanged(index, index, roles);
    return true;
}

Qt::ItemFlags FieldModel::flags(const QModelIndex &index) const
{
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

QVariant FieldModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (orientation) {
    case Qt::Horizontal:
        switch (role) {
        case Qt::DisplayRole:
            switch (section) {
            case 0:
                return tr("Name");
            case 1:
                return tr("Value");
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

int FieldModel::rowCount(const QModelIndex &parent) const
{
    return (!parent.isValid() && m_fields) ? static_cast<int>(m_fields->size()) + 1 : 0;
}

int FieldModel::columnCount(const QModelIndex &parent) const
{
    return !parent.isValid() ? 2 : 0;
}

bool FieldModel::insertRows(int row, int count, const QModelIndex &parent)
{
#ifdef PASSWORD_MANAGER_UNDO_SUPPORT
    if (undoStack()) {
        return push(make_unique<FieldModelInsertRowsCommand>(this, row, count));
    }
#endif
    if (parent.isValid() || row < 0 || count <= 0 || static_cast<size_t>(row) > m_fields->size()) {
        return false;
    }
    beginInsertRows(parent, row, row + count - 1);
    m_fields->insert(m_fields->begin() + row, static_cast<size_t>(count), Field(m_accountEntry));
    endInsertRows();
    return true;
}

bool FieldModel::removeRows(int row, int count, const QModelIndex &parent)
{
#ifdef PASSWORD_MANAGER_UNDO_SUPPORT
    if (undoStack()) {
        return push(make_unique<FieldModelRemoveRowsCommand>(this, row, count));
    }
#endif
    if (parent.isValid() || row < 0 || count <= 0 || static_cast<size_t>(row + count) > m_fields->size()) {
        return false;
    }
    beginRemoveRows(parent, row, row + count - 1);
    m_fields->erase(m_fields->begin() + row, m_fields->begin() + row + count);
    endRemoveRows();
    return true;
}

bool FieldModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    // FIXME: implement an undo/redo command

    // validate input parameter
    if (sourceParent.isValid() || destinationParent.isValid() || sourceRow < 0 || count <= 0 || destinationChild < 0
        || static_cast<size_t>(sourceRow + count) > m_fields->size() || static_cast<size_t>(destinationChild) >= m_fields->size()
        || (destinationChild >= sourceRow && destinationChild < (sourceRow + count))) {
        return false;
    }

    // begin the move
    if (destinationChild > sourceRow) {
        // move rows down: the third param is still counted in the initial array!
        beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild + count);
    } else {
        // move rows up
        beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild);
    }

    // reserve space for temporary copies (FIXME: possible to avoid this?)
    m_fields->reserve(m_fields->size() + static_cast<size_t>(count));
    vector<Io::Field> tmp(static_cast<size_t>(count));
    // move rows to temporary array
    move(m_fields->begin() + sourceRow, m_fields->begin() + sourceRow + count, tmp.begin());
    // erase slots of rows to be moved
    m_fields->erase(m_fields->begin() + sourceRow, m_fields->begin() + sourceRow + count);
    // insert rows again at their new position
    m_fields->insert(m_fields->begin() + destinationChild, tmp.begin(), tmp.end());

    endMoveRows();
    return true;
}

bool FieldModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (!QAbstractTableModel::dropMimeData(data, action, row, column, parent) && data->hasText()) {
        return setData(parent, data->text(), Qt::EditRole);
    }
    return false;
}

QStringList FieldModel::mimeTypes() const
{
    return QAbstractTableModel::mimeTypes() << QStringLiteral("text/plain");
}

QMimeData *FieldModel::mimeData(const QModelIndexList &indices) const
{
    QMimeData *const data = QAbstractTableModel::mimeData(indices);
    if (indices.isEmpty()) {
        return data;
    }
    QStringList result;
    result.reserve(indices.size());
    for (const QModelIndex &index : indices) {
        result << index.data(Qt::EditRole).toString();
    }
    data->setText(result.join(QChar('\n')));
    return data;
}

/*!
 * \brief Returns the field for the specified row.
 *
 * Might be nullptr if no account entry is assigned or if the row is out of range.
 */
const Field *FieldModel::field(size_t row) const
{
    if (m_fields && row < m_fields->size()) {
        return &(*m_fields)[row];
    }
    return nullptr;
}

} // namespace QtGui
