#ifndef FIELDMODEL_H
#define FIELDMODEL_H

#if defined(PASSWORD_MANAGER_GUI_QTWIDGETS) || defined(PASSWORD_MANAGER_ENABLE_UNDO_SUPPORT_FOR_QUICK_GUI)
#include "../gui/stacksupport.h"
#endif

#include <passwordfile/io/entry.h>

#include <QAbstractTableModel>

#include <vector>

namespace Io {
class Field;
}

namespace QtGui {

/*!
 * \brief The FieldModelRoles enum defines custom roles for the FieldModel class.
 */
enum FieldModelRoles {
    FieldTypeRole = Qt::UserRole + 1, /**< the field type */
    Key,
    Value,
    IsPassword,
    AlwaysActualValue,
    IsLastRow,
};

/*!
 * \brief The PasswordVisibility enum defines when passwords will be visible.
 */
enum PasswordVisibility {
    Always, /**< passwords are always visible */
    OnlyWhenEditing, /**< passwords are only visible when editing */
    Never /**< passwords are never visible */
};

class FieldModel : public QAbstractTableModel
#ifdef PASSWORD_MANAGER_UNDO_SUPPORT
    ,
                   public StackSupport
#endif
{
    Q_OBJECT
    Q_PROPERTY(Io::AccountEntry *accountEntry READ accountEntry WRITE setAccountEntry)
    Q_PROPERTY(PasswordVisibility passwordVisibility READ passwordVisibility WRITE setPasswordVisibility)

public:
    explicit FieldModel(QObject *parent = nullptr);
#ifdef PASSWORD_MANAGER_UNDO_SUPPORT
    explicit FieldModel(QUndoStack *undoStack, QObject *parent = nullptr);
#endif

    QHash<int, QByteArray> roleNames() const override;
    Io::AccountEntry *accountEntry();
    const Io::AccountEntry *accountEntry() const;
    void setAccountEntry(Io::AccountEntry *entry);
    std::vector<Io::Field> *fields();
    PasswordVisibility passwordVisibility() const;
    QVariant data(const QModelIndex &index, int role) const override;
    QMap<int, QVariant> itemData(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    Q_INVOKABLE bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    Q_INVOKABLE bool moveRows(
        const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indices) const override;
    Q_INVOKABLE const Io::Field *field(std::size_t row) const;

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
    if (m_fields) {
        emit dataChanged(index(0, 1), index(m_fields->size() - 1, 1), QVector<int>({Qt::DisplayRole, Qt::EditRole}));
    }
}
} // namespace QtGui

#endif // FIELDMODEL_H
