#include "./fieldmodel.h"
#ifdef MODEL_UNDO_SUPPORT
#include "gui/undocommands.h"
#endif

#include <passwordfile/io/field.h>

#include <QMimeData>
#include <QStringList>

using namespace std;
using namespace Io;

namespace QtGui {

/*!
 * \class FieldModel
 * \brief The FieldModel class provides a model interface for the fields of an AccountEntry.
 *
 * If MODEL_UNDO_SUPPORT the model supports Qt's undo framework.
 * \sa http://qt-project.org/doc/qt-5/qabstracttablemodel.html
 * \sa http://qt-project.org/doc/qt-5/qundo.html
 */

/*!
 * \brief Constructs a new field model.
 */
FieldModel::FieldModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_accountEntry(nullptr),
    m_fields(nullptr),
    m_passwordVisibility(PasswordVisibility::OnlyWhenEditing)
{}

#ifdef MODEL_UNDO_SUPPORT
/*!
 * \brief Constructs a new field model with the specified \a undoStack.
 *
 * This constructor is only available when MODEL_UNDO_SUPPORT is defined.
 */
FieldModel::FieldModel(QUndoStack *undoStack, QObject *parent) :
    QAbstractTableModel(parent),
    StackSupport(undoStack),
    m_accountEntry(nullptr),
    m_fields(nullptr)
{}
#endif

/*!
 * \brief Sets the account entry. Causes a model reset.
 *
 * The \a entry might be nullptr.
 * The caller keeps the ownership and should not destroy the \a entry as long it is assigned.
 */
void FieldModel::setAccountEntry(AccountEntry *entry)
{
    if(entry != m_accountEntry) {
        beginResetModel();
        if((m_accountEntry = entry)) {
            m_fields = &entry->fields();
        } else {
            m_fields = nullptr;
        }
        endResetModel();
    }
}

QVariant FieldModel::data(const QModelIndex &index, int role) const
{
    if(index.isValid() && m_fields && index.row() >= 0) {
        // return data for existent field
        if(static_cast<size_t>(index.row()) < m_fields->size()) {
            switch(role) {
            case Qt::DisplayRole:
            case Qt::EditRole:
                switch(index.column()) {
                case 0:
                    return QString::fromStdString(m_fields->at(index.row()).name());
                case 1: {
                    bool showPassword = m_fields->at(index.row()).type() != FieldType::Password;
                    if(!showPassword) {
                        switch(m_passwordVisibility) {
                        case PasswordVisibility::Always:
                            showPassword = true;
                            break;
                        case PasswordVisibility::OnlyWhenEditing:
                            showPassword = role == Qt::EditRole;
                            break;
                        case PasswordVisibility::Never:
                            showPassword = false;
                        }
                    }
                    return showPassword
                            ? QString::fromStdString(m_fields->at(index.row()).value())
                            : QString(m_fields->at(index.row()).value().size(), QChar(0x2022));
                }
                default:
                    ;
                }
                break;
            case FieldTypeRole:
                return static_cast<int>(m_fields->at(index.row()).type());
            default:
                ;
            }
        // return data for empty field at the end which enables the user to append fields
        } else if(static_cast<size_t>(index.row()) == m_fields->size()) {
            switch(role) {
            case Qt::DisplayRole:
            case Qt::EditRole:
                switch(index.column()) {
                case 0:
                    return QString();
                case 1:
                    return QString();
                default:
                    ;
                }
                break;
            default:
                ;
            }
        }
    }
    return QVariant();
}

QMap<int, QVariant> FieldModel::itemData(const QModelIndex &index) const
{
    static int roles[] = {Qt::EditRole, FieldTypeRole};
    QMap<int, QVariant> roleMap;
    for(int role : roles) {
        QVariant variantData = data(index, role);
        if (variantData.isValid()) {
            roleMap.insert(role, variantData);
        }
    }
    return roleMap;
}

bool FieldModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
#if MODEL_UNDO_SUPPORT
    if(undoStack()) {
        return push(new FieldModelSetValueCommand(this, index, value, role));
    }
#endif
    QVector<int> roles;
    if(index.isValid() && m_fields && index.row() >= 0) {
        // set data for existing field
        if(static_cast<size_t>(index.row()) < m_fields->size()) {
            switch(role) {
            case Qt::EditRole:
                switch(index.column()) {
                case 0:
                    m_fields->at(index.row()).setName(value.toString().toStdString());
                    roles << role;
                    break;
                case 1:
                    m_fields->at(index.row()).setValue(value.toString().toStdString());
                    roles << role;
                    break;
                default:
                    ;
                }
                break;
            case FieldTypeRole: {
                bool ok;
                int fieldType = value.toInt(&ok);
                if(ok && Field::isValidType(fieldType)) {
                    roles << role;
                    m_fields->at(index.row()).setType(static_cast<FieldType>(fieldType));
                }
                break;
            } default:
                ;
            }
            // remove last field if empty, showing an empty field at the end to enabled appending new rows is provided by the data method
            if(!roles.isEmpty() && static_cast<size_t>(index.row()) == m_fields->size() - 1 && m_fields->at(index.row()).isEmpty()) {
                beginRemoveRows(index.parent(), index.row(), index.row());
                m_fields->pop_back();
                endRemoveRows();
            }
        // set data for a new field emplaced at the end of the field list
        } else if(static_cast<size_t>(index.row()) == m_fields->size() && !value.toString().isEmpty()) {
            switch(role) {
            case Qt::DisplayRole:
            case Qt::EditRole:
                switch(index.column()) {
                case 0:
                    beginInsertRows(index.parent(), rowCount(), rowCount());
                    m_fields->emplace_back(m_accountEntry);
                    m_fields->back().setName(value.toString().toStdString());
                    endInsertRows();
                    roles << role;
                    break;
                case 1:
                    beginInsertRows(index.parent(), rowCount(), rowCount());
                    m_fields->emplace_back(m_accountEntry);
                    m_fields->back().setValue(value.toString().toStdString());
                    endInsertRows();
                    roles << role;
                    break;
                default:
                    ;
                }
                break;
            default:
                ;
            }
        }
    }
    // return false if nothing could be changed
    if(roles.isEmpty()) {
        return false;
    } else {
        // some roles affect other roles
        switch(role) {
        case Qt::EditRole:
            roles << Qt::DisplayRole;
            break;
        case FieldTypeRole:
            roles << Qt::DisplayRole << Qt::EditRole;
            break;
        default:
            ;
        }
    }
    // emit data changed signal on sucess
    emit dataChanged(index, index, roles);
    return true;
}

