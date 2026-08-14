#include "ui/ShortcutCaptureEdit.h"

#include <QFocusEvent>
#include <QKeyEvent>

ShortcutCaptureEdit::ShortcutCaptureEdit(QWidget *parent)
    : QLineEdit(parent) {
    setReadOnly(true);
    setClearButtonEnabled(false);
    setAlignment(Qt::AlignCenter);
    setPlaceholderText(QStringLiteral("None"));
    setMinimumWidth(112);
}

QKeySequence ShortcutCaptureEdit::sequence() const {
    return m_sequence;
}

void ShortcutCaptureEdit::setSequence(const QKeySequence &sequence) {
    const QKeySequence normalized(sequence.toString(QKeySequence::PortableText),
                                  QKeySequence::PortableText);
    if (m_sequence == normalized) {
        return;
    }
    m_sequence = normalized;
    refreshText();
}

void ShortcutCaptureEdit::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        clearFocus();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete) {
        if (!m_sequence.isEmpty()) {
            m_sequence = QKeySequence();
            refreshText();
            emit sequenceChanged(m_sequence);
        }
        event->accept();
        return;
    }
    switch (event->key()) {
    case Qt::Key_Control:
    case Qt::Key_Shift:
    case Qt::Key_Alt:
    case Qt::Key_Meta:
    case Qt::Key_AltGr:
        event->accept();
        return;
    default:
        break;
    }

    const Qt::KeyboardModifiers modifiers =
        event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier |
                              Qt::AltModifier | Qt::MetaModifier);
    const QKeySequence captured(QKeyCombination(modifiers, static_cast<Qt::Key>(event->key())));
    if (!captured.isEmpty() && captured.count() == 1) {
        m_sequence = QKeySequence(captured.toString(QKeySequence::PortableText),
                                  QKeySequence::PortableText);
        refreshText();
        emit sequenceChanged(m_sequence);
    }
    event->accept();
}

void ShortcutCaptureEdit::focusInEvent(QFocusEvent *event) {
    QLineEdit::focusInEvent(event);
    selectAll();
}

void ShortcutCaptureEdit::refreshText() {
    setText(m_sequence.toString(QKeySequence::NativeText));
    selectAll();
}
