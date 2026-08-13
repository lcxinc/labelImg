# Frameless Window Chrome Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Restore standard Windows resize, snap, maximize, and drag-restore behavior for the Qt frameless main window without double-click flicker.

**Architecture:** Extract native-style and hit-region calculations into a pure `WindowChrome` core module. Apply the required Windows style bits after the native handle is created, then let Windows exclusively own non-client resize and caption behavior while Qt handles only interactive title controls and non-Windows fallback behavior.

**Tech Stack:** C++17, Qt 6 Core/Gui/Widgets/Test, Win32 window styles and non-client messages, CMake/MSVC.

---

### Task 1: Add Pure Window Chrome Rules

**Files:**
- Create: `cpp/src/core/WindowChrome.h`
- Create: `cpp/src/core/WindowChrome.cpp`
- Modify: `cpp/CMakeLists.txt`
- Test: `cpp/tests/test_core.cpp`

**Step 1: Write the failing tests**

Add tests asserting that the Windows style helper adds caption, system-menu,
thick-frame, minimize, and maximize bits while preserving existing bits. Add
logical-coordinate hit tests for all edges, corners, caption, and client areas.

**Step 2: Run the focused test and verify RED**

Run:
`target\cpp-build\Release\labelImgCppTests.exe windowChromeStyleAddsStandardResizeAndSnapBits windowChromeHitRegionsUseLocalLogicalCoordinates`

Expected: build or test failure because `WindowChrome` does not exist.

**Step 3: Implement the minimal pure helpers**

Define `WindowHitRegion`, `windowsWindowChromeStyleMask()`,
`withWindowsWindowChromeStyle()`, `hasWindowsWindowChromeStyle()`, and
`windowHitRegion()` using local device-independent rectangles.

**Step 4: Rebuild and verify GREEN**

Run:
`D:\Qt\Tools\CMake_64\bin\cmake.exe --build target\cpp-build --config Release -j 4`

Then run the focused tests and expect both to pass.

### Task 2: Restore Native Resize And Snap Style

**Files:**
- Modify: `cpp/src/ui/MainWindow.h`
- Modify: `cpp/src/ui/MainWindow.cpp`
- Test: `cpp/tests/test_ui.cpp`

**Step 1: Write the failing Windows integration test**

Show `MainWindow`, read `GWL_STYLE`, and assert that the native handle contains
the complete window chrome style mask. Also assert the Qt maximum size has not
been fixed to the current size.

**Step 2: Build and verify RED**

Run the focused `mainWindowNativeStyleSupportsResizeAndSnap` UI test. Expected:
failure because `WS_THICKFRAME` and `WS_MAXIMIZEBOX` are absent.

**Step 3: Apply the native style once the handle exists**

Add `applyNativeWindowChrome()` to OR the required bits into `GWL_STYLE`, then
call `SetWindowPos(... SWP_FRAMECHANGED ...)`. Invoke it after frameless UI
installation and again on native handle recreation where needed.

**Step 4: Rebuild and verify GREEN**

Run the focused UI test and expect the style assertions to pass.

### Task 3: Make Resize Hit Testing DPI Correct

**Files:**
- Modify: `cpp/src/ui/MainWindow.cpp`
- Test: `cpp/tests/test_ui.cpp`

**Step 1: Write failing native hit-test tests**

Send `WM_NCHITTEST` at each native edge and corner of a shown normal window and
expect the corresponding `HT*` result. Cover the active test display's actual
DPI and assert an interior point remains `HTCLIENT`.

**Step 2: Run and verify RED**

Run `mainWindowNativeHitTestSupportsEveryResizeEdge`. Expected: current code
returns `HTCLIENT` on a scaled display.

**Step 3: Convert physical native points to local logical points**

Use `ScreenToClient` and `GetDpiForWindow` to convert the `WM_NCHITTEST` screen
point into Qt local device-independent coordinates. Run `windowHitRegion()`
against `MainWindow::rect()` and the title bar mapped into main-window space.

**Step 4: Rebuild and verify GREEN**

Run the focused native hit-test test and expect all edge/corner cases to pass.

### Task 4: Give Caption Behavior A Single Owner

**Files:**
- Modify: `cpp/src/ui/FramelessTitleBar.cpp`
- Modify: `cpp/src/ui/MainWindow.cpp`
- Test: `cpp/tests/test_ui.cpp`

**Step 1: Write failing title-area tests**

Assert that blank menu/toolbar/title areas return `HTCAPTION`, while menu
actions, toolbar buttons, and minimize/maximize/close buttons return
`HTCLIENT`. Observe window-state changes and require exactly one state
transition for each native caption double-click.

**Step 2: Run and verify RED**

Run the title hit-test and double-click tests. Expected: toolbar/menu blank
areas are client regions or duplicate Qt/native handling is observed.

**Step 3: Implement exclusive event ownership**

On Windows, disable `FramelessTitleBar`'s Qt drag and double-click fallback.
Treat only actual menu actions, toolbar actions, buttons, and input controls as
interactive client areas. Return `HTCAPTION` for remaining title-row space and
let DefWindowProc own drag, snap, maximize drag-restore, and double-click.

**Step 4: Rebuild and verify GREEN**

Run focused tests and confirm each double-click produces one state change and
all title controls remain clickable.

### Task 5: Full Verification And Launch

**Files:**
- Modify if needed: `cpp/docs/feature-parity-design.md`

**Step 1: Run all C++ tests**

Run:
`ctest --test-dir target\cpp-build -C Release --output-on-failure`

Expected: 2/2 tests pass.

**Step 2: Run Python regressions**

Run:
`python -m unittest discover tests`

Expected: all Python compatibility tests pass.

**Step 3: Build the distributable Release target**

Run:
`bin\build.bat`

Expected: `target\cpp-build\Release\labelImgCpp.exe` is rebuilt successfully.

**Step 4: Launch and inspect the native style**

Run `bin\run.bat`, verify the process path points to the newly built executable,
and inspect `GWL_STYLE` plus `WM_NCHITTEST` results. Manually smoke-test edge
resize, corner resize, Windows snap, double-click maximize/restore, and dragging
down from maximized state.

**Step 5: Commit the implementation**

Stage only the C++ source, tests, CMake, and documentation changed for this fix,
then commit with `fix: restore frameless window interactions`.