Qt::ItemFlags FieldModel::flags(const QModelIndex &index) const
{
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

QVariant FieldModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch(orientation) {
    case Qt::Horizontal:
        switch(role) {
        case Qt::DisplayRole:
            switch(section) {
            case 0:
                return tr("Name");
            case 1:
                return tr("Value");
            default:
                ;
            }
            break;
        default:
            ;
        }
        break;
    default:
        ;
    }
    return QVariant();
}

int FieldModel::rowCount(const QModelIndex &parent) const
{
    return (!parent.isValid() && m_fields) ? m_fields->size() + 1 : 0;
}

int FieldModel::columnCount(const QModelIndex &parent) const
{
    return !parent.isValid() ? 2 : 0;
}

bool FieldModel::insertRows(int row, int count, const QModelIndex &parent)
{
#ifdef MODEL_UNDO_SUPPORT
    if(undoStack()) {
        return push(new FieldModelInsertRowsCommand(this, row, count));
    }
#endif
    if(!parent.isValid() && row >= 0 && count > 0 && static_cast<size_t>(row) <= m_fields->size()) {
        beginInsertRows(parent, row, row + count - 1);
        m_fields->insert(m_fields->begin() + row, count, Field(m_accountEntry));
        endInsertRows();
        return true;
    }
    return false;
}

bool FieldModel::removeRows(int row, int count, const QModelIndex &parent)
{
#ifdef MODEL_UNDO_SUPPORT
    if(undoStack()) {
        return push(new FieldModelRemoveRowsCommand(this, row, count));
    }
#endif
    if(!parent.isValid() && row >= 0 && count > 0 && static_cast<size_t>(row + count) <= m_fields->size()) {
        beginRemoveRows(parent, row, row + count - 1);
        m_fields->erase(m_fields->begin() + row, m_fields->begin() + row + count);
        endRemoveRows();
        return true;
    }
    return false;
}

bool FieldModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if(!QAbstractTableModel::dropMimeData(data, action, row, column, parent)) {
        if(data->hasText()) {
            return setData(parent, data->text(), Qt::EditRole);
        }
    }
    return false;
}

QStringList FieldModel::mimeTypes() const
{
    return QAbstractTableModel::mimeTypes() << QStringLiteral("text/plain");
}

QMimeData *FieldModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *data = QAbstractTableModel::mimeData(indexes);
    if(!indexes.isEmpty()) {
        QStringList result;
        foreach(const QModelIndex &index, indexes) {
            result << index.data(Qt::EditRole).toString();
        }
        data->setText(result.join(QChar('\n')));
    }
    return data;
}

/*!
 * \brief Returns the field for the specified row.
 *
 * Might be nullptr if no account entry is assigned or if the row is out of range.
 */
const Field *FieldModel::field(size_t row) const
{
    if(m_fields && row < m_fields->size()) {
        return &m_fields->at(row);
    }
    return nullptr;
}

}


