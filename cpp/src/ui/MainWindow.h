#pragma once

#include "core/AnnotationIO.h"
#include "core/YoloDataset.h"
#include "core/AiAssistSession.h"
#include "core/LabelMeConfig.h"
#include "core/PerformanceMonitor.h"
#include "core/ShortcutRegistry.h"
#include "core/StringBundle.h"
#include "ui/Canvas.h"
#include "ui/FramelessTitleBar.h"
#include "ui/MiniMapOverlay.h"

#include <QAction>
#include <QByteArray>
#include <QCheckBox>
#include <QComboBox>
#include <QElapsedTimer>
#include <QHash>
#include <QCache>
#include <QThreadPool>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>
#include <QPlainTextEdit>
#include <QScrollArea>
#include <QSettings>
#include <QSet>
#include <QToolBar>
#include <QToolButton>

class QProcess;
class QLineEdit;
class QDoubleSpinBox;
class QSpinBox;
class QProgressBar;
class QDockWidget;
class QDragEnterEvent;
class QDropEvent;
class QThread;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr, const QString &defaultConfigPath = QString());
    void loadStartupArgs(const QStringList &arguments);
    bool openPath(const QString &path);
    bool openYoloDataset(const QString &path);
    bool openAnnotation(const QString &path);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

private slots:
    void openFile();
    void openDir();
    void openYoloDatasetDialog();
    void openAnnotationDialog();
    void openCurrentImageWithViewer();
    void revealCurrentImageInFolder();
    void openNextImage();
    void openPrevImage();
    void closeFile();
    void resetAllSettings();
    void resetLayout();
    void saveFile();
    void saveFileAs();
    void changeSaveDir();
    void changeFormat();
    void verifyImage();
    bool editCurrentLabel();
    void undoShapeOperation();
    void redoShapeOperation();
    void deleteCurrentShape();
    void deleteAllShapes();
    void copyCurrentShape();
    void copySelectedShapesToClipboard();
    void pasteShapesFromClipboard();
    void copyPreviousBoundingBoxes();
    void deleteCurrentImage();
    void deleteCurrentAnnotationFile();
    void toggleAllShapesVisible(bool visible);
    void toggleAllShapes();
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void fitWindow();
    void fitWidth();
    void brighten();
    void darken();
    void resetBrightness();
    void openBrightnessContrastDialog();
    void showCanvasContextMenu(const QPoint &globalPosition, const QPointF &imagePosition);
    void copyShapeHere();
    void moveShapeHere();
    void insertPolygonPointHere();
    void removePolygonPointHere();
    void removeSelectedPoint();
    void chooseBoxLineColor();
    void chooseShapeLineColor();
    void chooseShapeFillColor();
    void editLabelFlagPresets();
    void showInfoDialog();
    void showShortcutsDialog();
    void openTutorial();
    void showSettingsDialog();
    void setAdvancedMode(bool enabled);
    void setCreateMode();
    void repeatCreateMode();
    void setPolygonCreateMode();
    void setPointCreateMode();
    void setPointsCreateMode();
    void setAiPointsCreateMode();
    void setAiBoxCreateMode();
    void setLineCreateMode();
    void setLinestripCreateMode();
    void setCircleCreateMode();
    void setOrientedRectangleCreateMode();
    void setMaskCreateMode();
    void startAiTextAssist();
    void cancelAiAssist();
    void setEditMode();
    void setViewMode();
    void setEditabilityAllowed(bool allowed);
    void showLabelListContextMenu(const QPoint &position);
    void showFileListContextMenu(const QPoint &position);
    void openContextFile();
    void revealContextFile();
    void copyContextFilePath();
    void toggleContextFileMark();
    void deleteContextFile();
    void onCanvasScaleChanged(double oldScale, double newScale, const QPoint &widgetPosition);
    void onCanvasFrameRendered(double frameMs);
    void editCreatedShapeLabel(int index);
    void onCanvasShapeCreated(int index);
    void onAiSessionResponse(const QByteArray &payload);
    void onAiSessionProgress(const QString &modelName,
                             int fileIndex,
                             int fileCount,
                             const QString &fileName,
                             qint64 bytesDone,
                             qint64 bytesTotal);
    void onAiSessionFailed(const QString &message);
    void onCanvasSelectionChanged(int index);
    void onCanvasShapeEditStarted();
    void onCanvasShapeEditFinished(bool changed);
    void onCanvasShapesChanged();
    void onLabelSelectionChanged();
    void onLabelItemChanged(QListWidgetItem *item);
    void onFileDoubleClicked(QListWidgetItem *item);
    void onFilterChanged(int index);
    void changeLanguage(const QString &language);
    void setFileThumbnailMode(bool enabled);

