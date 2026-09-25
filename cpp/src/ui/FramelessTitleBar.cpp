#include "ui/FramelessTitleBar.h"

#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QToolBar>
#include <QToolButton>

namespace {
QIcon windowControlIcon(const QString &control) {
    QPixmap bitmap(32, 32);
    bitmap.setDevicePixelRatio(2);
    bitmap.fill(Qt::transparent);
    QPainter painter(&bitmap);
    painter.setPen(QPen(QColor(QStringLiteral("#17212b")), 1.2));
    if (control == QStringLiteral("minimizeButton")) {
        painter.drawLine(QPointF(3, 8), QPointF(13, 8));
    } else if (control == QStringLiteral("closeButton")) {
        painter.drawLine(QPointF(3, 3), QPointF(13, 13));
        painter.drawLine(QPointF(13, 3), QPointF(3, 13));
    } else if (control == QStringLiteral("restore")) {
        painter.drawRect(QRectF(5, 3, 8, 8));
        painter.fillRect(QRectF(3, 5, 8, 8), QColor(QStringLiteral("#eef8e8")));
        painter.drawRect(QRectF(3, 5, 8, 8));
    } else {
        painter.drawRect(QRectF(3, 3, 10, 10));
    }
    return QIcon(bitmap);
}
}

FramelessTitleBar::FramelessTitleBar(QWidget *parent)
    : QFrame(parent) {
    setObjectName(QStringLiteral("framelessTitleBar"));
    setFixedHeight(34);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(8, 0, 4, 0);
    m_layout->setSpacing(4);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(18, 18);
    m_iconLabel->setPixmap(style()->standardIcon(QStyle::SP_FileDialogDetailedView).pixmap(18, 18));

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName(QStringLiteral("framelessTitleLabel"));
    m_titleLabel->setMinimumWidth(80);
    m_titleLabel->setMaximumWidth(280);
    m_titleLabel->setTextInteractionFlags(Qt::NoTextInteraction);

    auto *minimizeButton = createButton(QStringLiteral("Minimize"), QStringLiteral("minimizeButton"));
    m_maximizeButton = createButton(QStringLiteral("Maximize"), QStringLiteral("maximizeButton"));
    auto *closeButton = createButton(QStringLiteral("Close"), QStringLiteral("closeButton"));

    connect(minimizeButton, &QToolButton::clicked, this, &FramelessTitleBar::minimizeRequested);
    connect(m_maximizeButton, &QToolButton::clicked, this, &FramelessTitleBar::maximizeRestoreRequested);
    connect(closeButton, &QToolButton::clicked, this, &FramelessTitleBar::closeRequested);

    m_layout->addWidget(m_iconLabel);
    m_layout->addWidget(m_titleLabel);
    m_layout->addStretch(1);
    m_layout->addWidget(minimizeButton);
    m_layout->addWidget(m_maximizeButton);
    m_layout->addWidget(closeButton);

    setStyleSheet(QStringLiteral(R"(
        #framelessTitleBar {
            background: #eef8e8;
            border-bottom: 1px solid #d9e7d2;
        }
        #framelessTitleLabel {
            color: #1f2933;
            font-size: 12px;
            font-weight: 600;
        }
        QToolButton {
            min-width: 44px;
            max-width: 44px;
            min-height: 32px;
            max-height: 32px;
            border: 0;
            border-radius: 0;
            color: #17212b;
            background: transparent;
            font-family: "Segoe UI";
            font-size: 16px;
            font-weight: 400;
        }
        QToolButton:hover {
            background: rgba(15, 23, 42, 20);
        }
        #closeButton:hover {
            background: #d93f38;
            color: white;
        }
    )"));
}

void FramelessTitleBar::setTitle(const QString &title) {
    const QString text = title.trimmed();
    m_titleLabel->setText(text);
    m_titleLabel->setToolTip(text);
    m_titleLabel->setVisible(!text.isEmpty());
}

