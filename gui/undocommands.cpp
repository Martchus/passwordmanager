#include "./undocommands.h"
#include "./stacksupport.h"

#include "../model/fieldmodel.h"
#include "../model/entrymodel.h"

#include <passwordfile/io/entry.h>

#include <QApplication>

using namespace std;
using namespace Io;

namespace QtGui {

/*!
 * \class CustomUndoCommand
 * \brief The CustomUndoCommand class acts as base class for undo commands used within the
 *        models of the application.
 * \sa http://qt-project.org/doc/qt-5/qundocommand.html
 */

/*!
 * \brief Constructs a new custom undo command with the specified \a stackSupport.
 */
CustomUndoCommand::CustomUndoCommand(StackSupport *stackSupport) :
    m_stackSupport(stackSupport),
    m_redoResult(false),
    m_undoResult(true),
    m_noop(false)
{}

void CustomUndoCommand::redo()
{
    if(m_undoResult) {
        StackAbsorper stackAbsorper(m_stackSupport);
        m_redoResult = internalRedo();
    }
}

void CustomUndoCommand::undo()
{
    if(m_redoResult) {
        StackAbsorper stackAbsorper(m_stackSupport);
        m_undoResult = internalUndo();
    }
}

/*!
 * \fn CustomUndoCommand::internalRedo()
 * \brief This method is internally called to perform the redo action.
 */

/*!
 * \fn CustomUndoCommand::internalUndo()
 * \brief This method is internally called to perform the undo action.
 */

/*!
 * \class FieldModelSetValueCommand
 * \brief Sets the value for the specified index and role in the specified field model.
 */

/*!
 * \brief Constructs a new command.
 */
FieldModelSetValueCommand::FieldModelSetValueCommand(FieldModel *model, const QModelIndex &index, const QVariant &value, int role) :
    CustomUndoCommand(model),
    m_account(model->accountEntry()),
    m_model(model),
    m_row(index.row()),
    m_col(index.column()),
    m_newValue(value),
    m_oldValue(model->data(index, role)),
    m_role(role)
{
    QString fieldName = model->index(m_row, 0, index.parent()).data().toString();
    switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch(m_col) {
        case 0:
            if(m_oldValue.toString().isEmpty()) {
                setText(QApplication::translate("undocommands", "setting field name to »%1«").arg(m_newValue.toString()));
            } else {
                setText(QApplication::translate("undocommands", "setting field name »%1« to »%2«").arg(m_oldValue.toString(), m_newValue.toString()));
            }
            break;
        case 1:
            if(fieldName.isEmpty()) {
                setText(QApplication::translate("undocommands", "setting value of empty field"));
            } else {
                setText(QApplication::translate("undocommands", "setting value of »%1« field").arg(fieldName));
            }
            break;
        }
        break;
    case FieldTypeRole:
        setText(QApplication::translate("undocommands", "setting type of »%1« field").arg(fieldName));
        break;
    default:
        setText(QApplication::translate("undocommands", "setting field property in row »%1«").arg(m_row + 1));
    }
    setNoop(m_oldValue == m_newValue);
}

bool FieldModelSetValueCommand::internalRedo()
{
    m_model->setAccountEntry(m_account);
    return m_model->setData(m_model->index(m_row, m_col), m_newValue, m_role);
}

bool FieldModelSetValueCommand::internalUndo()
{
    m_model->setAccountEntry(m_account);
    return m_model->setData(m_model->index(m_row, m_col), m_oldValue, m_role);
}

/*!
 * \class FieldModelInsertRowsCommand
 * \brief Inserts the specified number of rows before the specified row in the specified field model.
 */

/*!
 * \brief Constructs a new command.
 */
FieldModelInsertRowsCommand::FieldModelInsertRowsCommand(FieldModel *model, int row, int count) :
    CustomUndoCommand(model),
    m_account(model->accountEntry()),
    m_model(model),
    m_row(row),
    m_count(count)
{
    setText(QApplication::translate("undocommands", "insertion of %1 row(s) before row %2", 0, count).arg(count).arg(row + 1));
}

bool FieldModelInsertRowsCommand::internalRedo()
{
    m_model->setAccountEntry(m_account);
    return m_model->insertRows(m_row, m_count, QModelIndex());
}

bool FieldModelInsertRowsCommand::internalUndo()
{
    m_model->setAccountEntry(m_account);
    return m_model->removeRows(m_row, m_count, QModelIndex());
}

/*!
 * \class FieldModelRemoveRowsCommand
 * \brief Removes the specified number of rows at the specified row in the specified field model.
 */

/*!
 * \brief Constructs a new command.
 */