private:
    enum class SaveFormat { PascalVoc, Yolo, CreateMl, LabelMe };

    void createUi();
    void openOnnxDetection();
    void createActions();
    void initializeShortcutRegistry();
    void registerShortcutCommand(const QString &commandId, const QString &category,
                                 const QList<QAction *> &actions);
    void applyShortcutRegistry();
    void createMenusAndToolbars();
    void createFooterControls();
    void connectSignals();
    void installFramelessChrome();
    void applyNativeWindowChrome();
    void startSystemMove();
    void toggleMaximizeRestore();
    void updateFramelessChrome();
    void loadSettings();
    void saveLastFileByDir();
    void applyLabelMeConfig(const QVariantMap &values, const QStringList &cliLabels);
    bool persistLabelMeConfigValue(const QString &key, const QVariant &value);
    void saveSettings();
    void loadPredefinedClasses();
    void loadPredefinedClassesFromFile(const QString &path);
    void mergePersistedLabelHistory();
    void addClassLabel(const QString &label);
    QMap<QString, bool> labelFlagDefaultsForLabel(const QString &label) const;
    QMap<QString, bool> mergeLabelFlagDefaults(const QString &label, const QMap<QString, bool> &flags) const;
    void applyLabelFlagDefaults(QVector<Shape> *shapes) const;
    void applyConfiguredLabelColors(QVector<Shape> *shapes) const;
    bool loadImage(const QString &path);
    bool loadLabelMeWithRepair(const QString &path, AnnotationDocument *document);
    bool loadAnnotation(const QString &path);
    bool loadStandaloneLabelMe(const QString &path);
    AnnotationDocument annotationDocumentForImage(const QString &imagePath, const QSize &imageSize,
                                                  SaveFormat *detectedFormat = nullptr,
                                                  QString *errorMessage = nullptr) const;
    static AnnotationDocument readAnnotationForImage(const QString &imagePath, const QSize &imageSize,
                                                      const QString &saveDir, const QString &outputFilePath,
                                                      SaveFormat format, SaveFormat *detectedFormat = nullptr,
                                                      QString *errorMessage = nullptr, bool previewOnly = false,
                                                      const YoloDataset &dataset = {});
    static QImage readFileThumbnail(const QString &path, const QString &saveDir,
                                    const QString &outputFilePath, SaveFormat format, const YoloDataset &dataset = {});
    void loadAnnotationsForCurrentImage(QString *errorMessage = nullptr);
    void refreshLabels();
    void refreshUniqueLabelList();
    void syncShapeOrderFromLabelList();
    void populateFileList(bool reloadMetadata = true);
    void reloadImageQueue();
    void refreshSavedFileItem();
    void loadVisibleFileThumbnails();
    QThreadPool m_thumbnailPool;
    QCache<QString, QIcon> m_thumbnailCache{256};
    QHash<QString, QListWidgetItem *> m_fileItems;
    QSet<QString> m_displayedThumbnails;
    quint64 m_thumbnailGeneration = 0;
    bool m_thumbnailLoading = false;
    bool m_externalClipboardHasShapes = false;
    QStringList labelsForImage(const QString &path) const;
    void rebuildFileLabelFilterMenu(bool reloadLabels = true);
    QHash<QString, QStringList> m_fileLabels;
    QSet<QString> m_annotatedFiles;
    void updateFileDockTitle();
    void refreshTopLevelFlagsList();
    QString contextFilePath() const;
    void updateFileContextActions();
    void refreshFileListSelection();
    void scrollLabelListToCurrentShape();
    void scrollCanvasToCurrentShape();
    void refreshTexts();
    void refreshActionToolTips();
    void assignActionIcons();
    void refreshActions();
    void populateToolbarForMode();
    void rebuildRecentFilesMenu();
    void rebuildRecentDirsMenu();
    void addRecentFile(const QString &path);
    void addRecentDir(const QString &path);
    void loadRecentFile(const QString &path);
    void loadRecentDir(const QString &path);
    bool openDirectory(const QString &path);
    QString preferredImageForCurrentDir() const;
    QString preferredNewShapeLabel() const;
    QColor colorForLabel(const QString &label) const;
    QColor fillColorForLabel(const QString &label) const;
    void applyShapeLabelColors(Shape *shape) const;
    bool validateLabel(const QString &label) const;
    bool hasAnnotationForImage(const QString &imagePath) const;
    void rememberLastUsedLabel(const QString &label);
    void setCreateShapeMode(const QString &shapeType, QAction *activeAction);
    void updateAiModelAvailability(bool pointPrompt);
    void setFormat(SaveFormat format);
    void updateFitScale();
    void syncZoomWidget();
    void setCanvasImage(const QImage &image, const QSize &sourceSize);
    void upgradePreviewImage();
    void stopPreviewUpgradeThread();
    QPoint zoomAnchorPosition() const;
    void rememberScrollPosition(const QString &imagePath);
    void restoreScrollPosition(const QString &imagePath);
    void selectLabelRow(int row);
    void selectAdjacentLabel(int step);
    void selectAdjacentShape(int step);
    QString annotationPathForImage(const QString &imagePath) const;
    bool usesAnnotationPathOverride() const;
    void syncTopLevelFlagsEditor();
    AnnotationDocument currentDocument() const;
    void refreshWindowTitle();
    bool saveCurrentFile();
    void setDirty(bool dirty);
    bool maybeSave();
    QStringList scanImages(const QString &dirPath) const;
    bool importDroppedImageFiles(const QStringList &paths);
    QString currentFormatName() const;
    void updatePerformanceLabel();
    void resetWindowConfig();
    void resetShapeHistory();
    void recordShapeHistory();
    void restoreShapeHistorySnapshot(const QVector<Shape> &shapes);
    void syncModeFooterControls();
    void syncFormatFooterControls();
    void syncMainModeButton();
    void startAiAssist(int index);
    void finishAiAssist(int promptIndex, const QVector<Shape> &inferredShapes);
    void finishAiTextAssist(const QVector<Shape> &inferredShapes, double iouThreshold);
    QString aiBridgeScriptPath() const;

    StringBundle m_strings;
    QSettings m_settings;
    ShortcutRegistry m_shortcutRegistry;
    QHash<QString, QList<QAction *>> m_shortcutActions;
    Canvas *m_canvas = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QDockWidget *m_labelDock = nullptr;
    QDockWidget *m_shapeDock = nullptr;
    QDockWidget *m_fileDock = nullptr;
    QDockWidget *m_flagDock = nullptr;
    QListWidget *m_labelList = nullptr;
    QListWidget *m_uniqueLabelList = nullptr;
    QListWidget *m_fileList = nullptr;
    QListWidget *m_flagList = nullptr;
    QLineEdit *m_fileSearchEdit = nullptr;
    QLabel *m_fileDockTitleLabel = nullptr;
    QToolButton *m_fileLabelFilterButton = nullptr;
    QToolButton *m_fileDockCloseButton = nullptr;
    QComboBox *m_filterCombo = nullptr;
    QComboBox *m_aiModelCombo = nullptr;
    QComboBox *m_aiOutputFormatCombo = nullptr;
    QLineEdit *m_aiTextPromptEdit = nullptr;
    QComboBox *m_aiTextModelCombo = nullptr;
    QDoubleSpinBox *m_aiTextScoreSpin = nullptr;
    QDoubleSpinBox *m_aiTextIouSpin = nullptr;
    QToolButton *m_aiTextRunButton = nullptr;
    QProgressBar *m_aiProgressBar = nullptr;
    QToolButton *m_aiCancelButton = nullptr;
    QComboBox *m_defaultLabelCombo = nullptr;
    QCheckBox *m_useDefaultLabel = nullptr;
    QCheckBox *m_difficult = nullptr;
    QPlainTextEdit *m_topLevelFlagsEdit = nullptr;
    QLabel *m_coordinates = nullptr;
    QLabel *m_performanceLabel = nullptr;
    QToolButton *m_footerMiniMapButton = nullptr;
    QComboBox *m_footerFormatCombo = nullptr;
    QComboBox *m_footerModeCombo = nullptr;
    QToolButton *m_footerViewShortcut = nullptr;
    QToolButton *m_footerEditShortcut = nullptr;
    QToolButton *m_footerCreateShortcut = nullptr;
    QToolButton *m_mainModeButton = nullptr;
    QToolButton *m_openWithButton = nullptr;
    QSpinBox *m_zoomWidget = nullptr;
    QMenu *m_mainModeMenu = nullptr;
    QMenu *m_openWithMenu = nullptr;
    QMenu *m_fileLabelFilterMenu = nullptr;
    QMenu *m_fileListContextMenu = nullptr;
    MiniMapOverlay *m_miniMapOverlay = nullptr;
    PerformanceMonitor *m_performanceMonitor = nullptr;
    FramelessTitleBar *m_titleBar = nullptr;
    QWidget *m_topChrome = nullptr;
    QWidget *m_titleToolContainer = nullptr;

    QMenu *m_fileMenu = nullptr;
    QAction *m_onnxDetectionAction = nullptr;
    QMenu *m_openMoreMenu = nullptr;
    QAction *m_openYoloDatasetAction = nullptr;
    QMenu *m_viewMenu = nullptr;
    QMenu *m_helpMenu = nullptr;
    QMenu *m_languageMenu = nullptr;
    QMenu *m_recentFilesMenu = nullptr;
    QMenu *m_recentDirsMenu = nullptr;
    QToolBar *m_toolBar = nullptr;

    QAction *m_openAction = nullptr;
    QAction *m_openDirAction = nullptr;
    QAction *m_openAnnotationAction = nullptr;
    QAction *m_openWithImageViewerAction = nullptr;
    QAction *m_openFileLocationAction = nullptr;
    QAction *m_fileContextOpenAction = nullptr;
    QAction *m_fileContextRevealAction = nullptr;
    QAction *m_fileContextCopyPathAction = nullptr;
    QAction *m_fileContextMarkAction = nullptr;
    QAction *m_fileContextDeleteAction = nullptr;
    QAction *m_showFileDockAction = nullptr;
    QAction *m_showLabelDockAction = nullptr;
    QAction *m_showShapeDockAction = nullptr;
    QAction *m_showFlagDockAction = nullptr;
    QAction *m_closeAction = nullptr;
    QAction *m_quitAction = nullptr;
    QAction *m_resetAllAction = nullptr;
    QAction *m_resetLayoutAction = nullptr;
    QAction *m_saveAction = nullptr;
    QAction *m_saveAsAction = nullptr;
    QAction *m_changeSaveDirAction = nullptr;
    QAction *m_formatAction = nullptr;
    QAction *m_nextAction = nullptr;
    QAction *m_prevAction = nullptr;
    QAction *m_nextCopyAction = nullptr;
    QAction *m_prevCopyAction = nullptr;
    QAction *m_verifyAction = nullptr;
    QAction *m_editLabelAction = nullptr;
    QAction *m_undoAction = nullptr;
    QAction *m_undoLastPointAction = nullptr;
    QAction *m_redoAction = nullptr;
    QAction *m_prevShapeAction = nullptr;
    QAction *m_nextShapeAction = nullptr;
    QAction *m_prevLabelAction = nullptr;
    QAction *m_nextLabelAction = nullptr;
    QAction *m_deleteAction = nullptr;
    QAction *m_deleteAllShapesAction = nullptr;
    QAction *m_copyAction = nullptr;
    QAction *m_copyShapesAction = nullptr;
    QAction *m_pasteShapesAction = nullptr;
    QAction *m_copyHereAction = nullptr;
    QAction *m_moveHereAction = nullptr;
    QAction *m_insertPolygonPointAction = nullptr;
    QAction *m_addPointToEdgeAction = nullptr;
    QAction *m_removePolygonPointAction = nullptr;
    QAction *m_removeSelectedPointAction = nullptr;
    QAction *m_copyPreviousAction = nullptr;
    QAction *m_deleteImageAction = nullptr;
    QAction *m_deleteAnnotationAction = nullptr;
    QAction *m_createModeAction = nullptr;
    QAction *m_repeatCreateAction = nullptr;
    QAction *m_lastCreateAction = nullptr;
    QString m_lastCreateShapeType = QStringLiteral("rectangle");
    QAction *m_createPolygonModeAction = nullptr;
    QAction *m_createPointModeAction = nullptr;
    QAction *m_createPointsModeAction = nullptr;
    QAction *m_createAiPointsModeAction = nullptr;
    QAction *m_createAiBoxModeAction = nullptr;
    QAction *m_createLineModeAction = nullptr;
    QAction *m_createLinestripModeAction = nullptr;
    QAction *m_createCircleModeAction = nullptr;
    QAction *m_createOrientedRectangleModeAction = nullptr;
    QAction *m_createMaskModeAction = nullptr;
    QAction *m_maskEditAction = nullptr;
    QAction *m_editModeAction = nullptr;
    QAction *m_viewModeAction = nullptr;
    QAction *m_editabilityAction = nullptr;
    QAction *m_advancedModeAction = nullptr;
    QAction *m_hideAllAction = nullptr;
    QAction *m_showAllAction = nullptr;
    QAction *m_toggleAllAction = nullptr;
    QAction *m_zoomInAction = nullptr;
    QAction *m_zoomOutAction = nullptr;
    QAction *m_zoomOriginalAction = nullptr;
    QAction *m_fitWindowAction = nullptr;
    QAction *m_fitWidthAction = nullptr;
    QAction *m_brightenAction = nullptr;
    QAction *m_darkenAction = nullptr;
    QAction *m_brightnessOriginalAction = nullptr;
    QAction *m_brightnessContrastAction = nullptr;
    QAction *m_drawSquareAction = nullptr;
    QAction *m_fillDrawingAction = nullptr;
    QAction *m_boxLineColorAction = nullptr;
    QAction *m_shapeLineColorAction = nullptr;
    QAction *m_shapeFillColorAction = nullptr;
    QAction *m_infoAction = nullptr;
    QAction *m_shortcutsAction = nullptr;
    QAction *m_tutorialAction = nullptr;
    QAction *m_settingsAction = nullptr;
    QAction *m_autoSaveAction = nullptr;
    QAction *m_keepPreviousAction = nullptr;
    QAction *m_keepPreviousZoomAction = nullptr;
    QAction *m_keepPreviousBrightnessContrastAction = nullptr;
    QAction *m_singleClassAction = nullptr;
    QAction *m_displayLabelsAction = nullptr;
    QAction *m_embedImageDataAction = nullptr;
    QAction *m_editLabelFlagsAction = nullptr;
    QAction *m_miniMapAction = nullptr;
    QAction *m_showPerformanceAction = nullptr;
    QAction *m_samplingModeAction = nullptr;
    QAction *m_thumbnailModeAction = nullptr;

    QStringList m_classList;
    QVector<Shape> m_shapeClipboard;
    QStringList m_imageList;
    QString m_fileLabelFilter;
    QString m_fileNameFilter;
    QStringList m_recentFiles;
    QStringList m_recentDirs;
    QHash<QString, QString> m_lastFileByDir;
    QHash<QString, double> m_zoomByFile;
    QHash<QString, int> m_fitModeByFile;
    QHash<QString, int> m_brightnessByFile;
    QHash<QString, int> m_contrastByFile;
    QHash<QString, int> m_horizontalScrollByFile;
    QHash<QString, int> m_verticalScrollByFile;
    QByteArray m_defaultDockState;
    AiAssistSession *m_aiSession = nullptr;
    bool m_aiRequestRunning = false;
    enum class AiRequestKind { None, Point, Text };
    AiRequestKind m_aiRequestKind = AiRequestKind::None;
    double m_aiTextIouThreshold = 0.5;
    int m_aiPromptIndex = -1;
    QVector<Shape> m_aiPromptBaseShapes;
    bool m_aiPromptHistoryCaptured = false;
    bool m_aiTextPromptRunning = false;
    QSet<QString> m_viewedFiles;
    QSet<QString> m_markedFiles;
    QMap<QString, QStringList> m_labelFlagPresets;
    QMap<QString, bool> m_labelMeConfiguredFlags;
    QString m_fileContextPath;
    QString m_filePath;
    QThread *m_previewUpgradeThread = nullptr;
    QString m_configFilePath;
    QString m_defaultConfigPath;
    QString m_annotationPathOverride;
    SaveFormat m_annotationOverrideFormat = SaveFormat::LabelMe;
    bool m_hasAnnotationPathOverride = false;
    bool m_annotationLoadFailed = false;
    bool m_upgradingPreviewImage = false;
    QString m_labelMeImageData;
    QString m_labelMeImagePath;
    QString m_labelMeVersion;
    QMap<QString, bool> m_labelMeTopLevelFlags;
    QJsonObject m_labelMeOtherData;
    QString m_lastUsedLabel;
    QString m_validateLabelPolicy;
    QString m_dirPath;
    QString m_saveDir;
    YoloDataset m_yoloDataset;
    QStringList m_classesBeforeDataset;
    void leaveYoloDataset();
    // LabelMe's --output accepts a .json path for a single fixed annotation
    // file; keep it separate from the directory-based save setting.
    QString m_outputFilePath;
    int m_currentImageIndex = 0;
    bool m_copyPreviousNavigation = false;
    SaveFormat m_format = SaveFormat::PascalVoc;
    enum class FitMode { Manual, Window, Width };
    // LabelMe starts new images in fit-window mode; a per-file view state or
    // an explicit zoom action can still switch this to manual/fit-width.
    FitMode m_fitMode = FitMode::Window;
    QColor m_lineColor = QColor(0, 255, 0, 128);
    QColor m_fillColor = QColor(0, 0, 0, 64);
    QString m_labelMeShapeColorMode = QStringLiteral("auto");
    QColor m_labelMeDefaultShapeColor = QColor(0, 255, 0);
    int m_labelMeShiftAutoShapeColor = 0;
    QMap<QString, QColor> m_labelMeLabelColors;
    bool m_labelMeSortLabels = true;
    bool m_labelMeShowLabelTextField = true;
    QString m_labelMeLabelCompletion = QStringLiteral("startswith");
    bool m_labelMeFitToContentColumn = true;
    bool m_labelMeFitToContentRow = false;
    int m_labelMeNumBackups = 10;
    QPointF m_lastCanvasContextImagePos;
    QString m_resourcePerformanceText = "CPU 0.0% | MEM 0 MB";
    QElapsedTimer m_fpsTimer;
    int m_fpsFrameCount = 0;
    double m_displayFps = 0.0;
    bool m_dirty = false;
    bool m_autoSavePending = false;
    void scheduleAutoSave();
    bool m_configOverrides = false;
    bool m_verified = false;
    bool m_noSelectionSlot = false;
    bool m_restoringHistory = false;
    bool m_shapeHistoryPending = false;
    bool m_shapeEditDirtyBefore = false;
    QVector<QVector<Shape>> m_undoStack;
    QVector<QVector<Shape>> m_redoStack;
    QVector<Shape> m_lastShapeSnapshot;
};
