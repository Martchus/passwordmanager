#ifndef FIELDMODEL_H
#define FIELDMODEL_H

#include <passwordfile/io/entry.h>
#ifdef MODEL_UNDO_SUPPORT
#include "gui/stacksupport.h"
#endif

#include <QAbstractTableModel>

#include <vector>

namespace Io {
class Field;
}

namespace QtGui {

/*!
 * \brief The FieldModelRoles enum defines custom roles for the FieldModel class.
 */
enum FieldModelRoles
{
    FieldTypeRole = Qt::UserRole + 1 /**< the field type */
};

/*!
 * \brief The PasswordVisibility enum defines when passwords will be visible.
 */
enum PasswordVisibility
{
    Always, /**< passwords are always visible */
    OnlyWhenEditing, /**< passwords are only visible when editing */
    Never /**< passwords are never visible */
};

class FieldModel : public QAbstractTableModel
#ifdef MODEL_UNDO_SUPPORT
        , public StackSupport
#endif
{
    Q_OBJECT
public:    
    explicit FieldModel(QObject *parent = nullptr);
#ifdef MODEL_UNDO_SUPPORT
    explicit FieldModel(QUndoStack *undoStack, QObject *parent = nullptr);
#endif

    Io::AccountEntry *accountEntry();
    const Io::AccountEntry *accountEntry() const;
    void setAccountEntry(Io::AccountEntry *entry);
    std::vector<Io::Field> *fields();
    PasswordVisibility passwordVisibility() const;
    QVariant data(const QModelIndex &index, int role) const;
    QMap<int, QVariant> itemData(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool insertRows(int row, int count, const QModelIndex &parent);
    bool removeRows(int row, int count, const QModelIndex &parent);
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    const Io::Field *field(std::size_t row) const;

public Q_SLOTS:
    void setPasswordVisibility(PasswordVisibility passwordVisibility);
    void reset();

private:
    Io::AccountEntry *m_accountEntry;
    std::vector<Io::Field> *m_fields;
    PasswordVisibility m_passwordVisibility;
};

/*!
 * \brief Returns the account entry. Might be nullptr if no account entry has been set.
 *
 * The ownership remains unaltered.
 */
inline Io::AccountEntry *FieldModel::accountEntry()
{
    return m_accountEntry;
}

/*!
 * \brief Returns the account entry. Might be nullptr if no account entry has been set.
 *
 * The ownership remains unaltered.
 */
inline const Io::AccountEntry *FieldModel::accountEntry() const
{
    return m_accountEntry;
}

/*!
 * \brief Returns the fields of the account entry. Might be nullptr if no account entry is assigned.
 *
 * The ownership remains unaltered.
 */
inline std::vector<Io::Field> *FieldModel::fields()
{
    return m_fields;
}

/*!
 * \brief Resets the model. The account entry will be unset.
 */
inline void FieldModel::reset()
{
    setAccountEntry(nullptr);
}

/*!
 * \brief Returns the password visibility.
 */
inline PasswordVisibility FieldModel::passwordVisibility() const
{
    return m_passwordVisibility;
}

/*!
 * \brief Sets the password visibility.
 */
inline void FieldModel::setPasswordVisibility(PasswordVisibility passwordVisibility)
{
    m_passwordVisibility = passwordVisibility;
    if(m_fields) {
        emit dataChanged(index(0, 1), index(m_fields->size() - 1, 1), QVector<int>() << Qt::DisplayRole << Qt::EditRole);
    }
}

}

#endif // FIELDMODEL_H
