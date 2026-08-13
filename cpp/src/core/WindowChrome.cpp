#include "core/WindowChrome.h"

quintptr windowsWindowChromeStyleMask() {
    constexpr quintptr wsCaption = 0x00C00000;
    constexpr quintptr wsSysMenu = 0x00080000;
    constexpr quintptr wsThickFrame = 0x00040000;
    constexpr quintptr wsMinimizeBox = 0x00020000;
    constexpr quintptr wsMaximizeBox = 0x00010000;
    return wsCaption | wsSysMenu | wsThickFrame | wsMinimizeBox | wsMaximizeBox;
}

quintptr withWindowsWindowChromeStyle(quintptr style) {
    return style | windowsWindowChromeStyleMask();
}

bool hasWindowsWindowChromeStyle(quintptr style) {
    const quintptr mask = windowsWindowChromeStyleMask();
    return (style & mask) == mask;
}

WindowHitRegion windowHitRegion(const QRect &windowRect,
                                const QRect &titleRect,
                                const QPoint &localPosition,
                                int resizeBorder) {
    const int border = qMax(0, resizeBorder);
    const bool left = localPosition.x() >= windowRect.left() &&
                      localPosition.x() < windowRect.left() + border;
    const bool right = localPosition.x() <= windowRect.right() &&
                       localPosition.x() > windowRect.right() - border;
    const bool top = localPosition.y() >= windowRect.top() &&
                     localPosition.y() < windowRect.top() + border;
    const bool bottom = localPosition.y() <= windowRect.bottom() &&
                        localPosition.y() > windowRect.bottom() - border;

    if (top && left) return WindowHitRegion::TopLeft;
    if (top && right) return WindowHitRegion::TopRight;
    if (bottom && left) return WindowHitRegion::BottomLeft;
    if (bottom && right) return WindowHitRegion::BottomRight;
    if (left) return WindowHitRegion::Left;
    if (right) return WindowHitRegion::Right;
    if (top) return WindowHitRegion::Top;
    if (bottom) return WindowHitRegion::Bottom;
    if (titleRect.contains(localPosition)) return WindowHitRegion::Caption;
    return WindowHitRegion::Client;
}
