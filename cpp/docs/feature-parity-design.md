# labelImgCpp Feature Parity Design

## Goal

`labelImgCpp` is a Qt 6 Widgets / CMake / MSVC port that keeps the Python application as the behavioral reference until the C++ version is accepted for daily use.

## Feature Matrix

| Area | Python reference | C++ status | Acceptance |
| --- | --- | --- | --- |
| Image files | Open image or LabelMe JSON, open directory, startup directory, file list, filename search, previous/next, drag/drop, delete image, high-bit/multiband TIFF fallback | Implemented | File Open accepts image/LabelMe JSON paths; image paths import their containing directory, while standalone JSON opens in the dedicated annotation state. Qt-incompatible classic TIFF pages are normalized through the shared fallback before display or embedded-image encoding. |
| Annotation files | Auto-load VOC > YOLO > CreateML, open external annotation | Implemented | Loading XML/TXT/JSON updates format, shapes, and verified state; external LabelMe JSON retains its original save target and CreateML JSON arrays are distinguished from LabelMe objects. |
| Save formats | VOC, YOLO, CreateML, save/save-as, save dir | Implemented | Output round-trips with Chinese paths/labels and class order. |
| Labels | List selection, visibility checkbox, filter, edit, reorder, delete, difficult, validation, label dialog history/navigation, top-level flags dock, automatic label colors | Implemented | List and canvas selection stay synchronized; the edit dialog exposes a sorted label history with Up/Down navigation; drag reorder updates shape order and exact validation rejects labels outside the known label history; automatic colors use LabelMe's full 256-entry `imgviz` colormap and configured shift semantics. |
| Canvas drawing | Create rectangle, square mode, cancel, finish, preview, undo draft point, cancel label prompt | Implemented | W enters create mode with cross cursor; click/tiny drags do not leave low-pixel boxes; Esc cancels; cancelling the post-creation label prompt restores the draft; Undo Last Point/`Ctrl+Z` removes the last draft point; Return/Space/double-click finalize supported drafts. |
| Canvas editing | Select, Ctrl+A multi-select, move, vertex drag, add/remove polygon points, one-pixel nudging, bounds, delete all | Implemented | Shapes and vertices cannot move outside the pixmap; `Ctrl+A` selects all shapes in edit mode; hovered polygon/linestrip edges support LabelMe-style point insertion from the View/context action; Delete All Shapes is a single undoable operation. |
| Context menus | Canvas and label list menus, right-click shape selection, right-drag pan | Implemented | A plain right-click selects the hit shape before opening the menu; the canvas menu exposes every drawing mode plus edit/copy/delete, undo, Copy here/Move here, and polygon point actions; right-drag only scrolls after drag threshold to avoid jitter. |
| AI annotation | Point/box prompts and AI text-to-annotation | Implemented with optional runtime | Point/box prompts and comma-separated text prompts invoke the optional OSAM bridge asynchronously; text prompts preserve class/score metadata and apply score/IoU filtering. |
| View | Manual zoom, fit window, fit width, brightness, large-image preview | Implemented | Wheel zoom preserves the mouse anchor through scrollbar adjustment; fit modes update on resize; images above 4096 px use a bounded preview, retain original annotation dimensions, and upgrade to the full bitmap at 50% manual zoom. |
| Settings | Language, save dir, format, recent files, geometry/state, colors, Preferences dialog, save with image data, auto save, validate_label, display_label_popup, YAML/CLI config | Implemented | File > Settings exposes grouped language/format/validation/autosave/view controls, including the LabelMe-compatible new-shape label popup switch; `--config` YAML and common CLI overrides are applied before startup paths, with CLI precedence and a read-only Settings notice. Values use persisted QSettings keys and restore on restart. |
| Modes | Beginner/advanced create/edit split | Implemented | The frameless title-bar toolbar keeps a stable LabelMe-style primary order and adds advanced visibility/sampling tools only in advanced mode; W and V keep create/view state synchronized, while editability remains a Settings preference. |
| Help | Info and shortcuts | Implemented | Help menu opens local dialogs without network dependency. |

## Implementation Boundaries

- `MainWindow` owns menus, toolbars, docks, file state, settings, annotation loading/saving, language refresh, and user dialogs.
- `Canvas` owns pixmap rendering, shape hit testing, create/edit modes, vertex editing, constrained moves, zoom/brightness state, and mouse/keyboard interaction.
- `Shape` owns rectangle geometry primitives used by both canvas and tests.
- `AnnotationIO` stays UI-free and is the compatibility layer for VOC, YOLO, and CreateML.

## Regression Commands

```powershell
bin\build.bat -QtPrefix "D:\Qt\6.11.0\msvc2022_64"
$env:PATH='D:\Qt\6.11.0\msvc2022_64\bin;' + $env:PATH
ctest --test-dir target\cpp-build -C Release --output-on-failure
python -m unittest discover tests
```

## Remaining Gaps

- MainWindow-level regression coverage now includes mouse-centered scroll adjustment and real right-click copy/move menu wiring.
- The Windows deployment script now packages Qt runtime files, shared resources, the predefined-label file, and the optional AI bridge beside the executable; installer output still requires a local MSVC/Qt environment and is not code-signed.
- Continue comparing less-visible dialog wording and optional AI runtime behavior against Python during manual QA.