FieldModelRemoveRowsCommand::FieldModelRemoveRowsCommand(FieldModel *model, int row, int count) :
    CustomUndoCommand(model),
    m_account(model->accountEntry()),
    m_model(model),
    m_row(row),
    m_count(count)
{
    if(count == 1) {
        setText(QApplication::translate("undocommands", "removal of row %1", 0, count).arg(row + 1));
    } else {
        setText(QApplication::translate("undocommands", "removal of the rows %1 to %2", 0, count).arg(row + 1).arg(row + count));
    }
}

bool FieldModelRemoveRowsCommand::internalRedo()
{
    m_model->setAccountEntry(m_account);
    if(m_values.isEmpty()) {
        for(int row = m_row, end = m_row + m_count; row < end; ++row) {
            if(const Field *field = m_model->field(row)) {
                m_values << Field(*field);
            }
        }
    }
    return m_model->removeRows(m_row, m_count, QModelIndex());
}

bool FieldModelRemoveRowsCommand::internalUndo()
{
    m_model->setAccountEntry(m_account);
    bool res = m_model->insertRows(m_row, m_count, QModelIndex());
    for(int row = m_row, end = m_row + m_count, value = 0, values = m_values.size(); row < end && value < values; ++row, ++value) {
        m_model->setData(m_model->index(row, 0), QString::fromStdString(m_values.at(value).name()), Qt::EditRole);
        m_model->setData(m_model->index(row, 1), QString::fromStdString(m_values.at(value).value()), Qt::EditRole);
        m_model->setData(m_model->index(row, 0), static_cast<int>(m_values.at(value).type()), FieldTypeRole);
    }
    return res;
}

/*!
 * \brief Stores the entry path for the specified \a model and \a index in \a res.
 */
void indexToPath(EntryModel *model, const QModelIndex &index, list<string> &res)
{
    res.clear();
    if(Entry *entry = model->entry(index)) {
        entry->path(res);
    }
}

/*!
 * \brief Fetches the entry for the specified \a model and \a path.
 * \remarks The \a path will be modified. To prevent this use entryFromPathCpy().
 */
Entry *entryFromPath(EntryModel *model, list<string> &path)
{
    if(NodeEntry *rootEntry = model->rootEntry()) {
        return rootEntry->entryByPath(path);
    }
    return nullptr;
}

/*!
 * \brief Fetches the entry for the specified \a model and \a path.
 */
Entry *entryFromPathCpy(EntryModel *model, list<string> path)
{
    return entryFromPath(model, path);
}

/*!
 * \class EntryModelSetValueCommand
 * \brief Sets the value for the specified index and role in the specified entry model.
 */

/*!
 * \brief Constructs a new command.
 */
EntryModelSetValueCommand::EntryModelSetValueCommand(EntryModel *model, const QModelIndex &index, const QVariant &value, int role) :
    CustomUndoCommand(model),
    m_model(model),
    m_newValue(value),
    m_oldValue(model->data(index, role)),
    m_role(role)
{
    indexToPath(model, index, m_path);
    switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if(m_oldValue.toString().isEmpty()) {
            setText(QApplication::translate("undocommands", "setting entry name to »%1«").arg(m_newValue.toString()));
        } else {
            setText(QApplication::translate("undocommands", "setting entry name from »%1« to »%2«").arg(m_oldValue.toString(), m_newValue.toString()));
        }
        break;
    default:
        QString name = model->data(model->index(index.row(), 0, index.parent()), Qt::DisplayRole).toString();
        if(name.isEmpty()) {
            setText(QApplication::translate("undocommands", "setting property of an entry"));
        } else {
            setText(QApplication::translate("undocommands", "setting property of entry »%1«").arg(name));
        }
    }
    setNoop(m_oldValue == m_newValue);
}

bool EntryModelSetValueCommand::internalRedo()
{
    if(Entry *entry = entryFromPath(m_model, m_path)) {
        bool res = m_model->setData(m_model->index(entry), m_newValue, m_role);
        m_path.clear();
        entry->path(m_path);
        return res;
    }
    return false;
}

bool EntryModelSetValueCommand::internalUndo()
{
    if(Entry *entry = entryFromPath(m_model, m_path)) {
        bool res = m_model->setData(m_model->index(entry), m_oldValue, m_role);
        m_path.clear();
        entry->path(m_path);
        return res;
    }
    return false;
}

/*!
 * \class EntryModelInsertRowsCommand
 * \brief Modifies the specified number of rows before the specified row in the specified entry model under the specified parent.
 */

/*!
 * \brief Constructs a new command.
 */
EntryModelModifyRowsCommand::EntryModelModifyRowsCommand(EntryModel *model, int row, int count, const QModelIndex &parent) :
    CustomUndoCommand(model),
    m_model(model),
    m_row(row),
    m_count(count)
{
    indexToPath(model, parent, m_parentPath);
}

