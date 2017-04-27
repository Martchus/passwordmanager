#ifndef UNDOCOMMANDS_H
#define UNDOCOMMANDS_H

#include <passwordfile/io/field.h>

#include <QUndoCommand>
#include <QUndoStack>
#include <QList>
#include <QVariant>

QT_FORWARD_DECLARE_CLASS(QModelIndex)

namespace Io {
class Entry;
class AccountEntry;
}

namespace QtGui {

class FieldModel;
class EntryModel;
class StackSupport;

class CustomUndoCommand : public QUndoCommand
{
public:
    explicit CustomUndoCommand(StackSupport *stackSupport);
    bool redoResult() const;
    bool undoResult() const;
    bool isNoop() const;
    void redo();
    void undo();
protected:
    void setNoop(bool noop);
    virtual bool internalRedo() = 0;
    virtual bool internalUndo() = 0;
private:
    StackSupport *m_stackSupport;
    bool m_redoResult;
    bool m_undoResult;
    bool m_noop;
};

/*!
 * \brief Returns whether the redo action was successful.
 */
inline bool CustomUndoCommand::redoResult() const
{
    return m_redoResult;
}

/*!
 * \brief Returns whether the undo action was successful.
 */
inline bool CustomUndoCommand::undoResult() const
{
    return m_undoResult;
}

/*!
 * \brief Returns whether this command does nothing, eg. the new value equals the old value.
 */
inline bool CustomUndoCommand::isNoop() const
{
    return m_noop;
}

/*!
 * \brief Sets whether this command does nothing.
 *
 * Meant to be called in the constructor when subclassing and - for example - the new value equals the old value.
 */
inline void CustomUndoCommand::setNoop(bool noop)
{
    m_noop = noop;
}

class FieldModelSetValueCommand : public CustomUndoCommand
{
public:
    explicit FieldModelSetValueCommand(FieldModel *model, const QModelIndex &index, const QVariant &value, int role);
protected:
    bool internalRedo();
    bool internalUndo();
private:
    Io::AccountEntry *m_account;
    FieldModel *m_model;
    int m_row;
    int m_col;
    QVariant m_newValue;
    QVariant m_oldValue;
    int m_role;
};

class FieldModelInsertRowsCommand : public CustomUndoCommand
{
public:
    explicit FieldModelInsertRowsCommand(FieldModel *model, int row, int count);
protected:
    bool internalRedo();
    bool internalUndo();
private:
    Io::AccountEntry *m_account;
    FieldModel *m_model;
    int m_row;
    int m_count;
};

class FieldModelRemoveRowsCommand : public CustomUndoCommand
{
public:
    explicit FieldModelRemoveRowsCommand(FieldModel *model, int row, int count);
protected:
    bool internalRedo();
    bool internalUndo();
private:
    Io::AccountEntry *m_account;
    FieldModel *m_model;
    int m_row;
    int m_count;
    QList<Io::Field> m_values;
};

class EntryModelSetValueCommand : public CustomUndoCommand
{
public:
    explicit EntryModelSetValueCommand(EntryModel *model, const QModelIndex &index, const QVariant &value, int role);
protected:
    bool internalRedo();
    bool internalUndo();
private:
    EntryModel *m_model;
    std::list<std::string> m_path;
    QVariant m_newValue;
    QVariant m_oldValue;
    int m_role;
};

class EntryModelModifyRowsCommand : public CustomUndoCommand
{
public:
    ~EntryModelModifyRowsCommand();
protected:
    explicit EntryModelModifyRowsCommand(EntryModel *model, int row, int count, const QModelIndex &parent);
    bool internalRedo() = 0;
    bool internalUndo() = 0;
    bool insert();
    bool remove();
    EntryModel *m_model;
    std::list<std::string> m_parentPath;
    int m_row;
    int m_count;
    QList<Io::Entry *> m_values;
};

class EntryModelInsertRowsCommand : public EntryModelModifyRowsCommand
{
public:
    explicit EntryModelInsertRowsCommand(EntryModel *model, int row, int count, const QModelIndex &parent);
protected:
    bool internalRedo();
    bool internalUndo();
};

class EntryModelRemoveRowsCommand : public EntryModelModifyRowsCommand
{
public:
    explicit EntryModelRemoveRowsCommand(EntryModel *model, int row, int count, const QModelIndex &parent);
protected:
    bool internalRedo();
    bool internalUndo();
};

class EntryModelMoveRowsCommand : public CustomUndoCommand
{
public:
    explicit EntryModelMoveRowsCommand(EntryModel *model, const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild);
protected:
    bool internalRedo();
    bool internalUndo();
private:
    EntryModel *m_model;
    std::list<std::string> m_sourceParentPath;
    int m_sourceRow;
    int m_count;
    std::list<std::string> m_destParentPath;
    int m_destChild;
};

}

#endif // UNDOCOMMANDS_H
