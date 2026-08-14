# Settings And Shortcuts Design

## Problem

The current settings dialog is a compact two-tab form. Keyboard shortcuts are
split between hard-coded `QAction` setup, LabelMe-compatible YAML values, and
manual key handling in `MainWindow`. This makes shortcuts difficult to discover,
change, validate, and persist. A reassigned single-key shortcut can also leave
the old hard-coded key active.

## Goals

- Redesign settings as a clear category-based workspace.
- Expose every user-facing command that can safely receive a shortcut.
- Allow up to two shortcuts per command.
- Accept unmodified single keys such as `W`, `V`, `X`, `Delete`, and `Space`.
- Reject duplicate bindings before settings can be saved.
- Apply accepted changes immediately without restarting.
- Persist only user overrides and provide per-command and global reset actions.
- Keep text entry usable when single-key shortcuts exist.

## Settings Layout

The settings dialog uses a narrow category list on the left and an unframed
content stack on the right. Categories are:

- General
- View and Annotation
- Shortcuts

The General and View pages retain their current controls but use scroll areas,
consistent row spacing, section headings, and a stable minimum content width.
The dialog keeps standard OK and Cancel actions at the bottom.

The Shortcuts page contains:

- A search field filtering by localized command name and category.
- A compact table with Command, Primary, Secondary, and Reset columns.
- A Restore All Defaults command above the table.
- Inline conflict feedback below the table.

Shortcut cells enter capture mode when clicked. Escape cancels capture.
Backspace clears the selected binding. A single key is accepted without
requiring Ctrl, Alt, Shift, or Meta.

## Shortcut Registry

Add a `ShortcutRegistry` core model. Each command descriptor contains:

- Stable command ID.
- Category ID.
- Associated `QAction`.
- Up to two default `QKeySequence` values.
- Current values.

The registry is the only place that applies configurable shortcuts to actions.
It exposes portable-text serialization, override loading, reset operations, and
conflict detection. Command IDs, not localized display text, are persistence
keys.

Commands without a meaningful action or commands whose key behavior depends on
an active text editor remain non-configurable. Existing action defaults are
captured after action creation, including dual defaults such as `W / Ctrl+R`
and `Delete / X`.

## Precedence And Persistence

Shortcut precedence is:

1. User override in `QSettings` under `shortcuts/<commandId>`.
2. LabelMe configuration shortcut value when present.
3. Built-in C++ default.

Each stored value is a string list using `QKeySequence::PortableText`. Empty
lists intentionally disable a command shortcut. Values equal to the effective
default are removed from QSettings instead of duplicated.

The dialog edits a temporary snapshot. OK validates and commits the complete
snapshot atomically, applies it to all actions, and syncs QSettings. Cancel
discards all shortcut changes.

## Conflict Rules

Normalization uses portable key sequence text. A non-empty sequence may appear
only once across all configurable commands and once within a command. Conflicts
mark both affected rows, display the conflicting command names, and disable OK.

Reserved text-entry behavior is protected through action shortcut contexts and
the existing focus routing. Bare letter and number shortcuts do not trigger
while a line edit, text edit, plain text edit, spin box editor, editable combo,
or shortcut capture field owns keyboard focus.

## Runtime Key Routing

Configurable keys currently handled directly in `MainWindow::keyPressEvent` or
its event filter move to QAction-driven routing. This includes mode, label,
shape, and delete navigation keys. Non-command transient keys such as holding
Ctrl to constrain drawing remain direct canvas input.

This prevents a changed shortcut from leaving the old key active and keeps menu
shortcut hints synchronized automatically.

## Localization

Add string bundle keys for the new category, table headers, search placeholder,
capture state, reset commands, conflict message, and command category names in
English, Simplified Chinese, Traditional Chinese, and Japanese. Command names
continue to come from each localized QAction text.

## Testing

- Unit-test normalization, two-binding limits, conflict detection, resets, and
  QSettings serialization.
- UI-test category navigation, search filtering, single-key capture, clearing,
  conflict blocking, cancel rollback, OK persistence, and live QAction updates.
- Verify custom shortcuts survive a new `MainWindow` instance.
- Verify changing `W`, `V`, `Q`, `E`, `Z`, `C`, `S`, and `X` disables their old
  hard-coded behavior.
- Run complete C++ and Python regression suites and launch the Release build.

## Acceptance Criteria

Users can find any configurable command, assign one or two shortcuts including
bare single keys, detect and resolve conflicts before saving, reset defaults,
and use the new shortcuts immediately and after restart. Text editing remains
unaffected, and cancelled changes never alter live actions or persisted values.