void FramelessTitleBar::setMaximized(bool maximized) {
    m_maximizeButton->setIcon(windowControlIcon(maximized ? QStringLiteral("restore") : QStringLiteral("maximizeButton")));
    m_maximizeButton->setToolTip(maximized ? QStringLiteral("Restore") : QStringLiteral("Maximize"));
    m_maximizeButton->setAccessibleName(m_maximizeButton->toolTip());
}

void FramelessTitleBar::setToolWidget(QWidget *widget) {
    if (m_toolWidget == widget) {
        return;
    }
    if (m_toolWidget) {
        m_layout->removeWidget(m_toolWidget);
        m_toolWidget->setParent(nullptr);
    }
    m_toolWidget = widget;
    if (!m_toolWidget) {
        return;
    }
    m_toolWidget->setParent(this);
    installToolWidgetEventFilter(m_toolWidget);
    int insertIndex = 2;
    m_layout->insertWidget(insertIndex, m_toolWidget, 1);
}

bool FramelessTitleBar::eventFilter(QObject *watched, QEvent *event) {
#ifndef Q_OS_WIN
    if (m_toolWidget && event->type() == QEvent::MouseButtonDblClick) {
        auto *widget = qobject_cast<QWidget *>(watched);
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        const bool insideToolArea = widget && (widget == m_toolWidget || m_toolWidget->isAncestorOf(widget));
        const bool toolButton = qobject_cast<QToolButton *>(widget);
        if (insideToolArea && !toolButton && mouseEvent->button() == Qt::LeftButton) {
            emit maximizeRestoreRequested();
            event->accept();
            return true;
        }
    }
#endif
    return QFrame::eventFilter(watched, event);
}

void FramelessTitleBar::mouseDoubleClickEvent(QMouseEvent *event) {
#ifndef Q_OS_WIN
    if (event->button() == Qt::LeftButton) {
        emit maximizeRestoreRequested();
        event->accept();
        return;
    }
#endif
    QFrame::mouseDoubleClickEvent(event);
}

void FramelessTitleBar::mouseMoveEvent(QMouseEvent *event) {
#ifdef Q_OS_WIN
    QFrame::mouseMoveEvent(event);
    return;
#else
    if (!(event->buttons() & Qt::LeftButton) || m_dragStarted) {
        QFrame::mouseMoveEvent(event);
        return;
    }

    const QPoint delta = event->globalPosition().toPoint() - m_pressPosition;
    if (delta.manhattanLength() >= QApplication::startDragDistance()) {
        m_dragStarted = true;
        emit dragRequested();
        event->accept();
        return;
    }
    QFrame::mouseMoveEvent(event);
#endif
}

void FramelessTitleBar::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_pressPosition = event->globalPosition().toPoint();
        m_dragStarted = false;
    }
    QFrame::mousePressEvent(event);
}

void FramelessTitleBar::mouseReleaseEvent(QMouseEvent *event) {
    m_dragStarted = false;
    QFrame::mouseReleaseEvent(event);
}

void FramelessTitleBar::installToolWidgetEventFilter(QWidget *widget) {
    if (!widget) {
        return;
    }
    widget->installEventFilter(this);
    const QList<QWidget *> children = widget->findChildren<QWidget *>();
    for (QWidget *child : children) {
        child->installEventFilter(this);
    }
}

QToolButton *FramelessTitleBar::createButton(const QString &toolTip, const QString &objectName) const {
    auto *button = new QToolButton(const_cast<FramelessTitleBar *>(this));
    button->setObjectName(objectName);
    button->setAccessibleName(toolTip);
    button->setIcon(windowControlIcon(objectName));
    button->setIconSize(QSize(16, 16));
    button->setToolTip(toolTip);
    button->setCursor(Qt::ArrowCursor);
    button->setFocusPolicy(Qt::NoFocus);
    return button;
}
