#ifndef QTGUI_STACKSUPPORT_H
#define QTGUI_STACKSUPPORT_H

#include "./undocommands.h"

#include <QUndoStack>

#include <memory>

#define PASSWORD_MANAGER_UNDO_SUPPORT

namespace QtGui {

class StackAbsorper;

class StackSupport {
    friend class StackAbsorper;

public:
    StackSupport(QUndoStack *undoStack = nullptr);

protected:
    QUndoStack *undoStack();
    bool push(std::unique_ptr<CustomUndoCommand> command);
    void clearUndoStack();

private:
    QUndoStack *m_undoStack;
};

/*!
 * \brief Returns the undo stack for the current instance.
 */
inline QUndoStack *StackSupport::undoStack()
{
    return m_undoStack;
}

/*!
 * \brief Pushes the specified custom undo \a command to the undo stack and returns whether the redo action was successful.
 */
inline bool StackSupport::push(std::unique_ptr<CustomUndoCommand> command)
{
    if (!m_undoStack) {
        return false;
    }
    if (command->isNoop()) {
        return true; // doing nothing can never fail
    }
    auto *const rawCommand(command.release());
    m_undoStack->push(rawCommand);
    return rawCommand->redoResult();
}

/*!
 * \brief Clears the undo stack.
 */
inline void StackSupport::clearUndoStack()
{
    if (m_undoStack) {
        m_undoStack->clear();
    }
}

/*!
 * \brief The StackAbsorper class is used by the CustomUndoCommand class to prevent infinite recursion when pushing
 *        a new command to the stack.
 */
class StackAbsorper {
public:
    StackAbsorper(StackSupport *supported);
    ~StackAbsorper();
    QUndoStack *stack();

private:
    StackSupport *m_supported;
    QUndoStack *m_stack;
};

/*!
 * \brief Detaches the undo stack from the specified stack support temporary.
 */
inline StackAbsorper::StackAbsorper(StackSupport *supported)
    : m_supported(supported)
    , m_stack(supported->m_undoStack)
{
    m_supported->m_undoStack = nullptr;
}

/*!
 * \brief Restores the undo stack of the stack support.
 */
inline StackAbsorper::~StackAbsorper()
{
    m_supported->m_undoStack = m_stack;
}

/*!
 * \brief Returns the stack for the current instance.
 */
inline QUndoStack *StackAbsorper::stack()
{
    return m_stack;
}
} // namespace QtGui

#endif // QTGUI_STACKSUPPORT_H
