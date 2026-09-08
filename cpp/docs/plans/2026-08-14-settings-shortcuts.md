# Settings And Shortcuts Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Redesign the settings dialog and add a complete, conflict-safe, immediately applied shortcut editor with support for unmodified single keys.

**Architecture:** Introduce a pure `ShortcutRegistry` model for command metadata, defaults, overrides, normalization, and conflict detection. `MainWindow` owns the QAction catalog and applies registry values, while a dedicated settings page edits a temporary snapshot through two single-chord capture fields per command.

**Tech Stack:** C++17, Qt 6 Core/Gui/Widgets/Test, QSettings, QKeySequence, CMake/MSVC.

---

### Task 1: Implement Shortcut Registry Logic

**Files:**
- Create: `cpp/src/core/ShortcutRegistry.h`
- Create: `cpp/src/core/ShortcutRegistry.cpp`
- Modify: `cpp/CMakeLists.txt`
- Test: `cpp/tests/test_core.cpp`

**Step 1: Write failing registry tests**

Cover command registration, portable-text normalization, a maximum of two
single-chord bindings, duplicate detection across and within commands, per-row
reset, global reset, empty-list disabling, and QSettings override round-trip.

**Step 2: Run focused tests and verify RED**

Run `target\cpp-build\Release\labelImgCppTests.exe shortcutRegistryNormalizesAndLimitsBindings shortcutRegistryRejectsConflicts shortcutRegistryPersistsOnlyOverrides`.

Expected: compile failure because `ShortcutRegistry` does not exist.

**Step 3: Implement the minimal model**

Add `ShortcutCommand`, `ShortcutConflict`, and `ShortcutRegistry`. Keep the
model independent from widgets and QAction. Store at most two `QKeySequence`
objects per command and serialize with `QKeySequence::PortableText`.

**Step 4: Build and verify GREEN**

Build `labelImgCppTests`, run the focused tests, and expect all to pass.

**Step 5: Commit**

Commit as `feat: add shortcut registry model`.

### Task 2: Centralize Main Window Shortcut Actions

**Files:**
- Modify: `cpp/src/ui/MainWindow.h`
- Modify: `cpp/src/ui/MainWindow.cpp`
- Test: `cpp/tests/test_ui.cpp`

**Step 1: Write failing QAction routing tests**

Assert that the main window exposes stable command IDs, adds QAction-backed
previous/next-label commands, loads QSettings overrides after LabelMe defaults,
and removes old hard-coded behavior after `W`, `V`, `Z`, `C`, `Q`, `E`, and `X`
are reassigned.

**Step 2: Build and verify RED**

Run the focused routing tests. Expected: missing command registry and old keys
still execute through `keyPressEvent` or the canvas event filter.

**Step 3: Build and apply the command catalog**

Create explicit descriptors for all safe user-facing file, navigation, edit,
annotation, view, and help actions. Capture built-in defaults, update effective
defaults after LabelMe config, then apply QSettings overrides last.

Add QAction objects for previous and next label. Use `Delete / X` as the two
delete-shape defaults. Remove configurable command branches from
`keyPressEvent` and the canvas/label-list event filter; retain only transient
drawing modifier handling.

**Step 4: Protect text input**

Handle `QEvent::ShortcutOverride` so bare printable keys and editing keys are
accepted by focused text-entry controls and shortcut capture widgets before
window actions can trigger.

**Step 5: Build and verify GREEN**

Run the routing tests and existing shortcut/navigation tests.

**Step 6: Commit**

Commit as `refactor: centralize configurable shortcut actions`.

### Task 3: Redesign The Settings Shell

**Files:**
- Modify: `cpp/src/ui/MainWindow.cpp`
- Modify: `resources/strings/strings.properties`
- Modify: `resources/strings/strings-zh-CN.properties`
- Modify: `resources/strings/strings-zh-TW.properties`
- Modify: `resources/strings/strings-ja-JP.properties`
- Test: `cpp/tests/test_ui.cpp`

**Step 1: Write failing settings layout tests**

Open settings and assert the category list, stacked content, General page, View
and Annotation page, scrollable content, stable object names, and dialog buttons
exist. Verify language switching supplies all new visible strings.

**Step 2: Run and verify RED**

Expected: the existing QTabWidget layout has no category list or stacked pages.

**Step 3: Implement the category layout**

Replace tabs with a compact left QListWidget and right QStackedWidget. Put the
existing forms in scroll areas, retain current controls and persistence, and use
an 8px-or-less restrained settings surface consistent with the main UI.

**Step 4: Build and verify GREEN**

Run existing settings tests plus the new layout/localization tests.

**Step 5: Commit**

Commit as `feat: redesign settings navigation`.

### Task 4: Add The Shortcut Editor Page

**Files:**
- Create: `cpp/src/ui/ShortcutSequenceEdit.h`
- Create: `cpp/src/ui/ShortcutSequenceEdit.cpp`
- Modify: `cpp/CMakeLists.txt`
- Modify: `cpp/src/ui/MainWindow.cpp`
- Test: `cpp/tests/test_ui.cpp`

**Step 1: Write failing editor tests**

Verify all command rows are present, search filters rows, a bare `W` is
captured, Escape cancels, Backspace clears, conflicts mark both rows and disable
OK, per-row reset works, Restore All works, Cancel rolls back, and OK applies
and persists both bindings immediately.

**Step 2: Run and verify RED**

Expected: shortcut page and capture widget do not exist.

**Step 3: Implement single-chord capture**

Subclass QKeySequenceEdit with maximum sequence length one. Preserve the value
at capture start, restore it on Escape, clear it on Backspace, and emit changes
for unmodified keys without requiring a modifier.

**Step 4: Implement the table and validation**

Add search, category labels, two capture cells, icon reset buttons, Restore All,
inline conflict text, and row conflict styling. Edit a copied registry snapshot;
disable OK while conflicts exist.

**Step 5: Apply settings atomically**

On OK, persist all changed overrides, remove values equal to defaults, sync
QSettings, and update QAction shortcuts. On Cancel, discard the snapshot.

**Step 6: Build and verify GREEN**

Run all shortcut editor tests and existing settings tests.

**Step 7: Commit**

Commit as `feat: add configurable shortcut settings`.

### Task 5: Help Text, Regression, And Launch

**Files:**
- Modify: `cpp/src/ui/MainWindow.cpp`
- Modify: `cpp/docs/feature-parity-design.md`
- Test: `cpp/tests/test_ui.cpp`

**Step 1: Make shortcut help dynamic**

Generate shortcut help from the active registry so help text and menu hints
reflect custom bindings.

**Step 2: Run full C++ tests**

Run with `QT_QPA_PLATFORM=offscreen`:
`ctest --test-dir target\cpp-build -C Release --output-on-failure`.

Expected: 2/2 tests pass.

**Step 3: Run Python regressions**

Generate ignored `libs/resources.py` when needed, then run
`python -m unittest discover tests`.

Expected: 23 tests pass.

**Step 4: Build and launch Release**

Run `bin\build.ps1 -Configuration Release`, launch
`target\cpp-build\Release\labelImgCpp.exe`, and manually verify settings search,
single-key capture, conflict blocking, live menu hints, restart persistence, and
normal text entry.

**Step 5: Commit**

Commit as `feat: complete shortcut settings workflow`.