/*!
 * \brief Destroys the command.
 *
 * Removed entries will be deleted finally.
 */
EntryModelModifyRowsCommand::~EntryModelModifyRowsCommand()
{
    qDeleteAll(m_values);
}

/*!
 * \brief Inserts the buffered entries to the model.
 */
bool EntryModelModifyRowsCommand::insert()
{
    if(Entry *parentEntry = entryFromPathCpy(m_model, m_parentPath)) {
        if(m_model->insertEntries(m_row, m_model->index(parentEntry), m_values)) {
            m_values.clear();
            return true;
        }
    }
    return false;
}

/*!
 * \brief Removes the entries from the model.
 *
 * The entries and the model have been specified when constructing the class.
 *
 * The removed entries are buffered.
 */
bool EntryModelModifyRowsCommand::remove()
{
    if(Entry *parentEntry = entryFromPathCpy(m_model, m_parentPath)) {
        m_values = m_model->takeEntries(m_row, m_count, m_model->index(parentEntry));
        return !m_values.isEmpty();
    }
    return false;
}

/*!
 * \class EntryModelInsertRowsCommand
 * \brief Inserts the specified number of rows before the specified row in the specified entry model under the specified parent.
 */

/*!
 * \brief Constructs a new command.
 */
EntryModelInsertRowsCommand::EntryModelInsertRowsCommand(EntryModel *model, int row, int count, const QModelIndex &parent) :
    EntryModelModifyRowsCommand(model, row, count, parent)
{
    setText(QApplication::translate("undocommands", "insertion of %1 entry/entries", 0, count).arg(count));
    switch(m_model->insertType()) {
    case EntryType::Account:
        m_values << new AccountEntry;
        break;
    case EntryType::Node:
        m_values << new NodeEntry;
        break;
    }
}

bool EntryModelInsertRowsCommand::internalRedo()
{
    return insert();
}

bool EntryModelInsertRowsCommand::internalUndo()
{
    return remove();
}

/*!
 * \class EntryModelRemoveRowsCommand
 * \brief Removes the specified number of rows at the specified row in the specified entry model under the specified parent.
 */

/*!
 * \brief Constructs a new command.
 */
EntryModelRemoveRowsCommand::EntryModelRemoveRowsCommand(EntryModel *model, int row, int count, const QModelIndex &parent) :
    EntryModelModifyRowsCommand(model, row, count, parent)
{
    setText(QApplication::translate("undocommands", "removal of %1 entry/entries", 0, count).arg(count));
}

bool EntryModelRemoveRowsCommand::internalRedo()
{
    return remove();
}

bool EntryModelRemoveRowsCommand::internalUndo()
{
    return insert();
}

/*!
 * \class EntryModelMoveRowsCommand
 * \brief Moves the specified rows to the specified destination within the specified entry model.
 */

/*!
 * \brief Constructs a new command.
 */
EntryModelMoveRowsCommand::EntryModelMoveRowsCommand(EntryModel *model, const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) :
    CustomUndoCommand(model),
    m_model(model),
    m_sourceRow(sourceRow),
    m_count(count),
    m_destChild(destinationChild)
{
    indexToPath(model, sourceParent, m_sourceParentPath);
    indexToPath(model, destinationParent, m_destParentPath);
    setText(QApplication::translate("undocommands", "move of %1 entry/entries", 0, count).arg(count));
}

bool EntryModelMoveRowsCommand::internalRedo()
{
    if(m_count) {
        Entry *sourceParentEntry = entryFromPathCpy(m_model, m_sourceParentPath);
        Entry *destParentEntry = entryFromPathCpy(m_model, m_destParentPath);
        if(sourceParentEntry && destParentEntry) {
            return m_model->moveRows(m_model->index(sourceParentEntry), m_sourceRow, m_count, m_model->index(destParentEntry), m_destChild);
        }
        return false;
    }
    return true;
}

bool EntryModelMoveRowsCommand::internalUndo()
{
    if(m_count) {
        Entry *sourceParentEntry = entryFromPathCpy(m_model, m_sourceParentPath);
        Entry *destParentEntry = entryFromPathCpy(m_model, m_destParentPath);
        if(sourceParentEntry && destParentEntry) {
            int sourceRow = m_destChild;
            int destChild = m_sourceRow;
            // moves whithin the same parent needs special consideration
            if(sourceParentEntry == destParentEntry) {
                // move entry down
                if(m_sourceRow < m_destChild) {
                    sourceRow -= m_count;
                // move entry up
                } else if(m_sourceRow > m_destChild) {
                    destChild += m_count;
                // keep entry were it is
                } else {
                    return true;
                }
            }
            return m_model->moveRows(m_model->index(destParentEntry), sourceRow, m_count, m_model->index(sourceParentEntry), destChild);
        }
        return false;
    }
    return true;
}

}
