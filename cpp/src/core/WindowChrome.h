#pragma once

#include <QPoint>
#include <QRect>
#include <QtGlobal>

enum class WindowHitRegion {
    Client,
    Caption,
    Left,
    Right,
    Top,
    Bottom,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
};

quintptr windowsWindowChromeStyleMask();
quintptr withWindowsWindowChromeStyle(quintptr style);
bool hasWindowsWindowChromeStyle(quintptr style);

WindowHitRegion windowHitRegion(const QRect &windowRect,
                                const QRect &titleRect,
                                const QPoint &localPosition,
                                int resizeBorder);
