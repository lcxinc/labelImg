#pragma once

#include <QKeySequence>
#include <QLineEdit>

class ShortcutCaptureEdit : public QLineEdit {
    Q_OBJECT

public:
    explicit ShortcutCaptureEdit(QWidget *parent = nullptr);

    QKeySequence sequence() const;
    void setSequence(const QKeySequence &sequence);

signals:
    void sequenceChanged(const QKeySequence &sequence);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

private:
    void refreshText();

    QKeySequence m_sequence;
};
