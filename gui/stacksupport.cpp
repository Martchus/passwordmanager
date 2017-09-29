#include "./stacksupport.h"

namespace QtGui {

/*!
 * \class StackSupport
 * \brief The StackSupport class is used as base class for models supporting undoing of changes via QUndoStack.
 */

/*!
 * \brief Constructs a new stack support with the specified \a undoStack.
 */
StackSupport::StackSupport(QUndoStack *undoStack)
    : m_undoStack(undoStack)
{
}
} // namespace QtGui
