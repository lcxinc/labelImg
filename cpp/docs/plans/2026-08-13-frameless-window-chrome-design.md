# Frameless Window Chrome Design

## Problem

The Qt 6 application uses `Qt::FramelessWindowHint`, custom `WM_NCHITTEST`
handling, and Qt mouse handlers for title-bar actions. On Windows the native
window currently lacks `WS_THICKFRAME` and `WS_MAXIMIZEBOX`, so the system does
not start edge resizing or expose standard snap behavior. The hit-test code also
compares physical screen coordinates from Windows with device-independent Qt
geometry, which fails on scaled displays. Finally, title-bar double-click can be
handled by both native caption behavior and Qt event filters, causing a visible
maximize/restore flash.

## Goals

- Resize the normal window from every edge and corner.
- Preserve Windows snap layouts and system resizing behavior.
- Make hit testing correct at 100%, 125%, 150%, and mixed monitor scaling.
- Handle title-bar double-click exactly once without flicker.
- Restore a maximized window when its title bar is dragged downward, then keep
  following the pointer using standard Windows behavior.
- Keep menus, toolbar buttons, and window control buttons interactive.
- Preserve a Qt fallback on non-Windows platforms.

## Architecture

Add a small `WindowChrome` core module containing platform-independent hit-test
geometry plus Windows style-mask helpers. `MainWindow` applies the native style
after the native handle exists and delegates only Windows non-client behavior to
`WM_NCHITTEST`. Windows physical screen points are converted to Qt local logical
coordinates before hit testing.

On Windows, draggable title-bar regions return `HTCAPTION`; Windows then owns
move, maximize drag restore, snap, and double-click behavior. Qt title-bar mouse
move and double-click handlers remain active only as the non-Windows fallback.
Interactive descendants return `HTCLIENT` so their existing actions continue to
work.

## Event Ownership

- Window edges and corners: Windows `WM_NCHITTEST` only.
- Draggable title-bar background: Windows `HTCAPTION` only.
- Menu bar, tool buttons, editable controls: Qt client events only.
- Maximize button: existing Qt action, one state transition per click.
- Non-Windows title-bar drag and double-click: `QWindow::startSystemMove()` and
  the existing maximize/restore signal.

## State And Visual Behavior

Native resize and maximize styles are applied without changing Qt window flags.
The maximized state continues to drive the restore icon through
`QEvent::WindowStateChange`. Direct calls to `updateFramelessChrome()` are not
used to simulate state changes; the state-change event is the source of truth.
The normal geometry remains managed by Qt/Windows so restore and persisted
window size continue to work.

## Testing

- Unit-test Windows style masks and logical hit regions.
- On Windows, assert the shown native window contains resize, minimize,
  maximize, and system-menu styles.
- Send native hit tests to all four edges and four corners and verify the
  corresponding results on a scaled display.
- Verify a title-area double-click causes one maximize transition and a second
  double-click restores once.
- Verify interactive title children remain client areas.
- Run the complete Qt core/UI suite and launch the Release executable for a
  manual resize, snap, maximize, and drag-restore smoke test.

## Acceptance Criteria

The Release application can be resized from every edge and corner, snapped by
dragging, maximized/restored without flashing, and dragged out of maximized state
while remaining attached to the pointer. These behaviors must work with Windows
display scaling enabled and must not break title-bar controls.
