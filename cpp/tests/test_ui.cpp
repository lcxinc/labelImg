#include <QtTest>
#include <QtWidgets>

#include <QBuffer>

#include "core/AnnotationIO.h"
#include "core/LabelMeConfig.h"
#include "core/ResourcePaths.h"
#include "core/WindowChrome.h"
#include "ui/Canvas.h"
#include "ui/MiniMapOverlay.h"
#include "ui/MainWindow.h"

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

class UiTests : public QObject {
    Q_OBJECT

private slots:
    void init();
    void canvasCreatesAndCancelsShapes();
    void canvasStartsInEditModeLikeLabelMe();
    void canvasKeepsSeedWhenSwitchingCompatibleCreateMode();
    void canvasRetypesMovedDraftWithoutDiscardingCursor();
    void canvasRetypesMovedDraftIntoPolygonAtLastCursor();
    void canvasCreateModeUsesCrossCursorAndIgnoresTinyClicks();
    void canvasAltTemporarilyDisablesPolygonSnapping();
    void canvasSwitchingToCreateModeClearsSelection();
    void canvasCreateModeIgnoresClicksOutsidePixmap();
    void canvasRejectsDegeneratePolygonDraft();
    void canvasRejectsZeroLengthBasicShapeOnKeyboardFinish();
    void canvasRejectsDegenerateOrientedRectangleDraft();
    void canvasShiftCreatesLabelMeSquare();
    void canvasShiftResizesRectangleAsSquare();
    void canvasCreateModeCrosshairCanBeToggled();
    void canvasCrosshairCanBeConfiguredPerCreateShape();
    void canvasDoubleClickCloseCanBeToggled();
    void canvasPointSizeCanBeConfigured();
    void canvasLabelMePaletteCanBeConfigured();
    void canvasEpsilonCanBeConfigured();
    void canvasTinyCreateClickDoesNotEmitShapeChange();
    void canvasRightDragPansButRightClickOpensMenu();
    void canvasMiddleDragPansWithoutCreatingOrMovingShapes();
    void canvasMiddleDragDoesNotPanWhenImageFitsViewport();
    void canvasRightClickOnShapeSelectsItBeforeMenu();
    void canvasLeftDragPansEmptyImageInEditMode();
    void canvasLeftDragStillCreatesBoxInCreateMode();
    void canvasMoveKeepsGrabAnchorAtImageBoundary();
    void canvasViewModePansWithoutCreatingOrMovingShapes();
    void canvasViewModeUsesArrowCursorAndEmptyClickClearsSelection();
    void canvasViewModeSelectsShapeWithoutEditing();
    void canvasViewModeDoesNotRenderEditHandles();
    void canvasLeavingImageClearsHoverCursor();
    void canvasFocusOutReleasesInteractionCursor();
    void canvasEnteringImageRestoresModeCursor();
    void canvasCtrlASelectsAllShapesInEditMode();
    void canvasPointCreateModeCreatesPointOnClick();
    void canvasPointsCreateModeFinishesWithReturn();
    void canvasPointsCreateModeFinishesWithSpace();
    void canvasLinestripCreateModeFinishesWithCtrlClick();
    void canvasAiPointsUseShiftNegativeLabelsAndCtrlFinishes();
    void canvasAiPointsRenderNegativePromptMarker();
    void canvasUndoLastDrawingPointWithBackspace();
    void canvasUndoLastDrawingPoint();
    void canvasUndoLastBasicShapeDraftKeepsAnchor();
    void canvasPointsShapeRendersAndMovesOnePoint();
    void canvasSelectedPointsUseSingleMarker();
    void canvasPointsShapeHitTestingUsesScreenMarkerSize();
    void canvasPointMarkerScalesWithImageAtLowZoom();
    void canvasLineAndCircleCreateModesUseTwoPoints();
    void canvasOrientedRectangleCreateModeUsesThreeClicks();
    void canvasOrientedRectangleCreateModeFinishesWithReturn();
    void canvasOrientedRectangleCreateModeFinishesWithDoubleClick();
    void canvasRendersOrientedRectangleDirectionArrow();
    void canvasMaskCreateModeCreatesNativeMask();
    void canvasRendersMaskOnlyOnNonZeroPixels();
    void canvasRendersMaskBoundaryOutline();
    void canvasRendersMaskWithoutBitmapAsBoundingBox();
    void canvasMaskDoesNotResizeFromBoundary();
    void canvasMaskEditingPaintsAndErasesPixels();
    void canvasCtrlClickTogglesMultiSelection();
    void canvasClickingSelectedShapeTogglesSelectionWithoutEditing();
    void canvasUsesLabelMeShapeHitTesting();
    void canvasUnknownShapeUsesPathAndVertexEditing();
    void canvasSupportsMultiSelectionDuplicateAndDelete();
    void canvasDeleteCurrentSelectsNextShape();
    void canvasDiscardCreatedShapeClearsSelection();
    void canvasSetShapesSelectsCurrentShape();
    void canvasKeyboardMovesMultiSelectionTogether();
    void canvasCopiesSelectedGroupToContextPoint();
    void canvasMovesSelectedGroupToContextPoint();
    void canvasRightDragOnShapeSupportsCopyAndMove();
    void canvasRightDragOutsidePixmapDoesNotMovePreview();
    void canvasIgnoresRightDragBelowThreshold();
    void canvasRightDragDoesNotJumpWhenThresholdIsCrossed();
    void canvasPanDragUsesGlobalMouseDeltaWhenLocalPositionShifts();
    void canvasWheelZoomReportsAnchorPoint();
    void canvasWheelMatchesLabelMeModifierRouting();
    void canvasWheelUsesNaturalScrollSignal();
    void canvasCopiesAndMovesCurrentShapeToPoint();
    void canvasPolygonCreateModeFinishesWithReturn();
    void canvasPolygonCreateModeFinishesWithDoubleClick();
    void canvasPolygonCreateModeSnapsClosedOnFirstVertex();
    void canvasPolygonVertexDragMovesOnlyThatPoint();
    void canvasVertexPriorityBeatsOverlappingResizeHandle();
    void canvasAltClicksInsertAndRemovePolygonPoints();
    void canvasAltClickingVertexDoesNotInsertPolygonPoint();
    void canvasAddsPointToHoveredPolygonEdge();
    void canvasAddsPointToClosingPolygonEdgeAtLabelMeIndex();
    void canvasAddsPointToLinestripClosingEdge();
    void canvasAllowsEdgeInsertionOnImportedTwoPointPolygon();
    void canvasContextPointOperationsInsertAndRemovePolygonPoints();
    void canvasLineVertexDragMovesOnlyThatPoint();
    void canvasVertexDragOutsideProjectsToImageEdge();
    void canvasOrientedRectangleVertexDragKeepsShapeType();
    void canvasOrientedRectangleRotationHandleRotatesAroundCenter();
    void canvasOrientedRectangleRotationMatchesLabelMeWithoutClipping();
    void canvasOrientedRectangleVertexDragClipsAndPreservesParallelogram();
    void canvasVertexHandlesAreEasierToGrabWithoutCrossCursor();
    void canvasWindowStyleResizeUsesEdges();
    void canvasPointBackedCircleMovesFromNonVertexEdge();
    void canvasSmallSelectedShapeUsesCompactResizeHandles();
    void canvasResizeHandlesAreVisuallySmallerButHitAreaWider();
    void canvasBackspaceRemovesHoveredPolygonVertex();
    void canvasSelectedShapeFillIsHighlyTransparent();
    void canvasSelectedShapeUsesLabelPaletteFill();
    void canvasFillDrawingPreviewCanBeToggled();
    void canvasBrightnessAndContrastCanBeAdjusted();
    void canvasBrightnessContrastMatchesLabelMeEnhancement();
    void mainWindowDefaultsFillDrawingLikeLabelMe();
    void mainWindowDefaultsToFitWindowLikeLabelMe();
    void mainWindowFillDrawingActionIsPersisted();
    void mainWindowBrightnessContrastActionsArePersisted();
    void mainWindowToggleAllShapesVisibilityMatchesLabelMe();
    void mainWindowResetLayoutRestoresDefaultDockState();
    void canvasScaleClampSamplingAndOverview();
    void canvasPreviewKeepsOriginalCoordinateSize();
    void canvasUsesLabelMeOverscrollSlackWhenImageOverflows();
    void mainWindowLanguageActionRefreshesTexts();
    void mainWindowDrawingModeActionsRefreshLanguage();
    void mainWindowSecondaryTextsRefreshLanguage();
    void mainWindowClipboardStatusRefreshesLanguage();
    void canvasStatusTextRefreshesWithMainWindowLanguage();
    void mainWindowContextPlacementActionsRefreshLanguage();
    void mainWindowSettingsActionOpensAndPersists();
    void mainWindowSettingsEditsCanvasInteractionConfig();
    void mainWindowSettingsCanOpenConfigFileAsText();
    void mainWindowSettingsRejectsChangesWhenConfigWriteFails();
    void mainWindowExactLabelValidationRejectsUnknownLabel();
    void mainWindowExposesSaveWithImageDataInFileMenu();
    void mainWindowSaveAsUsesFormatDefaultSuffix();
    void mainWindowExposesDeleteAnnotationActionForCurrentLabelFile();
    void mainWindowDeletesCurrentAnnotationFileAndClearsShapes();
    void mainWindowCloseFileDisablesCanvasAndClearsActiveState();
    void mainWindowUsesFramelessChrome();
    void mainWindowNativeStyleSupportsResizeAndSnap();
    void mainWindowNativeHitTestSupportsEveryResizeEdge();
    void mainWindowUsesGeneratedCppIcon();
    void mainWindowEmbedsToolbarIntoFramelessTitleBar();
    void mainWindowTitleToolAreaDoubleClickTogglesMaximize();
    void mainWindowRecoversWindowPositionWhenSavedScreenIsUnavailable();
    void mainWindowPlacesMenusAndToolsInSingleTitleRow();
    void mainWindowTitleToolbarUsesIconOnlyButtons();
    void mainWindowFooterContainsModeFormatAndMiniMapControls();
    void mainWindowTopToolbarIncludesLabelMeFileActions();
    void mainWindowHasOpenWithDropdownForCurrentImage();
    void mainWindowTitleBarShowsCurrentFileNameAndCompactToolbar();
    void mainWindowTitleBarUsesStandardWindowControls();
    void mainWindowShowsPerformanceAndMiniMapControls();
    void mainWindowFooterControlsRefreshLanguage();
    void mainWindowHelpMenuIncludesTutorialAction();
    void mainWindowLabelPanelRefreshesLanguage();
    void mainWindowConstructionDoesNotConsumeTestRunnerArguments();
    void mainWindowSettingsDialogUsesActiveLanguage();
    void mainWindowFileListTextsRefreshLanguage();
    void miniMapDrawsVisibleShapeBoxes();
    void miniMapIsSemiTransparentUntilHovered();
    void mainWindowFpsUsesFrameCadenceNotRenderDuration();
    void mainWindowMiniMapClickChangesScrollbars();
    void mainWindowRestoresScrollPositionPerImage();
    void mainWindowCentersZoomedOutImages();
    void mainWindowFileListUsesReadableSelectionStyleAndBottomDock();
    void mainWindowFileListCheckStateTracksAnnotationPresence();
    void mainWindowIgnoresLegacyBottomFileDockState();
    void mainWindowViewMenuRestoresClosedRightDocks();
    void mainWindowFileListKeepsSelectionSeparateFromActiveFile();
    void mainWindowFileListContextMenuProvidesCommonFileActions();
    void mainWindowCanvasContextMenuProvidesPolygonPointActions();
    void mainWindowCanvasContextMenuIncludesShapeClipboardActions();
    void mainWindowCanvasContextMenuPlacementActionsUseContextPoint();
    void mainWindowCanvasContextMenuIncludesAllCreateModes();
    void mainWindowProvidesRemoveSelectedPointShortcut();
    void mainWindowFileThumbnailModeDefaultsOffAndCanToggle();
    void mainWindowFileThumbnailsDrawAnnotationBoxes();
    void mainWindowFileDockTitleShowsCountAndLabelFilter();
    void mainWindowFileListSearchFiltersByFilename();
    void mainWindowFileListSearchUsesLabelMeRegex();
    void mainWindowRecentDirectoryMenuKeepsLastTenAndSwitches();
    void mainWindowRecentDirectoryRestoresLastViewedFileInThatFolder();
    void mainWindowRecentDirectoryRestoresLastViewedFileAcrossSessions();
    void mainWindowRecentDirectoryMenuShowsFullPaths();
    void mainWindowFileListShowsVisitedFilesWithDimmedText();
    void mainWindowVShortcutSwitchesToViewMode();
    void mainWindowCtrlJIsViewModeAndVTogglesEditability();
    void mainWindowOnlyActiveCreateActionIsDisabled();
    void mainWindowPShortcutCreatesPolygonShape();
    void mainWindowProvidesLabelMeCreateShortcuts();
    void mainWindowProvidesSynchronizedZoomWidget();
    void mainWindowShortcutZoomKeepsViewportCenter();
    void mainWindowShortcutZoomUsesLabelMeMultiplicativeSteps();
    void mainWindowSpaceFinishesPolygonDraft();
    void mainWindowDisablesEditModeWhileDrawing();
    void mainWindowDisablesShapeActionsWhileDrawing();
    void mainWindowEnablesRemovePointOnlyOnDeletableVertex();
    void mainWindowCanCreateNativePointLineAndCircleShapes();
    void mainWindowCanCreateNativeOrientedRectangleShape();
    void mainWindowCanCreateNativeLinestripShape();
    void mainWindowCanCreateNativeMaskShape();
    void mainWindowExposesLabelMeAiPromptModes();
    void mainWindowDisablesUnsupportedAiPointModels();
    void mainWindowExposesLabelMeAiTextPromptControls();
    void mainWindowShowsAiDownloadProgressAndCanCancel();
    void mainWindowEmptyAiResultCancelsPrompt();
    void mainWindowAiTextPromptCreatesFilteredShapes();
    void mainWindowAiTextPromptIgnoresDifferentExistingLabels();
    void mainWindowQESelectPreviousNextBoxAndToggleSingleSelection();
    void mainWindowXDeletesCurrentShape();
    void mainWindowDeleteAllShapesActionClearsAndUndoRestores();
    void mainWindowDuplicateUsesSelectedShapesWithoutOffset();
    void mainWindowDeleteUsesCanvasSelection();
    void mainWindowShapeActionsIgnoreStaleLabelListSelection();
    void mainWindowCopiesAndPastesSelectedShapes();
    void mainWindowCopiesShapesAsLabelMeClipboardJsonAcrossWindows();
    void mainWindowUndoRedoCreateDeleteAndLabelEdit();
    void mainWindowUndoMoveIsOneHistoryStep();
    void mainWindowUndoCreateIsOneHistoryStep();
    void mainWindowUndoPreservesPointLabelsAndShapeOtherData();
    void mainWindowUndoClearsSelectionLikeLabelMe();
    void mainWindowKeyboardMoveMarksDirtyAndCanUndo();
    void mainWindowRejectsKeyboardDegenerateShapeWithoutDirtying();
    void mainWindowEditLabelDialogDefaultsToCurrentLabel();
    void mainWindowLabelEditDialogUsesCurrentLanguageForMetadataFields();
    void mainWindowBlankLabelKeepsEditDialogOpen();
    void mainWindowLabelEditArrowKeysNavigateLabelList();
    void mainWindowLabelEditCompleterAutocompletesPrefix();
    void mainWindowEditLabelDialogEditsLabelMeMetadata();
    void mainWindowEditLabelDialogAppliesCommonMetadataToMultipleShapes();
    void mainWindowMultiEditDisablesMixedLabelMeFields();
    void mainWindowAppliesLabelMeLabelFlagPresetsWhenLabelChanges();
    void mainWindowParsesLabelMeYamlInlineLabelFlagPresets();
    void mainWindowLoadsLabelMeLabelFlagPresetsFromFilePath();
    void mainWindowEditsLabelMeLabelFlagPresetsDialog();
    void mainWindowEditsExternalLabelMeLabelFlagPresetFile();
    void mainWindowPreservesLabelMeImageDataOnSave();
    void mainWindowDropsLabelMeImageDataWhenSaveWithImageDataDisabled();
    void mainWindowPreservesLabelMeTopLevelFlagsOnSave();
    void mainWindowEditsLabelMeTopLevelFlagsPanel();
    void mainWindowProvidesLabelMeTopLevelFlagsDock();
    void mainWindowLabelListShowsGroupAndEnabledFlags();
    void mainWindowLoadsAnnotationWithoutSelectingShape();
    void mainWindowLabelListKeepsMultiSelectionWhenTogglingVisibility();
    void mainWindowDifficultCheckboxSynchronizesLabelMeFlag();
    void mainWindowDifficultCheckboxUsesCanvasSelection();
    void mainWindowCanvasSelectionScrollsLabelList();
    void mainWindowPreservesLabelMeVersionAndImagePathOnSave();
    void mainWindowPreservesLabelMeTopLevelOtherDataOnSave();
    void mainWindowPreservesLabelMeShapeOtherDataOnSave();
    void mainWindowLoadsEmbeddedLabelMeJsonAsImage();
    void mainWindowLoadsExternalLabelMeJsonAsImage();
    void mainWindowRepairsInvalidLabelMeImageDataWithConfirmation();
    void mainWindowOpenPathAcceptsLabelMeJson();
    void mainWindowOpenAnnotationPreservesExternalLabelMePath();
    void mainWindowOpenAnnotationDetectsCreateMlJson();
    void mainWindowOpenAnnotationPreservesExternalCreateMlPath();
    void mainWindowOpenAnnotationPreservesExternalVocPath();
    void mainWindowAutoDetectsExistingLabelMeFormatBeforeSave();
    void mainWindowLabelListSupportsLabelMeStyleReordering();
    void mainWindowEmbedsSourceImageDataWhenLabelMeSettingEnabled();
    void mainWindowCreateBoxPromptsForLabelUsingLastUsedLabel();
    void mainWindowCanDisableNewShapeLabelPopup();
    void mainWindowShowsLabelPopupWhenPopupDisabledButNoPreferredLabel();
    void mainWindowCancelledLabelPromptRemovesDraft();
    void mainWindowCancelledLabelPromptPreservesExistingDirtyState();
    void mainWindowCancelledOrientedLabelPromptRemovesDraft();
    void mainWindowCancelledMaskLabelPromptRemovesDraft();
    void mainWindowCreateBoxPromptPrefersLastUsedOverDefaultLabel();
    void mainWindowUniqueLabelListSelectsLabelForNewShapeAndEscClears();
    void mainWindowCreatedEmptyShapePromptFallsBackToLastUsedLabel();
    void mainWindowKeepPreviousAnnotationCopiesShapesToEmptyImage();
    void mainWindowKeepPreviousZoomPreservesManualScale();
    void canvasHiddenShapesAreNotEditableOrSelectable();
    void canvasDirectionKeysOnlyMoveShapesInEditMode();
    void mainWindowDefaultsZoomShortcutsLikeLabelMe();
    void mainWindowDefaultsAutoSaveLikeLabelMe();
    void mainWindowSavePromptCanEnableAutoSave();
    void mainWindowAutoSaveWritesDuringEditing();
    void mainWindowStatusBarShowsLoadedImageMessage();
    void mainWindowAutomaticAnnotationFailureReportsError();
    void mainWindowAutomaticAnnotationFailureBlocksOverwriteSave();
    void mainWindowValidAnnotationClearsAutomaticLoadFailure();
    void mainWindowStatusBarShowsErrorForCorruptLabelMeFile();
    void mainWindowStatusBarShowsErrorForSaveFailure();
    void mainWindowTitleShowsDirtyMarkerUntilSave();
    void mainWindowDisablesSaveUntilDirty();
    void mainWindowMissingLabelMeImageReportsErrorAndKeepsState();
    void mainWindowOpenAnnotationReportsLoadFailure();
    void mainWindowSaveFailureBlocksCloseAndKeepsDirty();
    void mainWindowCreatesMissingLabelMeConfigFile();
    void mainWindowLoadsDefaultConfigPathAndMigratesLegacyKeys();
    void mainWindowResetConfigClearsOnlyWindowState();
    void mainWindowAppliesLabelMeCliCanvasOptions();
    void mainWindowMigratesLegacyAiModelConfig();
    void mainWindowAppliesLabelMeConfigFileAndCliLabels();
    void mainWindowAppliesLabelMeLabelColorConfig();
    void mainWindowShapeColorActionsUseCanvasSelection();
    void mainWindowAutoShapeColorUsesImgvizColormap();
    void mainWindowAppliesLabelMeShortcutConfig();
    void mainWindowAppliesLabelMeLabelDialogConfig();
    void mainWindowAppliesLabelMeFitToContentConfig();
    void mainWindowAppliesLabelMeUndoBackupLimit();
    void mainWindowAppliesLabelMeDockConfig();
    void mainWindowSeparatesLabelAndAnnotationDocks();
    void mainWindowAppliesLabelMeCliFlagsAndLabelFlags();
    void mainWindowAcceptsLabelMeConfigStringAndCliAliases();
    void mainWindowAppliesExtendedLabelMeDrawingShortcutsAndCrosshair();
    void mainWindowOutputJsonUsesFixedLabelMePath();
    void mainWindowEditableConfigPersistsSettings();
    void mainWindowSettingsCanEditLabelMeLabels();
    void mainWindowStartupDirectoryPopulatesFileList();
    void mainWindowStartupDirectoryUsesSupportedFormatsAndNaturalSort();
    void mainWindowUpgradesLargeImagePreviewOnZoom();
    void mainWindowLoadsExifOrientationForCanvas();
    void mainWindowAcceptsImageDropAndImportsFiles();
    void mainWindowNextPreviousKeepsFileListSelection();
    void mainWindowCtrlShiftNavigationCopiesPreviousShapes();
    void mainWindowRestoresPersistedLabelHistory();
    void mainWindowRestoresDefaultLabelSettings();
    void mainWindowInlineLabelEditUpdatesLabelHistory();
    void mainWindowLabelFilterOnlyTogglesShapeVisibility();
};

void UiTests::init() {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
}

namespace {
void resetTestSettings(const QString &name) {
    QCoreApplication::setOrganizationName("labelImgCppUiTests");
    QCoreApplication::setApplicationName(name);
    QSettings settings;
    settings.clear();
}

QAction *actionByShortcut(QObject *root, const QKeySequence &shortcut) {
    const QList<QAction *> actions = root->findChildren<QAction *>();
    for (QAction *action : actions) {
        if (action->shortcuts().contains(shortcut)) {
            return action;
        }
    }
    return nullptr;
}

QAction *languageAction(QObject *root, const QString &language) {
    const QList<QAction *> actions = root->findChildren<QAction *>();
    for (QAction *action : actions) {
        if (action->data().toString() == language) {
            return action;
        }
    }
    return nullptr;
}

class RejectDialogOnShow final : public QObject {
public:
    explicit RejectDialogOnShow(bool *seen, QObject *parent = nullptr)
        : QObject(parent), m_seen(seen) {}

protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (event->type() == QEvent::Show) {
            if (auto *dialog = qobject_cast<QDialog *>(watched)) {
                *m_seen = true;
                QTimer::singleShot(0, dialog, &QDialog::reject);
            }
        }
        return QObject::eventFilter(watched, event);
    }

private:
    bool *m_seen = nullptr;
};

class AcceptLabelDialogOnShow final : public QObject {
public:
    AcceptLabelDialogOnShow(bool *seen, QString *observed, QObject *parent = nullptr)
        : QObject(parent), m_seen(seen), m_observed(observed) {}

protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (event->type() == QEvent::Show) {
            if (auto *dialog = qobject_cast<QDialog *>(watched)) {
                auto *combo = dialog->findChild<QComboBox *>(QStringLiteral("labelEditCombo"));
                if (combo) {
                    *m_seen = true;
                    *m_observed = combo->currentText();
                    QTimer::singleShot(0, dialog, &QDialog::accept);
                }
            }
        }
        return QObject::eventFilter(watched, event);
    }

private:
    bool *m_seen = nullptr;
    QString *m_observed = nullptr;
};

class InspectLabelDialogOnShow final : public QObject {
public:
    InspectLabelDialogOnShow(bool *seen, QStringList *labels, QObject *parent = nullptr)
        : QObject(parent), m_seen(seen), m_labels(labels) {}

protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (event->type() == QEvent::Show) {
            if (auto *dialog = qobject_cast<QDialog *>(watched)) {
                *m_seen = true;
                for (QLabel *label : dialog->findChildren<QLabel *>()) {
                    if (!label->text().isEmpty()) {
                        m_labels->append(label->text());
                    }
                }
                for (QPlainTextEdit *edit : dialog->findChildren<QPlainTextEdit *>()) {
                    if (!edit->placeholderText().isEmpty()) {
                        m_labels->append(edit->placeholderText());
                    }
                }
                QTimer::singleShot(0, dialog, &QDialog::accept);
            }
        }
        return QObject::eventFilter(watched, event);
    }

private:
    bool *m_seen = nullptr;
    QStringList *m_labels = nullptr;
};

class AcceptNextLabelPromptOnShow final : public QObject {
public:
    explicit AcceptNextLabelPromptOnShow(QString label, QObject *parent = nullptr)
        : QObject(parent), m_label(label) {}

protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (event->type() == QEvent::Show) {
            if (auto *dialog = qobject_cast<QDialog *>(watched)) {
                auto *combo = dialog->findChild<QComboBox *>(QStringLiteral("labelEditCombo"));
                if (combo) {
                    if (!m_label.isEmpty()) {
                        combo->setCurrentText(m_label);
                    }
                    qApp->removeEventFilter(this);
                    QTimer::singleShot(0, dialog, &QDialog::accept);
                    deleteLater();
                }
            }
        }
        return QObject::eventFilter(watched, event);
    }

private:
    QString m_label;
};

class AcceptColorDialogOnShow final : public QObject {
public:
    AcceptColorDialogOnShow(bool *seen, const QColor &color, QObject *parent = nullptr)
        : QObject(parent), m_seen(seen), m_color(color) {}

protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (event->type() == QEvent::Show) {
            if (auto *dialog = qobject_cast<QColorDialog *>(watched)) {
                *m_seen = true;
                dialog->setCurrentColor(m_color);
                QTimer::singleShot(0, dialog, &QDialog::accept);
            }
        }
        return QObject::eventFilter(watched, event);
    }

private:
    bool *m_seen = nullptr;
    QColor m_color;
};

QString tinyMaskBase64() {
    QImage mask(6, 6, QImage::Format_Grayscale8);
    mask.fill(0);
    for (int y = 2; y <= 3; ++y) {
        for (int x = 2; x <= 3; ++x) {
            mask.setPixelColor(x, y, QColor(255, 255, 255));
        }
    }
    QByteArray pngData;
    QBuffer buffer(&pngData);
    buffer.open(QIODevice::WriteOnly);
    mask.save(&buffer, "PNG");
    return QString::fromLatin1(pngData.toBase64());
}

int countPreviewBoxPixels(const QImage &image) {
    int count = 0;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const QColor color = image.pixelColor(x, y);
            if (color.red() > 220 && color.green() > 170 && color.blue() < 130 && color.alpha() > 150) {
                ++count;
            }
        }
    }
    return count;
}

int countWhitePixelsInRect(const QImage &image, const QRect &rect) {
    int count = 0;
    const QRect bounded = rect.intersected(image.rect());
    for (int y = bounded.top(); y <= bounded.bottom(); ++y) {
        for (int x = bounded.left(); x <= bounded.right(); ++x) {
            const QColor color = image.pixelColor(x, y);
            if (color.alpha() > 200 && color.red() > 245 && color.green() > 245 && color.blue() > 245) {
                ++count;
            }
        }
    }
    return count;
}

void acceptNextLabelPrompt(const QString &label = QString()) {
    auto *filter = new AcceptNextLabelPromptOnShow(label, qApp);
    qApp->installEventFilter(filter);
}
}

void UiTests::canvasCreatesAndCancelsShapes() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateMode(true);
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
    QTest::mouseMove(&canvas, QPoint(50, 40));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 40));

    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.currentIndex(), 0);
    QCOMPARE(canvas.shapes().first().boundingRect().toRect(), QRect(10, 10, 40, 30));

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 60));
    QTest::mouseMove(&canvas, QPoint(80, 80));
    QTest::keyClick(&canvas, Qt::Key_Escape);

    QCOMPARE(canvas.shapes().size(), 1);
}

void UiTests::canvasStartsInEditModeLikeLabelMe() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromRect(QStringLiteral("box"), QRectF(10, 10, 30, 30), false)});
    QCOMPARE(canvas.selectedIndices(), QVector<int>({0}));

    // LabelMe constructs the canvas in EDIT mode. An empty click therefore
    // clears the active shape instead of entering rectangle creation.
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 80));

    QVERIFY(canvas.selectedIndices().isEmpty());
    QCOMPARE(canvas.currentIndex(), -1);
    QCOMPARE(canvas.shapes().size(), 1);
}

void UiTests::canvasAltTemporarilyDisablesPolygonSnapping() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateShapeType(QStringLiteral("polygon"));
    canvas.setCreateMode(true);
    canvas.resize(100, 100);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 20));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 60));
    QVERIFY(canvas.isDrawing());
    QVERIFY(canvas.snapping());

    QKeyEvent altPress(QEvent::KeyPress, Qt::Key_Alt, Qt::AltModifier);
    QApplication::sendEvent(&canvas, &altPress);
    QMouseEvent moveWithoutModifier(QEvent::MouseMove,
                                    QPointF(22, 22),
                                    QPointF(22, 22),
                                    canvas.mapToGlobal(QPoint(22, 22)),
                                    Qt::NoButton,
                                    Qt::NoButton,
                                    Qt::NoModifier);
    QApplication::sendEvent(&canvas, &moveWithoutModifier);
    QMouseEvent pressWithoutModifier(QEvent::MouseButtonPress,
                                     QPointF(22, 22),
                                     QPointF(22, 22),
                                     canvas.mapToGlobal(QPoint(22, 22)),
                                     Qt::LeftButton,
                                     Qt::LeftButton,
                                     Qt::NoModifier);
    QMouseEvent releaseWithoutModifier(QEvent::MouseButtonRelease,
                                       QPointF(22, 22),
                                       QPointF(22, 22),
                                       canvas.mapToGlobal(QPoint(22, 22)),
                                       Qt::LeftButton,
                                       Qt::NoButton,
                                       Qt::NoModifier);
    QApplication::sendEvent(&canvas, &pressWithoutModifier);
    QApplication::sendEvent(&canvas, &releaseWithoutModifier);

    // Alt temporarily disables snapping, but must not change the persisted
    // snapping preference or close the polygon at the nearby first vertex.
    QVERIFY(canvas.isDrawing());
    QCOMPARE(canvas.shapes().size(), 0);
    QVERIFY(canvas.snapping());

    QKeyEvent altRelease(QEvent::KeyRelease, Qt::Key_Alt, Qt::NoModifier);
    QApplication::sendEvent(&canvas, &altRelease);
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(22, 22));
    QCOMPARE(canvas.shapes().size(), 1);
    QVERIFY(!canvas.isDrawing());
    QVERIFY(canvas.snapping());
}

void UiTests::canvasKeepsSeedWhenSwitchingCompatibleCreateMode() {
    Canvas canvas;
    QPixmap pixmap(500, 500);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateMode(true);
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QVERIFY(canvas.isDrawing());
    canvas.setCreateShapeType(QStringLiteral("polygon"));
    QVERIFY(canvas.isDrawing());

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 20));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 50));
    QTest::keyClick(&canvas, Qt::Key_Return);

    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().shapeType, QStringLiteral("polygon"));
    QCOMPARE(canvas.shapes().first().points.first(), QPointF(20, 20));
}

void UiTests::canvasRetypesMovedDraftWithoutDiscardingCursor() {
    Canvas canvas;
    QPixmap pixmap(500, 500);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateMode(true);
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(&canvas, QPoint(50, 40));
    QVERIFY(canvas.isDrawing());

    canvas.setCreateShapeType(QStringLiteral("line"));

    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().shapeType, QStringLiteral("line"));
    QCOMPARE(canvas.shapes().first().points.first(), QPointF(20, 20));
    QCOMPARE(canvas.shapes().first().points.last(), QPointF(50, 40));
    QVERIFY(canvas.isDrawing());

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 40));

    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().points.first(), QPointF(20, 20));
    QCOMPARE(canvas.shapes().first().points.last(), QPointF(50, 40));
    QVERIFY(!canvas.isDrawing());
}

void UiTests::canvasRetypesMovedDraftIntoPolygonAtLastCursor() {
    Canvas canvas;
    QPixmap pixmap(500, 500);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateMode(true);
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(&canvas, QPoint(50, 40));
    canvas.setCreateShapeType(QStringLiteral("polygon"));

    QCOMPARE(canvas.shapes().size(), 0);
    QVERIFY(canvas.isDrawing());
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 40));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 40));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));

    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().shapeType, QStringLiteral("polygon"));
    QCOMPARE(canvas.shapes().first().points.first(), QPointF(20, 20));
    QCOMPARE(canvas.shapes().first().points.at(1), QPointF(50, 40));
    QVERIFY(!canvas.isDrawing());
}

void UiTests::canvasCreateModeUsesCrossCursorAndIgnoresTinyClicks() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateMode(true);
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QCOMPARE(canvas.cursor().shape(), Qt::CrossCursor);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(15, 15));
    QCOMPARE(canvas.shapes().size(), 0);

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(&canvas, QPoint(22, 22));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(22, 22));
    QCOMPARE(canvas.shapes().size(), 0);
}

void UiTests::canvasSwitchingToCreateModeClearsSelection() {
    Canvas canvas;
    canvas.setPixmap(QPixmap(120, 80));
    Shape shape = Shape::fromPolygon(QStringLiteral("poly"),
                                     {QPointF(10, 10), QPointF(60, 10), QPointF(40, 50)},
                                     false);
    canvas.setShapes({shape});
    canvas.setEditMode();
    canvas.setSelectedIndices({0});
    QVERIFY(canvas.hasSelection());

    canvas.setCreateMode(true);

    QVERIFY2(canvas.selectedIndices().isEmpty(),
             "Entering create mode must clear the existing shape selection");
    QCOMPARE(canvas.currentIndex(), -1);
}

void UiTests::canvasCreateModeIgnoresClicksOutsidePixmap() {
    auto *canvas = new Canvas;
    QScrollArea scrollArea;
    scrollArea.setWidgetResizable(false);
    scrollArea.setWidget(canvas);
    scrollArea.resize(220, 220);

    scrollArea.show();
    QVERIFY(QTest::qWaitForWindowExposed(&scrollArea));

    QPixmap pixmap(500, 500);
    pixmap.fill(Qt::white);
    canvas->setPixmap(pixmap);
    canvas->setCreateMode(true);
    QVERIFY(canvas->imageOriginOffset().x() > 0.0);

    // The canvas includes centered overscroll slack. This point is inside the
    // canvas widget but outside the image itself.
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(4, 4));
    QTest::mouseMove(canvas, QPoint(70, 70));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(70, 70));

    QCOMPARE(canvas->shapes().size(), 0);
    QVERIFY(!canvas->isDrawing());
}

void UiTests::canvasRejectsDegeneratePolygonDraft() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateShapeType(QStringLiteral("polygon"));
    canvas.setCreateMode(true);
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 20));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::keyClick(&canvas, Qt::Key_Return);

    QCOMPARE(canvas.shapes().size(), 0);
    QVERIFY(!canvas.isDrawing());
}

void UiTests::canvasRejectsZeroLengthBasicShapeOnKeyboardFinish() {
    const QStringList shapeTypes = {
        QStringLiteral("rectangle"), QStringLiteral("line"), QStringLiteral("circle")};
    for (const QString &shapeType : shapeTypes) {
        Canvas canvas;
        QPixmap pixmap(100, 100);
        pixmap.fill(Qt::white);
        canvas.setPixmap(pixmap);
        canvas.setCreateShapeType(shapeType);
        canvas.setCreateMode(true);
        canvas.resize(120, 120);
        canvas.show();
        QVERIFY(QTest::qWaitForWindowExposed(&canvas));

        QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(30, 30));
        QTest::keyClick(&canvas, Qt::Key_Return);

        QCOMPARE(canvas.shapes().size(), 0);
        QVERIFY(!canvas.isDrawing());
    }
}

void UiTests::canvasRejectsDegenerateOrientedRectangleDraft() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateShapeType(QStringLiteral("oriented_rectangle"));
    canvas.setCreateMode(true);
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(30, 30));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(30, 30));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 60));

    QCOMPARE(canvas.shapes().size(), 0);
    QVERIFY(!canvas.isDrawing());
}

void UiTests::canvasShiftCreatesLabelMeSquare() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateMode(true);
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::ShiftModifier, QPoint(10, 10));
    QMouseEvent moveEvent(QEvent::MouseMove, QPointF(40, 30),
                          canvas.mapToGlobal(QPoint(40, 30)), Qt::NoButton,
                          Qt::LeftButton, Qt::ShiftModifier);
    QApplication::sendEvent(&canvas, &moveEvent);
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::ShiftModifier, QPoint(40, 30));

    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().boundingRect(), QRectF(10, 10, 20, 20));
}

void UiTests::canvasShiftResizesRectangleAsSquare() {
    Canvas canvas;
    QPixmap pixmap(120, 120);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    Shape shape = Shape::fromRect(QStringLiteral("defect"), QRectF(20, 20, 30, 20), false);
    shape.groupId = 7;
    shape.description = QStringLiteral("keep metadata");
    shape.flags.insert(QStringLiteral("reviewed"), true);
    canvas.setShapes({shape});
    canvas.setEditMode();
    canvas.resize(140, 140);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::ShiftModifier, QPoint(50, 40));
    QMouseEvent moveEvent(QEvent::MouseMove, QPointF(80, 70),
                          canvas.mapToGlobal(QPoint(80, 70)), Qt::NoButton,
                          Qt::LeftButton, Qt::ShiftModifier);
    QApplication::sendEvent(&canvas, &moveEvent);
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::ShiftModifier, QPoint(80, 70));

    QCOMPARE(canvas.shapes().first().boundingRect(), QRectF(20, 20, 50, 50));
    QCOMPARE(canvas.shapes().first().groupId, 7);
    QCOMPARE(canvas.shapes().first().description, QStringLiteral("keep metadata"));
    QCOMPARE(canvas.shapes().first().flags.value(QStringLiteral("reviewed")), true);
}

void UiTests::canvasCreateModeCrosshairCanBeToggled() {
    Canvas canvas;
    QVERIFY(canvas.crosshairEnabled());
    canvas.setCrosshairEnabled(false);
    QVERIFY(!canvas.crosshairEnabled());
    canvas.setCrosshairEnabled(true);
    QVERIFY(canvas.crosshairEnabled());
}

void UiTests::canvasCrosshairCanBeConfiguredPerCreateShape() {
    Canvas canvas;
    QVERIFY(canvas.crosshairEnabledForShapeType(QStringLiteral("rectangle")));
    QVERIFY(!canvas.crosshairEnabledForShapeType(QStringLiteral("polygon")));
    QVERIFY(!canvas.crosshairEnabledForShapeType(QStringLiteral("circle")));
    canvas.setCrosshairEnabledForShapeType(QStringLiteral("rectangle"), false);
    canvas.setCrosshairEnabledForShapeType(QStringLiteral("polygon"), true);
    QVERIFY(!canvas.crosshairEnabledForShapeType(QStringLiteral("rectangle")));
    QVERIFY(canvas.crosshairEnabledForShapeType(QStringLiteral("polygon")));
    QVERIFY(!canvas.crosshairEnabledForShapeType(QStringLiteral("circle")));
}

void UiTests::canvasDoubleClickCloseCanBeToggled() {
    Canvas canvas;
    QVERIFY(canvas.doubleClickClose());
    canvas.setDoubleClickClose(false);
    QVERIFY(!canvas.doubleClickClose());
    canvas.setDoubleClickClose(true);
    QVERIFY(canvas.doubleClickClose());
}

void UiTests::canvasPointSizeCanBeConfigured() {
    Canvas canvas;
    QCOMPARE(canvas.pointSize(), 8);
    canvas.setPointSize(12);
    QCOMPARE(canvas.pointSize(), 12);
    canvas.setPointSize(0);
    QCOMPARE(canvas.pointSize(), 1);
}

void UiTests::canvasLabelMePaletteCanBeConfigured() {
    Canvas canvas;
    canvas.setVertexFillColor(QColor(10, 20, 30, 40));
    canvas.setHoverVertexFillColor(QColor(50, 60, 70, 80));
    canvas.setSelectedLineColor(QColor(90, 100, 110, 120));
    canvas.setSelectedFillColor(QColor(130, 140, 150, 160));
    QCOMPARE(canvas.vertexFillColor(), QColor(10, 20, 30, 40));
    QCOMPARE(canvas.hoverVertexFillColor(), QColor(50, 60, 70, 80));
    QCOMPARE(canvas.selectedLineColor(), QColor(90, 100, 110, 120));
    QCOMPARE(canvas.selectedFillColor(), QColor(130, 140, 150, 160));
}

void UiTests::canvasEpsilonCanBeConfigured() {
    Canvas canvas;
    QCOMPARE(canvas.epsilon(), 10.0);
    canvas.setEpsilon(18.0);
    QCOMPARE(canvas.epsilon(), 18.0);
    canvas.setEpsilon(-1.0);
    QCOMPARE(canvas.epsilon(), 0.0);
}

void UiTests::canvasTinyCreateClickDoesNotEmitShapeChange() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateMode(true);
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QSignalSpy shapeSpy(&canvas, &Canvas::shapesChanged);

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(&canvas, QPoint(22, 22));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(22, 22));

    QCOMPARE(canvas.shapes().size(), 0);
    QCOMPARE(shapeSpy.count(), 0);
}

void UiTests::canvasRightDragPansButRightClickOpensMenu() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QSignalSpy scrollSpy(&canvas, &Canvas::scrollRequested);
    QSignalSpy menuSpy(&canvas, &Canvas::contextMenuRequested);

    QTest::mousePress(&canvas, Qt::RightButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(&canvas, QPoint(45, 45));
    QTest::mouseMove(&canvas, QPoint(48, 47));
    QTest::mouseRelease(&canvas, Qt::RightButton, Qt::NoModifier, QPoint(48, 47));

    QVERIFY(scrollSpy.count() > 0);
    QCOMPARE(menuSpy.count(), 0);

    QTest::mouseClick(&canvas, Qt::RightButton, Qt::NoModifier, QPoint(30, 30));
    QCOMPARE(menuSpy.count(), 1);
}

void UiTests::canvasMiddleDragPansWithoutCreatingOrMovingShapes() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QSignalSpy scrollSpy(&canvas, &Canvas::scrollRequested);
    QSignalSpy shapeSpy(&canvas, &Canvas::shapesChanged);

    QTest::mousePress(&canvas, Qt::MiddleButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(&canvas, QPoint(23, 23));
    QTest::mouseMove(&canvas, QPoint(36, 36));
    QTest::mouseMove(&canvas, QPoint(40, 38));
    QTest::mouseRelease(&canvas, Qt::MiddleButton, Qt::NoModifier, QPoint(40, 38));

    QCOMPARE(canvas.shapes().size(), 0);
    QCOMPARE(shapeSpy.count(), 0);
    QCOMPARE(scrollSpy.count(), 1);
    QCOMPARE(scrollSpy.first().at(0).toInt(), 4);
    QCOMPARE(scrollSpy.first().at(1).toInt(), 2);
}

void UiTests::canvasMiddleDragDoesNotPanWhenImageFitsViewport() {
    QScrollArea scrollArea;
    scrollArea.resize(240, 220);
    auto *canvas = new Canvas;
    QPixmap pixmap(100, 80);
    pixmap.fill(Qt::white);
    canvas->setPixmap(pixmap);
    scrollArea.setWidget(canvas);
    scrollArea.show();
    QVERIFY(QTest::qWaitForWindowExposed(&scrollArea));

    QSignalSpy scrollSpy(canvas, &Canvas::scrollRequested);
    const QPoint start(canvas->width() / 2, canvas->height() / 2);
    const QPoint end = start + QPoint(40, 40);
    QTest::mousePress(canvas, Qt::MiddleButton, Qt::NoModifier, start);
    QTest::mouseMove(canvas, end);
    QTest::mouseRelease(canvas, Qt::MiddleButton, Qt::NoModifier, end);

    QCOMPARE(scrollSpy.count(), 0);
}

void UiTests::canvasRightClickOnShapeSelectsItBeforeMenu() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromRect(QStringLiteral("defect"), QRectF(20, 20, 40, 30), false)});
    canvas.setEditMode();
    canvas.setCurrentIndex(-1);
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QSignalSpy menuSpy(&canvas, &Canvas::contextMenuRequested);
    QTest::mouseClick(&canvas, Qt::RightButton, Qt::NoModifier, QPoint(35, 35));

    QCOMPARE(menuSpy.count(), 1);
    QCOMPARE(canvas.currentIndex(), 0);
}

void UiTests::canvasLeftDragPansEmptyImageInEditMode() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setEditMode();
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QSignalSpy scrollSpy(&canvas, &Canvas::scrollRequested);
    QSignalSpy shapeSpy(&canvas, &Canvas::shapesChanged);

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(&canvas, QPoint(23, 23));
    QTest::mouseMove(&canvas, QPoint(36, 36));
    QTest::mouseMove(&canvas, QPoint(40, 38));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(40, 38));

    QCOMPARE(canvas.shapes().size(), 0);
    QCOMPARE(shapeSpy.count(), 0);
    QCOMPARE(scrollSpy.count(), 1);
    QCOMPARE(scrollSpy.first().at(0).toInt(), 4);
    QCOMPARE(scrollSpy.first().at(1).toInt(), 2);
}

void UiTests::canvasLeftDragStillCreatesBoxInCreateMode() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateMode(true);
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QSignalSpy scrollSpy(&canvas, &Canvas::scrollRequested);

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(&canvas, QPoint(50, 45));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 45));

    QCOMPARE(scrollSpy.count(), 0);
    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().boundingRect().toRect(), QRect(20, 20, 30, 25));
}

void UiTests::canvasMoveKeepsGrabAnchorAtImageBoundary() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setEditMode();
    canvas.setShapes({Shape::fromRect(QStringLiteral("box"), QRectF(10, 10, 40, 40), false)});
    canvas.resize(100, 100);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    const auto widgetPoint = [&canvas](const QPointF &imagePoint) {
        return (canvas.imageOriginOffset() + imagePoint * canvas.scale()).toPoint();
    };

    QTest::mousePress(&canvas,
                      Qt::LeftButton,
                      Qt::NoModifier,
                      widgetPoint(QPointF(30, 30)));
    QTest::mouseMove(&canvas, widgetPoint(QPointF(0, 30)));
    QCOMPARE(canvas.shapes().first().boundingRect().topLeft(), QPointF(0, 10));

    // The grabbed point is still 10 px inside the box. Moving the cursor
    // back toward the image must not make the box jump away from the edge.
    QTest::mouseMove(&canvas, widgetPoint(QPointF(10, 30)));
    QCOMPARE(canvas.shapes().first().boundingRect().topLeft(), QPointF(0, 10));

    QTest::mouseMove(&canvas, widgetPoint(QPointF(30, 30)));
    QCOMPARE(canvas.shapes().first().boundingRect().topLeft(), QPointF(10, 10));
    QTest::mouseRelease(&canvas,
                        Qt::LeftButton,
                        Qt::NoModifier,
                        widgetPoint(QPointF(30, 30)));
}

void UiTests::canvasPointCreateModeCreatesPointOnClick() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateShapeType(QStringLiteral("point"));
    canvas.setCreateMode(true);
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QSignalSpy createdSpy(&canvas, &Canvas::shapeCreated);
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(30, 25));

    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().shapeType, QStringLiteral("point"));
    QCOMPARE(canvas.shapes().first().points, QVector<QPointF>{QPointF(30, 25)});
    QCOMPARE(createdSpy.count(), 1);
}

void UiTests::canvasPointsShapeRendersAndMovesOnePoint() {
    Canvas canvas;
    QPixmap pixmap(100, 80);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromPoints(QStringLiteral("landmarks"),
                                         QStringLiteral("points"),
                                         {QPointF(20, 20), QPointF(40, 40), QPointF(60, 20)},
                                         false)});
    canvas.setCurrentIndex(0);
    canvas.setEditMode();
    canvas.resize(100, 80);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(40, 40));
    QTest::mouseMove(&canvas, QPoint(45, 45));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(45, 45));

    const Shape shape = canvas.shapes().first();
    QCOMPARE(shape.shapeType, QStringLiteral("points"));
    QCOMPARE(shape.points, QVector<QPointF>({QPointF(20, 20), QPointF(45, 45), QPointF(60, 20)}));
}

void UiTests::canvasSelectedPointsUseSingleMarker() {
    const QVector<QPointF> points{QPointF(20, 20), QPointF(45, 45), QPointF(70, 20)};
    auto renderFirstMarker = [&points](bool selected) {
        Canvas canvas;
        QPixmap pixmap(100, 80);
        pixmap.fill(Qt::white);
        canvas.setPixmap(pixmap);
        Shape pointShape = Shape::fromPoints(QStringLiteral("landmarks"),
                                             QStringLiteral("points"),
                                             points,
                                             false);
        pointShape.lineColor = QColor(255, 0, 0, 255);
        canvas.setShapes({pointShape});
        canvas.setPointSize(4);
        canvas.setSelectedLineColor(QColor(0, 0, 255, 255));
        canvas.setVertexFillColor(QColor(255, 0, 0, 255));
        canvas.setCurrentIndex(selected ? 0 : -1);
        canvas.resize(100, 80);
        QImage rendered(100, 80, QImage::Format_ARGB32);
        rendered.fill(Qt::white);
        QPainter painter(&rendered);
        canvas.render(&painter);
        painter.end();

        int redPixels = 0;
        for (int y = 14; y <= 26; ++y) {
            for (int x = 14; x <= 26; ++x) {
                const QColor pixel = rendered.pixelColor(x, y);
                if (pixel.red() > 180 && pixel.green() < 80 && pixel.blue() < 80) {
                    ++redPixels;
                }
            }
        }
        return redPixels;
    };

    const int unselectedPixels = renderFirstMarker(false);
    const int selectedPixels = renderFirstMarker(true);
    QVERIFY(unselectedPixels > 0);
    QVERIFY2(qAbs(selectedPixels - unselectedPixels) <= 2,
             "Selecting a points shape must not add a second oversized marker");
}

void UiTests::canvasPointsCreateModeFinishesWithReturn() {
    Canvas canvas;
    QPixmap pixmap(120, 90);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateShapeType(QStringLiteral("points"));
    canvas.setCreateMode(true);
    canvas.resize(120, 90);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 45));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(85, 25));
    QTest::keyClick(&canvas, Qt::Key_Return);

    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().shapeType, QStringLiteral("points"));
    QCOMPARE(canvas.shapes().first().points,
             QVector<QPointF>({QPointF(20, 20), QPointF(50, 45), QPointF(85, 25)}));
}

void UiTests::canvasPointsCreateModeFinishesWithSpace() {
    Canvas canvas;
    QPixmap pixmap(120, 90);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateShapeType(QStringLiteral("points"));
    canvas.setCreateMode(true);
    canvas.resize(120, 90);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 45));
    QTest::keyClick(&canvas, Qt::Key_Space);

    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().shapeType, QStringLiteral("points"));
    QCOMPARE(canvas.shapes().first().points,
             QVector<QPointF>({QPointF(20, 20), QPointF(50, 45)}));
}

void UiTests::canvasLinestripCreateModeFinishesWithCtrlClick() {
    Canvas canvas;
    QPixmap pixmap(120, 90);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateShapeType(QStringLiteral("linestrip"));
    canvas.setCreateMode(true);
    canvas.resize(120, 90);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::ControlModifier, QPoint(85, 50));

    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().shapeType, QStringLiteral("linestrip"));
    QCOMPARE(canvas.shapes().first().points,
             QVector<QPointF>({QPointF(20, 20), QPointF(85, 50)}));
    QVERIFY(!canvas.isDrawing());
}

void UiTests::canvasAiPointsUseShiftNegativeLabelsAndCtrlFinishes() {
    Canvas canvas;
    QPixmap pixmap(120, 90);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateShapeType(QStringLiteral("ai_points_to_shape"));
    canvas.setCreateMode(true);
    canvas.resize(120, 90);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::ShiftModifier, QPoint(20, 20));
    QCOMPARE(canvas.promptPointLabels(), QVector<int>({0}));
    QCOMPARE(canvas.shapes().size(), 0);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::ControlModifier, QPoint(80, 60));

    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().shapeType, QStringLiteral("points"));
    QCOMPARE(canvas.shapes().first().points,
             QVector<QPointF>({QPointF(20, 20), QPointF(80, 60)}));
    QCOMPARE(canvas.shapes().first().pointLabels, QVector<int>({0, 1}));
    QCOMPARE(canvas.promptPointLabels(), QVector<int>());
}

void UiTests::canvasAiPointsRenderNegativePromptMarker() {
    Canvas canvas;
    QPixmap pixmap(100, 80);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateShapeType(QStringLiteral("ai_points_to_shape"));
    canvas.setCreateMode(true);
    canvas.setCrosshairEnabled(false);
    canvas.setLineColor(QColor(0, 255, 0, 255));
    canvas.resize(100, 80);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::ShiftModifier, QPoint(60, 40));
    QApplication::processEvents();

    const QImage rendered = canvas.grab().toImage();
    const qreal scaleX = rendered.width() / qreal(canvas.width());
    const qreal scaleY = rendered.height() / qreal(canvas.height());
    const auto renderedPoint = [&](const QPointF &point) {
        return QPoint(qBound(0, qRound(point.x() * scaleX), rendered.width() - 1),
                      qBound(0, qRound(point.y() * scaleY), rendered.height() - 1));
    };
    const QColor positive = rendered.pixelColor(renderedPoint(QPointF(20, 20)));
    const QColor negative = rendered.pixelColor(renderedPoint(QPointF(60, 40)));
    QVERIFY2(positive.green() > positive.red(), qPrintable(positive.name()));
    QVERIFY2(negative.red() > negative.green(), qPrintable(negative.name()));
}

void UiTests::canvasUndoLastDrawingPointWithBackspace() {
    Canvas canvas;
    QPixmap pixmap(120, 90);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateShapeType(QStringLiteral("polygon"));
    canvas.setCreateMode(true);
    canvas.resize(120, 90);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(15, 15));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(70, 15));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 60));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 65));
    QTest::keyClick(&canvas, Qt::Key_Backspace);
    QTest::keyClick(&canvas, Qt::Key_Return);

    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().points,
             QVector<QPointF>({QPointF(15, 15), QPointF(70, 15), QPointF(80, 60)}));
}

void UiTests::canvasUndoLastDrawingPoint() {
    Canvas canvas;
    QPixmap pixmap(120, 90);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateShapeType(QStringLiteral("polygon"));
    canvas.setCreateMode(true);
    canvas.resize(120, 90);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(15, 15));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(65, 15));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(70, 60));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 65));

    QVERIFY(canvas.undoLastDrawingPoint());
    QTest::keyClick(&canvas, Qt::Key_Return);

    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().shapeType, QStringLiteral("polygon"));
    QCOMPARE(canvas.shapes().first().points,
             QVector<QPointF>({QPointF(15, 15), QPointF(65, 15), QPointF(70, 60)}));
    QVERIFY(!canvas.undoLastDrawingPoint());
}

void UiTests::canvasUndoLastBasicShapeDraftKeepsAnchor() {
    Canvas canvas;
    QPixmap pixmap(120, 90);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateShapeType(QStringLiteral("rectangle"));
    canvas.setCreateMode(true);
    canvas.resize(120, 90);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(&canvas, QPoint(70, 55));
    QVERIFY(canvas.isDrawing());
    QVERIFY(canvas.undoLastDrawingPoint());
    QVERIFY(canvas.isDrawing());
    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().points,
             QVector<QPointF>({QPointF(20, 20), QPointF(20, 20)}));

    QTest::mouseMove(&canvas, QPoint(70, 55));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(70, 55));
    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().boundingRect(), QRectF(QPointF(20, 20), QPointF(70, 55)));
}

void UiTests::canvasPointsShapeHitTestingUsesScreenMarkerSize() {
    Canvas canvas;
    QPixmap pixmap(120, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromPoints(QStringLiteral("landmarks"),
                                        QStringLiteral("points"),
                                        {QPointF(40, 40)},
                                        false)});
    canvas.setCurrentIndex(-1);
    canvas.setPointSize(8);
    canvas.setScale(2.0);
    canvas.setViewMode();
    canvas.resize(canvas.sizeHint());
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    // The point center is at (80, 80) in widget pixels. At 200% zoom,
    // (85, 80) is five screen pixels away and must miss an 8px marker.
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(85, 80));
    QVERIFY(canvas.selectedIndices().isEmpty());

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(83, 80));
    QCOMPARE(canvas.selectedIndices(), QVector<int>({0}));
}

void UiTests::canvasPointMarkerScalesWithImageAtLowZoom() {
    Canvas canvas;
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);

    Shape point = Shape::fromPoints(QStringLiteral("point"),
                                    QStringLiteral("point"),
                                    {QPointF(32, 32)},
                                    false);
    point.lineColor = QColor(255, 0, 0, 255);
    canvas.setShapes({point});
    canvas.setCurrentIndex(-1);
    canvas.setPointSize(8);
    canvas.setScale(0.5);
    canvas.resize(32, 32);

    QImage rendered(32, 32, QImage::Format_ARGB32);
    rendered.fill(Qt::white);
    QPainter painter(&rendered);
    canvas.render(&painter);
    painter.end();

    int coloredPixels = 0;
    for (int y = 0; y < rendered.height(); ++y) {
        for (int x = 0; x < rendered.width(); ++x) {
            if (rendered.pixelColor(x, y).red() > 180 &&
                rendered.pixelColor(x, y).green() < 80) {
                ++coloredPixels;
            }
        }
    }

    // Point size is measured in image pixels like LabelMe. At 50% zoom an
    // 8px marker therefore occupies only about a 4px screen diameter.
    QVERIFY(coloredPixels > 0);
    QVERIFY(coloredPixels <= 30);
}

void UiTests::canvasLineAndCircleCreateModesUseTwoPoints() {
    const QVector<QString> shapeTypes{QStringLiteral("line"), QStringLiteral("circle")};
    for (const QString &shapeType : shapeTypes) {
        Canvas canvas;
        QPixmap pixmap(100, 100);
        pixmap.fill(Qt::white);
        canvas.setPixmap(pixmap);
        canvas.setCreateShapeType(shapeType);
        canvas.setCreateMode(true);
        canvas.resize(120, 120);
        canvas.show();
        QVERIFY(QTest::qWaitForWindowExposed(&canvas));

        QSignalSpy createdSpy(&canvas, &Canvas::shapeCreated);
        QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
        QTest::mouseMove(&canvas, QPoint(70, 45));
        QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(70, 45));

        QCOMPARE(canvas.shapes().size(), 1);
        QCOMPARE(canvas.shapes().first().shapeType, shapeType);
        QCOMPARE(canvas.shapes().first().points, QVector<QPointF>({QPointF(20, 20), QPointF(70, 45)}));
        QCOMPARE(createdSpy.count(), 1);
    }
}

void UiTests::canvasOrientedRectangleCreateModeUsesThreeClicks() {
    Canvas canvas;
    QPixmap pixmap(140, 120);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateShapeType(QStringLiteral("oriented_rectangle"));
    canvas.setCreateMode(true);
    canvas.resize(160, 140);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QSignalSpy createdSpy(&canvas, &Canvas::shapeCreated);
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 20));
    QCOMPARE(canvas.shapes().size(), 0);
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(85, 55));

    QCOMPARE(canvas.shapes().size(), 1);
    const Shape shape = canvas.shapes().first();
    QCOMPARE(shape.shapeType, QStringLiteral("oriented_rectangle"));
    QCOMPARE(shape.points.size(), 4);
    QCOMPARE(shape.points[0], QPointF(20, 20));
    QCOMPARE(shape.points[1], QPointF(80, 20));
    QCOMPARE(shape.points[2], QPointF(80, 55));
    QCOMPARE(shape.points[3], QPointF(20, 55));
    QCOMPARE(createdSpy.count(), 1);
}

void UiTests::canvasOrientedRectangleCreateModeFinishesWithReturn() {
    Canvas canvas;
    QPixmap pixmap(140, 120);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateShapeType(QStringLiteral("oriented_rectangle"));
    canvas.setCreateMode(true);
    canvas.resize(160, 140);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 20));
    QTest::mouseMove(&canvas, QPoint(85, 55));
    QTest::keyClick(&canvas, Qt::Key_Return);

    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().shapeType, QStringLiteral("oriented_rectangle"));
    QCOMPARE(canvas.shapes().first().points,
             QVector<QPointF>({QPointF(20, 20), QPointF(80, 20),
                               QPointF(80, 55), QPointF(20, 55)}));
    QVERIFY(!canvas.isDrawing());
}

void UiTests::canvasOrientedRectangleCreateModeFinishesWithDoubleClick() {
    Canvas canvas;
    QPixmap pixmap(140, 120);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateShapeType(QStringLiteral("oriented_rectangle"));
    canvas.setCreateMode(true);
    canvas.resize(160, 140);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 20));
    QTest::mouseMove(&canvas, QPoint(85, 55));
    QMouseEvent doubleClickEvent(QEvent::MouseButtonDblClick,
                                 QPointF(85, 55),
                                 canvas.mapToGlobal(QPoint(85, 55)),
                                 Qt::LeftButton,
                                 Qt::LeftButton,
                                 Qt::NoModifier);
    QApplication::sendEvent(&canvas, &doubleClickEvent);

    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().shapeType, QStringLiteral("oriented_rectangle"));
    QVERIFY(!canvas.isDrawing());
}

void UiTests::canvasRendersOrientedRectangleDirectionArrow() {
    Canvas canvas;
    QPixmap pixmap(120, 90);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setVertexFillColor(QColor(255, 0, 0, 255));

    Shape oriented = Shape::fromPoints(QStringLiteral("direction"),
                                       QStringLiteral("oriented_rectangle"),
                                       {QPointF(30, 30), QPointF(70, 30),
                                        QPointF(70, 50), QPointF(30, 50)},
                                       false);
    oriented.lineColor = QColor(255, 0, 0, 255);
    canvas.setShapes({oriented});
    canvas.setSelectedIndices({});
    canvas.resize(120, 90);

    QImage rendered(120, 90, QImage::Format_ARGB32);
    rendered.fill(Qt::white);
    QPainter painter(&rendered);
    canvas.render(&painter);
    painter.end();

    const QColor arrowTip = rendered.pixelColor(55, 40);
    QVERIFY2(arrowTip.red() > arrowTip.green() + 40,
             qPrintable(QStringLiteral("expected direction arrow at (55,40), got %1")
                            .arg(arrowTip.name(QColor::HexArgb))));
}

void UiTests::canvasMaskCreateModeCreatesNativeMask() {
    Canvas canvas;
    QPixmap pixmap(100, 80);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateShapeType(QStringLiteral("mask"));
    canvas.setCreateMode(true);
    canvas.resize(120, 100);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(&canvas, QPoint(35, 30));
    QTest::mouseMove(&canvas, QPoint(55, 42));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(55, 42));

    QCOMPARE(canvas.shapes().size(), 1);
    const Shape shape = canvas.shapes().first();
    QCOMPARE(shape.shapeType, QStringLiteral("mask"));
    QCOMPARE(shape.points.size(), 2);
    QVERIFY(!shape.maskData.isEmpty());
    QImage mask;
    QVERIFY(mask.loadFromData(QByteArray::fromBase64(shape.maskData.toLatin1()), "PNG"));
    QVERIFY(!mask.isNull());
    QVERIFY(shape.boundingRect().contains(QPointF(35, 30)));
    QVERIFY(shape.boundingRect().contains(QPointF(55, 42)));
}

void UiTests::canvasRendersMaskOnlyOnNonZeroPixels() {
    Canvas canvas;
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.resize(32, 32);

    Shape shape = Shape::fromPoints(QStringLiteral("mask"),
                                    QStringLiteral("mask"),
                                    {QPointF(10, 10), QPointF(15, 15)},
                                    false);
    shape.maskData = tinyMaskBase64();
    shape.lineColor = Qt::transparent;
    shape.fillColor = QColor(255, 0, 0, 220);
    canvas.setShapes({shape});
    canvas.setCurrentIndex(-1);

    QImage rendered(canvas.size(), QImage::Format_ARGB32);
    rendered.fill(Qt::transparent);
    QPainter painter(&rendered);
    canvas.render(&painter);

    const QColor falseMaskPixel(rendered.pixelColor(11, 14));
    const QColor trueMaskPixel(rendered.pixelColor(12, 12));
    QCOMPARE(falseMaskPixel, QColor(Qt::white));
    QVERIFY(trueMaskPixel != QColor(Qt::white));
}

void UiTests::canvasRendersMaskBoundaryOutline() {
    Canvas canvas;
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.resize(32, 32);

    Shape shape = Shape::fromPoints(QStringLiteral("mask"),
                                    QStringLiteral("mask"),
                                    {QPointF(10, 10), QPointF(15, 15)},
                                    false);
    shape.maskData = tinyMaskBase64();
    shape.lineColor = QColor(255, 0, 0, 255);
    shape.fillColor = Qt::transparent;
    canvas.setShapes({shape});
    canvas.setCurrentIndex(-1);

    QImage rendered(canvas.size(), QImage::Format_ARGB32);
    rendered.fill(Qt::transparent);
    QPainter painter(&rendered);
    canvas.render(&painter);

    const QColor boundaryPixel = rendered.pixelColor(13, 12);
    QVERIFY2(boundaryPixel.red() > 180 && boundaryPixel.green() < 100 && boundaryPixel.blue() < 100,
             qPrintable(QStringLiteral("expected red mask boundary, got %1").arg(boundaryPixel.name())));
}

void UiTests::canvasRendersMaskWithoutBitmapAsBoundingBox() {
    Canvas canvas;
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.resize(32, 32);

    Shape shape = Shape::fromPoints(QStringLiteral("mask"),
                                    QStringLiteral("mask"),
                                    {QPointF(10, 10), QPointF(20, 20)},
                                    false);
    shape.lineColor = QColor(255, 0, 0, 255);
    shape.fillColor = Qt::transparent;
    canvas.setShapes({shape});
    canvas.setCurrentIndex(-1);

    QImage rendered(canvas.size(), QImage::Format_ARGB32);
    rendered.fill(Qt::transparent);
    QPainter painter(&rendered);
    canvas.render(&painter);

    const QColor edge = rendered.pixelColor(10, 15);
    QVERIFY2(edge.red() > 180 && edge.green() < 100 && edge.blue() < 100,
             qPrintable(QStringLiteral("expected bbox fallback edge, got %1").arg(edge.name())));
}

void UiTests::canvasMaskDoesNotResizeFromBoundary() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);

    Shape shape = Shape::fromPoints(QStringLiteral("mask-label"),
                                    QStringLiteral("mask"),
                                    {QPointF(20, 20), QPointF(40, 40)},
                                    true);
    shape.maskData = tinyMaskBase64();
    shape.maskPresent = true;
    shape.description = QStringLiteral("mask description");
    shape.flags.insert(QStringLiteral("verified"), true);
    shape.labelMeOtherData.insert(QStringLiteral("source"), QStringLiteral("native"));
    canvas.setShapes({shape});
    canvas.setCurrentIndex(0);
    canvas.setEditMode();
    canvas.resize(100, 100);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(&canvas, QPoint(10, 10));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));

    QCOMPARE(canvas.shapes().size(), 1);
    const Shape resized = canvas.shapes().first();
    QCOMPARE(resized.shapeType, QStringLiteral("mask"));
    QCOMPARE(resized.label, QStringLiteral("mask-label"));
    QVERIFY(resized.difficult);
    QCOMPARE(resized.description, QStringLiteral("mask description"));
    QCOMPARE(resized.flags.value(QStringLiteral("verified")), true);
    QCOMPARE(resized.labelMeOtherData.value(QStringLiteral("source")).toString(), QStringLiteral("native"));
    QVERIFY(!resized.maskData.isEmpty());

    QImage mask;
    QVERIFY(mask.loadFromData(QByteArray::fromBase64(resized.maskData.toLatin1()), "PNG"));
    QCOMPARE(mask.size(), QSize(6, 6));
    QCOMPARE(resized.points.size(), 2);
    QCOMPARE(resized.points.first(), QPointF(20, 20));
    QCOMPARE(resized.points.last(), QPointF(40, 40));
}

void UiTests::canvasMaskEditingPaintsAndErasesPixels() {
    QImage sourceMask(20, 20, QImage::Format_Grayscale8);
    sourceMask.fill(255);
    QByteArray pngData;
    QBuffer buffer(&pngData);
    QVERIFY(buffer.open(QIODevice::WriteOnly));
    QVERIFY(sourceMask.save(&buffer, "PNG"));

    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);

    Shape shape = Shape::fromPoints(QStringLiteral("mask-label"),
                                    QStringLiteral("mask"),
                                    {QPointF(20, 20), QPointF(39, 39)},
                                    false);
    shape.maskData = QString::fromLatin1(pngData.toBase64());
    canvas.setShapes({shape});
    canvas.setCurrentIndex(0);
    canvas.setEditMode();
    canvas.setMaskEditing(true);
    canvas.resize(100, 100);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::ShiftModifier, QPoint(30, 30));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::ShiftModifier, QPoint(30, 30));

    Shape erased = canvas.shapes().first();
    QImage erasedMask;
    QVERIFY(erasedMask.loadFromData(QByteArray::fromBase64(erased.maskData.toLatin1()), "PNG"));
    QVERIFY(qGray(erasedMask.pixelColor(10, 10).rgb()) == 0);
    QVERIFY(qGray(erasedMask.pixelColor(0, 0).rgb()) > 0);

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 30));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 30));

    const Shape painted = canvas.shapes().first();
    QImage paintedMask;
    QVERIFY(paintedMask.loadFromData(QByteArray::fromBase64(painted.maskData.toLatin1()), "PNG"));
    QVERIFY(painted.boundingRect().right() > 39.0);
    const int paintedX = qRound(50.0 - painted.points.first().x());
    const int paintedY = qRound(30.0 - painted.points.first().y());
    QVERIFY(paintedX >= 0 && paintedX < paintedMask.width());
    QVERIFY(paintedY >= 0 && paintedY < paintedMask.height());
    QVERIFY(qGray(paintedMask.pixelColor(paintedX, paintedY).rgb()) > 0);
}

void UiTests::canvasCtrlClickTogglesMultiSelection() {
    Canvas canvas;
    QPixmap pixmap(100, 80);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromRect(QStringLiteral("first"), QRectF(10, 10, 15, 15), false),
                      Shape::fromRect(QStringLiteral("second"), QRectF(45, 20, 15, 15), false)});
    canvas.setEditMode();
    canvas.resize(100, 80);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(15, 15));
    QCOMPARE(canvas.selectedIndices(), QVector<int>({0}));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::ControlModifier, QPoint(50, 25));
    QCOMPARE(canvas.selectedIndices(), QVector<int>({0, 1}));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::ControlModifier, QPoint(15, 15));
    QCOMPARE(canvas.selectedIndices(), QVector<int>({1}));
}

void UiTests::canvasClickingSelectedShapeTogglesSelectionWithoutEditing() {
    Canvas canvas;
    QPixmap pixmap(160, 120);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromRect(QStringLiteral("box"), QRectF(30, 20, 80, 60), false)});
    canvas.setCurrentIndex(-1);
    canvas.setEditMode();
    canvas.resize(160, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(70, 50));
    QCOMPARE(canvas.selectedIndices(), QVector<int>({0}));
    const QVector<QPointF> originalPoints = canvas.shapes().first().points;
    QSignalSpy shapeSpy(&canvas, &Canvas::shapesChanged);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(70, 50));

    QVERIFY(canvas.selectedIndices().isEmpty());
    QCOMPARE(canvas.shapes().first().points, originalPoints);
    QCOMPARE(shapeSpy.count(), 0);
}

void UiTests::canvasUsesLabelMeShapeHitTesting() {
    Canvas canvas;
    QPixmap pixmap(140, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    const Shape line = Shape::fromPoints(QStringLiteral("line"),
                                         QStringLiteral("line"),
                                         {QPointF(20, 20), QPointF(120, 20)},
                                         false);
    const Shape points = Shape::fromPoints(QStringLiteral("landmarks"),
                                           QStringLiteral("points"),
                                           {QPointF(30, 70), QPointF(100, 70)},
                                           false);
    canvas.setShapes({line, points});
    canvas.setEditMode();
    canvas.resize(140, 100);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(70, 29));
    QCOMPARE(canvas.selectedIndices(), QVector<int>({0}));
    canvas.setSelectedIndices({});

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(65, 70));
    QVERIFY(canvas.selectedIndices().isEmpty());
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(30, 70));
    QCOMPARE(canvas.selectedIndices(), QVector<int>({1}));
}

void UiTests::canvasUnknownShapeUsesPathAndVertexEditing() {
    Canvas canvas;
    QPixmap pixmap(160, 120);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);

    Shape triangle = Shape::fromPoints(QStringLiteral("plugin"),
                                       QStringLiteral("triangle"),
                                       {QPointF(20, 20), QPointF(120, 20), QPointF(60, 90)},
                                       false);
    triangle.closed = true;
    canvas.setShapes({triangle});
    canvas.setCurrentIndex(-1);
    canvas.setEditMode();
    canvas.resize(160, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 45));
    QCOMPARE(canvas.selectedIndices(), QVector<int>({0}));
    canvas.setSelectedIndices({});

    // The point is inside the bounding box but outside the plugin triangle.
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(115, 85));
    QVERIFY(canvas.selectedIndices().isEmpty());

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(&canvas, QPoint(30, 30));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(30, 30));
    QCOMPARE(canvas.shapes().first().points.first(), QPointF(30, 30));
}

void UiTests::canvasSupportsMultiSelectionDuplicateAndDelete() {
    Canvas canvas;
    QPixmap pixmap(120, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);

    Shape first = Shape::fromRect(QStringLiteral("first"), QRectF(10, 10, 12, 12), false);
    first.description = QStringLiteral("first metadata");
    Shape second = Shape::fromRect(QStringLiteral("second"), QRectF(40, 20, 16, 10), true);
    second.flags.insert(QStringLiteral("occluded"), true);
    canvas.setShapes({first, second});
    canvas.setSelectedIndices({0, 1});

    QCOMPARE(canvas.selectedIndices(), QVector<int>({0, 1}));
    QCOMPARE(canvas.currentIndex(), 1);
    QVERIFY(canvas.duplicateSelected(QPointF(5, 7)));
    QCOMPARE(canvas.shapes().size(), 4);
    QCOMPARE(canvas.selectedIndices(), QVector<int>({2, 3}));
    QCOMPARE(canvas.shapes()[2].label, QStringLiteral("first"));
    QCOMPARE(canvas.shapes()[2].description, QStringLiteral("first metadata"));
    QCOMPARE(canvas.shapes()[3].difficult, true);
    QCOMPARE(canvas.shapes()[3].flags.value(QStringLiteral("occluded")), true);
    QCOMPARE(canvas.shapes()[2].boundingRect().topLeft(), QPointF(15, 17));

    canvas.deleteSelected();
    QCOMPARE(canvas.shapes().size(), 2);
    QVERIFY(canvas.selectedIndices().isEmpty());
    QCOMPARE(canvas.currentIndex(), -1);
}

void UiTests::canvasDeleteCurrentSelectsNextShape() {
    Canvas canvas;
    QPixmap pixmap(120, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromRect(QStringLiteral("first"), QRectF(10, 10, 12, 12), false),
                      Shape::fromRect(QStringLiteral("second"), QRectF(40, 20, 16, 10), false)});
    canvas.setCurrentIndex(0);

    canvas.deleteCurrent();

    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.currentIndex(), 0);
    QCOMPARE(canvas.selectedIndices(), QVector<int>({0}));
}

void UiTests::canvasDiscardCreatedShapeClearsSelection() {
    Canvas canvas;
    QPixmap pixmap(120, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromRect(QStringLiteral("existing"), QRectF(40, 20, 16, 10), false),
                      Shape::fromRect(QString(), QRectF(10, 10, 12, 12), false)});
    canvas.setCurrentIndex(1);
    QSignalSpy shapesChangedSpy(&canvas, &Canvas::shapesChanged);

    QVERIFY(canvas.discardShapeAt(1));

    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().label, QStringLiteral("existing"));
    QCOMPARE(canvas.currentIndex(), -1);
    QVERIFY(canvas.selectedIndices().isEmpty());
    QCOMPARE(shapesChangedSpy.count(), 0);
    QVERIFY(!canvas.discardShapeAt(10));
}

void UiTests::canvasSetShapesSelectsCurrentShape() {
    Canvas canvas;
    QPixmap pixmap(120, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromRect(QStringLiteral("first"), QRectF(10, 10, 12, 12), false),
                      Shape::fromRect(QStringLiteral("second"), QRectF(40, 20, 16, 10), false)});

    QCOMPARE(canvas.currentIndex(), 1);
    QCOMPARE(canvas.selectedIndices(), QVector<int>({1}));
}

void UiTests::canvasKeyboardMovesMultiSelectionTogether() {
    Canvas canvas;
    QPixmap pixmap(120, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromRect(QStringLiteral("first"), QRectF(10, 10, 12, 12), false),
                      Shape::fromRect(QStringLiteral("second"), QRectF(40, 20, 16, 10), false)});
    canvas.setSelectedIndices({0, 1});
    canvas.setEditMode();
    canvas.resize(120, 100);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::keyClick(&canvas, Qt::Key_Right);

    QCOMPARE(canvas.shapes().at(0).boundingRect().topLeft(), QPointF(15, 10));
    QCOMPARE(canvas.shapes().at(1).boundingRect().topLeft(), QPointF(45, 20));
    QCOMPARE(canvas.selectedIndices(), QVector<int>({0, 1}));
}

void UiTests::canvasCopiesSelectedGroupToContextPoint() {
    Canvas canvas;
    QPixmap pixmap(120, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);

    Shape first = Shape::fromRect(QStringLiteral("first"), QRectF(10, 10, 12, 12), false);
    first.description = QStringLiteral("first metadata");
    Shape second = Shape::fromRect(QStringLiteral("second"), QRectF(40, 20, 16, 10), true);
    second.flags.insert(QStringLiteral("occluded"), true);
    canvas.setShapes({first, second});
    canvas.setSelectedIndices({0, 1});

    QVERIFY(canvas.copyCurrentTo(QPointF(80, 70)));
    QCOMPARE(canvas.shapes().size(), 4);
    QCOMPARE(canvas.selectedIndices(), QVector<int>({2, 3}));
    QCOMPARE(canvas.shapes()[2].description, QStringLiteral("first metadata"));
    QCOMPARE(canvas.shapes()[3].flags.value(QStringLiteral("occluded")), true);
    QCOMPARE(canvas.shapes()[2].boundingRect().topLeft(), QPointF(57, 60));
    QCOMPARE(canvas.shapes()[3].boundingRect().topLeft(), QPointF(87, 70));
}

void UiTests::canvasMovesSelectedGroupToContextPoint() {
    Canvas canvas;
    QPixmap pixmap(120, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);

    canvas.setShapes({Shape::fromRect(QStringLiteral("first"), QRectF(10, 10, 12, 12), false),
                      Shape::fromRect(QStringLiteral("second"), QRectF(40, 20, 16, 10), true)});
    canvas.setSelectedIndices({0, 1});

    QVERIFY(canvas.moveCurrentTo(QPointF(80, 70)));
    QCOMPARE(canvas.shapes()[0].boundingRect().topLeft(), QPointF(57, 60));
    QCOMPARE(canvas.shapes()[1].boundingRect().topLeft(), QPointF(87, 70));
    QCOMPARE(canvas.selectedIndices(), QVector<int>({0, 1}));
}

void UiTests::canvasViewModePansWithoutCreatingOrMovingShapes() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromRect(QStringLiteral("defect"), QRectF(20, 20, 30, 30), false)});
    canvas.setViewMode();
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QSignalSpy scrollSpy(&canvas, &Canvas::scrollRequested);
    QSignalSpy shapeSpy(&canvas, &Canvas::shapesChanged);

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(25, 25));
    QTest::mouseMove(&canvas, QPoint(45, 45));
    QTest::mouseMove(&canvas, QPoint(50, 47));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 47));

    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().boundingRect().toRect(), QRect(20, 20, 30, 30));
    QCOMPARE(shapeSpy.count(), 0);
    QCOMPARE(scrollSpy.count(), 1);
    QCOMPARE(scrollSpy.first().at(0).toInt(), 5);
    QCOMPARE(scrollSpy.first().at(1).toInt(), 2);
}

void UiTests::canvasViewModeUsesArrowCursorAndEmptyClickClearsSelection() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromRect(QStringLiteral("defect"), QRectF(20, 20, 30, 30), false)});
    canvas.setViewMode();
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QCOMPARE(canvas.cursor().shape(), Qt::ArrowCursor);
    QCOMPARE(canvas.currentIndex(), 0);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 80));

    QCOMPARE(canvas.currentIndex(), -1);
    QCOMPARE(canvas.cursor().shape(), Qt::ArrowCursor);
}

void UiTests::canvasViewModeSelectsShapeWithoutEditing() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromRect(QStringLiteral("defect"), QRectF(20, 20, 30, 30), false)});
    canvas.setSelectedIndices({});
    canvas.setViewMode();
    canvas.resize(100, 100);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QSignalSpy shapeSpy(&canvas, &Canvas::shapesChanged);
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(30, 30));

    QCOMPARE(canvas.currentIndex(), 0);
    QCOMPARE(canvas.selectedIndices(), QVector<int>({0}));
    QCOMPARE(canvas.shapes().first().boundingRect(), QRectF(20, 20, 30, 30));
    QCOMPARE(shapeSpy.count(), 0);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 80));
    QCOMPARE(canvas.currentIndex(), -1);
    QCOMPARE(canvas.selectedIndices(), QVector<int>());
}

void UiTests::canvasViewModeDoesNotRenderEditHandles() {
    Canvas canvas;
    QPixmap pixmap(100, 80);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);

    Shape shape = Shape::fromRect(QStringLiteral("defect"), QRectF(20, 20, 30, 20), false);
    shape.lineColor = Qt::black;
    shape.fillColor = Qt::transparent;
    canvas.setShapes({shape});
    canvas.setSelectedLineColor(Qt::black);
    canvas.setVertexFillColor(Qt::white);
    canvas.setViewMode();
    canvas.resize(100, 80);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QImage rendered(canvas.size(), QImage::Format_ARGB32_Premultiplied);
    rendered.fill(Qt::white);
    QPainter painter(&rendered);
    canvas.render(&painter);

    // The rectangle outline remains visible, but the white edit handle that
    // editable mode uses at the corner must not be painted in view mode.
    const QColor corner = rendered.pixelColor(20, 20);
    QVERIFY2(corner.red() < 240,
             qPrintable(QStringLiteral("view-mode corner unexpectedly looks like an edit handle: %1")
                            .arg(corner.name(QColor::HexArgb))));
}

void UiTests::canvasLeavingImageClearsHoverCursor() {
    Canvas canvas;
    QPixmap pixmap(100, 80);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromRect(QStringLiteral("box"), QRectF(20, 20, 30, 20), false)});
    canvas.setEditMode();
    canvas.resize(100, 80);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    // Establish the transient state directly so the test is independent of
    // the native pointer position left by the preceding UI case.
    canvas.setCursor(Qt::SizeFDiagCursor);

    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(&canvas, &leaveEvent);
    QCOMPARE(canvas.cursor().shape(), Qt::ArrowCursor);
}

void UiTests::canvasFocusOutReleasesInteractionCursor() {
    Canvas canvas;
    QPixmap pixmap(100, 80);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromRect(QStringLiteral("box"), QRectF(20, 20, 30, 20), false)});
    canvas.setEditMode();
    canvas.resize(100, 80);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    // Establish the transient state directly so the test is independent of
    // the native pointer position left by the preceding UI case.
    canvas.setCursor(Qt::SizeFDiagCursor);

    QFocusEvent focusOut(QEvent::FocusOut);
    QApplication::sendEvent(&canvas, &focusOut);
    QCOMPARE(canvas.cursor().shape(), Qt::ArrowCursor);
}

void UiTests::canvasEnteringImageRestoresModeCursor() {
    Canvas canvas;
    canvas.setCreateMode(true);
    canvas.setCursor(Qt::ArrowCursor);

    QEnterEvent enterCreate(QPointF(10, 10), QPointF(10, 10), QPointF(10, 10));
    QApplication::sendEvent(&canvas, &enterCreate);
    QCOMPARE(canvas.cursor().shape(), Qt::CrossCursor);

    canvas.setEditMode();
    canvas.setCursor(Qt::CrossCursor);
    QEnterEvent enterEdit(QPointF(10, 10), QPointF(10, 10), QPointF(10, 10));
    QApplication::sendEvent(&canvas, &enterEdit);
    QCOMPARE(canvas.cursor().shape(), Qt::ArrowCursor);

    canvas.setViewMode();
    canvas.setCursor(Qt::CrossCursor);
    QEnterEvent enterView(QPointF(10, 10), QPointF(10, 10), QPointF(10, 10));
    QApplication::sendEvent(&canvas, &enterView);
    QCOMPARE(canvas.cursor().shape(), Qt::ArrowCursor);
}

void UiTests::canvasCtrlASelectsAllShapesInEditMode() {
    Canvas canvas;
    canvas.resize(320, 240);
    canvas.setPixmap(QPixmap(160, 120));
    canvas.setShapes({Shape::fromRect(QStringLiteral("a"), QRectF(10, 10, 30, 20), false),
                     Shape::fromRect(QStringLiteral("b"), QRectF(80, 50, 30, 20), false)});
    canvas.setEditMode();
    canvas.setCurrentIndex(0);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::keyClick(&canvas, Qt::Key_A, Qt::ControlModifier);

    QCOMPARE(canvas.selectedIndices(), QVector<int>({0, 1}));
}

void UiTests::canvasIgnoresRightDragBelowThreshold() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QSignalSpy scrollSpy(&canvas, &Canvas::scrollRequested);
    QSignalSpy menuSpy(&canvas, &Canvas::contextMenuRequested);

    QTest::mousePress(&canvas, Qt::RightButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(&canvas, QPoint(22, 22));
    QTest::mouseRelease(&canvas, Qt::RightButton, Qt::NoModifier, QPoint(22, 22));

    QCOMPARE(scrollSpy.count(), 0);
    QCOMPARE(menuSpy.count(), 1);
}

void UiTests::canvasRightDragDoesNotJumpWhenThresholdIsCrossed() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QSignalSpy scrollSpy(&canvas, &Canvas::scrollRequested);

    QTest::mousePress(&canvas, Qt::RightButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(&canvas, QPoint(23, 23));
    QTest::mouseMove(&canvas, QPoint(36, 36));
    QTest::mouseMove(&canvas, QPoint(38, 37));
    QTest::mouseRelease(&canvas, Qt::RightButton, Qt::NoModifier, QPoint(38, 37));

    QCOMPARE(scrollSpy.count(), 1);
    QCOMPARE(scrollSpy.first().at(0).toInt(), 2);
    QCOMPARE(scrollSpy.first().at(1).toInt(), 1);
}

void UiTests::canvasPanDragUsesGlobalMouseDeltaWhenLocalPositionShifts() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QSignalSpy scrollSpy(&canvas, &Canvas::scrollRequested);

    QMouseEvent press(QEvent::MouseButtonPress,
                      QPointF(80, 80),
                      QPointF(80, 80),
                      QPointF(100, 100),
                      Qt::RightButton,
                      Qt::RightButton,
                      Qt::NoModifier);
    QApplication::sendEvent(&canvas, &press);

    QMouseEvent thresholdMove(QEvent::MouseMove,
                              QPointF(95, 95),
                              QPointF(95, 95),
                              QPointF(115, 115),
                              Qt::NoButton,
                              Qt::RightButton,
                              Qt::NoModifier);
    QApplication::sendEvent(&canvas, &thresholdMove);

    QMouseEvent shiftedLocalMove(QEvent::MouseMove,
                                 QPointF(75, 74),
                                 QPointF(75, 74),
                                 QPointF(120, 119),
                                 Qt::NoButton,
                                 Qt::RightButton,
                                 Qt::NoModifier);
    QApplication::sendEvent(&canvas, &shiftedLocalMove);

    QCOMPARE(scrollSpy.count(), 1);
    QCOMPARE(scrollSpy.first().at(0).toInt(), 5);
    QCOMPARE(scrollSpy.first().at(1).toInt(), 4);

    QMouseEvent release(QEvent::MouseButtonRelease,
                        QPointF(75, 74),
                        QPointF(75, 74),
                        QPointF(120, 119),
                        Qt::RightButton,
                        Qt::NoButton,
                        Qt::NoModifier);
    QApplication::sendEvent(&canvas, &release);
}

void UiTests::canvasWheelZoomReportsAnchorPoint() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QSignalSpy scaleSpy(&canvas, &Canvas::scaleChanged);
    QWheelEvent event(QPointF(40, 50), canvas.mapToGlobal(QPoint(40, 50)),
                      QPoint(), QPoint(0, 120), Qt::NoButton, Qt::ControlModifier,
                      Qt::NoScrollPhase, false);
    QApplication::sendEvent(&canvas, &event);

    QCOMPARE(scaleSpy.count(), 1);
    QCOMPARE(scaleSpy.first().at(0).toDouble(), 1.0);
    QCOMPARE(scaleSpy.first().at(1).toDouble(), 1.11);
    QCOMPARE(scaleSpy.first().at(2).toPoint(), QPoint(40, 50));

    QWheelEvent second(QPointF(40, 50), canvas.mapToGlobal(QPoint(40, 50)),
                       QPoint(), QPoint(0, 120), Qt::NoButton, Qt::ControlModifier,
                       Qt::NoScrollPhase, false);
    QApplication::sendEvent(&canvas, &second);
    QCOMPARE(scaleSpy.count(), 2);
    QCOMPARE(scaleSpy.at(1).at(0).toDouble(), 1.11);
    QCOMPARE(scaleSpy.at(1).at(1).toDouble(), 1.23);
    QCOMPARE(scaleSpy.at(1).at(2).toPoint(), QPoint(40, 50));

    QWheelEvent third(QPointF(40, 50), canvas.mapToGlobal(QPoint(40, 50)),
                      QPoint(), QPoint(0, -120), Qt::NoButton, Qt::ControlModifier,
                      Qt::NoScrollPhase, false);
    QApplication::sendEvent(&canvas, &third);
    QCOMPARE(scaleSpy.count(), 3);
    QCOMPARE(scaleSpy.at(2).at(0).toDouble(), 1.23);
    QCOMPARE(scaleSpy.at(2).at(1).toDouble(), 1.10);
}

void UiTests::canvasWheelMatchesLabelMeModifierRouting() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QSignalSpy scaleSpy(&canvas, &Canvas::scaleChanged);
    QSignalSpy scrollSpy(&canvas, &Canvas::wheelScrollRequested);
    const QPoint local(40, 50);
    const QPoint global = canvas.mapToGlobal(local);

    QWheelEvent plain(QPointF(local), global, QPoint(), QPoint(0, 120), Qt::NoButton,
                      Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(&canvas, &plain);
    QCOMPARE(scaleSpy.count(), 0);
    QCOMPARE(scrollSpy.count(), 2);
    QCOMPARE(scrollSpy.at(0).at(0).toInt(), 0);
    QCOMPARE(scrollSpy.at(0).at(1).toInt(), 0);
    QCOMPARE(scrollSpy.at(1).at(0).toInt(), 0);
    QCOMPARE(scrollSpy.at(1).at(1).toInt(), 120);

    scrollSpy.clear();
    QWheelEvent shifted(QPointF(local), global, QPoint(), QPoint(0, 120), Qt::NoButton,
                        Qt::ShiftModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(&canvas, &shifted);
    QCOMPARE(scaleSpy.count(), 0);
    QCOMPARE(scrollSpy.count(), 1);
    QCOMPARE(scrollSpy.first().at(0).toInt(), 120);
    QCOMPARE(scrollSpy.first().at(1).toInt(), 0);
}

void UiTests::canvasWheelUsesNaturalScrollSignal() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QSignalSpy panSpy(&canvas, &Canvas::scrollRequested);
    QSignalSpy wheelSpy(&canvas, &Canvas::wheelScrollRequested);
    const QPoint local(40, 50);
    const QPoint global = canvas.mapToGlobal(local);

    QWheelEvent event(QPointF(local), global, QPoint(), QPoint(0, 120),
                      Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(&canvas, &event);

    QCOMPARE(panSpy.count(), 0);
    QCOMPARE(wheelSpy.count(), 2);
    QCOMPARE(wheelSpy.at(0).at(0).toInt(), 0);
    QCOMPARE(wheelSpy.at(0).at(1).toInt(), 0);
    QCOMPARE(wheelSpy.at(1).at(0).toInt(), 0);
    QCOMPARE(wheelSpy.at(1).at(1).toInt(), 120);
}

void UiTests::canvasCopiesAndMovesCurrentShapeToPoint() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromRect("box", QRectF(10, 10, 20, 20), false)});
    canvas.setCurrentIndex(0);

    QVERIFY(canvas.copyCurrentTo(QPointF(80, 80)));
    QCOMPARE(canvas.shapes().size(), 2);
    QCOMPARE(canvas.currentIndex(), 1);
    QCOMPARE(canvas.shapes().last().boundingRect().center(), QPointF(80, 80));

    QVERIFY(canvas.moveCurrentTo(QPointF(10, 10)));
    QCOMPARE(canvas.shapes().size(), 2);
    QCOMPARE(canvas.shapes().last().boundingRect().center(), QPointF(10, 10));
}

void UiTests::canvasRightDragOnShapeSupportsCopyAndMove() {
    auto widgetPoint = [](const Canvas &canvas, const QPointF &imagePoint) {
        return (canvas.imageOriginOffset() + imagePoint * canvas.scale()).toPoint();
    };

    Canvas copyCanvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    copyCanvas.setPixmap(pixmap);
    copyCanvas.setShapes({Shape::fromRect("box", QRectF(10, 10, 20, 20), false)});
    copyCanvas.setEditing(true);
    copyCanvas.resize(120, 120);
    copyCanvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&copyCanvas));

    QSignalSpy copyScrollSpy(&copyCanvas, &Canvas::scrollRequested);
    QSignalSpy copyMenuSpy(&copyCanvas, &Canvas::contextMenuRequested);
    const QPoint copyStart = widgetPoint(copyCanvas, QPointF(20, 20));
    const QPoint copyEnd = widgetPoint(copyCanvas, QPointF(60, 50));
    QTest::mousePress(&copyCanvas, Qt::RightButton, Qt::NoModifier, copyStart);
    QTest::mouseMove(&copyCanvas, copyEnd);
    QVERIFY(copyCanvas.hasPendingRightDrag());
    QCOMPARE(copyScrollSpy.count(), 0);
    QTest::mouseRelease(&copyCanvas, Qt::RightButton, Qt::NoModifier, copyEnd);
    QCOMPARE(copyMenuSpy.count(), 1);
    QVERIFY(copyCanvas.hasPendingRightDrag());
    QVERIFY(copyCanvas.finishRightDrag(true));
    QCOMPARE(copyCanvas.shapes().size(), 2);
    QCOMPARE(copyCanvas.shapes().at(0).boundingRect().center(), QPointF(20, 20));
    QCOMPARE(copyCanvas.shapes().at(1).boundingRect().center(), QPointF(60, 50));

    Canvas moveCanvas;
    moveCanvas.setPixmap(pixmap);
    moveCanvas.setShapes({Shape::fromRect("box", QRectF(10, 10, 20, 20), false)});
    moveCanvas.setEditing(true);
    moveCanvas.resize(120, 120);
    moveCanvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&moveCanvas));

    QSignalSpy moveScrollSpy(&moveCanvas, &Canvas::scrollRequested);
    QSignalSpy moveMenuSpy(&moveCanvas, &Canvas::contextMenuRequested);
    const QPoint moveStart = widgetPoint(moveCanvas, QPointF(20, 20));
    const QPoint moveEnd = widgetPoint(moveCanvas, QPointF(60, 50));
    QTest::mousePress(&moveCanvas, Qt::RightButton, Qt::NoModifier, moveStart);
    QTest::mouseMove(&moveCanvas, moveEnd);
    QVERIFY(moveCanvas.hasPendingRightDrag());
    QCOMPARE(moveScrollSpy.count(), 0);
    QTest::mouseRelease(&moveCanvas, Qt::RightButton, Qt::NoModifier, moveEnd);
    QCOMPARE(moveMenuSpy.count(), 1);
    QVERIFY(moveCanvas.hasPendingRightDrag());
    QVERIFY(moveCanvas.finishRightDrag(false));
    QCOMPARE(moveCanvas.shapes().size(), 1);
    QCOMPARE(moveCanvas.shapes().first().boundingRect().center(), QPointF(60, 50));
}

void UiTests::canvasRightDragOutsidePixmapDoesNotMovePreview() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromRect(QStringLiteral("box"), QRectF(10, 10, 40, 40), false)});
    canvas.setEditMode();
    canvas.resize(100, 100);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    const QPoint pressPoint(30, 30);
    QTest::mousePress(&canvas, Qt::RightButton, Qt::NoModifier, pressPoint);
    QTest::mouseMove(&canvas, QPoint(40, 30));
    QVERIFY(canvas.hasPendingRightDrag());

    QMouseEvent moveOutside(QEvent::MouseMove,
                            QPointF(-10, 30),
                            QPointF(-10, 30),
                            canvas.mapToGlobal(QPoint(-10, 30)),
                            Qt::NoButton,
                            Qt::RightButton,
                            Qt::NoModifier);
    QApplication::sendEvent(&canvas, &moveOutside);
    QVERIFY(canvas.hasPendingRightDrag());

    QMouseEvent releaseOutside(QEvent::MouseButtonRelease,
                               QPointF(-10, 30),
                               QPointF(-10, 30),
                               canvas.mapToGlobal(QPoint(-10, 30)),
                               Qt::RightButton,
                               Qt::NoButton,
                               Qt::NoModifier);
    QApplication::sendEvent(&canvas, &releaseOutside);
    QVERIFY(canvas.hasPendingRightDrag());
    QVERIFY(canvas.finishRightDrag(false));

    // LabelMe ignores drag updates outside the pixmap instead of clamping
    // the preview to the edge.
    QCOMPARE(canvas.shapes().first().boundingRect().topLeft(), QPointF(20, 10));
}

void UiTests::canvasPolygonCreateModeFinishesWithReturn() {
    Canvas canvas;
    QPixmap pixmap(120, 80);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateShapeType(QStringLiteral("polygon"));
    canvas.setCreateMode(true);
    canvas.resize(120, 80);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QSignalSpy createdSpy(&canvas, &Canvas::shapeCreated);
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 12));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(70, 55));
    QCOMPARE(canvas.shapes().size(), 0);

    QTest::keyClick(&canvas, Qt::Key_Return);

    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().shapeType, QStringLiteral("polygon"));
    QCOMPARE(canvas.shapes().first().points.size(), 3);
    QCOMPARE(canvas.shapes().first().points[0], QPointF(10, 10));
    QCOMPARE(createdSpy.count(), 1);
}

void UiTests::canvasPolygonCreateModeFinishesWithDoubleClick() {
    Canvas canvas;
    QPixmap pixmap(140, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateShapeType(QStringLiteral("polygon"));
    canvas.setCreateMode(true);
    canvas.resize(140, 100);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 20));
    QTest::mouseDClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(55, 70));

    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().shapeType, QStringLiteral("polygon"));
    QCOMPARE(canvas.shapes().first().points,
             QVector<QPointF>({QPointF(20, 20), QPointF(80, 20), QPointF(55, 70)}));
}

void UiTests::canvasPolygonCreateModeSnapsClosedOnFirstVertex() {
    Canvas canvas;
    QPixmap pixmap(140, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateShapeType(QStringLiteral("polygon"));
    canvas.setCreateMode(true);
    canvas.resize(140, 100);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 20));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(55, 70));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(21, 21));

    QCOMPARE(canvas.shapes().size(), 1);
    QCOMPARE(canvas.shapes().first().shapeType, QStringLiteral("polygon"));
    QCOMPARE(canvas.shapes().first().points,
             QVector<QPointF>({QPointF(20, 20), QPointF(80, 20), QPointF(55, 70)}));
    QVERIFY(!canvas.isDrawing());
}

void UiTests::canvasPolygonVertexDragMovesOnlyThatPoint() {
    Canvas canvas;
    QPixmap pixmap(120, 90);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromPolygon(QStringLiteral("poly"),
                                          {QPointF(20, 20), QPointF(90, 22), QPointF(70, 65)},
                                          false)});
    canvas.setCurrentIndex(0);
    canvas.setEditMode();
    canvas.resize(140, 110);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QSignalSpy changedSpy(&canvas, &Canvas::shapesChanged);
    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(&canvas, QPoint(32, 36));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(32, 36));

    const Shape shape = canvas.shapes().first();
    QCOMPARE(shape.shapeType, QStringLiteral("polygon"));
    QCOMPARE(shape.points.size(), 3);
    QCOMPARE(shape.points[0], QPointF(32, 36));
    QCOMPARE(shape.points[1], QPointF(90, 22));
    QCOMPARE(shape.points[2], QPointF(70, 65));
    QVERIFY(changedSpy.count() > 0);
}

void UiTests::canvasVertexPriorityBeatsOverlappingResizeHandle() {
    Canvas canvas;
    QPixmap pixmap(140, 100);
    pixmap.fill(Qt::white);
    const Shape polygon = Shape::fromPolygon(QStringLiteral("poly"),
                                              {QPointF(50, 20), QPointF(80, 45), QPointF(20, 45)},
                                              false);
    const Shape rectangle = Shape::fromRect(QStringLiteral("box"), QRectF(30, 20, 70, 50), false);
    // The rectangle is the topmost shape and has a resize edge at (50, 20).
    // LabelMe still gives the coincident polygon vertex first priority.
    canvas.setShapes({polygon, rectangle});
    canvas.setEditMode();
    canvas.resize(140, 100);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 20));
    QTest::mouseMove(&canvas, QPoint(56, 26));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(56, 26));

    QCOMPARE(canvas.shapes().at(0).points.first(), QPointF(56, 26));
    QCOMPARE(canvas.shapes().at(1).points.first(), QPointF(30, 20));
    QCOMPARE(canvas.shapes().at(1).points.last(), QPointF(100, 70));
}

void UiTests::canvasAltClicksInsertAndRemovePolygonPoints() {
    Canvas canvas;
    QPixmap pixmap(120, 90);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromPolygon(QStringLiteral("poly"),
                                          {QPointF(20, 20), QPointF(80, 20), QPointF(70, 65)},
                                          false)});
    canvas.setCurrentIndex(0);
    canvas.setEditMode();
    canvas.resize(140, 110);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QSignalSpy changedSpy(&canvas, &Canvas::shapesChanged);
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::AltModifier, QPoint(50, 20));

    Shape shape = canvas.shapes().first();
    QCOMPARE(shape.shapeType, QStringLiteral("polygon"));
    QCOMPARE(shape.points.size(), 4);
    QCOMPARE(shape.points[0], QPointF(20, 20));
    QCOMPARE(shape.points[1], QPointF(50, 20));
    QCOMPARE(shape.points[2], QPointF(80, 20));
    QCOMPARE(shape.points[3], QPointF(70, 65));

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::AltModifier | Qt::ShiftModifier, QPoint(50, 20));

    shape = canvas.shapes().first();
    QCOMPARE(shape.points.size(), 3);
    QCOMPARE(shape.points[0], QPointF(20, 20));
    QCOMPARE(shape.points[1], QPointF(80, 20));
    QCOMPARE(shape.points[2], QPointF(70, 65));
    QVERIFY(changedSpy.count() >= 2);
}

void UiTests::canvasAltClickingVertexDoesNotInsertPolygonPoint() {
    Canvas canvas;
    QPixmap pixmap(120, 90);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromPolygon(QStringLiteral("poly"),
                                          {QPointF(20, 20), QPointF(80, 20), QPointF(60, 70)},
                                          false)});
    canvas.setEditMode();
    canvas.resize(120, 90);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    // LabelMe prioritizes vertex hover over edge hover. Alt-clicking an
    // existing vertex must not insert a duplicate point at that coordinate.
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::AltModifier, QPoint(20, 20));
    QCOMPARE(canvas.shapes().first().points.size(), 3);
}

void UiTests::canvasAddsPointToHoveredPolygonEdge() {
    Canvas canvas;
    QPixmap pixmap(120, 90);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromPolygon(QStringLiteral("poly"),
                                          {QPointF(20, 20), QPointF(80, 20), QPointF(70, 65)},
                                          false)});
    canvas.setCurrentIndex(0);
    canvas.setEditMode();
    canvas.resize(140, 110);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QMouseEvent hoverEdge(QEvent::MouseMove,
                          QPointF(50, 20),
                          QPointF(50, 20),
                          canvas.mapToGlobal(QPoint(50, 20)),
                          Qt::NoButton,
                          Qt::NoButton,
                          Qt::NoModifier);
    QApplication::sendEvent(&canvas, &hoverEdge);
    QVERIFY(canvas.canAddPointToEdge());
    QSignalSpy changedSpy(&canvas, &Canvas::shapesChanged);
    QVERIFY(canvas.addPointToEdge());

    const Shape shape = canvas.shapes().first();
    QCOMPARE(shape.points.size(), 4);
    QCOMPARE(shape.points[1], QPointF(50, 20));
    QCOMPARE(canvas.currentIndex(), 0);
    QVERIFY(!canvas.canAddPointToEdge());
    QVERIFY(changedSpy.count() > 0);
}

void UiTests::canvasAddsPointToClosingPolygonEdgeAtLabelMeIndex() {
    Canvas canvas;
    QPixmap pixmap(120, 90);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromPolygon(QStringLiteral("poly"),
                                          {QPointF(20, 20), QPointF(80, 20), QPointF(70, 65)},
                                          false)});
    canvas.setCurrentIndex(0);
    canvas.setEditMode();
    canvas.resize(140, 110);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    // LabelMe indexes the closing edge (last -> first) as edge 0, so an
    // insertion at this edge becomes the new first vertex.
    QMouseEvent hoverEdge(QEvent::MouseMove,
                          QPointF(45, 42.5),
                          QPointF(45, 42.5),
                          canvas.mapToGlobal(QPoint(45, 42)),
                          Qt::NoButton,
                          Qt::NoButton,
                          Qt::NoModifier);
    QApplication::sendEvent(&canvas, &hoverEdge);
    QVERIFY(canvas.canAddPointToEdge());
    QVERIFY(canvas.addPointToEdge());

    const Shape shape = canvas.shapes().first();
    QCOMPARE(shape.points.size(), 4);
    QCOMPARE(shape.points[0], QPointF(45, 42.5));
    QCOMPARE(shape.points[1], QPointF(20, 20));
    QCOMPARE(shape.points[2], QPointF(80, 20));
    QCOMPARE(shape.points[3], QPointF(70, 65));
}

void UiTests::canvasAddsPointToLinestripClosingEdge() {
    Canvas canvas;
    QPixmap pixmap(120, 90);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromPoints(QStringLiteral("path"),
                                         QStringLiteral("linestrip"),
                                         {QPointF(20, 20), QPointF(80, 20), QPointF(80, 70)},
                                         false)});
    canvas.setCurrentIndex(0);
    canvas.setEditMode();
    canvas.resize(140, 110);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    // The closing edge is (last -> first). LabelMe inserts at its destination
    // vertex, so the new point must become the first point.
    QMouseEvent hoverEdge(QEvent::MouseMove,
                          QPointF(50, 45),
                          QPointF(50, 45),
                          canvas.mapToGlobal(QPoint(50, 45)),
                          Qt::NoButton,
                          Qt::NoButton,
                          Qt::NoModifier);
    QApplication::sendEvent(&canvas, &hoverEdge);
    QVERIFY(canvas.canAddPointToEdge());
    QVERIFY(canvas.addPointToEdge());

    const Shape shape = canvas.shapes().first();
    QCOMPARE(shape.points.size(), 4);
    QCOMPARE(shape.points[0], QPointF(50, 45));
    QCOMPARE(shape.points[1], QPointF(20, 20));
    QCOMPARE(shape.points[2], QPointF(80, 20));
    QCOMPARE(shape.points[3], QPointF(80, 70));
}

void UiTests::canvasAllowsEdgeInsertionOnImportedTwoPointPolygon() {
    Canvas canvas;
    QPixmap pixmap(120, 90);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromPoints(QStringLiteral("imported"),
                                         QStringLiteral("polygon"),
                                         {QPointF(20, 20), QPointF(80, 20)},
                                         false)});
    canvas.setCurrentIndex(0);
    canvas.setEditMode();
    canvas.resize(140, 110);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    // LabelMe's edge helper still exposes both segments for an imported
    // two-point polygon, allowing the user to repair its topology in place.
    QMouseEvent hoverEdge(QEvent::MouseMove,
                          QPointF(50, 20),
                          QPointF(50, 20),
                          canvas.mapToGlobal(QPoint(50, 20)),
                          Qt::NoButton,
                          Qt::NoButton,
                          Qt::NoModifier);
    QApplication::sendEvent(&canvas, &hoverEdge);
    QVERIFY(canvas.canAddPointToEdge());
    QVERIFY(canvas.addPointToEdge());
    QCOMPARE(canvas.shapes().first().points.size(), 3);
    QCOMPARE(canvas.shapes().first().points[1], QPointF(50, 20));
}

void UiTests::canvasContextPointOperationsInsertAndRemovePolygonPoints() {
    Canvas canvas;
    QPixmap pixmap(120, 90);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromPolygon(QStringLiteral("poly"),
                                          {QPointF(20, 20), QPointF(80, 20), QPointF(70, 65)},
                                          false)});
    canvas.setCurrentIndex(0);
    canvas.setEditMode();

    QSignalSpy changedSpy(&canvas, &Canvas::shapesChanged);
    QVERIFY(canvas.insertPointAt(QPointF(50, 20)));

    Shape shape = canvas.shapes().first();
    QCOMPARE(shape.shapeType, QStringLiteral("polygon"));
    QCOMPARE(shape.points.size(), 4);
    QCOMPARE(shape.points[1], QPointF(50, 20));
    QCOMPARE(canvas.currentIndex(), 0);

    QVERIFY(canvas.removePointAt(QPointF(50, 20)));
    shape = canvas.shapes().first();
    QCOMPARE(shape.points.size(), 3);
    QCOMPARE(shape.points[0], QPointF(20, 20));
    QCOMPARE(shape.points[1], QPointF(80, 20));
    QCOMPARE(shape.points[2], QPointF(70, 65));
    QVERIFY(changedSpy.count() >= 2);
}

void UiTests::canvasLineVertexDragMovesOnlyThatPoint() {
    Canvas canvas;
    QPixmap pixmap(120, 90);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromPoints(QStringLiteral("edge"),
                                         QStringLiteral("line"),
                                         {QPointF(20, 20), QPointF(80, 40)},
                                         false)});
    canvas.setCurrentIndex(0);
    canvas.setEditMode();
    canvas.resize(140, 110);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QSignalSpy changedSpy(&canvas, &Canvas::shapesChanged);
    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 40));
    QTest::mouseMove(&canvas, QPoint(90, 55));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(90, 55));

    const Shape shape = canvas.shapes().first();
    QCOMPARE(shape.shapeType, QStringLiteral("line"));
    QCOMPARE(shape.points.size(), 2);
    QCOMPARE(shape.points[0], QPointF(20, 20));
    QCOMPARE(shape.points[1], QPointF(90, 55));
    QVERIFY(changedSpy.count() > 0);
}

void UiTests::canvasVertexDragOutsideProjectsToImageEdge() {
    Canvas canvas;
    QPixmap pixmap(120, 90);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromPolygon(QStringLiteral("poly"),
                                         {QPointF(20, 20), QPointF(80, 20), QPointF(70, 65)},
                                         false)});
    canvas.setCurrentIndex(0);
    canvas.setEditMode();
    canvas.resize(120, 90);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    // From (20,20) to (-20,80), the ray intersects x=0 at y=50.
    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(&canvas, QPoint(-20, 80));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(-20, 80));

    const Shape shape = canvas.shapes().first();
    QCOMPARE(shape.points.first(), QPointF(0, 50));
}

void UiTests::canvasOrientedRectangleVertexDragKeepsShapeType() {
    Canvas canvas;
    QPixmap pixmap(140, 120);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromPoints(QStringLiteral("rotated"),
                                         QStringLiteral("oriented_rectangle"),
                                         {QPointF(30, 40), QPointF(90, 30), QPointF(100, 75), QPointF(40, 85)},
                                         false)});
    canvas.setCurrentIndex(0);
    canvas.setEditMode();
    canvas.resize(160, 140);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QSignalSpy changedSpy(&canvas, &Canvas::shapesChanged);
    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(90, 30));
    QTest::mouseMove(&canvas, QPoint(95, 35));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(95, 35));

    const Shape shape = canvas.shapes().first();
    QCOMPARE(shape.shapeType, QStringLiteral("oriented_rectangle"));
    QCOMPARE(shape.points.size(), 4);
    QCOMPARE(shape.points[0], QPointF(32, 49));
    QCOMPARE(shape.points[1], QPointF(95, 35));
    QCOMPARE(shape.points[2], QPointF(103, 71));
    QCOMPARE(shape.points[3], QPointF(40, 85));
    QVERIFY(changedSpy.count() > 0);
}

void UiTests::canvasOrientedRectangleRotationHandleRotatesAroundCenter() {
    Canvas canvas;
    QPixmap pixmap(140, 120);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromPoints(QStringLiteral("rotated"),
                                         QStringLiteral("oriented_rectangle"),
                                         {QPointF(20, 20), QPointF(80, 20), QPointF(80, 60), QPointF(20, 60)},
                                         false)});
    canvas.setCurrentIndex(0);
    canvas.setEditMode();
    canvas.resize(160, 140);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 40));
    QTest::mouseMove(&canvas, QPoint(20, 10));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 10));

    const Shape shape = canvas.shapes().first();
    QCOMPARE(shape.shapeType, QStringLiteral("oriented_rectangle"));
    QCOMPARE(shape.points.size(), 4);
    QCOMPARE(shape.boundingRect().center(), QPointF(50, 40));
    QVERIFY(shape.points[0] != QPointF(20, 20));
}

void UiTests::canvasOrientedRectangleRotationMatchesLabelMeWithoutClipping() {
    Canvas canvas;
    QPixmap pixmap(40, 40);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    const QVector<QPointF> original{
        QPointF(2, 2), QPointF(32, 2), QPointF(32, 18), QPointF(2, 18)};
    canvas.setShapes({Shape::fromPoints(QStringLiteral("edge"),
                                        QStringLiteral("oriented_rectangle"),
                                        original,
                                        false)});
    canvas.setCurrentIndex(0);
    canvas.setEditMode();
    canvas.resize(40, 40);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    // The top rotation handle is at (17, 2). Rotating it toward (35, 35)
    // moves one corner outside the image. LabelMe still applies that
    // rotation; only vertex/shape translation is bounded.
    QTest::mouseMove(&canvas, QPoint(17, 2));
    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(17, 2));
    QTest::mouseMove(&canvas, QPoint(35, 35));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(35, 35));

    QVERIFY(canvas.shapes().first().points != original);
    bool hasOutsideCorner = false;
    for (const QPointF &point : canvas.shapes().first().points) {
        if (point.x() < 0.0 || point.y() < 0.0 || point.x() > 40.0 || point.y() > 40.0) {
            hasOutsideCorner = true;
        }
    }
    QVERIFY(hasOutsideCorner);
}

void UiTests::canvasOrientedRectangleVertexDragClipsAndPreservesParallelogram() {
    Canvas canvas;
    QPixmap pixmap(160, 120);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    Shape shape = Shape::fromPoints(QStringLiteral("rect"),
                                    QStringLiteral("oriented_rectangle"),
                                    {QPointF(50, 50), QPointF(90, 30), QPointF(110, 70), QPointF(70, 90)},
                                    false);
    canvas.setShapes({shape});
    canvas.setEditMode();
    canvas.resize(160, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    const QVector<QPointF> original = canvas.shapes().first().points;
    QTest::mouseMove(&canvas, QPoint(110, 70));
    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(110, 70));
    QTest::mouseMove(&canvas, QPoint(-100, -100));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(-100, -100));

    const QVector<QPointF> moved = canvas.shapes().first().points;
    QVERIFY(moved != original);
    for (const QPointF &point : moved) {
        QVERIFY(point.x() >= -0.001 && point.x() <= pixmap.width() + 0.001);
        QVERIFY(point.y() >= -0.001 && point.y() <= pixmap.height() + 0.001);
    }

    const QPointF firstSide = moved[1] - moved[0];
    const QPointF oppositeSide = moved[2] - moved[3];
    QVERIFY(qAbs(firstSide.x() - oppositeSide.x()) < 0.001);
    QVERIFY(qAbs(firstSide.y() - oppositeSide.y()) < 0.001);
    for (int i = 0; i < moved.size(); ++i) {
        const QPointF edge = moved[(i + 1) % moved.size()] - moved[i];
        QVERIFY(std::hypot(edge.x(), edge.y()) > 1.0);
    }
}

void UiTests::canvasVertexHandlesAreEasierToGrabWithoutCrossCursor() {
    Canvas canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromRect(QStringLiteral("defect"), QRectF(20, 20, 30, 30), false)});
    canvas.setScale(8.0);
    canvas.setEditMode();
    canvas.resize(300, 300);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QMouseEvent hoverMove(QEvent::MouseMove,
                          QPointF(170, 170),
                          QPointF(170, 170),
                          canvas.mapToGlobal(QPoint(170, 170)),
                          Qt::NoButton,
                          Qt::NoButton,
                          Qt::NoModifier);
    QApplication::sendEvent(&canvas, &hoverMove);
    QCOMPARE(canvas.cursor().shape(), Qt::SizeFDiagCursor);

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(170, 170));
    QTest::mouseMove(&canvas, QPoint(186, 186));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(186, 186));

    const QRectF box = canvas.shapes().first().boundingRect();
    QVERIFY2(box.topLeft().x() > 20.5 && box.topLeft().y() > 20.5, qPrintable(QString::number(box.topLeft().x())));
    QCOMPARE(qRound(box.bottomRight().x()), 50);
    QCOMPARE(qRound(box.bottomRight().y()), 50);
}

void UiTests::canvasWindowStyleResizeUsesEdges() {
    Canvas canvas;
    QPixmap pixmap(120, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromRect(QStringLiteral("defect"), QRectF(20, 20, 30, 30), false)});
    canvas.setScale(2.0);
    canvas.setEditMode();
    canvas.resize(260, 220);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QMouseEvent hoverEdge(QEvent::MouseMove,
                          QPointF(100, 70),
                          QPointF(100, 70),
                          canvas.mapToGlobal(QPoint(100, 70)),
                          Qt::NoButton,
                          Qt::NoButton,
                          Qt::NoModifier);
    QApplication::sendEvent(&canvas, &hoverEdge);
    QCOMPARE(canvas.cursor().shape(), Qt::SizeHorCursor);

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(100, 70));
    QTest::mouseMove(&canvas, QPoint(120, 70));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(120, 70));

    const QRectF box = canvas.shapes().first().boundingRect();
    QCOMPARE(qRound(box.left()), 20);
    QCOMPARE(qRound(box.top()), 20);
    QCOMPARE(qRound(box.right()), 60);
    QCOMPARE(qRound(box.bottom()), 50);
}

void UiTests::canvasPointBackedCircleMovesFromNonVertexEdge() {
    Canvas canvas;
    QPixmap pixmap(160, 120);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromPoints(QStringLiteral("circle"),
                                        QStringLiteral("circle"),
                                        {QPointF(60, 60), QPointF(60, 30)},
                                        false)});
    canvas.setEditMode();
    canvas.resize(160, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    // The right-most circumference is not a stored vertex. LabelMe treats
    // this gesture as moving the whole circle, not resizing its bbox.
    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(90, 60));
    QTest::mouseMove(&canvas, QPoint(100, 60));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(100, 60));

    const Shape shape = canvas.shapes().first();
    QCOMPARE(shape.shapeType, QStringLiteral("circle"));
    QCOMPARE(shape.points.size(), 2);
    QCOMPARE(shape.points[0], QPointF(70, 60));
    QCOMPARE(shape.points[1], QPointF(70, 30));
}

void UiTests::canvasSmallSelectedShapeUsesCompactResizeHandles() {
    Canvas canvas;
    QPixmap pixmap(200, 200);
    pixmap.fill(QColor(30, 30, 30));
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromRect(QStringLiteral("tiny"), QRectF(90, 90, 8, 8), false)});
    canvas.setScale(0.5);
    canvas.setEditMode();
    canvas.resize(120, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QImage rendered(canvas.size(), QImage::Format_ARGB32);
    rendered.fill(Qt::transparent);
    canvas.render(&rendered);

    const int whitePixels = countWhitePixelsInRect(rendered, QRect(34, 34, 28, 28));
    QVERIFY2(whitePixels < 120, qPrintable(QStringLiteral("white handle pixels: %1").arg(whitePixels)));
}

void UiTests::canvasResizeHandlesAreVisuallySmallerButHitAreaWider() {
    Canvas canvas;
    QPixmap pixmap(120, 120);
    pixmap.fill(QColor(30, 30, 30));
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromRect(QStringLiteral("defect"), QRectF(40, 40, 40, 40), false)});
    canvas.setScale(1.0);
    canvas.setEditMode();
    canvas.resize(140, 140);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QImage rendered(canvas.size(), QImage::Format_ARGB32);
    rendered.fill(Qt::transparent);
    canvas.render(&rendered);
    const int whitePixels = countWhitePixelsInRect(rendered, QRect(30, 30, 60, 60));
    QVERIFY2(whitePixels < 1200, qPrintable(QStringLiteral("white handle pixels: %1").arg(whitePixels)));

    QMouseEvent hoverEdge(QEvent::MouseMove,
                          QPointF(27, 60),
                          QPointF(27, 60),
                          canvas.mapToGlobal(QPoint(27, 60)),
                          Qt::NoButton,
                          Qt::NoButton,
                          Qt::NoModifier);
    QApplication::sendEvent(&canvas, &hoverEdge);
    QCOMPARE(canvas.cursor().shape(), Qt::SizeHorCursor);
}

void UiTests::canvasBackspaceRemovesHoveredPolygonVertex() {
    Canvas canvas;
    QPixmap pixmap(120, 100);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromPolygon(QStringLiteral("defect"),
                                         {QPointF(20, 20), QPointF(80, 20), QPointF(90, 70), QPointF(25, 80)},
                                         false)});
    canvas.setEditMode();
    canvas.resize(120, 100);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QMouseEvent hoverVertex(QEvent::MouseMove,
                            QPointF(20, 20),
                            QPointF(20, 20),
                            canvas.mapToGlobal(QPoint(20, 20)),
                            Qt::NoButton,
                            Qt::NoButton,
                            Qt::NoModifier);
    QApplication::sendEvent(&canvas, &hoverVertex);
    QVERIFY(canvas.removeSelectedPoint());
    QCOMPARE(canvas.shapes().first().points.size(), 3);
    QCOMPARE(canvas.shapes().first().points.first(), QPointF(80, 20));
    QVERIFY(!canvas.removeSelectedPoint());
}

void UiTests::canvasSelectedShapeFillIsHighlyTransparent() {
    Canvas canvas;
    QPixmap pixmap(80, 80);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setShapes({Shape::fromRect(QStringLiteral("defect"), QRectF(20, 20, 40, 40), false)});
    canvas.setCurrentIndex(-1);
    canvas.resize(100, 100);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QImage rendered(canvas.size(), QImage::Format_ARGB32);
    rendered.fill(Qt::transparent);
    canvas.render(&rendered);

    const QColor center = rendered.pixelColor(40, 40);
    QVERIFY2(center.red() > 245, qPrintable(center.name(QColor::HexArgb)));
    QVERIFY2(center.green() > 220, qPrintable(center.name(QColor::HexArgb)));
    QVERIFY2(center.blue() > 220, qPrintable(center.name(QColor::HexArgb)));
}

void UiTests::canvasSelectedShapeUsesLabelPaletteFill() {
    Canvas canvas;
    QPixmap pixmap(80, 80);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    Shape shape = Shape::fromRect(QStringLiteral("defect"), QRectF(20, 20, 40, 40), false);
    shape.fillColor = QColor(255, 0, 0, 128);
    canvas.setShapes({shape});
    canvas.setCurrentIndex(0);
    canvas.setSelectedIndices({0});
    canvas.resize(80, 80);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QImage rendered(canvas.size(), QImage::Format_ARGB32);
    rendered.fill(Qt::transparent);
    canvas.render(&rendered);

    const QColor center = rendered.pixelColor(40, 40);
    QVERIFY2(center.red() > 220 && center.green() < 180 && center.blue() < 180,
             qPrintable(center.name(QColor::HexArgb)));
}

void UiTests::canvasFillDrawingPreviewCanBeToggled() {
    Canvas canvas;
    QPixmap pixmap(100, 80);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);
    canvas.setCreateShapeType(QStringLiteral("polygon"));
    canvas.setCreateMode(true);
    canvas.resize(100, 80);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QVERIFY(QMetaObject::invokeMethod(&canvas,
                                      "setFillDrawing",
                                      Qt::DirectConnection,
                                      Q_ARG(bool, true)));
    bool enabled = false;
    QVERIFY(QMetaObject::invokeMethod(&canvas,
                                      "fillDrawing",
                                      Qt::DirectConnection,
                                      Q_RETURN_ARG(bool, enabled)));
    QVERIFY(enabled);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 15));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 15));
    QMouseEvent hoverMove(QEvent::MouseMove,
                          QPointF(50, 60),
                          QPointF(50, 60),
                          canvas.mapToGlobal(QPoint(50, 60)),
                          Qt::NoButton,
                          Qt::NoButton,
                          Qt::NoModifier);
    QApplication::sendEvent(&canvas, &hoverMove);

    QImage rendered(canvas.size(), QImage::Format_ARGB32);
    rendered.fill(Qt::transparent);
    canvas.render(&rendered);
    const QColor center = rendered.pixelColor(50, 35);
    QVERIFY2(center != QColor(Qt::white), qPrintable(center.name(QColor::HexArgb)));
}

void UiTests::canvasBrightnessAndContrastCanBeAdjusted() {
    Canvas canvas;
    QImage source(2, 1, QImage::Format_RGB32);
    source.setPixelColor(0, 0, QColor(80, 100, 120));
    source.setPixelColor(1, 0, QColor(180, 160, 140));
    canvas.setPixmap(QPixmap::fromImage(source));
    canvas.resize(2, 1);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    int contrastSet = 0;
    QVERIFY(QMetaObject::invokeMethod(&canvas,
                                      "setContrast",
                                      Qt::DirectConnection,
                                      Q_ARG(int, 150)));
    QVERIFY(QMetaObject::invokeMethod(&canvas,
                                      "contrast",
                                      Qt::DirectConnection,
                                      Q_RETURN_ARG(int, contrastSet)));
    QCOMPARE(contrastSet, 150);

    QImage rendered(canvas.size(), QImage::Format_ARGB32);
    rendered.fill(Qt::transparent);
    canvas.render(&rendered);
    QVERIFY(rendered.pixelColor(0, 0) != QColor(80, 100, 120));
}

void UiTests::canvasBrightnessContrastMatchesLabelMeEnhancement() {
    Canvas canvas;
    QImage source(3, 1, QImage::Format_RGB32);
    source.setPixelColor(0, 0, QColor(10, 20, 30));
    source.setPixelColor(1, 0, QColor(100, 110, 120));
    source.setPixelColor(2, 0, QColor(200, 210, 220));
    canvas.setPixmap(QPixmap::fromImage(source));
    canvas.resize(3, 1);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    // LabelMe applies Brightness(1.5), then Contrast(0.5). Contrast blends
    // the brightened image with its grayscale mean, rather than with 128.
    canvas.setBrightness(75);
    canvas.setContrast(25);

    QImage rendered(canvas.size(), QImage::Format_ARGB32);
    rendered.fill(Qt::transparent);
    canvas.render(&rendered);
    QCOMPARE(rendered.pixelColor(0, 0), QColor(81, 89, 96));
    QCOMPARE(rendered.pixelColor(1, 0), QColor(149, 156, 164));
    QCOMPARE(rendered.pixelColor(2, 0), QColor(201, 201, 201));
}

void UiTests::mainWindowFillDrawingActionIsPersisted() {
    resetTestSettings("fill-drawing-action");
    QSettings settings;
    settings.setValue(QStringLiteral("view/fillDrawing"), false);
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QAction *fillDrawingAction = window.findChild<QAction *>(QStringLiteral("fillDrawingAction"));
    QVERIFY(fillDrawingAction);
    QVERIFY(fillDrawingAction->isCheckable());
    QVERIFY(!fillDrawingAction->isChecked());

    fillDrawingAction->setChecked(true);
    QCOMPARE(QSettings().value(QStringLiteral("view/fillDrawing")).toBool(), true);
}

void UiTests::mainWindowDefaultsFillDrawingLikeLabelMe() {
    resetTestSettings("default-fill-drawing-labelme");
    MainWindow window;
    QAction *fillDrawingAction = window.findChild<QAction *>(QStringLiteral("fillDrawingAction"));
    QVERIFY(fillDrawingAction);
    QVERIFY(fillDrawingAction->isChecked());
}

void UiTests::mainWindowDefaultsToFitWindowLikeLabelMe() {
    resetTestSettings("default-fit-window-labelme");
    MainWindow window;
    QAction *fitWindowAction = window.findChild<QAction *>(QStringLiteral("fitWindowAction"));
    QAction *fitWidthAction = window.findChild<QAction *>(QStringLiteral("fitWidthAction"));
    QVERIFY(fitWindowAction);
    QVERIFY(fitWidthAction);
    QVERIFY(fitWindowAction->isCheckable());
    QVERIFY(fitWindowAction->isChecked());
    QVERIFY(!fitWidthAction->isChecked());
}

void UiTests::mainWindowBrightnessContrastActionsArePersisted() {
    resetTestSettings("brightness-contrast-actions");
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QAction *brightnessContrastAction = window.findChild<QAction *>(QStringLiteral("brightnessContrastAction"));
    QAction *keepPreviousAction = window.findChild<QAction *>(QStringLiteral("keepPreviousBrightnessContrastAction"));
    QVERIFY(brightnessContrastAction);
    QVERIFY(keepPreviousAction);
    QVERIFY(keepPreviousAction->isCheckable());

    keepPreviousAction->setChecked(true);
    QCOMPARE(QSettings().value(QStringLiteral("view/keepPreviousBrightnessContrast")).toBool(), true);
}

void UiTests::mainWindowToggleAllShapesVisibilityMatchesLabelMe() {
    resetTestSettings("toggle-all-shapes");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("toggle.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QAction *toggleAction = window.findChild<QAction *>(QStringLiteral("toggleAllAction"));
    Canvas *canvas = window.findChild<Canvas *>();
    QVERIFY(toggleAction);
    QVERIFY(canvas);
    QCOMPARE(toggleAction->shortcut(), QKeySequence(QStringLiteral("T")));

    Shape first = Shape::fromRect(QStringLiteral("first"), QRectF(10, 10, 20, 20), false);
    Shape second = Shape::fromRect(QStringLiteral("second"), QRectF(40, 10, 20, 20), false);
    first.visible = false;
    second.visible = true;
    canvas->setShapes({first, second});
    toggleAction->trigger();
    for (const Shape &shape : canvas->shapes()) {
        QVERIFY(shape.visible);
    }

    toggleAction->trigger();
    for (const Shape &shape : canvas->shapes()) {
        QVERIFY(!shape.visible);
    }
}

void UiTests::mainWindowResetLayoutRestoresDefaultDockState() {
    resetTestSettings("reset-layout");
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QAction *resetLayoutAction = window.findChild<QAction *>(QStringLiteral("resetLayoutAction"));
    QDockWidget *fileDock = window.findChild<QDockWidget *>(QStringLiteral("files"));
    QVERIFY(resetLayoutAction);
    QVERIFY(fileDock);
    QVERIFY(fileDock->isVisible());

    fileDock->hide();
    QVERIFY(!fileDock->isVisible());
    QSettings settings;
    settings.setValue(QStringLiteral("window/state"), QByteArrayLiteral("stale-state"));
    resetLayoutAction->trigger();
    QVERIFY(fileDock->isVisible());
    QVERIFY(!settings.contains(QStringLiteral("window/state")));
}

void UiTests::canvasScaleClampSamplingAndOverview() {
    Canvas canvas;
    QPixmap pixmap(400, 200);
    pixmap.fill(Qt::white);
    canvas.setPixmap(pixmap);

    canvas.setScale(0.001);
    QCOMPARE(canvas.scale(), 0.005);
    canvas.setScale(20.0);
    QCOMPARE(canvas.scale(), 16.0);

    QCOMPARE(canvas.samplingMode(), Canvas::SamplingMode::FastNearest);
    canvas.setSamplingMode(Canvas::SamplingMode::Smooth);
    QCOMPARE(canvas.samplingMode(), Canvas::SamplingMode::Smooth);

    QImage overview = canvas.overviewImage(128);
    QVERIFY(!overview.isNull());
    QCOMPARE(qMax(overview.width(), overview.height()), 128);
    QCOMPARE(overview.size(), QSize(128, 64));

    QSignalSpy frameSpy(&canvas, &Canvas::frameRendered);
    canvas.resize(160, 120);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));
    canvas.update();
    QTest::qWait(50);
    QVERIFY(frameSpy.count() > 0);
}

void UiTests::canvasPreviewKeepsOriginalCoordinateSize() {
    Canvas canvas;
    QPixmap preview(80, 40);
    preview.fill(Qt::white);
    canvas.setPreviewPixmap(preview, QSize(400, 200));
    canvas.setShapes({Shape::fromRect(QStringLiteral("defect"),
                                      QRectF(320, 120, 60, 50),
                                      false)});

    QVERIFY(canvas.isPreviewImage());
    QCOMPARE(canvas.pixmapSize(), QSize(400, 200));
    QCOMPARE(canvas.shapes().first().boundingRect(), QRectF(320, 120, 60, 50));
    QVERIFY(!canvas.overviewImage(128).isNull());
}

void UiTests::canvasUsesLabelMeOverscrollSlackWhenImageOverflows() {
    QScrollArea scrollArea;
    scrollArea.resize(180, 160);
    auto *canvas = new Canvas;
    QPixmap pixmap(100, 80);
    pixmap.fill(Qt::white);
    canvas->setPixmap(pixmap);
    scrollArea.setWidget(canvas);
    scrollArea.show();
    QVERIFY(QTest::qWaitForWindowExposed(&scrollArea));

    canvas->setScale(2.0);
    QTest::qWait(20);
    const int scaledWidth = qRound(pixmap.width() * canvas->scale());
    const int viewportWidth = scrollArea.viewport()->width();
    const int expectedSlack = qMax(viewportWidth / 8,
                                   qMin(viewportWidth / 2, scaledWidth - viewportWidth));
    QCOMPARE(canvas->sizeHint().width(), scaledWidth + expectedSlack);
}

void UiTests::mainWindowUpgradesLargeImagePreviewOnZoom() {
    resetTestSettings("large-image-preview");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(5000, 2500, QImage::Format_RGB32);
    image.fill(QColor(60, 70, 80));
    const QString imagePath = dir.filePath(QStringLiteral("large-preview.png"));
    QVERIFY(image.save(imagePath, "PNG"));

    MainWindow window;
    QVERIFY(window.openPath(imagePath));
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QSpinBox *zoom = window.findChild<QSpinBox *>(QStringLiteral("zoomWidget"));
    QVERIFY(canvas);
    QVERIFY(zoom);
    QVERIFY(canvas->isPreviewImage());
    QCOMPARE(canvas->pixmapSize(), QSize(5000, 2500));

    canvas->setShapes({Shape::fromRect(QStringLiteral("defect"), QRectF(3200, 1200, 400, 300), false)});
    zoom->setValue(50);
    QTRY_VERIFY_WITH_TIMEOUT(!canvas->isPreviewImage(), 5000);
    QCOMPARE(canvas->pixmapSize(), QSize(5000, 2500));
    QCOMPARE(canvas->shapes().first().boundingRect(), QRectF(3200, 1200, 400, 300));
}

void UiTests::mainWindowLanguageActionRefreshesTexts() {
    resetTestSettings("language-switch");
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QAction *openAction = actionByShortcut(&window, QKeySequence::Open);
    QVERIFY(openAction);
    QAction *zhAction = languageAction(&window, "zh-CN");
    QVERIFY(zhAction);

    zhAction->trigger();
    QCOMPARE(openAction->text(), QString::fromUtf8("打开文件"));

    QAction *enAction = languageAction(&window, QStringLiteral("en"));
    QAction *resetLayoutAction = window.findChild<QAction *>(QStringLiteral("resetLayoutAction"));
    QVERIFY(enAction);
    QVERIFY(resetLayoutAction);
    enAction->trigger();
    resetLayoutAction->trigger();
    QCOMPARE(window.statusBar()->currentMessage(), QStringLiteral("Layout restored"));
}

void UiTests::mainWindowDrawingModeActionsRefreshLanguage() {
    resetTestSettings("drawing-mode-language");
    MainWindow window;

    const QList<QPair<QString, QString>> expected = {
        {QStringLiteral("createPolygonModeAction"), QStringLiteral("Create Polygon")},
        {QStringLiteral("createPointModeAction"), QStringLiteral("Create Point")},
        {QStringLiteral("createPointsModeAction"), QStringLiteral("Create Points")},
        {QStringLiteral("createAiPointsModeAction"), QStringLiteral("AI Point Prompt")},
        {QStringLiteral("createAiBoxModeAction"), QStringLiteral("AI Box Prompt")},
        {QStringLiteral("createLineModeAction"), QStringLiteral("Create Line")},
        {QStringLiteral("createLinestripModeAction"), QStringLiteral("Create Linestrip")},
        {QStringLiteral("createCircleModeAction"), QStringLiteral("Create Circle")},
        {QStringLiteral("createOrientedRectangleModeAction"), QStringLiteral("Create Oriented Rectangle")},
        {QStringLiteral("createMaskModeAction"), QStringLiteral("Create Mask")},
        {QStringLiteral("viewModeAction"), QStringLiteral("View Mode")},
        {QStringLiteral("editabilityAction"), QStringLiteral("Editable Annotations")},
    };
    QAction *enAction = languageAction(&window, QStringLiteral("en"));
    QVERIFY(enAction);

    enAction->trigger();
    for (const auto &entry : expected) {
        QAction *action = window.findChild<QAction *>(entry.first);
        QVERIFY2(action, qPrintable(entry.first));
        QCOMPARE(action->text(), entry.second);
    }
}

void UiTests::mainWindowSecondaryTextsRefreshLanguage() {
    resetTestSettings("secondary-language");
    MainWindow window;
    QAction *enAction = languageAction(&window, QStringLiteral("en"));
    QVERIFY(enAction);

    enAction->trigger();

    const QList<QPair<QString, QString>> expectedActions = {
        {QStringLiteral("openWithImageViewerAction"), QStringLiteral("Open Image Viewer")},
        {QStringLiteral("openFileLocationAction"), QStringLiteral("Open File Location")},
        {QStringLiteral("fileContextOpenAction"), QStringLiteral("Open This File")},
        {QStringLiteral("fileContextRevealAction"), QStringLiteral("Open Location")},
        {QStringLiteral("fileContextCopyPathAction"), QStringLiteral("Copy Path")},
        {QStringLiteral("fileContextMarkAction"), QStringLiteral("Mark")},
        {QStringLiteral("fileContextDeleteAction"), QStringLiteral("Delete File")},
        {QStringLiteral("nextCopyAction"), QStringLiteral("Next Image (Copy Labels)")},
        {QStringLiteral("prevCopyAction"), QStringLiteral("Previous Image (Copy Labels)")},
        {QStringLiteral("resetLayoutAction"), QStringLiteral("Reset Layout")},
        {QStringLiteral("prevShapeAction"), QStringLiteral("Previous Shape")},
        {QStringLiteral("nextShapeAction"), QStringLiteral("Next Shape")},
        {QStringLiteral("insertPolygonPointAction"), QStringLiteral("Insert Polygon/Linestrip Point")},
        {QStringLiteral("removePolygonPointAction"), QStringLiteral("Remove Polygon/Linestrip Point")},
        {QStringLiteral("removeSelectedPointAction"), QStringLiteral("Remove Selected Vertex")},
        {QStringLiteral("toggleAllAction"), QStringLiteral("Toggle All Annotation Visibility")},
        {QStringLiteral("brightnessContrastAction"), QStringLiteral("Brightness/Contrast")},
        {QStringLiteral("keepPreviousBrightnessContrastAction"), QStringLiteral("Keep Previous Brightness/Contrast")},
        {QStringLiteral("fillDrawingAction"), QStringLiteral("Fill Drawing Preview")},
        {QStringLiteral("keepPreviousAction"), QStringLiteral("Keep Previous Annotation")},
        {QStringLiteral("keepPreviousZoomAction"), QStringLiteral("Keep Previous Zoom")},
        {QStringLiteral("editLabelFlagsAction"), QStringLiteral("Edit LabelMe label_flags")},
        {QStringLiteral("miniMapAction"), QStringLiteral("Mini-map")},
        {QStringLiteral("showPerformanceAction"), QStringLiteral("Performance")},
        {QStringLiteral("thumbnailModeAction"), QStringLiteral("File Thumbnail Mode")},
    };
    for (const auto &entry : expectedActions) {
        QAction *action = window.findChild<QAction *>(entry.first);
        QVERIFY2(action, qPrintable(entry.first));
        QCOMPARE(action->text(), entry.second);
    }
    QAction *undoAction = actionByShortcut(&window, QKeySequence::Undo);
    QAction *redoAction = actionByShortcut(&window, QKeySequence::Redo);
    QVERIFY(undoAction);
    QVERIFY(redoAction);
    QCOMPARE(undoAction->text(), QStringLiteral("Undo"));
    QCOMPARE(redoAction->text(), QStringLiteral("Redo"));

    QLineEdit *fileSearch = window.findChild<QLineEdit *>(QStringLiteral("fileSearchEdit"));
    QToolButton *labelFilter = window.findChild<QToolButton *>(QStringLiteral("fileLabelFilterButton"));
    QToolButton *modeButton = window.findChild<QToolButton *>(QStringLiteral("mainModeButton"));
    QListWidget *uniqueLabels = window.findChild<QListWidget *>(QStringLiteral("uniqueLabelList"));
    QDockWidget *labelDock = window.findChild<QDockWidget *>(QStringLiteral("labels"));
    QMenu *recentDirs = window.findChild<QMenu *>(QStringLiteral("recentDirsMenu"));
    QVERIFY(fileSearch);
    QVERIFY(labelFilter);
    QVERIFY(modeButton);
    QVERIFY(uniqueLabels);
    QVERIFY(labelDock);
    QVERIFY(recentDirs);
    QCOMPARE(fileSearch->placeholderText(), QStringLiteral("Search files"));
    QCOMPARE(fileSearch->toolTip(), QStringLiteral("Filter current folder by file name"));
    QCOMPARE(labelFilter->text(), QStringLiteral("Filter"));
    QCOMPARE(labelFilter->toolTip(), QStringLiteral("Filter files by labels"));
    QCOMPARE(labelDock->windowTitle(), QStringLiteral("Label Panel"));
    QCOMPARE(recentDirs->title(), QStringLiteral("Recently Opened Folders"));

    QAction *zhAction = languageAction(&window, QStringLiteral("zh-CN"));
    QVERIFY(zhAction);
    zhAction->trigger();
    QVERIFY(modeButton->toolTip().contains(QString::fromUtf8("点击切换模式")));
    QVERIFY(uniqueLabels->toolTip().contains(QString::fromUtf8("新建标注")));
}

void UiTests::mainWindowClipboardStatusRefreshesLanguage() {
    resetTestSettings("clipboard-status-language");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("clipboard.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *zhAction = languageAction(&window, QStringLiteral("zh-CN"));
    QVERIFY(canvas);
    QVERIFY(zhAction);

    canvas->setShapes({Shape::fromRect(QStringLiteral("defect"), QRectF(10, 10, 20, 18), false)});
    canvas->setCurrentIndex(0);
    zhAction->trigger();

    QVERIFY(QMetaObject::invokeMethod(&window,
                                      "copySelectedShapesToClipboard",
                                      Qt::DirectConnection));
    const QString copiedMessage = window.statusBar()->currentMessage();
    QVERIFY(copiedMessage.contains(QString::fromUtf8("复制")));
    QVERIFY(!copiedMessage.contains(QStringLiteral("Copied")));

    QVERIFY(QMetaObject::invokeMethod(&window,
                                      "pasteShapesFromClipboard",
                                      Qt::DirectConnection));
    const QString pastedMessage = window.statusBar()->currentMessage();
    QVERIFY(pastedMessage.contains(QString::fromUtf8("粘贴")));
    QVERIFY(!pastedMessage.contains(QStringLiteral("Pasted")));
}

void UiTests::canvasStatusTextRefreshesWithMainWindowLanguage() {
    resetTestSettings("canvas-language");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath(QStringLiteral("language.jpg"))));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    Canvas *canvas = window.findChild<Canvas *>();
    QAction *polygonAction = window.findChild<QAction *>(QStringLiteral("createPolygonModeAction"));
    QAction *zhAction = languageAction(&window, QStringLiteral("zh-CN"));
    QVERIFY(canvas);
    QVERIFY(polygonAction);
    QVERIFY(zhAction);

    zhAction->trigger();
    polygonAction->trigger();
    QSignalSpy statusSpy(canvas, &Canvas::statusTextChanged);
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(canvas, QPoint(30, 30));
    QVERIFY(!statusSpy.isEmpty());
    QVERIFY(statusSpy.last().at(0).toString().contains(QString::fromUtf8("多边形")));
    canvas->cancelDrawing();
}

void UiTests::mainWindowContextPlacementActionsRefreshLanguage() {
    resetTestSettings("context-placement-language");
    MainWindow window;
    QAction *copyHereAction = window.findChild<QAction *>(QStringLiteral("copyHereAction"));
    QAction *moveHereAction = window.findChild<QAction *>(QStringLiteral("moveHereAction"));
    QAction *zhAction = languageAction(&window, "zh-CN");
    QVERIFY(copyHereAction);
    QVERIFY(moveHereAction);
    QVERIFY(zhAction);

    zhAction->trigger();
    QCOMPARE(copyHereAction->text(), QString::fromUtf8("复制到此处"));
    QCOMPARE(moveHereAction->text(), QString::fromUtf8("移动到此处"));
}

void UiTests::mainWindowSettingsActionOpensAndPersists() {
    resetTestSettings("settings-dialog");
    MainWindow window;
    QAction *settingsAction = window.findChild<QAction *>(QStringLiteral("settingsAction"));
    QVERIFY(settingsAction);
    QVERIFY(settingsAction->shortcuts().contains(QKeySequence(QStringLiteral("Ctrl+Shift+,"))));

    QTimer::singleShot(60, []() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QVERIFY(dialog);
        auto *autoSave = dialog->findChild<QCheckBox *>(QStringLiteral("settingsAutoSave"));
        auto *format = dialog->findChild<QComboBox *>(QStringLiteral("settingsFormatCombo"));
        auto *validateLabel = dialog->findChild<QComboBox *>(QStringLiteral("settingsValidateLabelCombo"));
        QVERIFY(autoSave);
        QVERIFY(format);
        QVERIFY(validateLabel);
        autoSave->setChecked(true);
        format->setCurrentText(QStringLiteral("LabelMe"));
        validateLabel->setCurrentIndex(validateLabel->findData(QStringLiteral("exact")));
        dialog->accept();
    });
    settingsAction->trigger();

    QSettings settings;
    QCOMPARE(settings.value(QStringLiteral("autosave")).toBool(), true);
    QCOMPARE(settings.value(QStringLiteral("labelFileFormat")).toInt(), 3);
    QCOMPARE(settings.value(QStringLiteral("labelme/validateLabel")).toString(), QStringLiteral("exact"));
}

void UiTests::mainWindowSettingsEditsCanvasInteractionConfig() {
    resetTestSettings("settings-canvas-interaction");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString configPath = dir.filePath(QStringLiteral("labelmerc"));
    QFile config(configPath);
    QVERIFY(config.open(QIODevice::WriteOnly | QIODevice::Text));
    config.write("epsilon: 5\nshape:\n  point_size: 6\ncanvas:\n  double_click: close\n  snapping: true\n  crosshair:\n    rectangle: true\n");
    config.close();

    MainWindow window(nullptr, configPath);
    Canvas *canvas = window.findChild<Canvas *>();
    QAction *settingsAction = window.findChild<QAction *>(QStringLiteral("settingsAction"));
    QVERIFY(canvas);
    QVERIFY(settingsAction);
    QCOMPARE(canvas->epsilon(), 5.0);
    QCOMPARE(canvas->pointSize(), 6);
    QVERIFY(canvas->doubleClickClose());
    QVERIFY(canvas->snapping());
    QVERIFY(canvas->crosshairEnabledForShapeType(QStringLiteral("rectangle")));

    QTimer::singleShot(60, []() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            QFAIL("settings dialog did not open");
            return;
        }
        auto *epsilon = dialog->findChild<QDoubleSpinBox *>(QStringLiteral("settingsEpsilon"));
        auto *pointSize = dialog->findChild<QSpinBox *>(QStringLiteral("settingsPointSize"));
        auto *doubleClick = dialog->findChild<QCheckBox *>(QStringLiteral("settingsDoubleClickClose"));
        auto *snapping = dialog->findChild<QCheckBox *>(QStringLiteral("settingsSnapping"));
        auto *crosshair = dialog->findChild<QCheckBox *>(QStringLiteral("settingsCrosshair"));
        if (!epsilon || !pointSize || !doubleClick || !snapping || !crosshair) {
            dialog->reject();
            return;
        }
        epsilon->setValue(18.5);
        pointSize->setValue(12);
        doubleClick->setChecked(false);
        snapping->setChecked(false);
        crosshair->setChecked(false);
        dialog->accept();
    });
    settingsAction->trigger();

    QCOMPARE(canvas->epsilon(), 18.5);
    QCOMPARE(canvas->pointSize(), 12);
    QVERIFY(!canvas->doubleClickClose());
    QVERIFY(!canvas->snapping());
    QVERIFY(!canvas->crosshairEnabledForShapeType(QStringLiteral("rectangle")));

    QVariantMap values;
    QString error;
    QVERIFY2(LabelMeConfig::loadFile(configPath, &values, &error), qPrintable(error));
    QCOMPARE(values.value(QStringLiteral("epsilon")).toDouble(), 18.5);
    QCOMPARE(values.value(QStringLiteral("shape.point_size")).toInt(), 12);
    QCOMPARE(values.value(QStringLiteral("canvas.double_click")).toString(), QStringLiteral("none"));
    QCOMPARE(values.value(QStringLiteral("canvas.snapping")).toBool(), false);
    QCOMPARE(values.value(QStringLiteral("canvas.crosshair.rectangle")).toBool(), false);
}

void UiTests::mainWindowSettingsCanOpenConfigFileAsText() {
    resetTestSettings("settings-open-config");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString configPath = dir.filePath(QStringLiteral("labelmerc"));
    QFile config(configPath);
    QVERIFY(config.open(QIODevice::WriteOnly | QIODevice::Text));
    config.write("auto_save: true\n");
    config.close();

    MainWindow window(nullptr, configPath);
    QAction *settingsAction = window.findChild<QAction *>(QStringLiteral("settingsAction"));
    QVERIFY(settingsAction);

    QTimer::singleShot(60, []() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            QFAIL("settings dialog did not open");
            return;
        }
        auto *openConfig = dialog->findChild<QPushButton *>(QStringLiteral("settingsOpenConfigButton"));
        if (!openConfig) {
            dialog->reject();
            QFAIL("settings dialog has no config text editor button");
            return;
        }
        QVERIFY(openConfig->isEnabled());
        dialog->reject();
    });
    settingsAction->trigger();
}

void UiTests::mainWindowSettingsRejectsChangesWhenConfigWriteFails() {
    resetTestSettings("settings-write-failure");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString configPath = dir.filePath(QStringLiteral("labelmerc"));
    QFile config(configPath);
    QVERIFY(config.open(QIODevice::WriteOnly | QIODevice::Text));
    config.write("auto_save: true\n");
    config.close();

    MainWindow window(nullptr, configPath);
    QAction *settingsAction = window.findChild<QAction *>(QStringLiteral("settingsAction"));
    QAction *autoSaveAction = window.findChild<QAction *>(QStringLiteral("autoSaveAction"));
    QVERIFY(settingsAction);
    QVERIFY(autoSaveAction);
    QVERIFY(autoSaveAction->isChecked());
    const QVariant initialAutoSaveSetting = QSettings().value(QStringLiteral("autosave"));

    QVERIFY(QFile::remove(configPath));
    QVERIFY(QDir().mkpath(configPath));

    QTimer::singleShot(60, []() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            QFAIL("settings dialog did not open");
            return;
        }
        auto *autoSave = dialog->findChild<QCheckBox *>(QStringLiteral("settingsAutoSave"));
        if (!autoSave) {
            dialog->reject();
            QFAIL("settings dialog has no auto-save control");
            return;
        }
        autoSave->setChecked(false);
        dialog->accept();
    });
    settingsAction->trigger();

    QVERIFY(autoSaveAction->isChecked());
    QCOMPARE(QSettings().value(QStringLiteral("autosave")), initialAutoSaveSetting);
    QVERIFY(QDir(configPath).removeRecursively());
}

void UiTests::mainWindowExactLabelValidationRejectsUnknownLabel() {
    resetTestSettings("exact-label-validation");
    QSettings settings;
    settings.setValue(QStringLiteral("labelme/validateLabel"), QStringLiteral("exact"));
    settings.setValue(QStringLiteral("labelHistory"), QStringList{QStringLiteral("allowed")});

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("validation.png"));
    QVERIFY(image.save(imagePath));

    AnnotationDocument doc;
    doc.imagePath = imagePath;
    doc.imageSize = image.size();
    doc.labelMeVersion = QStringLiteral("5.7.0");
    doc.shapes = {Shape::fromRect(QStringLiteral("allowed"), QRectF(5, 5, 20, 15), false)};
    const QString jsonPath = dir.filePath(QStringLiteral("validation.json"));
    QVERIFY(AnnotationIO::saveLabelMe(jsonPath, doc));

    MainWindow window;
    QVERIFY(window.openPath(jsonPath));
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    auto *canvas = window.findChild<Canvas *>();
    QAction *editAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+E")));
    QVERIFY(labelList);
    QVERIFY(canvas);
    QVERIFY(editAction);
    labelList->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);

    bool promptSeen = false;
    bool warningSeen = false;
    auto *watcher = new QTimer(&window);
    watcher->setInterval(20);
    connect(watcher, &QTimer::timeout, &window, [&]() {
        QWidget *modal = QApplication::activeModalWidget();
        if (auto *dialog = qobject_cast<QDialog *>(modal)) {
            if (dialog->objectName() == QStringLiteral("labelEditDialog")) {
                auto *combo = dialog->findChild<QComboBox *>(QStringLiteral("labelEditCombo"));
                if (combo) {
                    combo->setCurrentText(QStringLiteral("unknown"));
                    promptSeen = true;
                    dialog->accept();
                }
            }
        }
        if (auto *box = qobject_cast<QMessageBox *>(modal)) {
            warningSeen = true;
            box->accept();
            watcher->stop();
        }
    });
    watcher->start();
    editAction->trigger();

    QVERIFY(promptSeen);
    QVERIFY(warningSeen);
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("allowed"));
}

void UiTests::mainWindowExposesSaveWithImageDataInFileMenu() {
    resetTestSettings("save-with-image-data-action");
    MainWindow window;
    QAction *saveWithData = window.findChild<QAction *>(QStringLiteral("embedImageDataAction"));
    QVERIFY(saveWithData);
    QVERIFY(saveWithData->isCheckable());
    QVERIFY(saveWithData->text().contains(QStringLiteral("imageData")) ||
            saveWithData->text().contains(QString::fromUtf8("图像数据")));

    QMenu *fileMenu = window.findChild<QMenu *>(QStringLiteral("fileMenu"));
    QVERIFY(fileMenu);
    QVERIFY(fileMenu->actions().contains(saveWithData));
    saveWithData->setChecked(true);
    QSettings settings;
    QCOMPARE(settings.value(QStringLiteral("labelme/embedImageData")).toBool(), true);
}

void UiTests::mainWindowSaveAsUsesFormatDefaultSuffix() {
    resetTestSettings("save-as-default-suffix");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(40, 30, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("save-as-source.png"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *formatCombo = window.findChild<QComboBox *>(QStringLiteral("footerFormatCombo"));
    auto *saveAsAction = window.findChild<QAction *>(QStringLiteral("saveAsAction"));
    QVERIFY(formatCombo);
    QVERIFY(saveAsAction);
    const QStringList expectedSuffixes{QStringLiteral("xml"), QStringLiteral("txt"),
                                       QStringLiteral("json"), QStringLiteral("json")};
    QCOMPARE(formatCombo->count(), expectedSuffixes.size());
    for (int formatIndex = 0; formatIndex < expectedSuffixes.size(); ++formatIndex) {
        formatCombo->setCurrentIndex(formatIndex);
        QCoreApplication::processEvents();

        bool dialogSeen = false;
        QString observedSuffix;
        QString observedDirectory;
        const QString baseName = QStringLiteral("saved-%1").arg(formatIndex);
        QTimer::singleShot(0, &window, [&]() {
            for (QWidget *widget : QApplication::topLevelWidgets()) {
                auto *dialog = qobject_cast<QFileDialog *>(widget);
                if (!dialog || dialog->objectName() != QStringLiteral("saveAsDialog")) {
                    continue;
                }
                dialogSeen = true;
                observedSuffix = dialog->defaultSuffix();
                observedDirectory = dialog->directory().absolutePath();
                dialog->selectFile(dir.filePath(baseName));
                static_cast<QDialog *>(dialog)->accept();
                break;
            }
        });
        saveAsAction->trigger();

        QTRY_VERIFY_WITH_TIMEOUT(dialogSeen, 3000);
        QCOMPARE(observedSuffix, expectedSuffixes.at(formatIndex));
        QCOMPARE(QFileInfo(observedDirectory).absoluteFilePath(), dir.path());
        QVERIFY(QFileInfo::exists(dir.filePath(baseName + QStringLiteral(".") +
                                                expectedSuffixes.at(formatIndex))));
    }
}

void UiTests::mainWindowExposesDeleteAnnotationActionForCurrentLabelFile() {
    resetTestSettings("delete-annotation-action");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(40, 30, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("delete-label.png"));
    QVERIFY(image.save(imagePath));

    AnnotationDocument doc;
    doc.imagePath = imagePath;
    doc.imageSize = image.size();
    doc.labelMeVersion = QStringLiteral("5.7.0");
    doc.shapes = {Shape::fromRect(QStringLiteral("label"), QRectF(2, 3, 10, 8), false)};
    const QString jsonPath = dir.filePath(QStringLiteral("delete-label.json"));
    QVERIFY(AnnotationIO::saveLabelMe(jsonPath, doc));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *action = window.findChild<QAction *>(QStringLiteral("deleteAnnotationAction"));
    auto *fileMenu = window.findChild<QMenu *>(QStringLiteral("fileMenu"));
    QVERIFY(action);
    QVERIFY(fileMenu);
    QVERIFY(action->isEnabled());
    QVERIFY(fileMenu->actions().contains(action));
    QVERIFY(action->text().contains(QStringLiteral("annotation"), Qt::CaseInsensitive) ||
            action->text().contains(QString::fromUtf8("标注")));
}

void UiTests::mainWindowDeletesCurrentAnnotationFileAndClearsShapes() {
    resetTestSettings("delete-annotation-behavior");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(40, 30, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("delete-label.png"));
    QVERIFY(image.save(imagePath));

    AnnotationDocument doc;
    doc.imagePath = imagePath;
    doc.imageSize = image.size();
    doc.labelMeVersion = QStringLiteral("5.7.0");
    doc.shapes = {Shape::fromRect(QStringLiteral("label"), QRectF(2, 3, 10, 8), false)};
    const QString jsonPath = dir.filePath(QStringLiteral("delete-label.json"));
    QVERIFY(AnnotationIO::saveLabelMe(jsonPath, doc));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *canvas = window.findChild<Canvas *>();
    auto *fileList = window.findChild<QListWidget *>(QStringLiteral("fileList"));
    auto *action = window.findChild<QAction *>(QStringLiteral("deleteAnnotationAction"));
    QVERIFY(canvas);
    QVERIFY(fileList);
    QVERIFY(action);
    QCOMPARE(canvas->shapes().size(), 1);
    QVERIFY(action->isEnabled());

    QListWidgetItem *fileItem = nullptr;
    for (int i = 0; i < fileList->count(); ++i) {
        if (QFileInfo(fileList->item(i)->text()).absoluteFilePath() ==
            QFileInfo(imagePath).absoluteFilePath()) {
            fileItem = fileList->item(i);
            break;
        }
    }
    QVERIFY(fileItem);
    QCOMPARE(fileItem->checkState(), Qt::Checked);

    bool confirmationSeen = false;
    auto *watcher = new QTimer(&window);
    watcher->setInterval(10);
    connect(watcher, &QTimer::timeout, &window, [&]() {
        if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
            confirmationSeen = true;
            if (auto *yesButton = box->button(QMessageBox::Yes)) {
                yesButton->click();
            } else {
                box->accept();
            }
            watcher->stop();
        }
    });
    watcher->start();
    action->trigger();

    QVERIFY(confirmationSeen);
    QVERIFY(!QFileInfo::exists(jsonPath));
    QVERIFY(canvas->shapes().isEmpty());
    fileItem = nullptr;
    for (int i = 0; i < fileList->count(); ++i) {
        if (QFileInfo(fileList->item(i)->text()).absoluteFilePath() ==
            QFileInfo(imagePath).absoluteFilePath()) {
            fileItem = fileList->item(i);
            break;
        }
    }
    QVERIFY(fileItem);
    QCOMPARE(fileItem->checkState(), Qt::Unchecked);
    QVERIFY(!action->isEnabled());
}

void UiTests::mainWindowCloseFileDisablesCanvasAndClearsActiveState() {
    resetTestSettings("close-file-behavior");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(48, 32, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("close-file.png"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    QVERIFY(window.openPath(imagePath));
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *canvas = window.findChild<Canvas *>();
    auto *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    auto *fileList = window.findChild<QListWidget *>(QStringLiteral("fileList"));
    auto *closeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+W")));
    QVERIFY(canvas);
    QVERIFY(labelList);
    QVERIFY(fileList);
    QVERIFY(closeAction);
    QVERIFY(canvas->isEnabled());
    QVERIFY(closeAction->isEnabled());

    canvas->setShapes({Shape::fromRect(QStringLiteral("close_me"), QRectF(3, 4, 12, 9), false)});
    QVERIFY(!canvas->shapes().isEmpty());
    QVERIFY(QMetaObject::invokeMethod(&window, "closeFile", Qt::DirectConnection));

    QVERIFY(!canvas->isEnabled());
    QVERIFY(canvas->pixmapSize().isEmpty());
    QVERIFY(canvas->shapes().isEmpty());
    QCOMPARE(labelList->count(), 0);
    QVERIFY(fileList->selectedItems().isEmpty());
    QCOMPARE(window.windowTitle(), QStringLiteral("labelImgCpp"));
    QVERIFY(!closeAction->isEnabled());

    QVERIFY(window.openPath(imagePath));
    QVERIFY(canvas->isEnabled());
    QVERIFY(closeAction->isEnabled());
}

void UiTests::mainWindowUsesFramelessChrome() {
    resetTestSettings("frameless-chrome");
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QVERIFY(window.windowFlags().testFlag(Qt::FramelessWindowHint));
    QWidget *titleBar = window.findChild<QWidget *>("framelessTitleBar");
    QVERIFY(titleBar);
    QVERIFY(window.findChild<QToolButton *>("minimizeButton"));
    QVERIFY(window.findChild<QToolButton *>("maximizeButton"));
    QVERIFY(window.findChild<QToolButton *>("closeButton"));
    QVERIFY(window.menuWidget());
}

#ifdef Q_OS_WIN
LRESULT nativeHitTest(HWND hwnd, int screenX, int screenY) {
    const LPARAM point = MAKELPARAM(static_cast<short>(screenX), static_cast<short>(screenY));
    return SendMessageW(hwnd, WM_NCHITTEST, 0, point);
}
#endif

void UiTests::mainWindowNativeStyleSupportsResizeAndSnap() {
    resetTestSettings("native-resize-snap-style");
    MainWindow window;
    window.resize(900, 600);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QVERIFY(window.maximumWidth() > window.width());
    QVERIFY(window.maximumHeight() > window.height());

#ifdef Q_OS_WIN
    const HWND hwnd = reinterpret_cast<HWND>(window.winId());
    QVERIFY(hwnd);
    const quintptr style = static_cast<quintptr>(GetWindowLongPtrW(hwnd, GWL_STYLE));
    QVERIFY2(hasWindowsWindowChromeStyle(style),
             qPrintable(QStringLiteral("native style 0x%1 lacks resize/snap bits")
                            .arg(style, 0, 16)));
#endif
}

void UiTests::mainWindowNativeHitTestSupportsEveryResizeEdge() {
#ifndef Q_OS_WIN
    QSKIP("Native resize hit testing is Windows-specific");
#else
    resetTestSettings("native-resize-hit-test");
    MainWindow window;
    window.resize(900, 600);
    window.move(120, 120);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QVERIFY(!window.isMaximized());

    const HWND hwnd = reinterpret_cast<HWND>(window.winId());
    QVERIFY(hwnd);
    RECT nativeRect{};
    QVERIFY(GetWindowRect(hwnd, &nativeRect));

    const int left = nativeRect.left + 1;
    const int right = nativeRect.right - 2;
    const int top = nativeRect.top + 1;
    const int bottom = nativeRect.bottom - 2;
    const int centerX = nativeRect.left + (nativeRect.right - nativeRect.left) / 2;
    const int centerY = nativeRect.top + (nativeRect.bottom - nativeRect.top) / 2;

    QCOMPARE(nativeHitTest(hwnd, left, top), static_cast<LRESULT>(HTTOPLEFT));
    QCOMPARE(nativeHitTest(hwnd, centerX, top), static_cast<LRESULT>(HTTOP));
    QCOMPARE(nativeHitTest(hwnd, right, top), static_cast<LRESULT>(HTTOPRIGHT));
    QCOMPARE(nativeHitTest(hwnd, left, centerY), static_cast<LRESULT>(HTLEFT));
    QCOMPARE(nativeHitTest(hwnd, right, centerY), static_cast<LRESULT>(HTRIGHT));
    QCOMPARE(nativeHitTest(hwnd, left, bottom), static_cast<LRESULT>(HTBOTTOMLEFT));
    QCOMPARE(nativeHitTest(hwnd, centerX, bottom), static_cast<LRESULT>(HTBOTTOM));
    QCOMPARE(nativeHitTest(hwnd, right, bottom), static_cast<LRESULT>(HTBOTTOMRIGHT));
    QCOMPARE(nativeHitTest(hwnd, centerX, centerY), static_cast<LRESULT>(HTCLIENT));
#endif
}

void UiTests::mainWindowUsesGeneratedCppIcon() {
    resetTestSettings("generated-cpp-icon");
    MainWindow window;
    QVERIFY(!window.windowIcon().isNull());
}

void UiTests::mainWindowEmbedsToolbarIntoFramelessTitleBar() {
    resetTestSettings("frameless-toolbar");
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QWidget *titleBar = window.findChild<QWidget *>("framelessTitleBar");
    QToolBar *toolBar = window.findChild<QToolBar *>("mainToolBar");
    QVERIFY(titleBar);
    QVERIFY(toolBar);
    QVERIFY(titleBar->isAncestorOf(toolBar));
    QCOMPARE(window.toolBarArea(toolBar), Qt::NoToolBarArea);
    QVERIFY(toolBar->findChildren<QToolButton *>().size() > 3);
}

void UiTests::mainWindowTitleToolAreaDoubleClickTogglesMaximize() {
    resetTestSettings("title-tool-area-double-click");
    MainWindow window;
    window.resize(900, 600);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QToolBar *toolBar = window.findChild<QToolBar *>(QStringLiteral("mainToolBar"));
    QMenuBar *menuBar = window.findChild<QMenuBar *>();
    QVERIFY(toolBar);
    QVERIFY(menuBar);
    QVERIFY(!window.isMaximized());

    QTest::mouseDClick(menuBar, Qt::LeftButton, Qt::NoModifier, menuBar->rect().center());
    QTRY_VERIFY(window.isMaximized());

    QTest::mouseDClick(menuBar, Qt::LeftButton, Qt::NoModifier, menuBar->rect().center());
    QTRY_VERIFY(!window.isMaximized());

    QTest::mouseDClick(toolBar, Qt::LeftButton, Qt::NoModifier, QPoint(toolBar->width() - 8, toolBar->height() / 2));
    QTRY_VERIFY(window.isMaximized());

    QTest::mouseDClick(toolBar, Qt::LeftButton, Qt::NoModifier, QPoint(toolBar->width() - 8, toolBar->height() / 2));
    QTRY_VERIFY(!window.isMaximized());
}

void UiTests::mainWindowRecoversWindowPositionWhenSavedScreenIsUnavailable() {
    resetTestSettings("window-position-screen-recovery");
    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    QVERIFY(primaryScreen);
    const QRect available = primaryScreen->availableGeometry();
    QVERIFY(available.isValid());

    QSettings settings;
    settings.setValue(QStringLiteral("window/size"), QSize(320, 240));
    settings.setValue(QStringLiteral("window/position"),
                      QPoint(available.right() + 2000, available.bottom() + 2000));

    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    bool intersectsScreen = false;
    for (QScreen *screen : QGuiApplication::screens()) {
        if (screen && screen->availableGeometry().intersects(window.frameGeometry())) {
            intersectsScreen = true;
            break;
        }
    }
    QVERIFY2(intersectsScreen, "restored window should be moved back onto an available screen");
}

void UiTests::mainWindowPlacesMenusAndToolsInSingleTitleRow() {
    resetTestSettings("single-title-row");
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QWidget *titleBar = window.findChild<QWidget *>("framelessTitleBar");
    QMenuBar *menuBar = titleBar->findChild<QMenuBar *>();
    QToolBar *toolBar = window.findChild<QToolBar *>("mainToolBar");
    QVERIFY(titleBar);
    QVERIFY(menuBar);
    QVERIFY(toolBar);
    QVERIFY(titleBar->isAncestorOf(menuBar));
    QVERIFY(titleBar->isAncestorOf(toolBar));

    const int menuY = menuBar->mapToGlobal(QPoint(0, 0)).y();
    const int toolY = toolBar->mapToGlobal(QPoint(0, 0)).y();
    QVERIFY2(qAbs(menuY - toolY) <= 4, "menu and toolbar should share the same title row");

    QWidget *topChrome = window.findChild<QWidget *>("framelessTopChrome");
    QVERIFY(topChrome);
    QVERIFY(topChrome->layout());
    QCOMPARE(topChrome->layout()->count(), 1);
}

void UiTests::mainWindowTitleToolbarUsesIconOnlyButtons() {
    resetTestSettings("title-toolbar-icons");
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QToolBar *toolBar = window.findChild<QToolBar *>("mainToolBar");
    QVERIFY(toolBar);
    QCOMPARE(toolBar->toolButtonStyle(), Qt::ToolButtonIconOnly);

    auto verifyToolbarActions = [&]() {
        QCOMPARE(toolBar->iconSize(), QSize(22, 22));
        int actionCount = 0;
        for (QAction *action : toolBar->actions()) {
            if (action->isSeparator() || qobject_cast<QWidgetAction *>(action)) {
                continue;
            }
            ++actionCount;
            QVERIFY2(!action->icon().isNull(), qPrintable(action->text()));
            QVERIFY2(!action->toolTip().isEmpty(), qPrintable(action->text()));
            const QString iconFile = action->property("toolbarIconFile").toString();
            QVERIFY2(iconFile.startsWith(QStringLiteral("toolbar/")), qPrintable(action->text()));
        }
        QVERIFY(actionCount > 3);
    };

    verifyToolbarActions();
    QAction *advancedModeAction = window.findChild<QAction *>(QStringLiteral("advancedModeAction"));
    QVERIFY(advancedModeAction);
    advancedModeAction->trigger();
    verifyToolbarActions();
}

void UiTests::mainWindowFooterContainsModeFormatAndMiniMapControls() {
    resetTestSettings("footer-mode-format-minimap");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(40, 40, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QToolBar *toolBar = window.findChild<QToolBar *>(QStringLiteral("mainToolBar"));
    QVERIFY(toolBar);
    QAction *viewModeAction = window.findChild<QAction *>(QStringLiteral("viewModeAction"));
    QAction *editModeAction = window.findChild<QAction *>(QStringLiteral("editModeAction"));
    QAction *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QAction *miniMapAction = window.findChild<QAction *>(QStringLiteral("miniMapAction"));
    QAction *formatAction = window.findChild<QAction *>(QStringLiteral("formatAction"));
    QToolButton *mainModeButton = window.findChild<QToolButton *>(QStringLiteral("mainModeButton"));
    QToolButton *miniMapButton = window.findChild<QToolButton *>(QStringLiteral("footerMiniMapButton"));
    QComboBox *formatCombo = window.findChild<QComboBox *>(QStringLiteral("footerFormatCombo"));
    QComboBox *modeCombo = window.findChild<QComboBox *>(QStringLiteral("footerModeCombo"));
    QToolButton *viewShortcut = window.findChild<QToolButton *>(QStringLiteral("footerViewShortcut"));
    QToolButton *editShortcut = window.findChild<QToolButton *>(QStringLiteral("footerEditShortcut"));
    QToolButton *createShortcut = window.findChild<QToolButton *>(QStringLiteral("footerCreateShortcut"));
    QVERIFY(viewModeAction);
    QVERIFY(editModeAction);
    QVERIFY(createModeAction);
    QVERIFY(miniMapAction);
    QVERIFY(formatAction);
    QVERIFY(mainModeButton);
    QVERIFY(miniMapButton);
    QVERIFY(formatCombo);
    QVERIFY(modeCombo);
    QVERIFY(viewShortcut);
    QVERIFY(editShortcut);
    QVERIFY(createShortcut);

    QVERIFY(!toolBar->actions().contains(viewModeAction));
    QVERIFY(!toolBar->actions().contains(editModeAction));
    QVERIFY(!toolBar->actions().contains(createModeAction));
    QVERIFY(mainModeButton->menu());
    QCOMPARE(mainModeButton->popupMode(), QToolButton::InstantPopup);
    QVERIFY(mainModeButton->menu()->actions().contains(viewModeAction));
    QVERIFY(mainModeButton->menu()->actions().contains(editModeAction));
    QVERIFY(mainModeButton->menu()->actions().contains(createModeAction));
    QVERIFY(!toolBar->actions().contains(miniMapAction));
    QVERIFY(!toolBar->actions().contains(formatAction));
    QCOMPARE(miniMapButton->defaultAction(), miniMapAction);
    QVERIFY(!miniMapButton->text().isEmpty());

    QVERIFY(modeCombo->findText(QStringLiteral("查看 (V)")) >= 0);
    QVERIFY(modeCombo->findText(QStringLiteral("编辑")) >= 0);
    QVERIFY(modeCombo->findText(QStringLiteral("新建 (W)")) >= 0);
    QCOMPARE(viewShortcut->text(), QStringLiteral("V"));
    QCOMPARE(editShortcut->text(), QStringLiteral("编辑"));
    QCOMPARE(createShortcut->text(), QStringLiteral("W"));

    viewModeAction->trigger();
    QVERIFY(viewModeAction->isChecked());
    QCOMPARE(modeCombo->currentText(), QStringLiteral("查看 (V)"));
    QCOMPARE(mainModeButton->defaultAction(), viewModeAction);

    QCOMPARE(formatCombo->currentText(), QStringLiteral("PascalVOC"));
    QVERIFY(formatCombo->findText(QStringLiteral("LabelMe")) >= 0);
    formatCombo->setCurrentText(QStringLiteral("YOLO"));
    QCOMPARE(formatAction->text(), QStringLiteral("YOLO"));
    formatCombo->setCurrentText(QStringLiteral("LabelMe"));
    QCOMPARE(formatAction->text(), QStringLiteral("LabelMe"));
}

void UiTests::mainWindowFooterControlsRefreshLanguage() {
    resetTestSettings("footer-language-refresh");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(40, 40, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QComboBox *modeCombo = window.findChild<QComboBox *>(QStringLiteral("footerModeCombo"));
    QToolButton *viewShortcut = window.findChild<QToolButton *>(QStringLiteral("footerViewShortcut"));
    QToolButton *editShortcut = window.findChild<QToolButton *>(QStringLiteral("footerEditShortcut"));
    QToolButton *createShortcut = window.findChild<QToolButton *>(QStringLiteral("footerCreateShortcut"));
    QVERIFY(modeCombo);
    QVERIFY(viewShortcut);
    QVERIFY(editShortcut);
    QVERIFY(createShortcut);

    QAction *english = languageAction(&window, QStringLiteral("en"));
    QAction *simplifiedChinese = languageAction(&window, QStringLiteral("zh-CN"));
    QVERIFY(english);
    QVERIFY(simplifiedChinese);

    english->trigger();
    QCOMPARE(modeCombo->itemText(modeCombo->findData(QStringLiteral("view"))), QStringLiteral("View (V)"));
    QCOMPARE(modeCombo->itemText(modeCombo->findData(QStringLiteral("edit"))), QStringLiteral("Edit"));
    QCOMPARE(modeCombo->itemText(modeCombo->findData(QStringLiteral("create"))), QStringLiteral("Create (W)"));
    QCOMPARE(viewShortcut->toolTip(), QStringLiteral("View mode"));
    QCOMPARE(editShortcut->text(), QStringLiteral("Edit"));
    QCOMPARE(editShortcut->toolTip(), QStringLiteral("Edit mode"));
    QCOMPARE(createShortcut->text(), QStringLiteral("W"));
    QCOMPARE(createShortcut->toolTip(), QStringLiteral("Create box"));

    simplifiedChinese->trigger();
    QCOMPARE(modeCombo->itemText(modeCombo->findData(QStringLiteral("view"))), QString::fromUtf8("查看 (V)"));
    QCOMPARE(modeCombo->itemText(modeCombo->findData(QStringLiteral("edit"))), QString::fromUtf8("编辑"));
    QCOMPARE(modeCombo->itemText(modeCombo->findData(QStringLiteral("create"))), QString::fromUtf8("新建 (W)"));
    QCOMPARE(viewShortcut->toolTip(), QString::fromUtf8("查看模式"));
    QCOMPARE(editShortcut->text(), QString::fromUtf8("编辑"));
    QCOMPARE(editShortcut->toolTip(), QString::fromUtf8("编辑模式"));
    QCOMPARE(createShortcut->text(), QString::fromUtf8("W"));
    QCOMPARE(createShortcut->toolTip(), QString::fromUtf8("新建框"));
}

void UiTests::mainWindowHelpMenuIncludesTutorialAction() {
    resetTestSettings("help-menu-tutorial");
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QMenu *helpMenu = window.findChild<QMenu *>(QStringLiteral("helpMenu"));
    QAction *tutorialAction = window.findChild<QAction *>(QStringLiteral("tutorialAction"));
    QVERIFY(helpMenu);
    QVERIFY(tutorialAction);
    QVERIFY(helpMenu->actions().contains(tutorialAction));
    QVERIFY(!tutorialAction->toolTip().isEmpty());

    QAction *english = languageAction(&window, QStringLiteral("en"));
    QAction *simplifiedChinese = languageAction(&window, QStringLiteral("zh-CN"));
    QVERIFY(english);
    QVERIFY(simplifiedChinese);
    english->trigger();
    QCOMPARE(tutorialAction->text(), QStringLiteral("Tutorial"));
    simplifiedChinese->trigger();
    QCOMPARE(tutorialAction->text(), QString::fromUtf8("YouTube教学"));
}

void UiTests::mainWindowLabelPanelRefreshesLanguage() {
    resetTestSettings("label-panel-language-refresh");
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const QList<QPair<QString, QString>> englishLabels = {
        {QStringLiteral("newLabelTitle"), QStringLiteral("New Label")},
        {QStringLiteral("aiModelLabel"), QStringLiteral("AI Model")},
        {QStringLiteral("aiOutputLabel"), QStringLiteral("AI Output")},
        {QStringLiteral("aiTextPromptLabel"), QStringLiteral("AI Text Prompt")},
        {QStringLiteral("aiScoreLabel"), QStringLiteral("Score")},
        {QStringLiteral("aiIouLabel"), QStringLiteral("IoU")},
        {QStringLiteral("topLevelFlagsLabel"), QStringLiteral("LabelMe flags")},
    };
    const QList<QPair<QString, QString>> chineseLabels = {
        {QStringLiteral("newLabelTitle"), QString::fromUtf8("新建标签")},
        {QStringLiteral("aiModelLabel"), QString::fromUtf8("AI 模型")},
        {QStringLiteral("aiOutputLabel"), QString::fromUtf8("AI 输出")},
        {QStringLiteral("aiTextPromptLabel"), QString::fromUtf8("AI 文本提示")},
        {QStringLiteral("aiScoreLabel"), QStringLiteral("分数")},
        {QStringLiteral("aiIouLabel"), QStringLiteral("IoU")},
        {QStringLiteral("topLevelFlagsLabel"), QStringLiteral("LabelMe flags")},
    };

    QAction *english = languageAction(&window, QStringLiteral("en"));
    QAction *simplifiedChinese = languageAction(&window, QStringLiteral("zh-CN"));
    QVERIFY(english);
    QVERIFY(simplifiedChinese);
    english->trigger();
    for (const auto &entry : englishLabels) {
        QLabel *label = window.findChild<QLabel *>(entry.first);
        QVERIFY2(label, qPrintable(entry.first));
        QCOMPARE(label->text(), entry.second);
    }
    simplifiedChinese->trigger();
    for (const auto &entry : chineseLabels) {
        QLabel *label = window.findChild<QLabel *>(entry.first);
        QVERIFY2(label, qPrintable(entry.first));
        QCOMPARE(label->text(), entry.second);
    }
}

void UiTests::mainWindowConstructionDoesNotConsumeTestRunnerArguments() {
    resetTestSettings("constructor-startup-boundary");
    MainWindow window;
    Canvas *canvas = window.findChild<Canvas *>();
    QAction *viewerAction = window.findChild<QAction *>(QStringLiteral("openWithImageViewerAction"));
    QVERIFY(canvas);
    QVERIFY(viewerAction);
    QVERIFY(!viewerAction->isEnabled());
    QVERIFY(QApplication::activeModalWidget() == nullptr);
}

void UiTests::mainWindowSettingsDialogUsesActiveLanguage() {
    resetTestSettings("settings-language-refresh");
    MainWindow window;
    QAction *english = languageAction(&window, QStringLiteral("en"));
    QAction *settingsAction = window.findChild<QAction *>(QStringLiteral("settingsAction"));
    QVERIFY(english);
    QVERIFY(settingsAction);
    english->trigger();

    bool inspected = false;
    bool correct = false;
    QTimer::singleShot(60, [&]() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            return;
        }
        inspected = true;
        auto *tabs = dialog->findChild<QTabWidget *>();
        auto *displayLabelPopup = dialog->findChild<QCheckBox *>(QStringLiteral("settingsDisplayLabelPopup"));
        correct = tabs && displayLabelPopup &&
                  tabs->tabText(0) == QStringLiteral("General") &&
                  tabs->tabText(1) == QStringLiteral("View & Annotation") &&
                  displayLabelPopup->text() == QStringLiteral("Show label editor when creating annotation");
        dialog->reject();
    });
    settingsAction->trigger();
    QVERIFY(inspected);
    QVERIFY(correct);
}

void UiTests::mainWindowFileListTextsRefreshLanguage() {
    resetTestSettings("file-list-language-refresh");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(24, 24, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath(QStringLiteral("a.jpg"))));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), dir.path()});
    QAction *english = languageAction(&window, QStringLiteral("en"));
    QAction *simplifiedChinese = languageAction(&window, QStringLiteral("zh-CN"));
    QLabel *title = window.findChild<QLabel *>(QStringLiteral("fileDockTitleLabel"));
    QMenu *filterMenu = window.findChild<QMenu *>(QStringLiteral("fileLabelFilterMenu"));
    QVERIFY(english);
    QVERIFY(simplifiedChinese);
    QVERIFY(title);
    QVERIFY(filterMenu);
    QVERIFY(!filterMenu->actions().isEmpty());

    english->trigger();
    QCOMPARE(title->text(), QStringLiteral("File List (1)"));
    QCOMPARE(filterMenu->actions().first()->text(), QStringLiteral("All (1)"));

    simplifiedChinese->trigger();
    QCOMPARE(title->text(), QString::fromUtf8("文件列表 (1)"));
    QCOMPARE(filterMenu->actions().first()->text(), QString::fromUtf8("全部 (1)"));
}

void UiTests::mainWindowTopToolbarIncludesLabelMeFileActions() {
    resetTestSettings("toolbar-file-actions-in-menu");
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QToolBar *toolBar = window.findChild<QToolBar *>(QStringLiteral("mainToolBar"));
    QVERIFY(toolBar);
    QAction *openAction = actionByShortcut(&window, QKeySequence::Open);
    QAction *openDirAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+U")));
    QAction *changeSaveDirAction = window.findChild<QAction *>(QStringLiteral("changeSaveDirAction"));
    QAction *saveAction = window.findChild<QAction *>(QStringLiteral("saveAction"));
    QAction *previousAction = actionByShortcut(&window, QKeySequence(QStringLiteral("A")));
    QAction *nextAction = actionByShortcut(&window, QKeySequence(QStringLiteral("D")));
    QVERIFY(openAction);
    QVERIFY(openDirAction);
    QVERIFY(changeSaveDirAction);
    QVERIFY(saveAction);
    QVERIFY(previousAction);
    QVERIFY(nextAction);

    QVERIFY(toolBar->actions().contains(openAction));
    QVERIFY(toolBar->actions().contains(openDirAction));
    QVERIFY(toolBar->actions().contains(saveAction));
    QVERIFY(!toolBar->actions().contains(changeSaveDirAction));

    const auto indexOf = [toolBar](QAction *action) { return toolBar->actions().indexOf(action); };
    QVERIFY(indexOf(openAction) >= 0);
    QVERIFY(indexOf(openDirAction) > indexOf(openAction));
    QVERIFY(indexOf(previousAction) > indexOf(openDirAction));
    QVERIFY(indexOf(nextAction) > indexOf(previousAction));
    QVERIFY(indexOf(saveAction) > indexOf(nextAction));
}

void UiTests::mainWindowHasOpenWithDropdownForCurrentImage() {
    resetTestSettings("open-with-dropdown");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(20, 20, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath("open_with.jpg");
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *button = window.findChild<QToolButton *>(QStringLiteral("openWithButton"));
    auto *viewerAction = window.findChild<QAction *>(QStringLiteral("openWithImageViewerAction"));
    auto *locationAction = window.findChild<QAction *>(QStringLiteral("openFileLocationAction"));
    QVERIFY(button);
    QVERIFY(viewerAction);
    QVERIFY(locationAction);
    QCOMPARE(button->popupMode(), QToolButton::MenuButtonPopup);
    QCOMPARE(button->defaultAction(), viewerAction);
    QVERIFY(button->menu());
    QCOMPARE(button->menu()->actions().size(), 2);
    QCOMPARE(button->menu()->actions().at(0), viewerAction);
    QCOMPARE(button->menu()->actions().at(1), locationAction);
    QVERIFY(!button->isEnabled());
    QVERIFY(!viewerAction->isEnabled());
    QVERIFY(!locationAction->isEnabled());

    window.loadStartupArgs({"labelImgCpp", imagePath});
    QApplication::processEvents();

    QVERIFY(button->isEnabled());
    QVERIFY(viewerAction->isEnabled());
    QVERIFY(locationAction->isEnabled());
    QCOMPARE(viewerAction->text(), QString::fromUtf8("图像查看"));
    QCOMPARE(locationAction->text(), QString::fromUtf8("文件位置"));
}

void UiTests::mainWindowTitleBarShowsCurrentFileNameAndCompactToolbar() {
    resetTestSettings("titlebar-current-file-compact-toolbar");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(20, 20, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath("current_file.jpg");
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QLabel *titleLabel = window.findChild<QLabel *>(QStringLiteral("framelessTitleLabel"));
    QVERIFY(titleLabel);
    QVERIFY(titleLabel->isVisibleTo(&window));
    QCOMPARE(titleLabel->text(), QStringLiteral("current_file.jpg"));

    QToolBar *toolBar = window.findChild<QToolBar *>(QStringLiteral("mainToolBar"));
    QVERIFY(toolBar);
    const QList<QToolButton *> buttons = toolBar->findChildren<QToolButton *>();
    QVERIFY(buttons.size() > 3);
    for (QToolButton *button : buttons) {
        QVERIFY2(button->width() <= 36, qPrintable(button->toolTip()));
    }
}

void UiTests::mainWindowTitleBarUsesStandardWindowControls() {
    resetTestSettings("standard-window-controls");
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QToolButton *minimizeButton = window.findChild<QToolButton *>(QStringLiteral("minimizeButton"));
    QToolButton *maximizeButton = window.findChild<QToolButton *>(QStringLiteral("maximizeButton"));
    QToolButton *closeButton = window.findChild<QToolButton *>(QStringLiteral("closeButton"));
    QVERIFY(minimizeButton);
    QVERIFY(maximizeButton);
    QVERIFY(closeButton);

    QCOMPARE(minimizeButton->text(), QString::fromUtf8("\u2212"));
    QCOMPARE(maximizeButton->text(), QString::fromUtf8("\u25a1"));
    QCOMPARE(closeButton->text(), QString::fromUtf8("\u00d7"));
    QVERIFY(minimizeButton->width() >= 40);
    QVERIFY(maximizeButton->width() >= 40);
    QVERIFY(closeButton->width() >= 40);
}

void UiTests::mainWindowShowsPerformanceAndMiniMapControls() {
    resetTestSettings("perf-minimap-controls");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(120, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath("image.jpg");
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QLabel *performanceLabel = window.findChild<QLabel *>("performanceLabel");
    QAction *showPerformanceAction = window.findChild<QAction *>("showPerformanceAction");
    QAction *miniMapAction = window.findChild<QAction *>("miniMapAction");
    QAction *samplingModeAction = window.findChild<QAction *>("samplingModeAction");
    MiniMapOverlay *miniMap = window.findChild<MiniMapOverlay *>("miniMapOverlay");
    QVERIFY(performanceLabel);
    QVERIFY(showPerformanceAction);
    QVERIFY(miniMapAction);
    QVERIFY(samplingModeAction);
    QVERIFY(miniMap);

    QVERIFY(performanceLabel->isVisibleTo(&window));
    QVERIFY(miniMap->isVisible());
    showPerformanceAction->trigger();
    QVERIFY(!performanceLabel->isVisible());
    miniMapAction->trigger();
    QVERIFY(!miniMap->isVisible());

    QSettings settings;
    QCOMPARE(settings.value("view/showPerformance").toBool(), false);
    QCOMPARE(settings.value("view/miniMapEnabled").toBool(), false);
}

void UiTests::miniMapDrawsVisibleShapeBoxes() {
    auto *canvas = new Canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas->setPixmap(pixmap);
    canvas->setShapes({Shape::fromRect(QStringLiteral("defect"), QRectF(20, 20, 40, 30), false)});

    QScrollArea scrollArea;
    scrollArea.setWidget(canvas);
    scrollArea.resize(180, 160);
    MiniMapOverlay miniMap(canvas, &scrollArea, scrollArea.viewport());
    miniMap.setMiniMapEnabled(true);
    scrollArea.show();
    QVERIFY(QTest::qWaitForWindowExposed(&scrollArea));
    miniMap.refreshGeometry();
    QVERIFY(miniMap.isVisible());

    QImage rendered(miniMap.size(), QImage::Format_ARGB32);
    rendered.fill(Qt::transparent);
    miniMap.render(&rendered);

    QVERIFY2(countPreviewBoxPixels(rendered) > 8, "Mini map did not render visible shape boxes");
}

void UiTests::miniMapIsSemiTransparentUntilHovered() {
    auto *canvas = new Canvas;
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::white);
    canvas->setPixmap(pixmap);

    QScrollArea scrollArea;
    scrollArea.setWidget(canvas);
    scrollArea.resize(180, 160);
    MiniMapOverlay miniMap(canvas, &scrollArea, scrollArea.viewport());
    miniMap.setMiniMapEnabled(true);
    scrollArea.show();
    QVERIFY(QTest::qWaitForWindowExposed(&scrollArea));
    miniMap.refreshGeometry();
    QVERIFY(miniMap.isVisible());

    // The previous UI test may leave the offscreen pointer inside this newly
    // positioned child. Establish the state under test explicitly.
    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(&miniMap, &leaveEvent);

    QImage idle(miniMap.size(), QImage::Format_ARGB32);
    idle.fill(Qt::transparent);
    miniMap.render(&idle);
    QVERIFY2(idle.pixelColor(idle.rect().center()).alpha() < 190,
             qPrintable(idle.pixelColor(idle.rect().center()).name(QColor::HexArgb)));

    QEnterEvent enter(QPointF(5, 5), QPointF(5, 5), miniMap.mapToGlobal(QPoint(5, 5)));
    QApplication::sendEvent(&miniMap, &enter);
    QImage hovered(miniMap.size(), QImage::Format_ARGB32);
    hovered.fill(Qt::transparent);
    miniMap.render(&hovered);
    QVERIFY2(hovered.pixelColor(hovered.rect().center()).alpha() > idle.pixelColor(idle.rect().center()).alpha(),
             qPrintable(hovered.pixelColor(hovered.rect().center()).name(QColor::HexArgb)));
}

void UiTests::mainWindowFpsUsesFrameCadenceNotRenderDuration() {
    resetTestSettings("fps-cadence");
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QLabel *performanceLabel = window.findChild<QLabel *>(QStringLiteral("performanceLabel"));
    QVERIFY(performanceLabel);

    QVERIFY(QMetaObject::invokeMethod(&window, "onCanvasFrameRendered", Qt::DirectConnection, Q_ARG(double, 0.01)));
    const QString text = performanceLabel->text();
    QRegularExpression match(QStringLiteral("FPS\\s+([0-9]+(?:\\.[0-9]+)?)"));
    const QRegularExpressionMatch result = match.match(text);
    QVERIFY2(result.hasMatch(), qPrintable(text));

    const double fps = result.captured(1).toDouble();
    QVERIFY2(fps < 1000.0, qPrintable(text));
}

void UiTests::mainWindowMiniMapClickChangesScrollbars() {
    resetTestSettings("minimap-scroll");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(800, 600, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath("large.jpg");
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.resize(500, 400);
    window.loadStartupArgs({"labelImgCpp", imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QScrollArea *scrollArea = window.findChild<QScrollArea *>();
    MiniMapOverlay *miniMap = window.findChild<MiniMapOverlay *>("miniMapOverlay");
    QVERIFY(canvas);
    QVERIFY(scrollArea);
    QVERIFY(miniMap);

    canvas->setScale(2.0);
    QApplication::processEvents();
    QVERIFY(scrollArea->horizontalScrollBar()->maximum() > 0);
    QVERIFY(scrollArea->verticalScrollBar()->maximum() > 0);

    const int oldH = scrollArea->horizontalScrollBar()->value();
    const int oldV = scrollArea->verticalScrollBar()->value();
    QPoint clickPoint = miniMap->rect().center();
    QTest::mouseClick(miniMap, Qt::LeftButton, Qt::NoModifier, clickPoint);

    QVERIFY(scrollArea->horizontalScrollBar()->value() != oldH ||
            scrollArea->verticalScrollBar()->value() != oldV);
}

void UiTests::mainWindowRestoresScrollPositionPerImage() {
    resetTestSettings("per-image-scroll-position");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    for (const QString &name : {QStringLiteral("a.jpg"), QStringLiteral("b.jpg")}) {
        QImage image(900, 700, QImage::Format_RGB32);
        image.fill(name == QStringLiteral("a.jpg") ? QColor(240, 240, 240) : QColor(220, 220, 220));
        QVERIFY(image.save(dir.filePath(name)));
    }

    MainWindow window;
    window.resize(500, 400);
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *canvas = window.findChild<Canvas *>();
    auto *scrollArea = window.findChild<QScrollArea *>();
    auto *zoomWidget = window.findChild<QSpinBox *>(QStringLiteral("zoomWidget"));
    auto *nextAction = actionByShortcut(&window, QKeySequence(QStringLiteral("D")));
    auto *prevAction = actionByShortcut(&window, QKeySequence(QStringLiteral("A")));
    QVERIFY(canvas);
    QVERIFY(scrollArea);
    QVERIFY(zoomWidget);
    QVERIFY(nextAction);
    QVERIFY(prevAction);

    // This test asserts scroll positions in image pixels. Explicitly leave
    // LabelMe's default FIT_WINDOW mode before creating overflow.
    zoomWidget->setValue(150);
    QCoreApplication::processEvents();
    auto *hBar = scrollArea->horizontalScrollBar();
    auto *vBar = scrollArea->verticalScrollBar();
    QVERIFY(hBar->maximum() > 0);
    QVERIFY(vBar->maximum() > 0);
    const int expectedH = qMax(1, hBar->maximum() * 3 / 5);
    const int expectedV = qMax(1, vBar->maximum() * 2 / 5);
    hBar->setValue(expectedH);
    vBar->setValue(expectedV);

    nextAction->trigger();
    QCoreApplication::processEvents();
    QVERIFY(hBar->maximum() > 0);
    QVERIFY(vBar->maximum() > 0);
    hBar->setValue(hBar->maximum());
    vBar->setValue(vBar->maximum());
    prevAction->trigger();
    QCoreApplication::processEvents();

    QCOMPARE(hBar->value(), expectedH);
    QCOMPARE(vBar->value(), expectedV);
}

void UiTests::mainWindowCentersZoomedOutImages() {
    resetTestSettings("center-zoomed-out-image");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath("small.jpg");
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.resize(500, 400);
    window.loadStartupArgs({"labelImgCpp", imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QScrollArea *scrollArea = window.findChild<QScrollArea *>();
    QVERIFY(canvas);
    QVERIFY(scrollArea);

    canvas->setScale(0.5);
    QApplication::processEvents();

    QTRY_COMPARE(canvas->size(), QSize(50, 40));
    const QPoint canvasCenterInViewport = canvas->mapTo(scrollArea->viewport(), canvas->rect().center());
    const QPoint viewportCenter = scrollArea->viewport()->rect().center();
    QVERIFY(qAbs(canvasCenterInViewport.x() - viewportCenter.x()) <= 1);
    QVERIFY(qAbs(canvasCenterInViewport.y() - viewportCenter.y()) <= 1);
}

void UiTests::mainWindowFileListUsesReadableSelectionStyleAndBottomDock() {
    resetTestSettings("file-list-style");
    MainWindow window;
    QListWidget *fileList = window.findChild<QListWidget *>("fileList");
    QDockWidget *fileDock = window.findChild<QDockWidget *>("files");
    QDockWidget *labelDock = window.findChild<QDockWidget *>("labels");
    QVERIFY(fileList);
    QVERIFY(fileDock);
    QVERIFY(labelDock);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QVERIFY(fileList->font().pointSize() >= 10);
    QVERIFY(!fileList->wordWrap());
    QCOMPARE(fileList->textElideMode(), Qt::ElideMiddle);
    QVERIFY(fileList->styleSheet().contains("QListWidget::item:selected"));
    QVERIFY(fileList->styleSheet().contains("QListWidget::item[activeFile=\"true\"]"));
    QVERIFY(fileList->styleSheet().contains("background"));
    QCOMPARE(window.dockWidgetArea(fileDock), Qt::RightDockWidgetArea);
    QCOMPARE(window.dockWidgetArea(labelDock), Qt::RightDockWidgetArea);
    QVERIFY(fileDock->geometry().top() >= labelDock->geometry().top());
}

void UiTests::mainWindowFileListCheckStateTracksAnnotationPresence() {
    resetTestSettings("file-list-annotation-state");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(40, 30, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("state.png"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *fileList = window.findChild<QListWidget *>(QStringLiteral("fileList"));
    QVERIFY(fileList);
    QCOMPARE(fileList->count(), 1);
    QVERIFY(fileList->item(0)->flags().testFlag(Qt::ItemIsUserCheckable));
    QCOMPARE(fileList->item(0)->checkState(), Qt::Unchecked);

    QVERIFY(QMetaObject::invokeMethod(&window, "saveFile", Qt::DirectConnection));
    QTRY_COMPARE(fileList->item(0)->checkState(), Qt::Checked);
}

void UiTests::mainWindowIgnoresLegacyBottomFileDockState() {
    resetTestSettings("legacy-bottom-file-dock-state");
    QByteArray legacyState;
    {
        MainWindow seed;
        QDockWidget *fileDock = seed.findChild<QDockWidget *>("files");
        QVERIFY(fileDock);
        seed.addDockWidget(Qt::BottomDockWidgetArea, fileDock);
        legacyState = seed.saveState();
    }
    QVERIFY(!legacyState.isEmpty());

    QSettings settings;
    settings.setValue("window/state", legacyState);

    MainWindow window;
    QDockWidget *fileDock = window.findChild<QDockWidget *>("files");
    QDockWidget *labelDock = window.findChild<QDockWidget *>("labels");
    QVERIFY(fileDock);
    QVERIFY(labelDock);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QCOMPARE(window.dockWidgetArea(fileDock), Qt::RightDockWidgetArea);
    QCOMPARE(window.dockWidgetArea(labelDock), Qt::RightDockWidgetArea);
    QVERIFY(fileDock->geometry().top() >= labelDock->geometry().top());
}

void UiTests::mainWindowViewMenuRestoresClosedRightDocks() {
    resetTestSettings("restore-right-docks-from-view-menu");
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QDockWidget *fileDock = window.findChild<QDockWidget *>(QStringLiteral("files"));
    QDockWidget *labelDock = window.findChild<QDockWidget *>(QStringLiteral("labels"));
    QAction *showFileDockAction = window.findChild<QAction *>(QStringLiteral("showFileDockAction"));
    QAction *showLabelDockAction = window.findChild<QAction *>(QStringLiteral("showLabelDockAction"));
    QMenu *viewMenu = window.findChild<QMenu *>(QStringLiteral("viewMenu"));
    QVERIFY(fileDock);
    QVERIFY(labelDock);
    QVERIFY(showFileDockAction);
    QVERIFY(showLabelDockAction);
    QVERIFY(viewMenu);
    QVERIFY(viewMenu->actions().contains(showFileDockAction));
    QVERIFY(viewMenu->actions().contains(showLabelDockAction));

    fileDock->hide();
    labelDock->hide();
    QApplication::processEvents();
    QVERIFY(!fileDock->isVisible());
    QVERIFY(!labelDock->isVisible());

    showFileDockAction->trigger();
    showLabelDockAction->trigger();
    QApplication::processEvents();

    QVERIFY(fileDock->isVisible());
    QVERIFY(labelDock->isVisible());
}

void UiTests::mainWindowFileListKeepsSelectionSeparateFromActiveFile() {
    resetTestSettings("file-selection-active-state");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage first(10, 10, QImage::Format_RGB32);
    first.fill(Qt::white);
    QImage second(10, 10, QImage::Format_RGB32);
    second.fill(Qt::black);
    QVERIFY(first.save(dir.filePath("a.jpg")));
    QVERIFY(second.save(dir.filePath("b.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    QListWidget *fileList = window.findChild<QListWidget *>("fileList");
    QVERIFY(fileList);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListWidgetItem *firstItem = fileList->item(0);
    QListWidgetItem *secondItem = fileList->item(1);
    QVERIFY(firstItem);
    QVERIFY(secondItem);
    QCOMPARE(QFileInfo(firstItem->text()).fileName(), QString("a.jpg"));
    QCOMPARE(QFileInfo(secondItem->text()).fileName(), QString("b.jpg"));
    QCOMPARE(firstItem->data(Qt::UserRole + 1).toBool(), true);
    QCOMPARE(secondItem->data(Qt::UserRole + 1).toBool(), false);
    QVERIFY(window.windowTitle().contains("a.jpg"));

    QTest::mouseClick(fileList->viewport(), Qt::LeftButton, Qt::NoModifier,
                      fileList->visualItemRect(secondItem).center());

    QCOMPARE(fileList->currentItem(), secondItem);
    QCOMPARE(firstItem->data(Qt::UserRole + 1).toBool(), true);
    QCOMPARE(secondItem->data(Qt::UserRole + 1).toBool(), false);
    QVERIFY(window.windowTitle().contains("a.jpg"));

    QTest::mouseDClick(fileList->viewport(), Qt::LeftButton, Qt::NoModifier,
                       fileList->visualItemRect(secondItem).center());

    QCOMPARE(fileList->currentItem(), secondItem);
    QCOMPARE(firstItem->data(Qt::UserRole + 1).toBool(), false);
    QCOMPARE(secondItem->data(Qt::UserRole + 1).toBool(), true);
    QVERIFY(window.windowTitle().contains("b.jpg"));
}

void UiTests::mainWindowFileListContextMenuProvidesCommonFileActions() {
    resetTestSettings("file-list-context-menu");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage first(10, 10, QImage::Format_RGB32);
    first.fill(Qt::white);
    QImage second(10, 10, QImage::Format_RGB32);
    second.fill(Qt::black);
    QVERIFY(first.save(dir.filePath("a.jpg")));
    QVERIFY(second.save(dir.filePath("b.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    QListWidget *fileList = window.findChild<QListWidget *>(QStringLiteral("fileList"));
    QMenu *contextMenu = window.findChild<QMenu *>(QStringLiteral("fileListContextMenu"));
    QAction *openAction = window.findChild<QAction *>(QStringLiteral("fileContextOpenAction"));
    QAction *revealAction = window.findChild<QAction *>(QStringLiteral("fileContextRevealAction"));
    QAction *copyPathAction = window.findChild<QAction *>(QStringLiteral("fileContextCopyPathAction"));
    QAction *markAction = window.findChild<QAction *>(QStringLiteral("fileContextMarkAction"));
    QAction *deleteAction = window.findChild<QAction *>(QStringLiteral("fileContextDeleteAction"));
    QVERIFY(fileList);
    QVERIFY(contextMenu);
    QVERIFY(openAction);
    QVERIFY(revealAction);
    QVERIFY(copyPathAction);
    QVERIFY(markAction);
    QVERIFY(deleteAction);
    QCOMPARE(fileList->contextMenuPolicy(), Qt::CustomContextMenu);

    QListWidgetItem *firstItem = fileList->item(0);
    QListWidgetItem *secondItem = fileList->item(1);
    QVERIFY(firstItem);
    QVERIFY(secondItem);
    QCOMPARE(firstItem->data(Qt::UserRole + 1).toBool(), true);
    QCOMPARE(secondItem->data(Qt::UserRole + 1).toBool(), false);

    fileList->setCurrentItem(secondItem, QItemSelectionModel::ClearAndSelect);
    markAction->trigger();
    QCOMPARE(secondItem->data(Qt::UserRole + 3).toBool(), true);
    QVERIFY(secondItem->background().color().isValid());
    QCOMPARE(firstItem->data(Qt::UserRole + 1).toBool(), true);
    QCOMPARE(secondItem->data(Qt::UserRole + 1).toBool(), false);

    copyPathAction->trigger();
    QCOMPARE(QApplication::clipboard()->text(), secondItem->text());

    openAction->trigger();
    QCOMPARE(firstItem->data(Qt::UserRole + 1).toBool(), false);
    QCOMPARE(secondItem->data(Qt::UserRole + 1).toBool(), true);
    QVERIFY(window.windowTitle().contains(QStringLiteral("b.jpg")));
}

void UiTests::mainWindowCanvasContextMenuProvidesPolygonPointActions() {
    resetTestSettings("canvas-context-polygon-point-actions");

    MainWindow window;
    QAction *insertAction = window.findChild<QAction *>(QStringLiteral("insertPolygonPointAction"));
    QAction *edgeAction = window.findChild<QAction *>(QStringLiteral("addPointToEdgeAction"));
    QAction *removeAction = window.findChild<QAction *>(QStringLiteral("removePolygonPointAction"));

    QVERIFY(insertAction);
    QVERIFY(edgeAction);
    QVERIFY(removeAction);
    QVERIFY(insertAction->text().contains(QString::fromUtf8("插入")));
    QVERIFY(edgeAction->text().contains(QString::fromUtf8("边")));
    QVERIFY(removeAction->text().contains(QString::fromUtf8("删除")));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(120, 90, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("edge.jpg"));
    QVERIFY(image.save(imagePath));
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    Canvas *canvas = window.findChild<Canvas *>();
    QVERIFY(canvas);
    canvas->setShapes({Shape::fromPolygon(QStringLiteral("poly"),
                                           {QPointF(20, 20), QPointF(80, 20), QPointF(70, 65)},
                                           false)});
    canvas->setEditMode();
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    canvas->setScale(1.0);
    QMouseEvent hoverEdge(QEvent::MouseMove,
                          QPointF(50, 20),
                          QPointF(50, 20),
                          canvas->mapToGlobal(QPoint(50, 20)),
                          Qt::NoButton,
                          Qt::NoButton,
                          Qt::NoModifier);
    QApplication::sendEvent(canvas, &hoverEdge);
    QVERIFY(edgeAction->isEnabled());
    edgeAction->trigger();
    QCOMPARE(canvas->shapes().first().points.size(), 4);
    QCOMPARE(canvas->shapes().first().points[1], QPointF(50, 20));
}

void UiTests::mainWindowCanvasContextMenuIncludesShapeClipboardActions() {
    resetTestSettings("canvas-context-shape-clipboard-actions");

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(120, 90, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("context.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *copyAction = window.findChild<QAction *>(QStringLiteral("copyShapesAction"));
    QAction *pasteAction = window.findChild<QAction *>(QStringLiteral("pasteShapesAction"));
    QAction *undoAction = actionByShortcut(&window, QKeySequence::Undo);
    QVERIFY(canvas);
    QVERIFY(copyAction);
    QVERIFY(pasteAction);
    QVERIFY(undoAction);

    canvas->setShapes({Shape::fromRect(QStringLiteral("defect"), QRectF(15, 15, 30, 22), false)});
    canvas->setSelectedIndices({0});
    QApplication::processEvents();
    copyAction->trigger();
    QVERIFY(pasteAction->isEnabled());

    bool menuInspected = false;
    bool hasCopy = false;
    bool hasPaste = false;
    bool hasUndo = false;
    QTimer::singleShot(0, [&]() {
        auto *popup = qobject_cast<QMenu *>(QApplication::activePopupWidget());
        if (!popup) {
            return;
        }
        menuInspected = true;
        const QList<QAction *> actions = popup->actions();
        hasCopy = actions.contains(copyAction);
        hasPaste = actions.contains(pasteAction);
        hasUndo = actions.contains(undoAction);
        popup->close();
    });
    QTimer::singleShot(250, []() {
        if (auto *popup = qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
            popup->close();
        }
    });

    QVERIFY(QMetaObject::invokeMethod(&window,
                                      "showCanvasContextMenu",
                                      Qt::DirectConnection,
                                      Q_ARG(QPoint, canvas->mapToGlobal(QPoint(40, 35))),
                                      Q_ARG(QPointF, QPointF(40, 35))));
    QVERIFY(menuInspected);
    QVERIFY(hasCopy);
    QVERIFY(hasPaste);
    QVERIFY(hasUndo);
}

void UiTests::mainWindowCanvasContextMenuPlacementActionsUseContextPoint() {
    resetTestSettings("canvas-context-placement-actions");

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(140, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("placement.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *copyHereAction = window.findChild<QAction *>(QStringLiteral("copyHereAction"));
    QAction *moveHereAction = window.findChild<QAction *>(QStringLiteral("moveHereAction"));
    QVERIFY(canvas);
    QVERIFY(copyHereAction);
    QVERIFY(moveHereAction);

    canvas->setShapes({Shape::fromRect(QStringLiteral("defect"), QRectF(15, 15, 30, 22), false)});
    canvas->setSelectedIndices({0});
    QApplication::processEvents();
    QVERIFY(copyHereAction->isEnabled());
    QVERIFY(moveHereAction->isEnabled());

    const auto triggerContextAction = [&](QAction *target, const QPointF &imagePosition) {
        bool inspected = false;
        bool present = false;
        QTimer::singleShot(0, [&]() {
            auto *popup = qobject_cast<QMenu *>(QApplication::activePopupWidget());
            if (!popup) {
                return;
            }
            inspected = true;
            present = popup->actions().contains(target);
            if (present) {
                target->trigger();
            }
            popup->close();
        });
        QTimer::singleShot(250, []() {
            if (auto *popup = qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
                popup->close();
            }
        });
        QVERIFY(QMetaObject::invokeMethod(&window,
                                          "showCanvasContextMenu",
                                          Qt::DirectConnection,
                                          Q_ARG(QPoint, canvas->mapToGlobal(QPoint(60, 45))),
                                          Q_ARG(QPointF, imagePosition)));
        QVERIFY(inspected);
        QVERIFY(present);
    };

    triggerContextAction(copyHereAction, QPointF(80, 60));
    QCOMPARE(canvas->shapes().size(), 2);
    QCOMPARE(canvas->currentIndex(), 1);
    QCOMPARE(canvas->shapes().at(1).boundingRect().center(), QPointF(80, 60));

    triggerContextAction(moveHereAction, QPointF(35, 30));
    QCOMPARE(canvas->shapes().size(), 2);
    QCOMPARE(canvas->currentIndex(), 1);
    QCOMPARE(canvas->shapes().at(1).boundingRect().center(), QPointF(35, 30));
}

void UiTests::mainWindowCanvasContextMenuIncludesAllCreateModes() {
    resetTestSettings("canvas-context-create-modes");

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(140, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("create-modes.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QVERIFY(canvas);
    const QStringList actionNames = {
        QStringLiteral("createPolygonModeAction"),
        QStringLiteral("createModeAction"),
        QStringLiteral("createOrientedRectangleModeAction"),
        QStringLiteral("createCircleModeAction"),
        QStringLiteral("createPointModeAction"),
        QStringLiteral("createPointsModeAction"),
        QStringLiteral("createLineModeAction"),
        QStringLiteral("createLinestripModeAction"),
        QStringLiteral("createAiPointsModeAction"),
        QStringLiteral("createAiBoxModeAction"),
        QStringLiteral("createMaskModeAction")};
    QList<QAction *> expectedActions;
    for (const QString &name : actionNames) {
        QAction *action = name == QStringLiteral("createModeAction")
                              ? actionByShortcut(&window, QKeySequence(QStringLiteral("W")))
                              : window.findChild<QAction *>(name);
        QVERIFY(action);
        expectedActions.append(action);
    }

    bool menuInspected = false;
    QStringList missingActions;
    QTimer::singleShot(0, [&]() {
        auto *popup = qobject_cast<QMenu *>(QApplication::activePopupWidget());
        if (!popup) {
            return;
        }
        menuInspected = true;
        for (int i = 0; i < expectedActions.size(); ++i) {
            if (!popup->actions().contains(expectedActions.at(i))) {
                missingActions.append(actionNames.at(i));
            }
        }
        popup->close();
    });
    QTimer::singleShot(250, []() {
        if (auto *popup = qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
            popup->close();
        }
    });

    QVERIFY(QMetaObject::invokeMethod(&window,
                                      "showCanvasContextMenu",
                                      Qt::DirectConnection,
                                      Q_ARG(QPoint, canvas->mapToGlobal(QPoint(60, 45))),
                                      Q_ARG(QPointF, QPointF(60, 45))));
    QVERIFY(menuInspected);
    QVERIFY2(missingActions.isEmpty(), qPrintable(missingActions.join(QStringLiteral(", "))));
}

void UiTests::mainWindowProvidesRemoveSelectedPointShortcut() {
    resetTestSettings("remove-selected-point-shortcut");

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(120, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("a.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    Canvas *canvas = window.findChild<Canvas *>();
    QAction *removeAction = window.findChild<QAction *>(QStringLiteral("removeSelectedPointAction"));
    QVERIFY(canvas);
    QVERIFY(removeAction);
    QVERIFY(removeAction->shortcuts().contains(QKeySequence(Qt::Key_Backspace)));

    QPixmap pixmap(120, 100);
    pixmap.fill(Qt::white);
    canvas->setPixmap(pixmap);
    canvas->setShapes({Shape::fromPolygon(QStringLiteral("defect"),
                                          {QPointF(20, 20), QPointF(80, 20), QPointF(90, 70), QPointF(25, 80)},
                                          false)});
    canvas->setEditMode();
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    canvas->setScale(1.0);

    QMouseEvent hoverVertex(QEvent::MouseMove,
                            QPointF(20, 20),
                            QPointF(20, 20),
                            canvas->mapToGlobal(QPoint(20, 20)),
                            Qt::NoButton,
                            Qt::NoButton,
                            Qt::NoModifier);
    QApplication::sendEvent(canvas, &hoverVertex);
    QVERIFY(removeAction->isEnabled());
    canvas->setFocus();
    QTest::keyClick(canvas, Qt::Key_Backspace);
    QCOMPARE(canvas->shapes().first().points.size(), 3);
}

void UiTests::mainWindowFileThumbnailModeDefaultsOffAndCanToggle() {
    resetTestSettings("file-thumbnail-mode");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage first(40, 30, QImage::Format_RGB32);
    first.fill(Qt::red);
    QImage second(30, 40, QImage::Format_RGB32);
    second.fill(Qt::blue);
    QVERIFY(first.save(dir.filePath("a.jpg")));
    QVERIFY(second.save(dir.filePath("b.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    QListWidget *fileList = window.findChild<QListWidget *>("fileList");
    QAction *thumbnailAction = window.findChild<QAction *>(QStringLiteral("thumbnailModeAction"));
    QVERIFY(fileList);
    QVERIFY(thumbnailAction);

    QVERIFY(!thumbnailAction->isChecked());
    QCOMPARE(fileList->viewMode(), QListView::ListMode);
    QCOMPARE(fileList->iconSize(), QSize(0, 0));
    QCOMPARE(fileList->count(), 2);
    QVERIFY(fileList->item(0)->icon().isNull());

    thumbnailAction->trigger();

    QVERIFY(thumbnailAction->isChecked());
    QCOMPARE(fileList->viewMode(), QListView::ListMode);
    QCOMPARE(fileList->iconSize(), QSize(72, 54));
    QCOMPARE(fileList->count(), 2);
    QVERIFY(!fileList->item(0)->icon().isNull());
    QVERIFY(fileList->item(0)->sizeHint().height() >= 60);
    QCOMPARE(QFileInfo(fileList->item(0)->text()).fileName(), QString("a.jpg"));

    QSettings settings;
    QCOMPARE(settings.value("view/fileThumbnails").toBool(), true);
}

void UiTests::mainWindowFileThumbnailsDrawAnnotationBoxes() {
    resetTestSettings("file-thumbnail-boxes");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath("a.jpg");
    QVERIFY(image.save(imagePath));

    AnnotationDocument doc;
    doc.imagePath = imagePath;
    doc.imageSize = image.size();
    doc.depth = 3;
    doc.shapes = {Shape::fromRect(QStringLiteral("defect"), QRectF(20, 15, 30, 20), false)};
    QVERIFY(AnnotationIO::savePascalVoc(dir.filePath("a.xml"), doc));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    QListWidget *fileList = window.findChild<QListWidget *>(QStringLiteral("fileList"));
    QAction *thumbnailAction = window.findChild<QAction *>(QStringLiteral("thumbnailModeAction"));
    QVERIFY(fileList);
    QVERIFY(thumbnailAction);

    thumbnailAction->trigger();
    QCOMPARE(fileList->count(), 1);
    QVERIFY(!fileList->item(0)->icon().isNull());
    const QImage thumbnail = fileList->item(0)->icon().pixmap(fileList->iconSize()).toImage();

    QVERIFY2(countPreviewBoxPixels(thumbnail) > 8, "File thumbnail did not render annotation boxes");
}

void UiTests::mainWindowFileDockTitleShowsCountAndLabelFilter() {
    resetTestSettings("file-dock-label-filter");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString aPath = dir.filePath("a.jpg");
    const QString bPath = dir.filePath("b.jpg");
    const QString cPath = dir.filePath("c.jpg");
    QVERIFY(image.save(aPath));
    QVERIFY(image.save(bPath));
    QVERIFY(image.save(cPath));

    AnnotationDocument aDoc;
    aDoc.imagePath = aPath;
    aDoc.imageSize = image.size();
    aDoc.depth = 3;
    aDoc.shapes = {Shape::fromRect(QStringLiteral("scratch"), QRectF(10, 10, 20, 20), false)};
    QVERIFY(AnnotationIO::savePascalVoc(dir.filePath("a.xml"), aDoc));

    AnnotationDocument bDoc;
    bDoc.imagePath = bPath;
    bDoc.imageSize = image.size();
    bDoc.depth = 3;
    bDoc.shapes = {
        Shape::fromRect(QStringLiteral("scratch"), QRectF(10, 10, 20, 20), false),
        Shape::fromRect(QStringLiteral("dent"), QRectF(30, 20, 20, 20), false),
    };
    QVERIFY(AnnotationIO::savePascalVoc(dir.filePath("b.xml"), bDoc));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});

    QListWidget *fileList = window.findChild<QListWidget *>(QStringLiteral("fileList"));
    QLabel *titleLabel = window.findChild<QLabel *>(QStringLiteral("fileDockTitleLabel"));
    QToolButton *filterButton = window.findChild<QToolButton *>(QStringLiteral("fileLabelFilterButton"));
    QVERIFY(fileList);
    QVERIFY(titleLabel);
    QVERIFY(filterButton);
    QMenu *filterMenu = filterButton->menu();
    QVERIFY(filterMenu);
    QCOMPARE(fileList->count(), 3);
    QVERIFY(titleLabel->text().contains(QStringLiteral("3")));

    QAction *scratchAction = nullptr;
    QAction *dentAction = nullptr;
    for (QAction *action : filterMenu->actions()) {
        if (action->text().contains(QStringLiteral("scratch"))) {
            scratchAction = action;
        }
        if (action->text().contains(QStringLiteral("dent"))) {
            dentAction = action;
        }
    }
    QVERIFY(scratchAction);
    QVERIFY(dentAction);
    QVERIFY(scratchAction->text().contains(QStringLiteral("2")));
    QVERIFY(dentAction->text().contains(QStringLiteral("1")));

    scratchAction->trigger();
    QCOMPARE(fileList->count(), 2);
    QVERIFY(titleLabel->text().contains(QStringLiteral("2/3")));
    QVERIFY(titleLabel->text().contains(QStringLiteral("scratch")));

    QAction *allAction = filterMenu->actions().constFirst();
    allAction->trigger();
    QCOMPARE(fileList->count(), 3);
    QVERIFY(titleLabel->text().contains(QStringLiteral("3")));
}

void UiTests::mainWindowFileListSearchFiltersByFilename() {
    resetTestSettings("file-list-search");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(32, 24, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("alpha.jpg")));
    QVERIFY(image.save(dir.filePath("beta.jpg")));
    QVERIFY(image.save(dir.filePath("gamma.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});

    QListWidget *fileList = window.findChild<QListWidget *>(QStringLiteral("fileList"));
    QLineEdit *search = window.findChild<QLineEdit *>(QStringLiteral("fileSearchEdit"));
    QLabel *titleLabel = window.findChild<QLabel *>(QStringLiteral("fileDockTitleLabel"));
    QVERIFY(fileList);
    QVERIFY(search);
    QVERIFY(titleLabel);
    QCOMPARE(fileList->count(), 3);

    search->setText(QStringLiteral("BETA"));
    QCOMPARE(fileList->count(), 1);
    QVERIFY(fileList->item(0)->text().contains(QStringLiteral("beta.jpg")));
    QVERIFY(titleLabel->text().contains(QStringLiteral("1/3")));
    QVERIFY(titleLabel->text().contains(QStringLiteral("BETA")));

    search->clear();
    QCOMPARE(fileList->count(), 3);
    QVERIFY(titleLabel->text().contains(QStringLiteral("3")));
}

void UiTests::mainWindowFileListSearchUsesLabelMeRegex() {
    resetTestSettings("file-list-search-regex");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(32, 24, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("alpha.jpg")));
    QVERIFY(image.save(dir.filePath("beta.jpg")));
    QVERIFY(image.save(dir.filePath("gamma.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});

    QListWidget *fileList = window.findChild<QListWidget *>(QStringLiteral("fileList"));
    QLineEdit *search = window.findChild<QLineEdit *>(QStringLiteral("fileSearchEdit"));
    QVERIFY(fileList);
    QVERIFY(search);
    QCOMPARE(fileList->count(), 3);

    // LabelMe treats the file search field as a case-insensitive regex.
    search->setText(QStringLiteral("(alpha|gamma)\\.jpg$"));
    QCOMPARE(fileList->count(), 2);
    QVERIFY(fileList->item(0)->text().endsWith(QStringLiteral("alpha.jpg")));
    QVERIFY(fileList->item(1)->text().endsWith(QStringLiteral("gamma.jpg")));

    // An invalid regex is ignored by LabelMe instead of hiding every file.
    search->setText(QStringLiteral("["));
    QCOMPARE(fileList->count(), 3);
}

void UiTests::mainWindowRecentDirectoryMenuKeepsLastTenAndSwitches() {
    resetTestSettings("recent-directories");
    QTemporaryDir root;
    QVERIFY(root.isValid());

    QStringList dirs;
    for (int i = 0; i < 11; ++i) {
        const QString dirPath = root.filePath(QStringLiteral("dir%1").arg(i, 2, 10, QLatin1Char('0')));
        QVERIFY(QDir().mkpath(dirPath));
        QImage image(10, 10, QImage::Format_RGB32);
        image.fill(QColor::fromHsv((i * 30) % 360, 180, 220));
        QVERIFY(image.save(QDir(dirPath).filePath(QStringLiteral("image%1.jpg").arg(i))));
        dirs.append(QFileInfo(dirPath).absoluteFilePath());
    }

    MainWindow window;
    for (const QString &dir : dirs) {
        window.loadStartupArgs({"labelImgCpp", dir});
    }

    QMenu *recentDirsMenu = window.findChild<QMenu *>(QStringLiteral("recentDirsMenu"));
    QListWidget *fileList = window.findChild<QListWidget *>(QStringLiteral("fileList"));
    QVERIFY(recentDirsMenu);
    QVERIFY(fileList);
    QCOMPARE(recentDirsMenu->actions().size(), 10);
    QCOMPARE(recentDirsMenu->actions().first()->data().toString(), dirs.last());
    QVERIFY(!recentDirsMenu->actions().last()->data().toString().contains(QStringLiteral("dir00")));

    QAction *targetAction = nullptr;
    for (QAction *action : recentDirsMenu->actions()) {
        if (action->data().toString() == dirs.at(5)) {
            targetAction = action;
            break;
        }
    }
    QVERIFY(targetAction);
    targetAction->trigger();

    QVERIFY(fileList->currentItem());
    QCOMPARE(QFileInfo(fileList->currentItem()->text()).absolutePath(), dirs.at(5));
}

void UiTests::mainWindowRecentDirectoryRestoresLastViewedFileInThatFolder() {
    resetTestSettings("recent-directory-last-viewed-file");
    QTemporaryDir root;
    QVERIFY(root.isValid());

    const QString firstDir = root.filePath("first");
    const QString secondDir = root.filePath("second");
    QVERIFY(QDir().mkpath(firstDir));
    QVERIFY(QDir().mkpath(secondDir));

    QImage image(10, 10, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(QDir(firstDir).filePath("a.jpg")));
    QVERIFY(image.save(QDir(firstDir).filePath("b.jpg")));
    QVERIFY(image.save(QDir(secondDir).filePath("c.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", QFileInfo(firstDir).absoluteFilePath()});
    QAction *nextAction = actionByShortcut(&window, QKeySequence(QStringLiteral("D")));
    QListWidget *fileList = window.findChild<QListWidget *>(QStringLiteral("fileList"));
    QMenu *recentDirsMenu = window.findChild<QMenu *>(QStringLiteral("recentDirsMenu"));
    QVERIFY(nextAction);
    QVERIFY(fileList);
    QVERIFY(recentDirsMenu);

    nextAction->trigger();
    QVERIFY(fileList->currentItem());
    QCOMPARE(QFileInfo(fileList->currentItem()->text()).fileName(), QStringLiteral("b.jpg"));

    window.loadStartupArgs({"labelImgCpp", QFileInfo(secondDir).absoluteFilePath()});
    QCOMPARE(QFileInfo(fileList->currentItem()->text()).fileName(), QStringLiteral("c.jpg"));

    QAction *firstDirAction = nullptr;
    const QString firstAbsolute = QFileInfo(firstDir).absoluteFilePath();
    for (QAction *action : recentDirsMenu->actions()) {
        if (action->data().toString() == firstAbsolute) {
            firstDirAction = action;
            break;
        }
    }
    QVERIFY(firstDirAction);
    firstDirAction->trigger();

    QVERIFY(fileList->currentItem());
    QCOMPARE(QFileInfo(fileList->currentItem()->text()).fileName(), QStringLiteral("b.jpg"));
}

void UiTests::mainWindowRecentDirectoryRestoresLastViewedFileAcrossSessions() {
    resetTestSettings("recent-directory-last-viewed-file-persistence");
    QTemporaryDir root;
    QVERIFY(root.isValid());

    const QString directory = root.filePath(QStringLiteral("images"));
    QVERIFY(QDir().mkpath(directory));
    QImage image(10, 10, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(QDir(directory).filePath(QStringLiteral("a.jpg"))));
    QVERIFY(image.save(QDir(directory).filePath(QStringLiteral("b.jpg"))));

    const QString absoluteDirectory = QFileInfo(directory).absoluteFilePath();
    {
        MainWindow firstWindow;
        firstWindow.loadStartupArgs({QStringLiteral("labelImgCpp"), absoluteDirectory});
        QAction *nextAction = actionByShortcut(&firstWindow, QKeySequence(QStringLiteral("D")));
        QListWidget *fileList = firstWindow.findChild<QListWidget *>(QStringLiteral("fileList"));
        QVERIFY(nextAction);
        QVERIFY(fileList);
        QVERIFY(fileList->currentItem());
        QCOMPARE(QFileInfo(fileList->currentItem()->text()).fileName(), QStringLiteral("a.jpg"));

        nextAction->trigger();
        QVERIFY(fileList->currentItem());
        QCOMPARE(QFileInfo(fileList->currentItem()->text()).fileName(), QStringLiteral("b.jpg"));
        QVERIFY(firstWindow.close());
    }

    MainWindow secondWindow;
    secondWindow.loadStartupArgs({QStringLiteral("labelImgCpp"), absoluteDirectory});
    QListWidget *fileList = secondWindow.findChild<QListWidget *>(QStringLiteral("fileList"));
    QVERIFY(fileList);
    QVERIFY(fileList->currentItem());
    QCOMPARE(QFileInfo(fileList->currentItem()->text()).fileName(), QStringLiteral("b.jpg"));
}

void UiTests::mainWindowRecentDirectoryMenuShowsFullPaths() {
    resetTestSettings("recent-directories-full-path");
    QTemporaryDir root;
    QVERIFY(root.isValid());

    const QString firstDir = root.filePath("line-a/shared-name");
    const QString secondDir = root.filePath("line-b/shared-name");
    QVERIFY(QDir().mkpath(firstDir));
    QVERIFY(QDir().mkpath(secondDir));
    QImage image(10, 10, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(QDir(firstDir).filePath("a.jpg")));
    QVERIFY(image.save(QDir(secondDir).filePath("b.jpg")));

    const QString firstAbsolute = QFileInfo(firstDir).absoluteFilePath();
    const QString secondAbsolute = QFileInfo(secondDir).absoluteFilePath();

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", firstAbsolute});
    window.loadStartupArgs({"labelImgCpp", secondAbsolute});

    QMenu *recentDirsMenu = window.findChild<QMenu *>(QStringLiteral("recentDirsMenu"));
    QVERIFY(recentDirsMenu);
    QCOMPARE(recentDirsMenu->actions().size(), 2);

    QAction *latest = recentDirsMenu->actions().first();
    QVERIFY2(latest->text().contains(QDir::toNativeSeparators(secondAbsolute)), qPrintable(latest->text()));
    QCOMPARE(latest->toolTip(), secondAbsolute);
    QVERIFY2(recentDirsMenu->actions().last()->text().contains(QDir::toNativeSeparators(firstAbsolute)),
             qPrintable(recentDirsMenu->actions().last()->text()));
}

void UiTests::mainWindowFileListShowsVisitedFilesWithDimmedText() {
    resetTestSettings("visited-file-list-state");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(10, 10, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));
    QVERIFY(image.save(dir.filePath("b.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    QListWidget *fileList = window.findChild<QListWidget *>(QStringLiteral("fileList"));
    QAction *nextAction = actionByShortcut(&window, QKeySequence(QStringLiteral("D")));
    QVERIFY(fileList);
    QVERIFY(nextAction);
    QCOMPARE(fileList->count(), 2);

    nextAction->trigger();

    QListWidgetItem *visitedItem = fileList->item(0);
    QListWidgetItem *activeItem = fileList->item(1);
    QVERIFY(visitedItem);
    QVERIFY(activeItem);
    QCOMPARE(QFileInfo(activeItem->text()).fileName(), QStringLiteral("b.jpg"));
    QCOMPARE(visitedItem->data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(visitedItem->foreground().color(), QColor(107, 114, 128));
    QVERIFY(!activeItem->data(Qt::UserRole + 2).toBool());
}

void UiTests::mainWindowVShortcutSwitchesToViewMode() {
    resetTestSettings("ctrl-j-view-mode-shortcut");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QAction *viewModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("V")));
    QAction *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QAction *editModeAction = window.findChild<QAction *>(QStringLiteral("editModeAction"));
    QVERIFY(viewModeAction);
    QVERIFY(createModeAction);
    QVERIFY(editModeAction);
    QVERIFY2(!actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+J"))),
             "Ctrl+J must no longer be the default view-mode shortcut");

    createModeAction->trigger();
    QVERIFY(createModeAction->isChecked());

    viewModeAction->trigger();
    QVERIFY(viewModeAction->isChecked());
    QVERIFY(!createModeAction->isChecked());
    QVERIFY(!editModeAction->isChecked());

    Canvas *canvas = window.findChild<Canvas *>();
    QVERIFY(canvas);
    const int originalShapeCount = canvas->shapes().size();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(canvas, QPoint(45, 45));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(45, 45));
    QCOMPARE(canvas->shapes().size(), originalShapeCount);
}

void UiTests::mainWindowCtrlJIsViewModeAndVTogglesEditability() {
    resetTestSettings("ctrl-j-view-v-editability");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QAction *viewModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("V")));
    QAction *editabilityAction = window.findChild<QAction *>(QStringLiteral("editabilityAction"));
    QAction *editModeAction = window.findChild<QAction *>(QStringLiteral("editModeAction"));
    QAction *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QComboBox *modeCombo = window.findChild<QComboBox *>(QStringLiteral("footerModeCombo"));
    Canvas *canvas = window.findChild<Canvas *>();
    QVERIFY(viewModeAction);
    QVERIFY(editabilityAction);
    QVERIFY(editModeAction);
    QVERIFY(createModeAction);
    QVERIFY(modeCombo);
    QVERIFY(canvas);

    QCOMPARE(viewModeAction->objectName(), QStringLiteral("viewModeAction"));
    QCOMPARE(editabilityAction->objectName(), QStringLiteral("editabilityAction"));
    QVERIFY(editabilityAction->isCheckable());
    QVERIFY(editabilityAction->isChecked());
    QVERIFY(editabilityAction->shortcut().isEmpty());
    QCOMPARE(modeCombo->itemText(modeCombo->findData(QStringLiteral("view"))), QStringLiteral("查看 (V)"));
    QCOMPARE(modeCombo->itemText(modeCombo->findData(QStringLiteral("edit"))), QStringLiteral("编辑"));

    createModeAction->trigger();
    QVERIFY(createModeAction->isChecked());
    viewModeAction->trigger();
    QVERIFY(viewModeAction->isChecked());
    QVERIFY(!editModeAction->isChecked());
    QVERIFY(!createModeAction->isChecked());

    const int originalShapeCount = canvas->shapes().size();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(canvas, QPoint(45, 45));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(45, 45));
    QCOMPARE(canvas->shapes().size(), originalShapeCount);

    editabilityAction->trigger();
    QVERIFY(!editabilityAction->isChecked());
    QVERIFY(!editModeAction->isEnabled());
    QVERIFY(!createModeAction->isEnabled());
    QVERIFY(viewModeAction->isChecked());

    editabilityAction->trigger();
    QVERIFY(editabilityAction->isChecked());
    QVERIFY(editModeAction->isEnabled());
    QVERIFY(createModeAction->isEnabled());
}

void UiTests::mainWindowOnlyActiveCreateActionIsDisabled() {
    resetTestSettings("active-create-action-state");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("a.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const QStringList actionNames = {
        QStringLiteral("createModeAction"),
        QStringLiteral("createPolygonModeAction"),
        QStringLiteral("createPointModeAction"),
        QStringLiteral("createPointsModeAction"),
        QStringLiteral("createAiPointsModeAction"),
        QStringLiteral("createAiBoxModeAction"),
        QStringLiteral("createLineModeAction"),
        QStringLiteral("createLinestripModeAction"),
        QStringLiteral("createCircleModeAction"),
        QStringLiteral("createOrientedRectangleModeAction"),
        QStringLiteral("createMaskModeAction")};
    QList<QAction *> actions;
    for (const QString &name : actionNames) {
        QAction *action = window.findChild<QAction *>(name);
        QVERIFY2(action, qPrintable(name));
        actions.append(action);
    }

    QAction *polygonAction = window.findChild<QAction *>(QStringLiteral("createPolygonModeAction"));
    QAction *editAction = window.findChild<QAction *>(QStringLiteral("editModeAction"));
    QVERIFY(polygonAction);
    QVERIFY(editAction);
    polygonAction->trigger();

    for (QAction *action : actions) {
        QCOMPARE(action->isEnabled(), action != polygonAction);
    }

    editAction->trigger();
    for (QAction *action : actions) {
        QVERIFY(action->isEnabled());
    }
}

void UiTests::mainWindowPShortcutCreatesPolygonShape() {
    resetTestSettings("p-shortcut-create-polygon");
    QSettings settings;
    settings.setValue("labelHistory", QStringList{QStringLiteral("poly")});
    settings.setValue("lastUsedLabel", QStringLiteral("poly"));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(120, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QListWidget *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    QAction *polygonAction = actionByShortcut(&window, QKeySequence(QStringLiteral("P")));
    QAction *editModeAction = window.findChild<QAction *>(QStringLiteral("editModeAction"));
    QVERIFY(canvas);
    QVERIFY(labelList);
    QVERIFY(polygonAction);
    QCOMPARE(polygonAction->objectName(), QStringLiteral("createPolygonModeAction"));

    acceptNextLabelPrompt(QStringLiteral("poly"));
    polygonAction->trigger();
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 12));
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(70, 55));
    QTest::keyClick(canvas, Qt::Key_Return);

    QCOMPARE(canvas->shapes().size(), 1);
    QCOMPARE(canvas->shapes().first().shapeType, QStringLiteral("polygon"));
    QCOMPARE(canvas->shapes().first().points.size(), 3);
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("poly"));
    QCOMPARE(labelList->count(), 1);
    QCOMPARE(labelList->item(0)->text(), QStringLiteral("poly"));
    QVERIFY(editModeAction->isChecked());
}

void UiTests::mainWindowProvidesLabelMeCreateShortcuts() {
    resetTestSettings("labelme-create-shortcuts");
    MainWindow window;
    auto *rectangleAction = window.findChild<QAction *>(QStringLiteral("createModeAction"));
    auto *polygonAction = window.findChild<QAction *>(QStringLiteral("createPolygonModeAction"));
    auto *saveDirAction = window.findChild<QAction *>(QStringLiteral("changeSaveDirAction"));
    auto *undoLastPointAction = window.findChild<QAction *>(QStringLiteral("undoLastPointAction"));
    QVERIFY(rectangleAction);
    QVERIFY(polygonAction);
    QVERIFY(saveDirAction);
    QVERIFY(undoLastPointAction);
    QVERIFY(rectangleAction->shortcuts().contains(QKeySequence(QStringLiteral("Ctrl+R"))));
    QVERIFY(polygonAction->shortcuts().contains(QKeySequence(QStringLiteral("Ctrl+N"))));
    QVERIFY(saveDirAction->shortcuts().contains(QKeySequence(QStringLiteral("Ctrl+Shift+R"))));
    QVERIFY(!saveDirAction->shortcuts().contains(QKeySequence(QStringLiteral("Ctrl+R"))));
    QCOMPARE(undoLastPointAction->shortcut(), QKeySequence::Undo);
}

void UiTests::mainWindowProvidesSynchronizedZoomWidget() {
    resetTestSettings("labelme-zoom-widget");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(120, 90, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("zoom.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *zoomWidget = window.findChild<QSpinBox *>(QStringLiteral("zoomWidget"));
    auto *canvas = window.findChild<Canvas *>();
    auto *fitWindowAction = window.findChild<QAction *>(QStringLiteral("fitWindowAction"));
    QVERIFY(zoomWidget);
    QVERIFY(canvas);
    QVERIFY(fitWindowAction);
    QCOMPARE(zoomWidget->minimum(), 1);
    QCOMPARE(zoomWidget->maximum(), 1600);
    QCOMPARE(zoomWidget->suffix(), QStringLiteral(" %"));
    QCOMPARE(zoomWidget->buttonSymbols(), QAbstractSpinBox::NoButtons);

    zoomWidget->setValue(125);
    QTRY_COMPARE_WITH_TIMEOUT(qRound(canvas->scale() * 100.0), 125, 1000);
    QVERIFY(!fitWindowAction->isChecked());

    canvas->setScale(0.75);
    QTRY_COMPARE_WITH_TIMEOUT(zoomWidget->value(), 75, 1000);
}

void UiTests::mainWindowShortcutZoomKeepsViewportCenter() {
    resetTestSettings("labelme-shortcut-zoom-anchor");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(2200, 1600, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("large.jpg")));

    MainWindow window;
    window.resize(900, 700);
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *canvas = window.findChild<Canvas *>();
    auto *scrollArea = window.findChild<QScrollArea *>(QStringLiteral("scrollArea"));
    auto *zoomInAction = window.findChild<QAction *>(QStringLiteral("zoomInAction"));
    QVERIFY(canvas);
    QVERIFY(scrollArea);
    QVERIFY(zoomInAction);

    canvas->setScale(1.0);
    QCoreApplication::processEvents();
    auto *viewport = scrollArea->viewport();
    auto *horizontal = scrollArea->horizontalScrollBar();
    auto *vertical = scrollArea->verticalScrollBar();
    horizontal->setValue(qMin(260, horizontal->maximum()));
    vertical->setValue(qMin(190, vertical->maximum()));

    const auto imagePointAtViewportCenter = [&]() {
        const QPoint viewportCenter = viewport->rect().center();
        const QPoint canvasPoint = canvas->mapFrom(viewport, viewportCenter);
        return QPointF(canvasPoint.x() / canvas->scale(), canvasPoint.y() / canvas->scale()) -
               canvas->imageOriginOffset();
    };
    const QPointF before = imagePointAtViewportCenter();
    zoomInAction->trigger();
    QCoreApplication::processEvents();
    const QPointF after = imagePointAtViewportCenter();
    QVERIFY2(QLineF(before, after).length() < 1.5,
             qPrintable(QStringLiteral("before=(%1,%2), after=(%3,%4)")
                            .arg(before.x()).arg(before.y()).arg(after.x()).arg(after.y())));
}

void UiTests::mainWindowShortcutZoomUsesLabelMeMultiplicativeSteps() {
    resetTestSettings("labelme-multiplicative-zoom");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(120, 90, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("zoom-steps.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *canvas = window.findChild<Canvas *>();
    auto *zoomInAction = window.findChild<QAction *>(QStringLiteral("zoomInAction"));
    auto *zoomOutAction = window.findChild<QAction *>(QStringLiteral("zoomOutAction"));
    auto *zoomWidget = window.findChild<QSpinBox *>(QStringLiteral("zoomWidget"));
    QVERIFY(canvas);
    QVERIFY(zoomInAction);
    QVERIFY(zoomOutAction);
    QVERIFY(zoomWidget);

    zoomWidget->setValue(100);
    zoomInAction->trigger();
    QCOMPARE(qRound(canvas->scale() * 100.0), 111);
    zoomInAction->trigger();
    QCOMPARE(qRound(canvas->scale() * 100.0), 123);
    zoomOutAction->trigger();
    QCOMPARE(qRound(canvas->scale() * 100.0), 110);
}

void UiTests::mainWindowSpaceFinishesPolygonDraft() {
    resetTestSettings("space-finishes-polygon-draft");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(120, 90, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *polygonAction = window.findChild<QAction *>(QStringLiteral("createPolygonModeAction"));
    QAction *verifyAction = window.findChild<QAction *>(QStringLiteral("verifyAction"));
    QAction *undoLastPointAction = window.findChild<QAction *>(QStringLiteral("undoLastPointAction"));
    QVERIFY(canvas);
    QVERIFY(polygonAction);
    QVERIFY(verifyAction);
    QVERIFY(undoLastPointAction);

    acceptNextLabelPrompt(QStringLiteral("space"));
    polygonAction->trigger();
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 12));
    QVERIFY(undoLastPointAction->isEnabled());
    undoLastPointAction->trigger();
    QCOMPARE(canvas->shapes().size(), 0);
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(70, 55));
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(55, 75));
    QVERIFY(!verifyAction->isEnabled());
    QTest::keyClick(canvas, Qt::Key_Space);

    QCOMPARE(canvas->shapes().size(), 1);
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("space"));
    QVERIFY(verifyAction->isEnabled());
}

void UiTests::mainWindowDisablesEditModeWhileDrawing() {
    resetTestSettings("edit-mode-disabled-while-drawing");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(120, 90, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *canvas = window.findChild<Canvas *>();
    auto *polygonAction = window.findChild<QAction *>(QStringLiteral("createPolygonModeAction"));
    auto *editModeAction = window.findChild<QAction *>(QStringLiteral("editModeAction"));
    QVERIFY(canvas);
    QVERIFY(polygonAction);
    QVERIFY(editModeAction);

    polygonAction->trigger();
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
    QVERIFY(canvas->isDrawing());
    QVERIFY2(!editModeAction->isEnabled(), "Edit mode must be disabled while a draft shape is active");

    QTest::keyClick(canvas, Qt::Key_Escape);
    QVERIFY(!canvas->isDrawing());
    QVERIFY(editModeAction->isEnabled());
}

void UiTests::mainWindowDisablesShapeActionsWhileDrawing() {
    resetTestSettings("shape-actions-disabled-while-drawing");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(120, 90, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *canvas = window.findChild<Canvas *>();
    auto *rectangleAction = window.findChild<QAction *>(QStringLiteral("createModeAction"));
    auto *editLabelAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+E")));
    auto *deleteAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Delete")));
    auto *duplicateAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+D")));
    auto *copyAction = window.findChild<QAction *>(QStringLiteral("copyShapesAction"));
    QVERIFY(canvas);
    QVERIFY(rectangleAction);
    QVERIFY(editLabelAction);
    QVERIFY(deleteAction);
    QVERIFY(duplicateAction);
    QVERIFY(copyAction);

    rectangleAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
    QVERIFY(canvas->isDrawing());
    QVERIFY2(!editLabelAction->isEnabled(), "Edit label must be disabled while drawing");
    QVERIFY2(!deleteAction->isEnabled(), "Delete shape must be disabled while drawing");
    QVERIFY2(!duplicateAction->isEnabled(), "Duplicate shape must be disabled while drawing");
    QVERIFY2(!copyAction->isEnabled(), "Copy shapes must be disabled while drawing");

    QTest::keyClick(canvas, Qt::Key_Escape);
    QVERIFY(!canvas->isDrawing());
}

void UiTests::mainWindowEnablesRemovePointOnlyOnDeletableVertex() {
    resetTestSettings("remove-point-action-state");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(140, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *canvas = window.findChild<Canvas *>();
    auto *removePointAction = window.findChild<QAction *>(QStringLiteral("removeSelectedPointAction"));
    QVERIFY(canvas);
    QVERIFY(removePointAction);

    Shape polygon = Shape::fromPolygon(QStringLiteral("poly"),
                                       {QPointF(20, 20), QPointF(80, 20), QPointF(70, 70), QPointF(20, 60)},
                                       false);
    canvas->setShapes({polygon});
    canvas->setEditMode();
    const auto sendMouseMove = [canvas](const QPointF &position) {
        QMouseEvent event(QEvent::MouseMove,
                          position,
                          canvas->mapToGlobal(position.toPoint()),
                          Qt::NoButton,
                          Qt::NoButton,
                          Qt::NoModifier);
        QApplication::sendEvent(canvas, &event);
    };
    sendMouseMove(QPointF(130, 90));
    QVERIFY2(!removePointAction->isEnabled(), "Remove point must be disabled away from a vertex");

    const QPointF origin = canvas->imageOriginOffset();
    const QPointF vertexWidget = origin + polygon.points.first() * canvas->scale();
    sendMouseMove(vertexWidget);
    QVERIFY2(removePointAction->isEnabled(), "Remove point must be enabled on a deletable vertex");

    sendMouseMove(QPointF(130, 90));
    QVERIFY2(!removePointAction->isEnabled(), "Moving away from a vertex must clear the remove-point action");

    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(canvas, &leaveEvent);
    QVERIFY2(!removePointAction->isEnabled(), "Leaving the canvas must clear vertex actions");
}

void UiTests::mainWindowCanCreateNativePointLineAndCircleShapes() {
    resetTestSettings("create-native-point-line-circle");
    QSettings settings;
    settings.setValue("labelHistory", QStringList{QStringLiteral("native")});
    settings.setValue("lastUsedLabel", QStringLiteral("native"));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(140, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QListWidget *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    QAction *pointAction = window.findChild<QAction *>(QStringLiteral("createPointModeAction"));
    QAction *lineAction = window.findChild<QAction *>(QStringLiteral("createLineModeAction"));
    QAction *circleAction = window.findChild<QAction *>(QStringLiteral("createCircleModeAction"));
    QToolButton *mainModeButton = window.findChild<QToolButton *>(QStringLiteral("mainModeButton"));
    QVERIFY(canvas);
    QVERIFY(labelList);
    QVERIFY(pointAction);
    QVERIFY(lineAction);
    QVERIFY(circleAction);
    QVERIFY(mainModeButton);
    QVERIFY(mainModeButton->menu()->actions().contains(pointAction));
    QVERIFY(mainModeButton->menu()->actions().contains(lineAction));
    QVERIFY(mainModeButton->menu()->actions().contains(circleAction));

    acceptNextLabelPrompt(QStringLiteral("native"));
    pointAction->trigger();
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));

    acceptNextLabelPrompt(QStringLiteral("native"));
    lineAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(40, 25));
    QTest::mouseMove(canvas, QPoint(90, 35));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(90, 35));

    acceptNextLabelPrompt(QStringLiteral("native"));
    circleAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(70, 60));
    QTest::mouseMove(canvas, QPoint(100, 60));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(100, 60));

    QCOMPARE(canvas->shapes().size(), 3);
    QCOMPARE(canvas->shapes()[0].shapeType, QStringLiteral("point"));
    QCOMPARE(canvas->shapes()[1].shapeType, QStringLiteral("line"));
    QCOMPARE(canvas->shapes()[2].shapeType, QStringLiteral("circle"));
    QCOMPARE(labelList->count(), 3);
}

void UiTests::mainWindowCanCreateNativeOrientedRectangleShape() {
    resetTestSettings("create-native-oriented-rectangle");
    QSettings settings;
    settings.setValue("labelHistory", QStringList{QStringLiteral("rotated")});
    settings.setValue("lastUsedLabel", QStringLiteral("rotated"));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(140, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QListWidget *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    QAction *orientedAction = window.findChild<QAction *>(QStringLiteral("createOrientedRectangleModeAction"));
    QToolButton *mainModeButton = window.findChild<QToolButton *>(QStringLiteral("mainModeButton"));
    QAction *editModeAction = window.findChild<QAction *>(QStringLiteral("editModeAction"));
    QVERIFY(canvas);
    QVERIFY(labelList);
    QVERIFY(orientedAction);
    QVERIFY(mainModeButton);
    QVERIFY(editModeAction);
    QVERIFY(mainModeButton->menu()->actions().contains(orientedAction));

    canvas->setScale(1.0);
    acceptNextLabelPrompt(QStringLiteral("rotated"));
    orientedAction->trigger();
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 20));
    QCOMPARE(canvas->shapes().size(), 0);
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(85, 55));

    QCOMPARE(canvas->shapes().size(), 1);
    const Shape shape = canvas->shapes().first();
    QCOMPARE(shape.shapeType, QStringLiteral("oriented_rectangle"));
    QCOMPARE(shape.label, QStringLiteral("rotated"));
    QCOMPARE(shape.points.size(), 4);
    QCOMPARE(shape.points[0], QPointF(20, 20));
    QCOMPARE(shape.points[1], QPointF(80, 20));
    QCOMPARE(shape.points[2], QPointF(80, 55));
    QCOMPARE(shape.points[3], QPointF(20, 55));
    QCOMPARE(labelList->count(), 1);
    QCOMPARE(labelList->item(0)->text(), QStringLiteral("rotated"));
    QVERIFY(editModeAction->isChecked());
}

void UiTests::mainWindowCanCreateNativeLinestripShape() {
    resetTestSettings("create-native-linestrip");
    QSettings settings;
    settings.setValue("labelHistory", QStringList{QStringLiteral("path")});
    settings.setValue("lastUsedLabel", QStringLiteral("path"));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(140, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QListWidget *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    QAction *linestripAction = window.findChild<QAction *>(QStringLiteral("createLinestripModeAction"));
    QToolButton *mainModeButton = window.findChild<QToolButton *>(QStringLiteral("mainModeButton"));
    QAction *editModeAction = window.findChild<QAction *>(QStringLiteral("editModeAction"));
    QVERIFY(canvas);
    QVERIFY(labelList);
    QVERIFY(linestripAction);
    QVERIFY(mainModeButton);
    QVERIFY(editModeAction);
    QVERIFY(mainModeButton->menu()->actions().contains(linestripAction));

    canvas->setScale(1.0);
    acceptNextLabelPrompt(QStringLiteral("path"));
    linestripAction->trigger();
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 25));
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(90, 55));
    QCOMPARE(canvas->shapes().size(), 0);
    QTest::keyClick(canvas, Qt::Key_Return);

    QCOMPARE(canvas->shapes().size(), 1);
    const Shape shape = canvas->shapes().first();
    QCOMPARE(shape.shapeType, QStringLiteral("linestrip"));
    QCOMPARE(shape.label, QStringLiteral("path"));
    QCOMPARE(shape.points, QVector<QPointF>({QPointF(20, 20), QPointF(60, 25), QPointF(90, 55)}));
    QCOMPARE(labelList->count(), 1);
    QCOMPARE(labelList->item(0)->text(), QStringLiteral("path"));
    QVERIFY(editModeAction->isChecked());
}

void UiTests::mainWindowCanCreateNativeMaskShape() {
    resetTestSettings("create-native-mask");
    QSettings settings;
    settings.setValue("labelHistory", QStringList{QStringLiteral("mask-label")});
    settings.setValue("lastUsedLabel", QStringLiteral("mask-label"));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(140, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *canvas = window.findChild<Canvas *>();
    auto *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    auto *maskAction = window.findChild<QAction *>(QStringLiteral("createMaskModeAction"));
    auto *maskEditAction = window.findChild<QAction *>(QStringLiteral("maskEditAction"));
    QVERIFY(canvas);
    QVERIFY(labelList);
    QVERIFY(maskAction);
    QVERIFY(maskEditAction);

    acceptNextLabelPrompt(QStringLiteral("mask-label"));
    maskAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(canvas, QPoint(45, 35));
    QTest::mouseMove(canvas, QPoint(80, 55));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 55));

    QCOMPARE(canvas->shapes().size(), 1);
    QCOMPARE(canvas->shapes().first().shapeType, QStringLiteral("mask"));
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("mask-label"));
    QVERIFY(!canvas->shapes().first().maskData.isEmpty());
    QCOMPARE(labelList->count(), 1);

    QVERIFY(maskEditAction->isEnabled());
    maskEditAction->trigger();
    QVERIFY(canvas->maskEditing());
    maskEditAction->trigger();
    QVERIFY(!canvas->maskEditing());
}

void UiTests::mainWindowExposesLabelMeAiPromptModes() {
    resetTestSettings("labelme-ai-prompt-modes");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(120, 90, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("ai.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *aiPointsAction = window.findChild<QAction *>(QStringLiteral("createAiPointsModeAction"));
    QAction *aiBoxAction = window.findChild<QAction *>(QStringLiteral("createAiBoxModeAction"));
    QComboBox *aiModelCombo = window.findChild<QComboBox *>(QStringLiteral("aiModelCombo"));
    QComboBox *aiOutputCombo = window.findChild<QComboBox *>(QStringLiteral("aiOutputFormatCombo"));
    QVERIFY(canvas);
    QVERIFY(aiPointsAction);
    QVERIFY(aiBoxAction);
    QVERIFY(aiModelCombo);
    QVERIFY(aiOutputCombo);
    QVERIFY(!aiModelCombo->isEnabled());
    QVERIFY(!aiOutputCombo->isEnabled());

    aiBoxAction->trigger();
    QVERIFY(aiBoxAction->isChecked());
    QCOMPARE(canvas->createShapeType(), QStringLiteral("ai_box_to_shape"));
    QVERIFY(aiModelCombo->isEnabled());
    QVERIFY(aiOutputCombo->isEnabled());
    QVERIFY(aiModelCombo->findData(QStringLiteral("sam3:latest")) >= 0);
    aiOutputCombo->setCurrentIndex(aiOutputCombo->findData(QStringLiteral("mask")));
    QCOMPARE(aiOutputCombo->currentData().toString(), QStringLiteral("mask"));

    aiPointsAction->trigger();
    QVERIFY(aiPointsAction->isChecked());
    QCOMPARE(canvas->createShapeType(), QStringLiteral("ai_points_to_shape"));
}

void UiTests::mainWindowDisablesUnsupportedAiPointModels() {
    resetTestSettings("labelme-ai-point-model-availability");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(120, 90, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("ai-model.jpg"));
    QVERIFY(image.save(imagePath));
    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    auto *aiPointsAction = window.findChild<QAction *>(QStringLiteral("createAiPointsModeAction"));
    auto *aiBoxAction = window.findChild<QAction *>(QStringLiteral("createAiBoxModeAction"));
    auto *aiModelCombo = window.findChild<QComboBox *>(QStringLiteral("aiModelCombo"));
    QVERIFY(aiPointsAction);
    QVERIFY(aiBoxAction);
    QVERIFY(aiModelCombo);

    const int sam3Index = aiModelCombo->findData(QStringLiteral("sam3:latest"));
    QVERIFY(sam3Index >= 0);
    QVERIFY(aiModelCombo->model());
    auto *model = qobject_cast<QStandardItemModel *>(aiModelCombo->model());
    QVERIFY(model);
    QVERIFY(model->item(sam3Index));
    QVERIFY(model->item(sam3Index)->flags() & Qt::ItemIsEnabled);

    aiPointsAction->trigger();
    QVERIFY(aiPointsAction->isChecked());
    QCOMPARE(aiModelCombo->currentData().toString(), QStringLiteral("sam2:latest"));
    QVERIFY(!(model->item(sam3Index)->flags() & Qt::ItemIsEnabled));

    aiBoxAction->trigger();
    QVERIFY(model->item(sam3Index)->flags() & Qt::ItemIsEnabled);
}

void UiTests::mainWindowExposesLabelMeAiTextPromptControls() {
    resetTestSettings("labelme-ai-text-prompt-controls");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(120, 90, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("ai-text.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *promptEdit = window.findChild<QLineEdit *>(QStringLiteral("aiTextPromptEdit"));
    auto *textModelCombo = window.findChild<QComboBox *>(QStringLiteral("aiTextModelCombo"));
    auto *scoreSpin = window.findChild<QDoubleSpinBox *>(QStringLiteral("aiTextScoreSpin"));
    auto *iouSpin = window.findChild<QDoubleSpinBox *>(QStringLiteral("aiTextIouSpin"));
    auto *runButton = window.findChild<QToolButton *>(QStringLiteral("aiTextRunButton"));
    QVERIFY(promptEdit);
    QVERIFY(textModelCombo);
    QVERIFY(scoreSpin);
    QVERIFY(iouSpin);
    QVERIFY(runButton);
    QVERIFY(!runButton->isEnabled());
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    auto *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QVERIFY(createModeAction);
    createModeAction->trigger();
    promptEdit->setText(QStringLiteral("person, sofa"));
    QTRY_VERIFY(runButton->isEnabled());
    QCOMPARE(scoreSpin->value(), 0.1);
    QCOMPARE(iouSpin->value(), 0.5);
    QVERIFY(textModelCombo->findData(QStringLiteral("yoloworld:latest")) >= 0);
}

void UiTests::mainWindowShowsAiDownloadProgressAndCanCancel() {
    resetTestSettings("ai-download-progress-cancel");
    QSettings settings;
    settings.setValue(QStringLiteral("ai/bridgePath"),
                      ResourcePaths::filePath(QStringLiteral("cpp/tests/fixtures/progress_ai_bridge.py")));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(120, 90, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("ai-progress.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *promptEdit = window.findChild<QLineEdit *>(QStringLiteral("aiTextPromptEdit"));
    auto *runButton = window.findChild<QToolButton *>(QStringLiteral("aiTextRunButton"));
    auto *progressBar = window.findChild<QProgressBar *>(QStringLiteral("aiProgressBar"));
    auto *cancelButton = window.findChild<QToolButton *>(QStringLiteral("aiCancelButton"));
    auto *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QVERIFY(promptEdit);
    QVERIFY(runButton);
    QVERIFY(progressBar);
    QVERIFY(cancelButton);
    QVERIFY(createModeAction);
    QVERIFY(!progressBar->isVisible());
    QVERIFY(!cancelButton->isVisible());

    createModeAction->trigger();
    promptEdit->setText(QStringLiteral("person"));
    QTRY_VERIFY(runButton->isEnabled());
    runButton->click();

    QTRY_VERIFY_WITH_TIMEOUT(progressBar->isVisible(), 3000);
    QTRY_COMPARE_WITH_TIMEOUT(progressBar->maximum(), 512, 3000);
    QVERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("model.bin")));
    QVERIFY(cancelButton->isVisible());

    cancelButton->click();
    QTRY_VERIFY_WITH_TIMEOUT(!progressBar->isVisible(), 3000);
    QVERIFY(!cancelButton->isVisible());
    QVERIFY(runButton->isEnabled());
}

void UiTests::mainWindowEmptyAiResultCancelsPrompt() {
    resetTestSettings("empty-ai-result-cancels-prompt");
    QSettings settings;
    settings.setValue(QStringLiteral("autosave"), true);
    settings.setValue(QStringLiteral("ai/bridgePath"),
                      ResourcePaths::filePath(QStringLiteral("cpp/tests/fixtures/empty_ai_bridge.py")));
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(120, 90, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("empty-ai.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto *canvas = window.findChild<Canvas *>();
    auto *aiBoxAction = window.findChild<QAction *>(QStringLiteral("createAiBoxModeAction"));
    QVERIFY(canvas);
    QVERIFY(aiBoxAction);

    aiBoxAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(canvas, QPoint(70, 60));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(70, 60));

    QTRY_VERIFY_WITH_TIMEOUT(canvas->shapes().isEmpty(), 4000);
}

void UiTests::mainWindowAiTextPromptCreatesFilteredShapes() {
    resetTestSettings("ai-text-prompt-creates-filtered-shapes");
    QSettings settings;
    settings.setValue(QStringLiteral("ai/bridgePath"),
                      ResourcePaths::filePath(QStringLiteral("cpp/tests/fixtures/text_ai_bridge.py")));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(160, 120, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("text-ai.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *canvas = window.findChild<Canvas *>();
    auto *promptEdit = window.findChild<QLineEdit *>(QStringLiteral("aiTextPromptEdit"));
    auto *scoreSpin = window.findChild<QDoubleSpinBox *>(QStringLiteral("aiTextScoreSpin"));
    auto *runButton = window.findChild<QToolButton *>(QStringLiteral("aiTextRunButton"));
    auto *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QVERIFY(canvas);
    QVERIFY(promptEdit);
    QVERIFY(scoreSpin);
    QVERIFY(runButton);
    QVERIFY(createModeAction);

    createModeAction->trigger();
    promptEdit->setText(QStringLiteral("person, sofa"));
    scoreSpin->setValue(0.5);
    QTRY_VERIFY(runButton->isEnabled());
    runButton->click();

    QTRY_VERIFY_WITH_TIMEOUT(canvas->shapes().size() == 1, 4000);
    const Shape shape = canvas->shapes().first();
    QCOMPARE(shape.label, QStringLiteral("person"));
    QCOMPARE(shape.shapeType, QStringLiteral("rectangle"));
    QVERIFY(shape.descriptionPresent);
    QVERIFY(shape.description.contains(QStringLiteral("score=0.85")));
    QCOMPARE(canvas->selectedIndices(), QVector<int>({0}));
    QVERIFY(runButton->isEnabled());
}

void UiTests::mainWindowAiTextPromptIgnoresDifferentExistingLabels() {
    resetTestSettings("ai-text-prompt-different-existing-label");
    QSettings settings;
    settings.setValue(QStringLiteral("ai/bridgePath"),
                      ResourcePaths::filePath(QStringLiteral("cpp/tests/fixtures/text_ai_bridge.py")));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(160, 120, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("text-existing.jpg"));
    QVERIFY(image.save(imagePath));

    AnnotationDocument existing;
    existing.imagePath = imagePath;
    existing.imageSize = image.size();
    existing.shapes = {
        Shape::fromRect(QStringLiteral("cat"), QRectF(20, 15, 50, 65), false)};
    const QString annotationPath = dir.filePath(QStringLiteral("text-existing.json"));
    QVERIFY2(AnnotationIO::saveLabelMe(annotationPath, existing),
             qPrintable(annotationPath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *canvas = window.findChild<Canvas *>();
    auto *promptEdit = window.findChild<QLineEdit *>(QStringLiteral("aiTextPromptEdit"));
    auto *scoreSpin = window.findChild<QDoubleSpinBox *>(QStringLiteral("aiTextScoreSpin"));
    auto *runButton = window.findChild<QToolButton *>(QStringLiteral("aiTextRunButton"));
    auto *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QVERIFY(canvas);
    QVERIFY(promptEdit);
    QVERIFY(scoreSpin);
    QVERIFY(runButton);
    QVERIFY(createModeAction);
    QCOMPARE(canvas->shapes().size(), 1);
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("cat"));

    createModeAction->trigger();
    promptEdit->setText(QStringLiteral("person, sofa"));
    scoreSpin->setValue(0.0);
    QTRY_VERIFY(runButton->isEnabled());
    runButton->click();

    QTRY_VERIFY_WITH_TIMEOUT(canvas->shapes().size() == 3, 4000);
    QStringList labels;
    for (const Shape &shape : canvas->shapes()) {
        labels.append(shape.label);
    }
    QVERIFY(labels.contains(QStringLiteral("cat")));
    QVERIFY(labels.contains(QStringLiteral("person")));
    QVERIFY(labels.contains(QStringLiteral("sofa")));
}

void UiTests::mainWindowQESelectPreviousNextBoxAndToggleSingleSelection() {
    resetTestSettings("qe-cycle-shapes");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(120, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QListWidget *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    QAction *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QAction *prevShapeAction = window.findChild<QAction *>(QStringLiteral("prevShapeAction"));
    QAction *nextShapeAction = window.findChild<QAction *>(QStringLiteral("nextShapeAction"));
    QVERIFY(canvas);
    QVERIFY(labelList);
    QVERIFY(createModeAction);
    QVERIFY(prevShapeAction);
    QVERIFY(nextShapeAction);

    acceptNextLabelPrompt();
    createModeAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
    QTest::mouseMove(canvas, QPoint(35, 30));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(35, 30));
    acceptNextLabelPrompt();
    createModeAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 45));
    QTest::mouseMove(canvas, QPoint(80, 70));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 70));
    QCOMPARE(canvas->shapes().size(), 2);
    QCOMPARE(canvas->currentIndex(), 1);

    canvas->setFocus();
    QTest::keyClick(canvas, Qt::Key_Q);
    QCOMPARE(canvas->currentIndex(), 0);
    QCOMPARE(labelList->currentRow(), 0);
    QTest::keyClick(canvas, Qt::Key_E);
    QCOMPARE(canvas->currentIndex(), 1);
    QCOMPARE(labelList->currentRow(), 1);

    canvas->setCurrentIndex(-1);
    labelList->clearSelection();
    QTest::keyClick(canvas, Qt::Key_E);
    QCOMPARE(canvas->currentIndex(), 0);

    QTest::keyClick(canvas, Qt::Key_X);
    QCOMPARE(canvas->shapes().size(), 1);
    QCOMPARE(canvas->currentIndex(), 0);
    QCOMPARE(canvas->selectedIndices(), QVector<int>({0}));

    QVERIFY(nextShapeAction->isEnabled());
    nextShapeAction->trigger();
    QCOMPARE(canvas->currentIndex(), -1);
    prevShapeAction->trigger();
    QCOMPARE(canvas->currentIndex(), 0);
}

void UiTests::mainWindowXDeletesCurrentShape() {
    resetTestSettings("x-delete-current-shape");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QVERIFY(canvas);
    QVERIFY(createModeAction);

    acceptNextLabelPrompt();
    createModeAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(canvas, QPoint(60, 50));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 50));
    QCOMPARE(canvas->shapes().size(), 1);
    QCOMPARE(canvas->currentIndex(), 0);

    canvas->setFocus();
    QTest::keyClick(canvas, Qt::Key_X);
    QCOMPARE(canvas->shapes().size(), 0);
    QCOMPARE(canvas->currentIndex(), -1);
}

void UiTests::mainWindowDeleteAllShapesActionClearsAndUndoRestores() {
    resetTestSettings("delete-all-shapes-action");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(140, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("delete-all.jpg"));
    QVERIFY(image.save(imagePath));

    AnnotationDocument document;
    document.imagePath = imagePath;
    document.imageSize = image.size();
    document.depth = 3;
    document.shapes = {Shape::fromRect(QStringLiteral("one"), QRectF(10, 10, 20, 20), false),
                       Shape::fromRect(QStringLiteral("two"), QRectF(60, 40, 25, 15), false)};
    QVERIFY(AnnotationIO::savePascalVoc(dir.filePath(QStringLiteral("delete-all.xml")), document));

    MainWindow window;
    QVERIFY(window.openPath(imagePath));
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QListWidget *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    QAction *deleteAllAction = window.findChild<QAction *>(QStringLiteral("deleteAllShapesAction"));
    QAction *undoAction = actionByShortcut(&window, QKeySequence::Undo);
    QVERIFY(canvas);
    QVERIFY(labelList);
    QVERIFY(deleteAllAction);
    QVERIFY(undoAction);

    QCOMPARE(canvas->shapes().size(), 2);
    QVERIFY(deleteAllAction->isEnabled());

    deleteAllAction->trigger();
    QCOMPARE(canvas->shapes().size(), 0);
    QCOMPARE(labelList->count(), 0);

    undoAction->trigger();
    QCOMPARE(canvas->shapes().size(), 2);
    QCOMPARE(labelList->count(), 2);
}

void UiTests::mainWindowDeleteUsesCanvasSelection() {
    resetTestSettings("labelme-delete-selected-shape");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(140, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("a.jpg"));
    QVERIFY(image.save(imagePath));
    AnnotationDocument document;
    document.imagePath = imagePath;
    document.imageSize = image.size();
    document.shapes = {Shape::fromRect(QStringLiteral("canvas-selected"), QRectF(15, 20, 24, 18), false),
                       Shape::fromRect(QStringLiteral("list-current"), QRectF(80, 60, 12, 12), false)};
    QVERIFY(AnnotationIO::savePascalVoc(dir.filePath(QStringLiteral("a.xml")), document));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QListWidget *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    QAction *deleteAction = actionByShortcut(&window, QKeySequence(Qt::Key_Delete));
    QVERIFY(canvas);
    QVERIFY(labelList);
    QVERIFY(deleteAction);

    QCOMPARE(labelList->count(), 2);
    canvas->setSelectedIndices({0});
    {
        QSignalBlocker blocker(labelList);
        labelList->setCurrentRow(1);
    }
    QCOMPARE(canvas->currentIndex(), 0);
    QCOMPARE(labelList->currentRow(), 1);

    deleteAction->trigger();

    QCOMPARE(canvas->shapes().size(), 1);
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("list-current"));
}

void UiTests::mainWindowShapeActionsIgnoreStaleLabelListSelection() {
    resetTestSettings("labelme-shape-actions-ignore-stale-list-selection");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(140, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("a.jpg"));
    QVERIFY(image.save(imagePath));

    AnnotationDocument document;
    document.imagePath = imagePath;
    document.imageSize = image.size();
    document.shapes = {
        Shape::fromRect(QStringLiteral("first"), QRectF(15, 20, 24, 18), false),
        Shape::fromRect(QStringLiteral("second"), QRectF(80, 60, 12, 12), false),
    };
    QVERIFY(AnnotationIO::saveLabelMe(dir.filePath(QStringLiteral("a.json")), document));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *canvas = window.findChild<Canvas *>();
    auto *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    auto *deleteAction = actionByShortcut(&window, QKeySequence(Qt::Key_Delete));
    auto *editLabelAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+E")));
    QVERIFY(canvas);
    QVERIFY(labelList);
    QVERIFY(deleteAction);
    QVERIFY(editLabelAction);
    QCOMPARE(canvas->shapes().size(), 2);

    canvas->setSelectedIndices({});
    {
        QSignalBlocker blocker(labelList);
        labelList->setCurrentRow(1, QItemSelectionModel::ClearAndSelect);
    }
    QCOMPARE(canvas->currentIndex(), -1);
    QCOMPARE(labelList->currentRow(), 1);

    QVERIFY(QMetaObject::invokeMethod(&window,
                                      "onCanvasSelectionChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(int, -1)));
    QVERIFY2(!deleteAction->isEnabled(), "stale list highlight must not enable deletion");
    QVERIFY2(!editLabelAction->isEnabled(), "stale list highlight must not enable label editing");

    // The slot must keep the same source-of-truth rule even when invoked
    // directly during a queued signal/update race.
    QVERIFY(QMetaObject::invokeMethod(&window, "deleteCurrentShape", Qt::DirectConnection));
    QCOMPARE(canvas->shapes().size(), 2);
}

void UiTests::mainWindowDuplicateUsesSelectedShapesWithoutOffset() {
    resetTestSettings("labelme-duplicate-shapes");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(140, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath(QStringLiteral("a.jpg"))));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), dir.filePath(QStringLiteral("a.jpg"))});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *duplicateAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+D")));
    QVERIFY(canvas);
    QVERIFY(duplicateAction);

    const Shape original = Shape::fromRect(QStringLiteral("selected"), QRectF(15, 20, 24, 18), false);
    const Shape unrelated = Shape::fromRect(QStringLiteral("unrelated"), QRectF(80, 60, 12, 12), false);
    canvas->setShapes({original, unrelated});
    canvas->setSelectedIndices({0});
    QApplication::processEvents();

    QVERIFY(duplicateAction->isEnabled());
    duplicateAction->trigger();

    QCOMPARE(canvas->shapes().size(), 3);
    QCOMPARE(canvas->shapes().at(2).points, original.points);
    QCOMPARE(canvas->shapes().at(2).label, original.label);
    QCOMPARE(canvas->selectedIndices(), QVector<int>({2}));
    QCOMPARE(canvas->currentIndex(), 2);
}

void UiTests::mainWindowCopiesAndPastesSelectedShapes() {
    resetTestSettings("shape-clipboard");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(140, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *copyAction = window.findChild<QAction *>(QStringLiteral("copyShapesAction"));
    QAction *pasteAction = window.findChild<QAction *>(QStringLiteral("pasteShapesAction"));
    QVERIFY(canvas);
    QVERIFY(copyAction);
    QVERIFY(pasteAction);

    Shape first = Shape::fromRect(QStringLiteral("first"), QRectF(10, 10, 15, 12), false);
    first.description = QStringLiteral("copy description");
    first.lineColor = QColor(12, 34, 56, 200);
    first.fillColor = QColor(65, 43, 21, 90);
    Shape second = Shape::fromRect(QStringLiteral("second"), QRectF(45, 20, 18, 10), true);
    second.flags.insert(QStringLiteral("occluded"), true);
    canvas->setShapes({first, second});
    canvas->setSelectedIndices({0, 1});
    QApplication::processEvents();

    QVERIFY(copyAction->isEnabled());
    copyAction->trigger();
    QVERIFY(pasteAction->isEnabled());
    canvas->setSelectedIndices({});
    pasteAction->trigger();

    QCOMPARE(canvas->shapes().size(), 4);
    QCOMPARE(canvas->selectedIndices(), QVector<int>({2, 3}));
    QCOMPARE(canvas->shapes()[2].description, QStringLiteral("copy description"));
    QCOMPARE(canvas->shapes()[3].flags.value(QStringLiteral("occluded")), true);
    QCOMPARE(canvas->shapes()[2].label, QStringLiteral("first"));
    QCOMPARE(canvas->shapes()[2].lineColor, first.lineColor);
    QCOMPARE(canvas->shapes()[2].fillColor, first.fillColor);
}

void UiTests::mainWindowCopiesShapesAsLabelMeClipboardJsonAcrossWindows() {
    resetTestSettings("shape-clipboard-labelme-json");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(140, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("clipboard-json.jpg"));
    QVERIFY(image.save(imagePath));

    auto *clipboard = QApplication::clipboard();
    QVERIFY(clipboard);
    clipboard->clear();

    MainWindow source;
    source.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    Canvas *sourceCanvas = source.findChild<Canvas *>();
    QAction *copyAction = source.findChild<QAction *>(QStringLiteral("copyShapesAction"));
    QVERIFY(sourceCanvas);
    QVERIFY(copyAction);

    Shape sourceShape = Shape::fromPoints(QStringLiteral("缺陷"),
                                          QStringLiteral("polygon"),
                                          {QPointF(10, 10), QPointF(45, 12), QPointF(30, 40)},
                                          false);
    sourceShape.groupId = 7;
    sourceShape.description = QStringLiteral("from LabelMe clipboard");
    sourceShape.flags.insert(QStringLiteral("occluded"), true);
    sourceCanvas->setShapes({sourceShape});
    sourceCanvas->setSelectedIndices({0});
    QApplication::processEvents();
    copyAction->trigger();

    QVERIFY(clipboard->mimeData()->hasFormat(QStringLiteral("application/x-labelme-shapes")));
    const QJsonDocument copied = QJsonDocument::fromJson(
        clipboard->mimeData()->data(QStringLiteral("application/x-labelme-shapes")));
    QVERIFY(copied.isArray());
    QCOMPARE(copied.array().size(), 1);
    QCOMPARE(copied.array().first().toObject().value(QStringLiteral("label")).toString(),
             QStringLiteral("缺陷"));

    MainWindow destination;
    destination.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    Canvas *destinationCanvas = destination.findChild<Canvas *>();
    QAction *pasteAction = destination.findChild<QAction *>(QStringLiteral("pasteShapesAction"));
    QVERIFY(destinationCanvas);
    QVERIFY(pasteAction);
    QVERIFY(pasteAction->isEnabled());
    pasteAction->trigger();

    QCOMPARE(destinationCanvas->shapes().size(), 1);
    const Shape pasted = destinationCanvas->shapes().first();
    QCOMPARE(pasted.label, QStringLiteral("缺陷"));
    QCOMPARE(pasted.groupId, 7);
    QCOMPARE(pasted.description, QStringLiteral("from LabelMe clipboard"));
    QCOMPARE(pasted.flags.value(QStringLiteral("occluded")), true);
    QCOMPARE(pasted.points, sourceShape.points);
    clipboard->clear();
}

void UiTests::mainWindowUndoRedoCreateDeleteAndLabelEdit() {
    resetTestSettings("undo-redo-shape-ops");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QListWidget *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    QAction *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QAction *editModeAction = window.findChild<QAction *>(QStringLiteral("editModeAction"));
    QAction *undoAction = actionByShortcut(&window, QKeySequence::Undo);
    QAction *redoAction = actionByShortcut(&window, QKeySequence::Redo);
    QAction *deleteAction = actionByShortcut(&window, QKeySequence(Qt::Key_Delete));
    QVERIFY(canvas);
    QVERIFY(labelList);
    QVERIFY(createModeAction);
    QVERIFY(editModeAction);
    QVERIFY(undoAction);
    QVERIFY(redoAction);
    QVERIFY(deleteAction);

    acceptNextLabelPrompt();
    createModeAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(canvas, QPoint(60, 50));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 50));
    QCOMPARE(canvas->shapes().size(), 1);
    QVERIFY(editModeAction->isChecked());

    undoAction->trigger();
    QCOMPARE(canvas->shapes().size(), 0);
    redoAction->trigger();
    QCOMPARE(canvas->shapes().size(), 1);
    canvas->setSelectedIndices({0});

    deleteAction->trigger();
    QCOMPARE(canvas->shapes().size(), 0);
    undoAction->trigger();
    QCOMPARE(canvas->shapes().size(), 1);

    QVERIFY(labelList->item(0));
    const QString originalLabel = canvas->shapes().first().label;
    labelList->item(0)->setText(QStringLiteral("edited_label"));
    QApplication::processEvents();
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("edited_label"));

    undoAction->trigger();
    QCOMPARE(canvas->shapes().first().label, originalLabel);
    redoAction->trigger();
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("edited_label"));
}

void UiTests::mainWindowUndoMoveIsOneHistoryStep() {
    resetTestSettings("undo-move-is-one-step");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(120, 90, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("move.jpg"));
    QVERIFY(image.save(imagePath));

    AnnotationDocument document;
    document.imagePath = imagePath;
    document.imageSize = image.size();
    document.shapes = {Shape::fromRect(QStringLiteral("defect"), QRectF(10, 10, 20, 15), false)};
    QVERIFY(AnnotationIO::savePascalVoc(dir.filePath(QStringLiteral("move.xml")), document));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *undoAction = actionByShortcut(&window, QKeySequence::Undo);
    QVERIFY(canvas);
    QVERIFY(undoAction);
    canvas->setScale(1.0);
    canvas->setEditMode();
    canvas->setCurrentIndex(0);
    const QRectF original = canvas->shapes().first().boundingRect();

    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(15, 15));
    QTest::mouseMove(canvas, QPoint(18, 17));
    QTest::mouseMove(canvas, QPoint(24, 22));
    QTest::mouseMove(canvas, QPoint(31, 28));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(31, 28));
    QVERIFY(canvas->shapes().first().boundingRect().topLeft() != original.topLeft());

    undoAction->trigger();
    QCOMPARE(canvas->shapes().first().boundingRect(), original);
}

void UiTests::mainWindowKeyboardMoveMarksDirtyAndCanUndo() {
    resetTestSettings("keyboard-move-history");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(120, 90, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("keyboard.jpg"));
    QVERIFY(image.save(imagePath));

    AnnotationDocument document;
    document.imagePath = imagePath;
    document.imageSize = image.size();
    document.shapes = {Shape::fromRect(QStringLiteral("defect"), QRectF(10, 10, 20, 15), false)};
    QVERIFY(AnnotationIO::savePascalVoc(dir.filePath(QStringLiteral("keyboard.xml")), document));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *undoAction = actionByShortcut(&window, QKeySequence::Undo);
    QVERIFY(canvas);
    QVERIFY(undoAction);
    canvas->setScale(1.0);
    canvas->setEditMode();
    canvas->setCurrentIndex(0);
    canvas->setFocus();
    const QRectF original = canvas->shapes().first().boundingRect();

    QTest::keyClick(canvas, Qt::Key_Right);
    QCOMPARE(canvas->shapes().first().boundingRect().topLeft(), original.topLeft() + QPointF(5, 0));
    QVERIFY(window.windowTitle().endsWith(QStringLiteral("*")));
    QVERIFY(undoAction->isEnabled());

    undoAction->trigger();
    QCOMPARE(canvas->shapes().first().boundingRect(), original);
}

void UiTests::mainWindowRejectsKeyboardDegenerateShapeWithoutDirtying() {
    resetTestSettings("keyboard-degenerate-no-dirty");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(120, 90, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("degenerate.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QVERIFY(canvas);
    QVERIFY(createModeAction);

    createModeAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(30, 30));
    QTest::keyClick(canvas, Qt::Key_Return);

    QCOMPARE(canvas->shapes().size(), 0);
    QVERIFY(!window.windowTitle().endsWith(QStringLiteral("*")));
}

void UiTests::mainWindowUndoCreateIsOneHistoryStep() {
    resetTestSettings("undo-create-is-one-step");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(120, 90, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("create.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QAction *undoAction = actionByShortcut(&window, QKeySequence::Undo);
    QVERIFY(canvas);
    QVERIFY(createModeAction);
    QVERIFY(undoAction);

    acceptNextLabelPrompt();
    createModeAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(15, 15));
    QTest::mouseMove(canvas, QPoint(20, 18));
    QTest::mouseMove(canvas, QPoint(28, 24));
    QTest::mouseMove(canvas, QPoint(36, 31));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(36, 31));
    QCOMPARE(canvas->shapes().size(), 1);

    undoAction->trigger();
    QCOMPARE(canvas->shapes().size(), 0);
}

void UiTests::mainWindowUndoPreservesPointLabelsAndShapeOtherData() {
    resetTestSettings("undo-preserves-shape-fields");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(120, 90, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("shape-fields.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *undoAction = actionByShortcut(&window, QKeySequence::Undo);
    QVERIFY(canvas);
    QVERIFY(undoAction);

    Shape shape = Shape::fromPoints(QStringLiteral("prompt"), QStringLiteral("points"),
                                    {QPointF(20, 20), QPointF(40, 40)}, false);
    shape.pointLabels = {1, 0};
    shape.labelMeOtherData.insert(QStringLiteral("score"), 0.75);
    canvas->setShapes({shape});
    canvas->setAllShapesVisible(true);
    QApplication::processEvents();

    canvas->shapesRef()[0].pointLabels = {0, 1};
    canvas->shapesRef()[0].labelMeOtherData.insert(QStringLiteral("score"), 0.25);
    canvas->setAllShapesVisible(true);
    QApplication::processEvents();

    QVERIFY(undoAction->isEnabled());
    undoAction->trigger();
    QCOMPARE(canvas->shapes().size(), 1);
    QCOMPARE(canvas->shapes().first().pointLabels, QVector<int>({1, 0}));
    QCOMPARE(canvas->shapes().first().labelMeOtherData.value(QStringLiteral("score")).toDouble(), 0.75);
}

void UiTests::mainWindowUndoClearsSelectionLikeLabelMe() {
    resetTestSettings("undo-clears-selection-labelme");
    QSettings settings;
    settings.setValue(QStringLiteral("labelFileFormat"), 3);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("selection.jpg"));
    QVERIFY(image.save(imagePath));

    AnnotationDocument document;
    document.imagePath = imagePath;
    document.imageSize = image.size();
    document.shapes = {
        Shape::fromRect(QStringLiteral("first"), QRectF(5, 5, 15, 12), false),
        Shape::fromRect(QStringLiteral("second"), QRectF(35, 25, 18, 14), false),
    };
    QVERIFY(AnnotationIO::saveLabelMe(dir.filePath(QStringLiteral("selection.json")), document));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *canvas = window.findChild<Canvas *>();
    auto *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    auto *undoAction = actionByShortcut(&window, QKeySequence::Undo);
    QVERIFY(canvas);
    QVERIFY(labelList);
    QVERIFY(undoAction);
    QCOMPARE(labelList->count(), 2);

    labelList->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);
    QCOMPARE(canvas->selectedIndices(), QVector<int>({0}));
    labelList->item(0)->setCheckState(Qt::Unchecked);
    QVERIFY(undoAction->isEnabled());

    undoAction->trigger();
    QVERIFY(canvas->selectedIndices().isEmpty());
    QVERIFY(labelList->selectedItems().isEmpty());
}

void UiTests::mainWindowEditLabelDialogDefaultsToCurrentLabel() {
    resetTestSettings("edit-label-current-default");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QListWidget *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    QAction *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QAction *editLabelAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+E")));
    QVERIFY(canvas);
    QVERIFY(labelList);
    QVERIFY(createModeAction);
    QVERIFY(editLabelAction);

    acceptNextLabelPrompt();
    createModeAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(canvas, QPoint(60, 50));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 50));
    QCOMPARE(labelList->count(), 1);
    labelList->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);

    const QString currentLabel = QStringLiteral("rare_current_label");
    canvas->shapesRef()[0].label = currentLabel;

    bool inspected = false;
    QString observedText;
    QString observedSelectedText;
    QTimer::singleShot(50, [&]() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (dialog) {
            if (auto *combo = dialog->findChild<QComboBox *>()) {
                observedText = combo->currentText();
                if (combo->lineEdit()) {
                    observedSelectedText = combo->lineEdit()->selectedText();
                }
                inspected = true;
            }
            dialog->reject();
        }
    });
    editLabelAction->trigger();

    QVERIFY(inspected);
    QCOMPARE(observedText, currentLabel);
    QCOMPARE(observedSelectedText, currentLabel);
}

void UiTests::mainWindowLabelEditDialogUsesCurrentLanguageForMetadataFields() {
    resetTestSettings("edit-label-dialog-language");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("a.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *editLabelAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+E")));
    QVERIFY(canvas);
    QVERIFY(editLabelAction);

    Shape shape = Shape::fromRect(QStringLiteral("defect"), QRectF(10, 10, 30, 20), false);
    canvas->setShapes({shape});
    canvas->setSelectedIndices({0});
    QAction *zhAction = languageAction(&window, QStringLiteral("zh-CN"));
    QVERIFY(zhAction);
    zhAction->trigger();
    QApplication::processEvents();

    bool seen = false;
    QStringList labels;
    InspectLabelDialogOnShow inspector(&seen, &labels);
    QApplication::instance()->installEventFilter(&inspector);
    editLabelAction->trigger();
    QApplication::instance()->removeEventFilter(&inspector);

    QVERIFY(seen);
    QVERIFY(labels.contains(QStringLiteral("分组 ID")));
    QVERIFY(labels.contains(QStringLiteral("标签描述")));
}

void UiTests::mainWindowBlankLabelKeepsEditDialogOpen() {
    resetTestSettings("edit-label-blank-keeps-open");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("a.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QListWidget *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    QAction *createAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QAction *editLabelAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+E")));
    QVERIFY(canvas);
    QVERIFY(labelList);
    QVERIFY(createAction);
    QVERIFY(editLabelAction);

    acceptNextLabelPrompt(QStringLiteral("original"));
    createAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
    QTest::mouseMove(canvas, QPoint(40, 35));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(40, 35));
    QCOMPARE(canvas->shapes().size(), 1);
    labelList->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);

    bool dialogInspected = false;
    bool dialogStayedOpen = false;
    QTimer::singleShot(50, [&]() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            return;
        }
        auto *combo = dialog->findChild<QComboBox *>(QStringLiteral("labelEditCombo"));
        if (!combo || !combo->lineEdit()) {
            dialog->reject();
            return;
        }
        dialogInspected = true;
        combo->lineEdit()->clear();
        QTest::keyClick(combo->lineEdit(), Qt::Key_Return);
        QTimer::singleShot(50, [&, dialog]() {
            dialogStayedOpen = dialog->isVisible();
            if (dialog->isVisible()) {
                dialog->reject();
            }
        });
    });
    editLabelAction->trigger();

    QVERIFY(dialogInspected);
    QVERIFY(dialogStayedOpen);
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("original"));
}

void UiTests::mainWindowLabelEditArrowKeysNavigateLabelList() {
    resetTestSettings("label-edit-arrow-navigation");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("a.jpg"));
    QVERIFY(image.save(imagePath));
    const QString classFilePath = dir.filePath(QStringLiteral("classes.txt"));
    QFile classFile(classFilePath);
    QVERIFY(classFile.open(QIODevice::WriteOnly | QIODevice::Text));
    classFile.write("cat\ndog\nperson\n");
    classFile.close();

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath, classFilePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *createAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QAction *editLabelAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+E")));
    QVERIFY(canvas);
    QVERIFY(createAction);
    QVERIFY(editLabelAction);

    acceptNextLabelPrompt(QStringLiteral("cat"));
    createAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(canvas, QPoint(60, 50));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 50));
    QCOMPARE(canvas->shapes().size(), 1);

    bool inspected = false;
    QString selectedLabel;
    QTimer::singleShot(50, [&]() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            return;
        }
        auto *labelList = dialog->findChild<QListWidget *>(QStringLiteral("labelEditList"));
        auto *labelCombo = dialog->findChild<QComboBox *>(QStringLiteral("labelEditCombo"));
        if (!labelList || !labelCombo || !labelCombo->lineEdit()) {
            dialog->reject();
            return;
        }
        labelList->setCurrentRow(0);
        QTest::keyClick(labelCombo->lineEdit(), Qt::Key_Down);
        selectedLabel = labelList->currentItem() ? labelList->currentItem()->text() : QString();
        inspected = true;
        dialog->reject();
    });
    editLabelAction->trigger();

    QVERIFY(inspected);
    QCOMPARE(selectedLabel, QStringLiteral("dog"));
}

void UiTests::mainWindowLabelEditCompleterAutocompletesPrefix() {
    resetTestSettings("label-edit-completer");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("a.jpg"));
    QVERIFY(image.save(imagePath));
    const QString classFilePath = dir.filePath(QStringLiteral("classes.txt"));
    QFile classFile(classFilePath);
    QVERIFY(classFile.open(QIODevice::WriteOnly | QIODevice::Text));
    classFile.write("cat\ndog\nperson\n");
    classFile.close();

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath, classFilePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *createAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QAction *editLabelAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+E")));
    QVERIFY(canvas);
    QVERIFY(createAction);
    QVERIFY(editLabelAction);

    acceptNextLabelPrompt(QStringLiteral("cat"));
    createAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(canvas, QPoint(60, 50));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 50));
    QCOMPARE(canvas->shapes().size(), 1);

    bool inspected = false;
    QString completion;
    QTimer::singleShot(50, [&]() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            return;
        }
        auto *combo = dialog->findChild<QComboBox *>(QStringLiteral("labelEditCombo"));
        if (!combo || !combo->lineEdit() || !combo->completer()) {
            dialog->reject();
            return;
        }
        combo->lineEdit()->clear();
        QTest::keyClick(combo->lineEdit(), Qt::Key_P);
        QCoreApplication::processEvents();
        completion = combo->completer()->currentCompletion();
        inspected = true;
        dialog->reject();
    });
    editLabelAction->trigger();

    QVERIFY(inspected);
    QCOMPARE(completion, QStringLiteral("person"));
}

void UiTests::mainWindowEditLabelDialogEditsLabelMeMetadata() {
    resetTestSettings("edit-labelme-metadata");
    QSettings settings;
    settings.setValue("labelHistory", QStringList{QStringLiteral("old"), QStringLiteral("updated")});
    settings.setValue("lastUsedLabel", QStringLiteral("old"));
    settings.setValue(QStringLiteral("labelme/labelFlags"), QStringLiteral("{.*: [occluded, reviewed]}"));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QListWidget *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    QAction *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QAction *editLabelAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+E")));
    QVERIFY(canvas);
    QVERIFY(labelList);
    QVERIFY(createModeAction);
    QVERIFY(editLabelAction);

    acceptNextLabelPrompt(QStringLiteral("old"));
    createModeAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(canvas, QPoint(60, 50));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 50));
    QCOMPARE(canvas->shapes().size(), 1);
    labelList->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);

    bool dialogInspected = false;
    bool metadataControlsFound = false;
    QTimer::singleShot(50, [&]() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            return;
        }
        dialogInspected = true;
        auto *labelCombo = dialog->findChild<QComboBox *>(QStringLiteral("labelEditCombo"));
        auto *groupIdSpin = dialog->findChild<QSpinBox *>(QStringLiteral("labelGroupIdSpin"));
        auto *descriptionEdit = dialog->findChild<QPlainTextEdit *>(QStringLiteral("labelDescriptionEdit"));
        auto *flagsEdit = dialog->findChild<QPlainTextEdit *>(QStringLiteral("labelFlagsEdit"));
        auto *flagsList = dialog->findChild<QListWidget *>(QStringLiteral("labelFlagsChecklist"));
        metadataControlsFound = dialog->objectName() == QStringLiteral("labelEditDialog") &&
                                labelCombo && groupIdSpin && descriptionEdit && flagsEdit && flagsList;
        if (!metadataControlsFound) {
            dialog->reject();
            return;
        }
        QCOMPARE(labelCombo->currentText(), QStringLiteral("old"));
        QCOMPARE(flagsList->count(), 2);
        labelCombo->setCurrentText(QStringLiteral("updated"));
        groupIdSpin->setValue(12);
        descriptionEdit->setPlainText(QString::fromUtf8("复检通过\n需要复核"));
        flagsEdit->setPlainText(QStringLiteral("occluded=true\nreviewed=false"));
        dialog->accept();
    });
    editLabelAction->trigger();

    QVERIFY(dialogInspected);
    QVERIFY(metadataControlsFound);
    const Shape shape = canvas->shapes().first();
    QCOMPARE(shape.label, QStringLiteral("updated"));
    QCOMPARE(shape.groupId, 12);
    QCOMPARE(shape.description, QString::fromUtf8("复检通过\n需要复核"));
    QCOMPARE(shape.flags.value(QStringLiteral("occluded")), true);
    QCOMPARE(shape.flags.value(QStringLiteral("reviewed")), false);
    QCOMPARE(labelList->item(0)->text(), QStringLiteral("updated (12) [occluded]"));
}

void UiTests::mainWindowEditLabelDialogAppliesCommonMetadataToMultipleShapes() {
    resetTestSettings("edit-labelme-multiple");
    QSettings settings;
    settings.setValue("labelHistory", QStringList{QStringLiteral("old"), QStringLiteral("updated")});
    settings.setValue("lastUsedLabel", QStringLiteral("old"));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("a.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *editLabelAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+E")));
    QVERIFY(canvas);
    QVERIFY(editLabelAction);

    Shape first = Shape::fromRect(QStringLiteral("old"), QRectF(10, 10, 20, 20), false);
    Shape second = Shape::fromRect(QStringLiteral("old"), QRectF(40, 10, 20, 20), false);
    first.groupId = 4;
    second.groupId = 4;
    first.description = QStringLiteral("before");
    second.description = QStringLiteral("before");
    canvas->setShapes({first, second});
    canvas->setSelectedIndices({0, 1});
    QApplication::processEvents();

    bool dialogInspected = false;
    QTimer::singleShot(50, [&]() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            return;
        }
        dialogInspected = true;
        auto *labelCombo = dialog->findChild<QComboBox *>(QStringLiteral("labelEditCombo"));
        auto *groupIdSpin = dialog->findChild<QSpinBox *>(QStringLiteral("labelGroupIdSpin"));
        auto *descriptionEdit = dialog->findChild<QPlainTextEdit *>(QStringLiteral("labelDescriptionEdit"));
        auto *flagsEdit = dialog->findChild<QPlainTextEdit *>(QStringLiteral("labelFlagsEdit"));
        QVERIFY(labelCombo);
        QVERIFY(groupIdSpin);
        QVERIFY(descriptionEdit);
        QVERIFY(flagsEdit);
        QVERIFY(labelCombo->isEnabled());
        QVERIFY(groupIdSpin->isEnabled());
        QVERIFY(descriptionEdit->isEnabled());
        labelCombo->setCurrentText(QStringLiteral("updated"));
        groupIdSpin->setValue(9);
        descriptionEdit->setPlainText(QString::fromUtf8("共同复检"));
        flagsEdit->setPlainText(QStringLiteral("reviewed=true"));
        dialog->accept();
    });
    editLabelAction->trigger();

    QVERIFY(dialogInspected);
    QCOMPARE(canvas->shapes().size(), 2);
    for (const Shape &shape : canvas->shapes()) {
        QCOMPARE(shape.label, QStringLiteral("updated"));
        QCOMPARE(shape.groupId, 9);
        QCOMPARE(shape.description, QString::fromUtf8("共同复检"));
        QCOMPARE(shape.flags.value(QStringLiteral("reviewed")), true);
    }
}

void UiTests::mainWindowMultiEditDisablesMixedLabelMeFields() {
    resetTestSettings("edit-labelme-mixed");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("a.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *editLabelAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+E")));
    QVERIFY(canvas);
    QVERIFY(editLabelAction);

    Shape first = Shape::fromRect(QStringLiteral("first"), QRectF(10, 10, 20, 20), false);
    Shape second = Shape::fromRect(QStringLiteral("second"), QRectF(40, 10, 20, 20), false);
    first.groupId = 1;
    second.groupId = 2;
    first.description = QStringLiteral("one");
    second.description = QStringLiteral("two");
    first.flags.insert(QStringLiteral("a"), true);
    second.flags.insert(QStringLiteral("b"), true);
    canvas->setShapes({first, second});
    canvas->setSelectedIndices({0, 1});
    QApplication::processEvents();

    bool dialogInspected = false;
    bool mixedLabelListDisabled = false;
    QTimer::singleShot(50, [&]() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            return;
        }
        dialogInspected = true;
        QVERIFY(!dialog->findChild<QComboBox *>(QStringLiteral("labelEditCombo"))->isEnabled());
        mixedLabelListDisabled = !dialog->findChild<QListWidget *>(QStringLiteral("labelEditList"))->isEnabled();
        QVERIFY(!dialog->findChild<QSpinBox *>(QStringLiteral("labelGroupIdSpin"))->isEnabled());
        QVERIFY(!dialog->findChild<QPlainTextEdit *>(QStringLiteral("labelDescriptionEdit"))->isEnabled());
        QVERIFY(!dialog->findChild<QPlainTextEdit *>(QStringLiteral("labelFlagsEdit"))->isEnabled());
        dialog->reject();
    });
    editLabelAction->trigger();

    QVERIFY(dialogInspected);
    QVERIFY(mixedLabelListDisabled);
    QCOMPARE(canvas->shapes().at(0).label, QStringLiteral("first"));
    QCOMPARE(canvas->shapes().at(1).label, QStringLiteral("second"));
    QCOMPARE(canvas->shapes().at(0).groupId, 1);
    QCOMPARE(canvas->shapes().at(1).groupId, 2);
}

void UiTests::mainWindowAppliesLabelMeLabelFlagPresetsWhenLabelChanges() {
    resetTestSettings("labelme-label-flag-presets");
    QSettings settings;
    settings.setValue("labelHistory", QStringList{QStringLiteral("old"), QStringLiteral("dog")});
    settings.setValue("lastUsedLabel", QStringLiteral("old"));
    settings.setValue("labelme/labelFlags", QStringLiteral("dog=occluded,truncated\nperson.*=male,tall"));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QVERIFY(canvas);
    QVERIFY(createModeAction);

    bool promptInspected = false;
    bool initialFlagsEmpty = false;
    bool presetFlagsShown = false;
    QTimer::singleShot(50, [&]() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            return;
        }
        auto *labelCombo = dialog->findChild<QComboBox *>(QStringLiteral("labelEditCombo"));
        auto *flagsEdit = dialog->findChild<QPlainTextEdit *>(QStringLiteral("labelFlagsEdit"));
        if (!labelCombo || !flagsEdit) {
            dialog->reject();
            return;
        }
        initialFlagsEmpty = flagsEdit->toPlainText().trimmed().isEmpty();
        labelCombo->setCurrentText(QStringLiteral("dog"));
        const QString flagText = flagsEdit->toPlainText();
        presetFlagsShown = flagText.contains(QStringLiteral("occluded=false")) &&
                           flagText.contains(QStringLiteral("truncated=false"));
        promptInspected = true;
        dialog->accept();
    });

    createModeAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(canvas, QPoint(60, 50));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 50));

    QVERIFY(promptInspected);
    QVERIFY(initialFlagsEmpty);
    QVERIFY(presetFlagsShown);
    QCOMPARE(canvas->shapes().size(), 1);
    const Shape shape = canvas->shapes().first();
    QCOMPARE(shape.label, QStringLiteral("dog"));
    QVERIFY(shape.flags.contains(QStringLiteral("occluded")));
    QVERIFY(shape.flags.contains(QStringLiteral("truncated")));
    QCOMPARE(shape.flags.value(QStringLiteral("occluded")), false);
    QCOMPARE(shape.flags.value(QStringLiteral("truncated")), false);
}

void UiTests::mainWindowParsesLabelMeYamlInlineLabelFlagPresets() {
    resetTestSettings("labelme-yaml-inline-label-flag-presets");
    QSettings settings;
    settings.setValue("labelHistory", QStringList{QStringLiteral("old"), QStringLiteral("dog")});
    settings.setValue("lastUsedLabel", QStringLiteral("old"));
    settings.setValue("labelme/labelFlags", QStringLiteral("{dog: [occluded, truncated], person.*: [male, tall]}"));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QVERIFY(canvas);
    QVERIFY(createModeAction);

    bool promptInspected = false;
    bool presetFlagsShown = false;
    QTimer::singleShot(50, [&]() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            return;
        }
        auto *labelCombo = dialog->findChild<QComboBox *>(QStringLiteral("labelEditCombo"));
        auto *flagsEdit = dialog->findChild<QPlainTextEdit *>(QStringLiteral("labelFlagsEdit"));
        if (!labelCombo || !flagsEdit) {
            dialog->reject();
            return;
        }
        labelCombo->setCurrentText(QStringLiteral("dog"));
        const QString flagText = flagsEdit->toPlainText();
        presetFlagsShown = flagText.contains(QStringLiteral("occluded=false")) &&
                           flagText.contains(QStringLiteral("truncated=false")) &&
                           !flagText.contains(QStringLiteral("male=false"));
        promptInspected = true;
        dialog->accept();
    });

    createModeAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(canvas, QPoint(60, 50));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 50));

    QVERIFY(promptInspected);
    QVERIFY(presetFlagsShown);
    QCOMPARE(canvas->shapes().size(), 1);
    const Shape shape = canvas->shapes().first();
    QCOMPARE(shape.label, QStringLiteral("dog"));
    QVERIFY(shape.flags.contains(QStringLiteral("occluded")));
    QVERIFY(shape.flags.contains(QStringLiteral("truncated")));
    QVERIFY(!shape.flags.contains(QStringLiteral("male")));
}

void UiTests::mainWindowLoadsLabelMeLabelFlagPresetsFromFilePath() {
    resetTestSettings("labelme-label-flag-presets-file-path");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString flagsPath = dir.filePath(QStringLiteral("label_flags.yaml"));
    QFile flagsFile(flagsPath);
    QVERIFY(flagsFile.open(QIODevice::WriteOnly | QIODevice::Text));
    flagsFile.write("{dog: [occluded, truncated], person.*: [male, tall]}");
    flagsFile.close();

    QSettings settings;
    settings.setValue("labelHistory", QStringList{QStringLiteral("old"), QStringLiteral("dog")});
    settings.setValue("lastUsedLabel", QStringLiteral("old"));
    settings.setValue("labelme/labelFlags", flagsPath);

    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QVERIFY(canvas);
    QVERIFY(createModeAction);

    bool promptInspected = false;
    bool presetFlagsShown = false;
    QTimer::singleShot(50, [&]() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            return;
        }
        auto *labelCombo = dialog->findChild<QComboBox *>(QStringLiteral("labelEditCombo"));
        auto *flagsEdit = dialog->findChild<QPlainTextEdit *>(QStringLiteral("labelFlagsEdit"));
        if (!labelCombo || !flagsEdit) {
            dialog->reject();
            return;
        }
        labelCombo->setCurrentText(QStringLiteral("dog"));
        const QString flagText = flagsEdit->toPlainText();
        presetFlagsShown = flagText.contains(QStringLiteral("occluded=false")) &&
                           flagText.contains(QStringLiteral("truncated=false")) &&
                           !flagText.contains(QStringLiteral("male=false"));
        promptInspected = true;
        dialog->accept();
    });

    createModeAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(canvas, QPoint(60, 50));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 50));

    QVERIFY(promptInspected);
    QVERIFY(presetFlagsShown);
    QCOMPARE(canvas->shapes().size(), 1);
    const Shape shape = canvas->shapes().first();
    QCOMPARE(shape.label, QStringLiteral("dog"));
    QVERIFY(shape.flags.contains(QStringLiteral("occluded")));
    QVERIFY(shape.flags.contains(QStringLiteral("truncated")));
    QVERIFY(!shape.flags.contains(QStringLiteral("male")));
}

void UiTests::mainWindowEditsLabelMeLabelFlagPresetsDialog() {
    resetTestSettings("labelme-label-flag-presets-dialog");
    QSettings settings;
    settings.setValue("labelHistory", QStringList{QStringLiteral("old"), QStringLiteral("dog")});
    settings.setValue("lastUsedLabel", QStringLiteral("old"));
    settings.setValue("labelme/labelFlags", QStringLiteral("cat=old_flag"));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QAction *editLabelFlagsAction = window.findChild<QAction *>(QStringLiteral("editLabelFlagsAction"));
    Canvas *canvas = window.findChild<Canvas *>();
    QAction *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QVERIFY(editLabelFlagsAction);
    QVERIFY(canvas);
    QVERIFY(createModeAction);

    bool configDialogSeen = false;
    QTimer::singleShot(50, [&]() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            return;
        }
        auto *edit = dialog->findChild<QPlainTextEdit *>(QStringLiteral("labelFlagsConfigEdit"));
        if (!edit) {
            dialog->reject();
            return;
        }
        QCOMPARE(edit->toPlainText(), QStringLiteral("cat=old_flag"));
        edit->setPlainText(QStringLiteral("{dog: [occluded, truncated]}"));
        configDialogSeen = true;
        dialog->accept();
    });
    editLabelFlagsAction->trigger();

    QVERIFY(configDialogSeen);
    QCOMPARE(settings.value(QStringLiteral("labelme/labelFlags")).toString(), QStringLiteral("{dog: [occluded, truncated]}"));

    bool labelDialogSeen = false;
    bool presetFlagsShown = false;
    QTimer::singleShot(50, [&]() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            return;
        }
        auto *labelCombo = dialog->findChild<QComboBox *>(QStringLiteral("labelEditCombo"));
        auto *flagsEdit = dialog->findChild<QPlainTextEdit *>(QStringLiteral("labelFlagsEdit"));
        if (!labelCombo || !flagsEdit) {
            dialog->reject();
            return;
        }
        labelCombo->setCurrentText(QStringLiteral("dog"));
        const QString flagText = flagsEdit->toPlainText();
        presetFlagsShown = flagText.contains(QStringLiteral("occluded=false")) &&
                           flagText.contains(QStringLiteral("truncated=false"));
        labelDialogSeen = true;
        dialog->accept();
    });

    createModeAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(canvas, QPoint(60, 50));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 50));

    QVERIFY(labelDialogSeen);
    QVERIFY(presetFlagsShown);
    QCOMPARE(canvas->shapes().size(), 1);
    const Shape shape = canvas->shapes().first();
    QCOMPARE(shape.label, QStringLiteral("dog"));
    QVERIFY(shape.flags.contains(QStringLiteral("occluded")));
    QVERIFY(shape.flags.contains(QStringLiteral("truncated")));
}

void UiTests::mainWindowEditsExternalLabelMeLabelFlagPresetFile() {
    resetTestSettings("labelme-label-flag-presets-external-edit");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString flagsPath = dir.filePath(QStringLiteral("label_flags.yaml"));
    QFile flagsFile(flagsPath);
    QVERIFY(flagsFile.open(QIODevice::WriteOnly | QIODevice::Text));
    flagsFile.write("cat=old_flag");
    flagsFile.close();

    QSettings settings;
    settings.setValue("labelHistory", QStringList{QStringLiteral("old"), QStringLiteral("dog")});
    settings.setValue("lastUsedLabel", QStringLiteral("old"));
    settings.setValue("labelme/labelFlags", flagsPath);

    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QAction *editLabelFlagsAction = window.findChild<QAction *>(QStringLiteral("editLabelFlagsAction"));
    Canvas *canvas = window.findChild<Canvas *>();
    QAction *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QVERIFY(editLabelFlagsAction);
    QVERIFY(canvas);
    QVERIFY(createModeAction);

    bool configDialogSeen = false;
    QTimer::singleShot(50, [&]() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            return;
        }
        auto *edit = dialog->findChild<QPlainTextEdit *>(QStringLiteral("labelFlagsConfigEdit"));
        if (!edit) {
            dialog->reject();
            return;
        }
        QCOMPARE(edit->toPlainText(), QStringLiteral("cat=old_flag"));
        edit->setPlainText(QStringLiteral("{dog: [occluded, truncated]}"));
        configDialogSeen = true;
        dialog->accept();
    });
    editLabelFlagsAction->trigger();

    QVERIFY(configDialogSeen);
    QCOMPARE(settings.value(QStringLiteral("labelme/labelFlags")).toString(), flagsPath);
    QFile updatedFlagsFile(flagsPath);
    QVERIFY(updatedFlagsFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QCOMPARE(QString::fromUtf8(updatedFlagsFile.readAll()), QStringLiteral("{dog: [occluded, truncated]}"));

    bool labelDialogSeen = false;
    bool presetFlagsShown = false;
    QTimer::singleShot(50, [&]() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            return;
        }
        auto *labelCombo = dialog->findChild<QComboBox *>(QStringLiteral("labelEditCombo"));
        auto *flagsEdit = dialog->findChild<QPlainTextEdit *>(QStringLiteral("labelFlagsEdit"));
        if (!labelCombo || !flagsEdit) {
            dialog->reject();
            return;
        }
        labelCombo->setCurrentText(QStringLiteral("dog"));
        const QString flagText = flagsEdit->toPlainText();
        presetFlagsShown = flagText.contains(QStringLiteral("occluded=false")) &&
                           flagText.contains(QStringLiteral("truncated=false"));
        labelDialogSeen = true;
        dialog->accept();
    });

    createModeAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(canvas, QPoint(60, 50));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 50));

    QVERIFY(labelDialogSeen);
    QVERIFY(presetFlagsShown);
    QCOMPARE(canvas->shapes().size(), 1);
    const Shape shape = canvas->shapes().first();
    QCOMPARE(shape.label, QStringLiteral("dog"));
    QVERIFY(shape.flags.contains(QStringLiteral("occluded")));
    QVERIFY(shape.flags.contains(QStringLiteral("truncated")));
}

void UiTests::mainWindowPreservesLabelMeImageDataOnSave() {
    resetTestSettings("labelme-image-data-mainwindow-save");
    QSettings settings;
    settings.setValue("labelFileFormat", 3);
    settings.setValue("labelme/embedImageData", true);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(64, 48, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath("a.jpg");
    QVERIFY(image.save(imagePath));

    QFile imageFile(imagePath);
    QVERIFY(imageFile.open(QIODevice::ReadOnly));
    const QString imagePayload = QString::fromLatin1(imageFile.readAll().toBase64());
    QJsonObject shape;
    shape["label"] = QStringLiteral("defect");
    shape["shape_type"] = QStringLiteral("rectangle");
    QJsonArray points;
    points.append(QJsonArray{10.0, 12.0});
    points.append(QJsonArray{30.0, 28.0});
    shape["points"] = points;

    QJsonObject root;
    root["version"] = QStringLiteral("5.0.0");
    root["flags"] = QJsonObject();
    root["shapes"] = QJsonArray{shape};
    root["imagePath"] = QStringLiteral("a.jpg");
    root["imageData"] = imagePayload;
    root["imageHeight"] = image.height();
    root["imageWidth"] = image.width();

    QFile jsonFile(dir.filePath("a.json"));
    QVERIFY(jsonFile.open(QIODevice::WriteOnly));
    jsonFile.write(QJsonDocument(root).toJson());
    jsonFile.close();

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QVERIFY(QMetaObject::invokeMethod(&window, "saveFile", Qt::DirectConnection));

    QFile savedFile(dir.filePath("a.json"));
    QVERIFY(savedFile.open(QIODevice::ReadOnly));
    const QJsonObject savedRoot = QJsonDocument::fromJson(savedFile.readAll()).object();
    QCOMPARE(savedRoot.value(QStringLiteral("imageData")).toString(), imagePayload);
}

void UiTests::mainWindowDropsLabelMeImageDataWhenSaveWithImageDataDisabled() {
    resetTestSettings("labelme-image-data-disabled-mainwindow-save");
    QSettings settings;
    settings.setValue("labelFileFormat", 3);
    settings.setValue("labelme/embedImageData", false);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(32, 24, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath("a.jpg");
    QVERIFY(image.save(imagePath));
    QFile imageFile(imagePath);
    QVERIFY(imageFile.open(QIODevice::ReadOnly));
    const QString imagePayload = QString::fromLatin1(imageFile.readAll().toBase64());

    QJsonObject shape;
    shape["label"] = QStringLiteral("defect");
    shape["shape_type"] = QStringLiteral("rectangle");
    shape["points"] = QJsonArray{QJsonArray{2.0, 3.0}, QJsonArray{15.0, 18.0}};
    QJsonObject root;
    root["version"] = QStringLiteral("5.0.0");
    root["flags"] = QJsonObject();
    root["shapes"] = QJsonArray{shape};
    root["imagePath"] = QStringLiteral("a.jpg");
    root["imageData"] = imagePayload;
    root["imageHeight"] = image.height();
    root["imageWidth"] = image.width();

    QFile jsonFile(dir.filePath("a.json"));
    QVERIFY(jsonFile.open(QIODevice::WriteOnly));
    jsonFile.write(QJsonDocument(root).toJson());
    jsonFile.close();

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QVERIFY(QMetaObject::invokeMethod(&window, "saveFile", Qt::DirectConnection));

    QFile savedFile(dir.filePath("a.json"));
    QVERIFY(savedFile.open(QIODevice::ReadOnly));
    const QJsonObject savedRoot = QJsonDocument::fromJson(savedFile.readAll()).object();
    QVERIFY(savedRoot.value(QStringLiteral("imageData")).isNull());
}

void UiTests::mainWindowPreservesLabelMeTopLevelFlagsOnSave() {
    resetTestSettings("labelme-top-level-flags-mainwindow-save");
    QSettings settings;
    settings.setValue("labelFileFormat", 3);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(64, 48, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    QJsonObject flags;
    flags["verified"] = true;
    flags["needs_review"] = true;
    flags["accepted"] = false;

    QJsonObject root;
    root["version"] = QStringLiteral("5.0.0");
    root["flags"] = flags;
    root["shapes"] = QJsonArray();
    root["imagePath"] = QStringLiteral("a.jpg");
    root["imageData"] = QJsonValue::Null;
    root["imageHeight"] = image.height();
    root["imageWidth"] = image.width();

    QFile jsonFile(dir.filePath("a.json"));
    QVERIFY(jsonFile.open(QIODevice::WriteOnly));
    jsonFile.write(QJsonDocument(root).toJson());
    jsonFile.close();

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QVERIFY(QMetaObject::invokeMethod(&window, "saveFile", Qt::DirectConnection));

    QFile savedFile(dir.filePath("a.json"));
    QVERIFY(savedFile.open(QIODevice::ReadOnly));
    const QJsonObject savedFlags = QJsonDocument::fromJson(savedFile.readAll()).object()
                                     .value(QStringLiteral("flags")).toObject();
    QCOMPARE(savedFlags.value(QStringLiteral("verified")).toBool(), true);
    QCOMPARE(savedFlags.value(QStringLiteral("needs_review")).toBool(), true);
    QCOMPARE(savedFlags.value(QStringLiteral("accepted")).toBool(), false);
}

void UiTests::mainWindowEditsLabelMeTopLevelFlagsPanel() {
    resetTestSettings("labelme-top-level-flags-panel");
    QSettings settings;
    settings.setValue("labelFileFormat", 3);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(64, 48, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    QJsonObject root;
    root["version"] = QStringLiteral("5.0.0");
    root["flags"] = QJsonObject{{QStringLiteral("verified"), true},
                                  {QStringLiteral("needs_review"), true},
                                  {QStringLiteral("accepted"), false}};
    root["shapes"] = QJsonArray();
    root["imagePath"] = QStringLiteral("a.jpg");
    root["imageData"] = QJsonValue::Null;
    root["imageHeight"] = image.height();
    root["imageWidth"] = image.width();

    QFile jsonFile(dir.filePath("a.json"));
    QVERIFY(jsonFile.open(QIODevice::WriteOnly));
    jsonFile.write(QJsonDocument(root).toJson());
    jsonFile.close();

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *flagsEdit = window.findChild<QPlainTextEdit *>(QStringLiteral("topLevelFlagsEdit"));
    QVERIFY(flagsEdit);
    QVERIFY(flagsEdit->toPlainText().contains(QStringLiteral("needs_review=true")));
    QVERIFY(flagsEdit->toPlainText().contains(QStringLiteral("accepted=false")));
    QVERIFY(!flagsEdit->toPlainText().contains(QStringLiteral("verified")));

    flagsEdit->setPlainText(QStringLiteral("needs_review=false\naccepted=true\nexported=true"));
    QVERIFY(QMetaObject::invokeMethod(&window, "saveFile", Qt::DirectConnection));

    QFile savedFile(dir.filePath("a.json"));
    QVERIFY(savedFile.open(QIODevice::ReadOnly));
    const QJsonObject savedFlags = QJsonDocument::fromJson(savedFile.readAll()).object()
                                     .value(QStringLiteral("flags")).toObject();
    QCOMPARE(savedFlags.value(QStringLiteral("verified")).toBool(), true);
    QCOMPARE(savedFlags.value(QStringLiteral("needs_review")).toBool(), false);
    QCOMPARE(savedFlags.value(QStringLiteral("accepted")).toBool(), true);
    QCOMPARE(savedFlags.value(QStringLiteral("exported")).toBool(), true);
}

void UiTests::mainWindowProvidesLabelMeTopLevelFlagsDock() {
    resetTestSettings("labelme-top-level-flags-dock");
    QSettings settings;
    settings.setValue("labelFileFormat", 3);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(64, 48, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    QJsonObject root;
    root[QStringLiteral("version")] = QStringLiteral("5.0.0");
    root[QStringLiteral("flags")] = QJsonObject{{QStringLiteral("needs_review"), true},
                                                 {QStringLiteral("accepted"), false}};
    root[QStringLiteral("shapes")] = QJsonArray();
    root[QStringLiteral("imagePath")] = QStringLiteral("a.jpg");
    root[QStringLiteral("imageData")] = QJsonValue::Null;
    root[QStringLiteral("imageHeight")] = image.height();
    root[QStringLiteral("imageWidth")] = image.width();

    QFile jsonFile(dir.filePath("a.json"));
    QVERIFY(jsonFile.open(QIODevice::WriteOnly));
    jsonFile.write(QJsonDocument(root).toJson());
    jsonFile.close();

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QDockWidget *flagDock = window.findChild<QDockWidget *>(QStringLiteral("flags"));
    QListWidget *flagList = window.findChild<QListWidget *>(QStringLiteral("flagList"));
    QVERIFY(flagDock);
    QVERIFY(flagList);
    QCOMPARE(flagList->count(), 2);

    QListWidgetItem *needsReview = nullptr;
    QListWidgetItem *accepted = nullptr;
    for (int i = 0; i < flagList->count(); ++i) {
        if (flagList->item(i)->text() == QStringLiteral("needs_review")) {
            needsReview = flagList->item(i);
        } else if (flagList->item(i)->text() == QStringLiteral("accepted")) {
            accepted = flagList->item(i);
        }
    }
    QVERIFY(needsReview);
    QVERIFY(accepted);
    QCOMPARE(needsReview->checkState(), Qt::Checked);
    QCOMPARE(accepted->checkState(), Qt::Unchecked);

    needsReview->setCheckState(Qt::Unchecked);
    accepted->setCheckState(Qt::Checked);
    QVERIFY(QMetaObject::invokeMethod(&window, "saveFile", Qt::DirectConnection));

    QFile savedFile(dir.filePath("a.json"));
    QVERIFY(savedFile.open(QIODevice::ReadOnly));
    const QJsonObject savedFlags = QJsonDocument::fromJson(savedFile.readAll()).object()
                                     .value(QStringLiteral("flags")).toObject();
    QCOMPARE(savedFlags.value(QStringLiteral("needs_review")).toBool(), false);
    QCOMPARE(savedFlags.value(QStringLiteral("accepted")).toBool(), true);
}

void UiTests::mainWindowLabelListShowsGroupAndEnabledFlags() {
    resetTestSettings("label-list-shows-shape-metadata");
    QSettings settings;
    settings.setValue("labelFileFormat", 3);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(64, 48, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    QJsonObject shape;
    shape[QStringLiteral("label")] = QStringLiteral("defect");
    shape[QStringLiteral("points")] = QJsonArray{QJsonArray{10, 10}, QJsonArray{30, 30}};
    shape[QStringLiteral("group_id")] = 7;
    shape[QStringLiteral("description")] = QStringLiteral("native");
    shape[QStringLiteral("shape_type")] = QStringLiteral("rectangle");
    shape[QStringLiteral("flags")] = QJsonObject{{QStringLiteral("occluded"), true},
                                                  {QStringLiteral("reviewed"), false}};

    QJsonObject root;
    root[QStringLiteral("version")] = QStringLiteral("5.0.0");
    root[QStringLiteral("flags")] = QJsonObject();
    root[QStringLiteral("shapes")] = QJsonArray{shape};
    root[QStringLiteral("imagePath")] = QStringLiteral("a.jpg");
    root[QStringLiteral("imageData")] = QJsonValue::Null;
    root[QStringLiteral("imageHeight")] = image.height();
    root[QStringLiteral("imageWidth")] = image.width();

    QFile jsonFile(dir.filePath("a.json"));
    QVERIFY(jsonFile.open(QIODevice::WriteOnly));
    jsonFile.write(QJsonDocument(root).toJson());
    jsonFile.close();

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    QListWidget *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    Canvas *canvas = window.findChild<Canvas *>();
    QVERIFY(labelList);
    QVERIFY(canvas);
    QCOMPARE(labelList->count(), 1);
    QVERIFY(labelList->item(0)->text().contains(QStringLiteral("defect")));
    QVERIFY(labelList->item(0)->text().contains(QStringLiteral("(7)")));
    QVERIFY(labelList->item(0)->text().contains(QStringLiteral("[occluded]")));
    QCOMPARE(labelList->item(0)->data(Qt::UserRole).toString(), QStringLiteral("defect"));
    QVERIFY(!labelList->item(0)->icon().isNull());
    QCOMPARE(labelList->item(0)->background().style(), Qt::NoBrush);

    labelList->item(0)->setCheckState(Qt::Unchecked);
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("defect"));
    QVERIFY(!canvas->shapes().first().visible);
}

void UiTests::mainWindowLoadsAnnotationWithoutSelectingShape() {
    resetTestSettings("load-annotation-without-selection");
    QSettings settings;
    settings.setValue(QStringLiteral("labelFileFormat"), 3);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("a.jpg"));
    QVERIFY(image.save(imagePath));

    AnnotationDocument document;
    document.imagePath = imagePath;
    document.imageSize = image.size();
    document.shapes = {
        Shape::fromRect(QStringLiteral("first"), QRectF(5, 5, 15, 12), false),
        Shape::fromRect(QStringLiteral("second"), QRectF(35, 25, 18, 14), false),
    };
    const QString annotationPath = dir.filePath(QStringLiteral("a.json"));
    QVERIFY(AnnotationIO::saveLabelMe(annotationPath, document));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *canvas = window.findChild<Canvas *>();
    auto *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    QVERIFY(canvas);
    QVERIFY(labelList);
    QCOMPARE(canvas->shapes().size(), 2);
    QVERIFY(canvas->selectedIndices().isEmpty());
    QCOMPARE(canvas->currentIndex(), -1);
    QVERIFY(labelList->selectedItems().isEmpty());
}

void UiTests::mainWindowLabelListKeepsMultiSelectionWhenTogglingVisibility() {
    resetTestSettings("label-list-multiselection-visibility");
    QSettings settings;
    settings.setValue(QStringLiteral("labelFileFormat"), 3);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("multi.jpg"));
    QVERIFY(image.save(imagePath));

    AnnotationDocument document;
    document.imagePath = QStringLiteral("multi.jpg");
    document.imageSize = image.size();
    document.shapes = {
        Shape::fromRect(QStringLiteral("first"), QRectF(5, 5, 15, 12), false),
        Shape::fromRect(QStringLiteral("second"), QRectF(30, 10, 15, 12), false),
        Shape::fromRect(QStringLiteral("third"), QRectF(55, 15, 15, 12), false),
    };
    const QString annotationPath = dir.filePath(QStringLiteral("multi.json"));
    QVERIFY(AnnotationIO::saveLabelMe(annotationPath, document));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    auto *canvas = window.findChild<Canvas *>();
    QVERIFY(labelList);
    QVERIFY(canvas);
    QCOMPARE(labelList->count(), 3);

    labelList->selectionModel()->select(labelList->model()->index(0, 0),
                                        QItemSelectionModel::ClearAndSelect);
    labelList->selectionModel()->select(labelList->model()->index(1, 0),
                                        QItemSelectionModel::Select);
    QCOMPARE(labelList->selectedItems().size(), 2);

    const QRect secondRect = labelList->visualItemRect(labelList->item(1));
    QVERIFY(secondRect.isValid());
    QStyleOptionViewItem option;
    option.initFrom(labelList);
    option.rect = secondRect;
    option.features = QStyleOptionViewItem::HasCheckIndicator;
    option.checkState = Qt::Checked;
    const QRect checkRect = labelList->style()->subElementRect(
        QStyle::SE_ItemViewItemCheckIndicator, &option, labelList);
    QVERIFY(checkRect.isValid());
    QTest::mouseClick(labelList->viewport(), Qt::LeftButton, Qt::NoModifier,
                      checkRect.center());

    QCOMPARE(labelList->item(1)->checkState(), Qt::Unchecked);
    QCOMPARE(labelList->selectedItems().size(), 2);
    QCOMPARE(canvas->selectedIndices(), QVector<int>({0, 1}));
    QVERIFY(!canvas->shapes().at(0).visible);
    QVERIFY(!canvas->shapes().at(1).visible);
}

void UiTests::mainWindowDifficultCheckboxSynchronizesLabelMeFlag() {
    resetTestSettings("labelme-difficult-checkbox-sync");
    QSettings settings;
    settings.setValue(QStringLiteral("labelFileFormat"), 3);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(64, 48, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("difficult.jpg"));
    QVERIFY(image.save(imagePath));

    const QJsonObject shape{
        {QStringLiteral("label"), QStringLiteral("defect")},
        {QStringLiteral("points"), QJsonArray{QJsonArray{8, 7}, QJsonArray{28, 24}}},
        {QStringLiteral("group_id"), QJsonValue::Null},
        {QStringLiteral("description"), QString()},
        {QStringLiteral("shape_type"), QStringLiteral("rectangle")},
        {QStringLiteral("flags"), QJsonObject{{QStringLiteral("difficult"), true}}},
    };
    const QJsonObject root{
        {QStringLiteral("version"), QStringLiteral("5.7.0")},
        {QStringLiteral("flags"), QJsonObject{}},
        {QStringLiteral("shapes"), QJsonArray{shape}},
        {QStringLiteral("imagePath"), QStringLiteral("difficult.jpg")},
        {QStringLiteral("imageData"), QJsonValue::Null},
        {QStringLiteral("imageHeight"), image.height()},
        {QStringLiteral("imageWidth"), image.width()},
    };
    QFile jsonFile(dir.filePath(QStringLiteral("difficult.json")));
    QVERIFY(jsonFile.open(QIODevice::WriteOnly));
    jsonFile.write(QJsonDocument(root).toJson());
    jsonFile.close();

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *canvas = window.findChild<Canvas *>();
    auto *difficult = window.findChild<QCheckBox *>(QStringLiteral("difficultCheckBox"));
    QVERIFY(canvas);
    QVERIFY(difficult);
    QCOMPARE(canvas->shapes().size(), 1);
    QVERIFY(canvas->shapes().first().difficult);
    canvas->setCurrentIndex(0);
    QCoreApplication::processEvents();
    QVERIFY(difficult->isChecked());

    difficult->setChecked(false);
    QVERIFY(!canvas->shapes().first().difficult);
    QVERIFY(canvas->shapes().first().flags.contains(QStringLiteral("difficult")));
    QVERIFY(!canvas->shapes().first().flags.value(QStringLiteral("difficult")));

    QVERIFY(QMetaObject::invokeMethod(&window, "saveFile", Qt::DirectConnection));
    QFile savedFile(dir.filePath(QStringLiteral("difficult.json")));
    QVERIFY(savedFile.open(QIODevice::ReadOnly));
    const QJsonObject savedShape = QJsonDocument::fromJson(savedFile.readAll()).object()
                                       .value(QStringLiteral("shapes")).toArray().first().toObject();
    QVERIFY(savedShape.value(QStringLiteral("flags")).toObject().contains(QStringLiteral("difficult")));
    QCOMPARE(savedShape.value(QStringLiteral("flags")).toObject()
                 .value(QStringLiteral("difficult")).toBool(), false);
}

void UiTests::mainWindowDifficultCheckboxUsesCanvasSelection() {
    resetTestSettings("labelme-difficult-checkbox-canvas-selection");
    QSettings settings;
    settings.setValue("labelFileFormat", 3);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("selection.jpg"));
    QVERIFY(image.save(imagePath));

    AnnotationDocument document;
    document.imagePath = QStringLiteral("selection.jpg");
    document.imageSize = image.size();
    document.shapes = {
        Shape::fromRect(QStringLiteral("canvas-selected"), QRectF(5, 5, 18, 14), false),
        Shape::fromRect(QStringLiteral("stale-list-row"), QRectF(35, 25, 18, 14), false),
    };
    QVERIFY(AnnotationIO::saveLabelMe(dir.filePath(QStringLiteral("selection.json")), document));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *canvas = window.findChild<Canvas *>();
    auto *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    auto *difficult = window.findChild<QCheckBox *>(QStringLiteral("difficultCheckBox"));
    QVERIFY(canvas);
    QVERIFY(labelList);
    QVERIFY(difficult);
    QCOMPARE(canvas->shapes().size(), 2);

    canvas->setCurrentIndex(0);
    QApplication::processEvents();
    {
        QSignalBlocker blocker(labelList);
        labelList->setCurrentRow(1, QItemSelectionModel::ClearAndSelect);
    }
    QCOMPARE(canvas->currentIndex(), 0);
    QCOMPARE(labelList->currentRow(), 1);

    difficult->setChecked(true);
    QVERIFY(canvas->shapes().at(0).difficult);
    QVERIFY(!canvas->shapes().at(1).difficult);
}

void UiTests::mainWindowCanvasSelectionScrollsLabelList() {
    resetTestSettings("labelme-canvas-selection-scrolls-label-list");
    QSettings settings;
    settings.setValue("labelFileFormat", 3);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(120, 120, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("scroll.jpg"));
    QVERIFY(image.save(imagePath));

    AnnotationDocument document;
    document.imagePath = QStringLiteral("scroll.jpg");
    document.imageSize = image.size();
    for (int i = 0; i < 40; ++i) {
        document.shapes.push_back(Shape::fromRect(
            QStringLiteral("label-%1").arg(i),
            QRectF(2 + (i % 8) * 14, 2 + (i / 8) * 22, 8, 8),
            false));
    }
    QVERIFY(AnnotationIO::saveLabelMe(dir.filePath(QStringLiteral("scroll.json")), document));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *canvas = window.findChild<Canvas *>();
    auto *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    QVERIFY(canvas);
    QVERIFY(labelList);
    QCOMPARE(labelList->count(), 40);
    QVERIFY(labelList->verticalScrollBar()->maximum() > 0);

    labelList->scrollToItem(labelList->item(labelList->count() - 1),
                            QAbstractItemView::PositionAtBottom);
    QApplication::processEvents();
    QVERIFY(labelList->verticalScrollBar()->value() > 0);

    canvas->setCurrentIndex(0);
    QApplication::processEvents();
    const QRect firstRect = labelList->visualItemRect(labelList->item(0));
    QVERIFY2(firstRect.intersects(labelList->viewport()->rect()),
             qPrintable(QStringLiteral("first label is not visible: %1").arg(firstRect.top())));
}

void UiTests::mainWindowPreservesLabelMeVersionAndImagePathOnSave() {
    resetTestSettings("labelme-version-imagepath-mainwindow-save");
    QSettings settings;
    settings.setValue("labelFileFormat", 3);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(64, 48, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));
    QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("images")));
    QVERIFY(image.save(dir.filePath("images/a.jpg")));

    QJsonObject root;
    root["version"] = QStringLiteral("5.7.0");
    root["flags"] = QJsonObject();
    root["shapes"] = QJsonArray();
    root["imagePath"] = QStringLiteral("images/a.jpg");
    root["imageData"] = QJsonValue::Null;
    root["imageHeight"] = image.height();
    root["imageWidth"] = image.width();

    QFile jsonFile(dir.filePath("a.json"));
    QVERIFY(jsonFile.open(QIODevice::WriteOnly));
    jsonFile.write(QJsonDocument(root).toJson());
    jsonFile.close();

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QVERIFY(QMetaObject::invokeMethod(&window, "saveFile", Qt::DirectConnection));

    QFile savedFile(dir.filePath("a.json"));
    QVERIFY(savedFile.open(QIODevice::ReadOnly));
    const QJsonObject savedRoot = QJsonDocument::fromJson(savedFile.readAll()).object();
    QCOMPARE(savedRoot.value(QStringLiteral("version")).toString(), QStringLiteral("5.7.0"));
    QCOMPARE(savedRoot.value(QStringLiteral("imagePath")).toString(), QStringLiteral("images/a.jpg"));
}

void UiTests::mainWindowPreservesLabelMeTopLevelOtherDataOnSave() {
    resetTestSettings("labelme-other-data-mainwindow-save");
    QSettings settings;
    settings.setValue("labelFileFormat", 3);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(64, 48, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    QJsonObject root;
    root["version"] = QStringLiteral("5.7.0");
    root["flags"] = QJsonObject();
    root["shapes"] = QJsonArray();
    root["imagePath"] = QStringLiteral("a.jpg");
    root["imageData"] = QJsonValue::Null;
    root["imageHeight"] = image.height();
    root["imageWidth"] = image.width();
    root["metadata"] = QJsonObject{{QStringLiteral("source"), QStringLiteral("line-7")},
                                    {QStringLiteral("reviewed"), false}};
    root["confidence"] = 0.82;

    QFile jsonFile(dir.filePath("a.json"));
    QVERIFY(jsonFile.open(QIODevice::WriteOnly));
    jsonFile.write(QJsonDocument(root).toJson());
    jsonFile.close();

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QVERIFY(QMetaObject::invokeMethod(&window, "saveFile", Qt::DirectConnection));

    QFile savedFile(dir.filePath("a.json"));
    QVERIFY(savedFile.open(QIODevice::ReadOnly));
    const QJsonObject savedRoot = QJsonDocument::fromJson(savedFile.readAll()).object();
    QCOMPARE(savedRoot.value(QStringLiteral("metadata")).toObject().value(QStringLiteral("source")).toString(), QStringLiteral("line-7"));
    QCOMPARE(savedRoot.value(QStringLiteral("metadata")).toObject().value(QStringLiteral("reviewed")).toBool(), false);
    QCOMPARE(savedRoot.value(QStringLiteral("confidence")).toDouble(), 0.82);
}

void UiTests::mainWindowPreservesLabelMeShapeOtherDataOnSave() {
    resetTestSettings("labelme-shape-other-data-mainwindow-save");
    QSettings settings;
    settings.setValue("labelFileFormat", 3);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(64, 48, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    QJsonObject shape;
    shape["label"] = QStringLiteral("defect");
    shape["points"] = QJsonArray{QJsonArray{4.0, 5.0}, QJsonArray{24.0, 25.0}};
    shape["group_id"] = QJsonValue::Null;
    shape["description"] = QJsonValue::Null;
    shape["shape_type"] = QStringLiteral("rectangle");
    shape["flags"] = QJsonObject();
    shape["mask"] = QJsonValue::Null;
    shape["score"] = 0.64;
    shape["attributes"] = QJsonObject{{QStringLiteral("source"), QStringLiteral("ui-test")}};

    QJsonObject root;
    root["version"] = QStringLiteral("5.7.0");
    root["flags"] = QJsonObject();
    root["shapes"] = QJsonArray{shape};
    root["imagePath"] = QStringLiteral("a.jpg");
    root["imageData"] = QJsonValue::Null;
    root["imageHeight"] = image.height();
    root["imageWidth"] = image.width();

    QFile jsonFile(dir.filePath("a.json"));
    QVERIFY(jsonFile.open(QIODevice::WriteOnly));
    jsonFile.write(QJsonDocument(root).toJson());
    jsonFile.close();

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QVERIFY(QMetaObject::invokeMethod(&window, "saveFile", Qt::DirectConnection));

    QFile savedFile(dir.filePath("a.json"));
    QVERIFY(savedFile.open(QIODevice::ReadOnly));
    const QJsonObject savedShape = QJsonDocument::fromJson(savedFile.readAll()).object()
                                       .value(QStringLiteral("shapes")).toArray().first().toObject();
    QCOMPARE(savedShape.value(QStringLiteral("score")).toDouble(), 0.64);
    QCOMPARE(savedShape.value(QStringLiteral("attributes")).toObject().value(QStringLiteral("source")).toString(), QStringLiteral("ui-test"));
}

void UiTests::mainWindowLoadsEmbeddedLabelMeJsonAsImage() {
    resetTestSettings("labelme-embedded-json-startup");
    QSettings().setValue(QStringLiteral("labelme/embedImageData"), true);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(3, 2, QImage::Format_RGB32);
    image.fill(Qt::white);
    image.setPixelColor(1, 1, Qt::black);
    QByteArray pngBytes;
    QBuffer buffer(&pngBytes);
    QVERIFY(buffer.open(QIODevice::WriteOnly));
    QVERIFY(image.save(&buffer, "PNG"));

    QJsonObject shape;
    shape["label"] = QStringLiteral("embedded_box");
    shape["shape_type"] = QStringLiteral("rectangle");
    shape["points"] = QJsonArray{QJsonArray{0.0, 0.0}, QJsonArray{2.0, 1.0}};
    shape["group_id"] = QJsonValue::Null;
    shape["flags"] = QJsonObject();

    QJsonObject root;
    root["version"] = QStringLiteral("5.7.0");
    root["flags"] = QJsonObject();
    root["shapes"] = QJsonArray{shape};
    root["imagePath"] = QStringLiteral("embedded.png");
    root["imageData"] = QString::fromLatin1(pngBytes.toBase64());
    root["imageHeight"] = image.height();
    root["imageWidth"] = image.width();

    const QString jsonPath = dir.filePath(QStringLiteral("embedded.json"));
    QFile jsonFile(jsonPath);
    QVERIFY(jsonFile.open(QIODevice::WriteOnly));
    jsonFile.write(QJsonDocument(root).toJson());
    jsonFile.close();

    QTimer::singleShot(0, []() {
        if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
            box->accept();
        }
    });

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", jsonPath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QVERIFY(canvas);
    QCOMPARE(canvas->pixmapSize(), QSize(3, 2));
    QCOMPARE(canvas->shapes().size(), 1);
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("embedded_box"));

    QVERIFY(QMetaObject::invokeMethod(&window, "saveFile", Qt::DirectConnection));
    QFile savedFile(jsonPath);
    QVERIFY(savedFile.open(QIODevice::ReadOnly));
    const QJsonObject savedRoot = QJsonDocument::fromJson(savedFile.readAll()).object();
    QCOMPARE(savedRoot.value(QStringLiteral("version")).toString(), QStringLiteral("5.7.0"));
    QCOMPARE(savedRoot.value(QStringLiteral("imagePath")).toString(), QStringLiteral("embedded.png"));
    QVERIFY(savedRoot.value(QStringLiteral("imageData")).isString());
}

void UiTests::mainWindowLoadsExternalLabelMeJsonAsImage() {
    resetTestSettings("labelme-external-json-startup");

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(7, 5, QImage::Format_RGB32);
    image.fill(Qt::white);
    image.setPixelColor(2, 3, Qt::black);
    const QString imagePath = dir.filePath(QStringLiteral("source.png"));
    QVERIFY(image.save(imagePath));

    QJsonObject shape;
    shape["label"] = QStringLiteral("external_box");
    shape["shape_type"] = QStringLiteral("rectangle");
    shape["points"] = QJsonArray{QJsonArray{1.0, 1.0}, QJsonArray{5.0, 4.0}};
    shape["flags"] = QJsonObject();

    QJsonObject root;
    root["version"] = QStringLiteral("5.7.0");
    root["flags"] = QJsonObject();
    root["shapes"] = QJsonArray{shape};
    root["imagePath"] = QStringLiteral("source.png");
    root["imageData"] = QJsonValue::Null;
    root["imageHeight"] = image.height();
    root["imageWidth"] = image.width();

    const QString jsonPath = dir.filePath(QStringLiteral("external.json"));
    QFile jsonFile(jsonPath);
    QVERIFY(jsonFile.open(QIODevice::WriteOnly));
    jsonFile.write(QJsonDocument(root).toJson());
    jsonFile.close();

    QTimer::singleShot(0, []() {
        if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
            box->reject();
        }
    });

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", jsonPath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QVERIFY(canvas);
    QCOMPARE(canvas->pixmapSize(), image.size());
    QCOMPARE(canvas->shapes().size(), 1);
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("external_box"));
}

void UiTests::mainWindowRepairsInvalidLabelMeImageDataWithConfirmation() {
    resetTestSettings("labelme-repair-invalid-image-data");

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(7, 5, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("repair-source.png"));
    QVERIFY(image.save(imagePath));

    QJsonObject shape;
    shape[QStringLiteral("label")] = QStringLiteral("repair_box");
    shape[QStringLiteral("shape_type")] = QStringLiteral("rectangle");
    shape[QStringLiteral("points")] = QJsonArray{QJsonArray{1.0, 1.0}, QJsonArray{5.0, 4.0}};
    shape[QStringLiteral("flags")] = QJsonObject();
    QJsonObject root;
    root[QStringLiteral("version")] = QStringLiteral("5.7.0");
    root[QStringLiteral("flags")] = QJsonObject();
    root[QStringLiteral("shapes")] = QJsonArray{shape};
    root[QStringLiteral("imagePath")] = QStringLiteral("repair-source.png");
    root[QStringLiteral("imageData")] = QStringLiteral("broken-image-data");
    root[QStringLiteral("imageHeight")] = 99;
    root[QStringLiteral("imageWidth")] = 99;

    const QString jsonPath = dir.filePath(QStringLiteral("repair-source.json"));
    QFile jsonFile(jsonPath);
    QVERIFY(jsonFile.open(QIODevice::WriteOnly | QIODevice::Text));
    jsonFile.write(QJsonDocument(root).toJson());
    jsonFile.close();

    QTimer::singleShot(0, []() {
        if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
            if (auto *yesButton = box->button(QMessageBox::Yes)) {
                yesButton->click();
            }
        }
    });

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), jsonPath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QVERIFY(canvas);
    QCOMPARE(canvas->pixmapSize(), image.size());
    QCOMPARE(canvas->shapes().size(), 1);
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("repair_box"));
    QVERIFY(QMetaObject::invokeMethod(&window, "saveFile", Qt::DirectConnection));
    QFile saved(jsonPath);
    QVERIFY(saved.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonObject savedRoot = QJsonDocument::fromJson(saved.readAll()).object();
    QVERIFY(savedRoot.value(QStringLiteral("imageData")).isNull());
    QCOMPARE(savedRoot.value(QStringLiteral("imageWidth")).toInt(), image.width());
    QCOMPARE(savedRoot.value(QStringLiteral("imageHeight")).toInt(), image.height());
}

void UiTests::mainWindowOpenPathAcceptsLabelMeJson() {
    resetTestSettings("labelme-open-path-json");

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(9, 6, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("open-source.png"));
    QVERIFY(image.save(imagePath));

    QJsonObject shape;
    shape[QStringLiteral("label")] = QStringLiteral("opened_json");
    shape[QStringLiteral("points")] = QJsonArray{QJsonArray{1.0, 1.0}, QJsonArray{6.0, 4.0}};
    shape[QStringLiteral("shape_type")] = QStringLiteral("rectangle");
    shape[QStringLiteral("flags")] = QJsonObject();

    QJsonObject root;
    root[QStringLiteral("version")] = QStringLiteral("5.7.0");
    root[QStringLiteral("flags")] = QJsonObject();
    root[QStringLiteral("shapes")] = QJsonArray{shape};
    root[QStringLiteral("imagePath")] = QStringLiteral("open-source.png");
    root[QStringLiteral("imageData")] = QJsonValue::Null;
    root[QStringLiteral("imageHeight")] = image.height();
    root[QStringLiteral("imageWidth")] = image.width();

    const QString jsonPath = dir.filePath(QStringLiteral("open-source.json"));
    QFile jsonFile(jsonPath);
    QVERIFY(jsonFile.open(QIODevice::WriteOnly | QIODevice::Text));
    jsonFile.write(QJsonDocument(root).toJson());
    jsonFile.close();

    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QVERIFY(window.openPath(jsonPath));

    Canvas *canvas = window.findChild<Canvas *>();
    auto *fileList = window.findChild<QListWidget *>(QStringLiteral("fileList"));
    QVERIFY(canvas);
    QVERIFY(fileList);
    QCOMPARE(canvas->pixmapSize(), image.size());
    QCOMPARE(canvas->shapes().size(), 1);
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("opened_json"));
    QVERIFY(!fileList->isEnabled());
    QCOMPARE(fileList->count(), 0);

    QVERIFY(window.openPath(imagePath));
    QVERIFY(fileList->isEnabled());
    QVERIFY(fileList->count() >= 1);
    QVERIFY(fileList->findItems(imagePath, Qt::MatchExactly).size() == 1);
}

void UiTests::mainWindowOpenAnnotationPreservesExternalLabelMePath() {
    resetTestSettings("open-external-labelme-annotation");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(60, 40, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("source.png"));
    QVERIFY(image.save(imagePath));

    AnnotationDocument doc;
    doc.imagePath = imagePath;
    doc.imageSize = image.size();
    doc.labelMeVersion = QStringLiteral("5.7.0");
    doc.shapes = {Shape::fromRect(QStringLiteral("external"), QRectF(3, 4, 15, 10), false)};
    const QString externalPath = dir.filePath(QStringLiteral("custom-label.json"));
    QVERIFY(AnnotationIO::saveLabelMe(externalPath, doc));

    MainWindow window;
    QVERIFY(window.openPath(imagePath));
    QVERIFY(window.openAnnotation(externalPath));
    auto *canvas = window.findChild<Canvas *>();
    auto *format = window.findChild<QComboBox *>(QStringLiteral("footerFormatCombo"));
    QVERIFY(canvas);
    QVERIFY(format);
    QCOMPARE(canvas->shapes().size(), 1);
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("external"));
    QCOMPARE(format->currentText(), QStringLiteral("LabelMe"));

    QVERIFY(QMetaObject::invokeMethod(&window, "saveFile", Qt::DirectConnection));
    QVERIFY(QFileInfo::exists(externalPath));
    QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("source.json"))));
    QFile saved(externalPath);
    QVERIFY(saved.open(QIODevice::ReadOnly));
    const QJsonObject root = QJsonDocument::fromJson(saved.readAll()).object();
    QCOMPARE(root.value(QStringLiteral("shapes")).toArray().size(), 1);
}

void UiTests::mainWindowOpenAnnotationDetectsCreateMlJson() {
    resetTestSettings("open-external-createml-annotation");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(80, 50, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("source.png"));
    QVERIFY(image.save(imagePath));

    AnnotationDocument doc;
    doc.imagePath = imagePath;
    doc.imageSize = image.size();
    doc.shapes = {Shape::fromRect(QStringLiteral("createml"), QRectF(5, 6, 20, 12), false)};
    const QString externalPath = dir.filePath(QStringLiteral("annotations.json"));
    QVERIFY(AnnotationIO::saveCreateMl(externalPath, doc));

    MainWindow window;
    QVERIFY(window.openPath(imagePath));
    QVERIFY(window.openAnnotation(externalPath));
    auto *canvas = window.findChild<Canvas *>();
    auto *format = window.findChild<QComboBox *>(QStringLiteral("footerFormatCombo"));
    QVERIFY(canvas);
    QVERIFY(format);
    QCOMPARE(canvas->shapes().size(), 1);
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("createml"));
    QCOMPARE(format->currentText(), QStringLiteral("CreateML"));
}

void UiTests::mainWindowOpenAnnotationPreservesExternalCreateMlPath() {
    resetTestSettings("open-external-createml-save-target");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(80, 50, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("source.png"));
    QVERIFY(image.save(imagePath));

    AnnotationDocument doc;
    doc.imagePath = imagePath;
    doc.imageSize = image.size();
    doc.shapes = {Shape::fromRect(QStringLiteral("createml"), QRectF(5, 6, 20, 12), false)};
    const QString externalPath = dir.filePath(QStringLiteral("custom-createml.json"));
    QVERIFY(AnnotationIO::saveCreateMl(externalPath, doc));

    MainWindow window;
    QVERIFY(window.openPath(imagePath));
    QVERIFY(window.openAnnotation(externalPath));
    QVERIFY(QMetaObject::invokeMethod(&window, "saveFile", Qt::DirectConnection));
    QVERIFY(QFileInfo::exists(externalPath));
    QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("source.json"))));
}

void UiTests::mainWindowOpenAnnotationPreservesExternalVocPath() {
    resetTestSettings("open-external-voc-save-target");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(80, 50, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("source.png"));
    QVERIFY(image.save(imagePath));

    AnnotationDocument doc;
    doc.imagePath = imagePath;
    doc.imageSize = image.size();
    doc.shapes = {Shape::fromRect(QStringLiteral("voc"), QRectF(5, 6, 20, 12), false)};
    const QString externalPath = dir.filePath(QStringLiteral("custom-voc.xml"));
    QVERIFY(AnnotationIO::savePascalVoc(externalPath, doc));

    MainWindow window;
    QVERIFY(window.openPath(imagePath));
    QVERIFY(window.openAnnotation(externalPath));
    QVERIFY(QMetaObject::invokeMethod(&window, "saveFile", Qt::DirectConnection));
    QVERIFY(QFileInfo::exists(externalPath));
    QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("source.xml"))));

    QVERIFY(QMetaObject::invokeMethod(&window, "changeFormat", Qt::DirectConnection));
    QVERIFY(QMetaObject::invokeMethod(&window, "saveFile", Qt::DirectConnection));
    QVERIFY(QFileInfo::exists(dir.filePath(QStringLiteral("source.txt"))));
}

void UiTests::mainWindowAutoDetectsExistingLabelMeFormatBeforeSave() {
    resetTestSettings("auto-detect-labelme-format");
    QSettings settings;
    settings.setValue(QStringLiteral("labelFileFormat"), 0);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(64, 48, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("detected.png"));
    QVERIFY(image.save(imagePath));

    AnnotationDocument doc;
    doc.imagePath = imagePath;
    doc.imageSize = image.size();
    doc.labelMeVersion = QStringLiteral("5.7.0");
    doc.shapes = {Shape::fromRect(QStringLiteral("labelme_shape"), QRectF(4, 5, 20, 14), false)};
    const QString jsonPath = dir.filePath(QStringLiteral("detected.json"));
    QVERIFY(AnnotationIO::saveLabelMe(jsonPath, doc));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *formatCombo = window.findChild<QComboBox *>(QStringLiteral("footerFormatCombo"));
    QVERIFY(formatCombo);
    QCOMPARE(formatCombo->currentText(), QStringLiteral("LabelMe"));
    QVERIFY(QMetaObject::invokeMethod(&window, "saveFile", Qt::DirectConnection));
    QVERIFY(QFileInfo::exists(jsonPath));
    QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("detected.xml"))));

    QFile saved(jsonPath);
    QVERIFY(saved.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonObject savedRoot = QJsonDocument::fromJson(saved.readAll()).object();
    QCOMPARE(savedRoot.value(QStringLiteral("shapes")).toArray().first().toObject()
                 .value(QStringLiteral("label")).toString(),
             QStringLiteral("labelme_shape"));
}

void UiTests::mainWindowLabelListSupportsLabelMeStyleReordering() {
    resetTestSettings("labelme-label-list-reorder");

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("reorder.png"));
    QVERIFY(image.save(imagePath));

    QJsonArray jsonShapes;
    const QStringList labels{QStringLiteral("one"), QStringLiteral("two"), QStringLiteral("three")};
    for (int i = 0; i < labels.size(); ++i) {
        QJsonObject shape;
        shape[QStringLiteral("label")] = labels.at(i);
        shape[QStringLiteral("points")] = QJsonArray{QJsonArray{1.0 + i * 10.0, 2.0},
                                                       QJsonArray{8.0 + i * 10.0, 12.0}};
        shape[QStringLiteral("shape_type")] = QStringLiteral("rectangle");
        shape[QStringLiteral("flags")] = QJsonObject();
        jsonShapes.append(shape);
    }
    QJsonObject root;
    root[QStringLiteral("version")] = QStringLiteral("5.7.0");
    root[QStringLiteral("flags")] = QJsonObject();
    root[QStringLiteral("shapes")] = jsonShapes;
    root[QStringLiteral("imagePath")] = QStringLiteral("reorder.png");
    root[QStringLiteral("imageData")] = QJsonValue::Null;
    root[QStringLiteral("imageHeight")] = image.height();
    root[QStringLiteral("imageWidth")] = image.width();

    const QString jsonPath = dir.filePath(QStringLiteral("reorder.json"));
    QFile jsonFile(jsonPath);
    QVERIFY(jsonFile.open(QIODevice::WriteOnly | QIODevice::Text));
    jsonFile.write(QJsonDocument(root).toJson());
    jsonFile.close();

    MainWindow window;
    QVERIFY(window.openPath(jsonPath));
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    auto *canvas = window.findChild<Canvas *>();
    QVERIFY(labelList);
    QVERIFY(canvas);
    QCOMPARE(labelList->dragDropMode(), QAbstractItemView::InternalMove);
    QVERIFY(labelList->model()->moveRows(QModelIndex(), 0, 1, QModelIndex(), 3));

    QCOMPARE(canvas->shapes().size(), 3);
    QCOMPARE(canvas->shapes().at(0).label, QStringLiteral("two"));
    QCOMPARE(canvas->shapes().at(1).label, QStringLiteral("three"));
    QCOMPARE(canvas->shapes().at(2).label, QStringLiteral("one"));
    QCOMPARE(labelList->item(0)->text(), QStringLiteral("two"));
}

void UiTests::mainWindowEmbedsSourceImageDataWhenLabelMeSettingEnabled() {
    resetTestSettings("labelme-embed-source-image-data");
    QSettings settings;
    settings.setValue("labelFileFormat", 3);
    settings.setValue("labelme/embedImageData", true);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(5, 4, QImage::Format_RGB32);
    image.fill(Qt::white);
    image.setPixelColor(2, 1, Qt::black);
    const QString imagePath = dir.filePath(QStringLiteral("source.png"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QVERIFY(QMetaObject::invokeMethod(&window, "saveFile", Qt::DirectConnection));

    QFile savedFile(dir.filePath(QStringLiteral("source.json")));
    QVERIFY(savedFile.open(QIODevice::ReadOnly));
    const QJsonObject savedRoot = QJsonDocument::fromJson(savedFile.readAll()).object();
    QCOMPARE(savedRoot.value(QStringLiteral("imagePath")).toString(), QStringLiteral("source.png"));
    QVERIFY(savedRoot.value(QStringLiteral("imageData")).isString());

    const QByteArray decoded = QByteArray::fromBase64(savedRoot.value(QStringLiteral("imageData")).toString().toLatin1());
    QVERIFY(!decoded.isEmpty());
    QImage decodedImage;
    QVERIFY(decodedImage.loadFromData(decoded));
    QCOMPARE(decodedImage.size(), image.size());
}

void UiTests::mainWindowCreateBoxPromptsForLabelUsingLastUsedLabel() {
    resetTestSettings("create-box-label-prompt-last-used");
    QSettings settings;
    settings.setValue("labelHistory", QStringList{QStringLiteral("alpha"), QStringLiteral("beta")});
    settings.setValue("lastUsedLabel", QStringLiteral("alpha"));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QListWidget *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    QAction *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QVERIFY(canvas);
    QVERIFY(labelList);
    QVERIFY(createModeAction);

    bool firstPromptSeen = false;
    QTimer::singleShot(50, [&]() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            return;
        }
        auto *combo = dialog->findChild<QComboBox *>();
        QVERIFY(combo);
        QCOMPARE(combo->currentText(), QStringLiteral("alpha"));
        combo->setCurrentText(QStringLiteral("beta"));
        firstPromptSeen = true;
        dialog->accept();
    });
    createModeAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
    QTest::mouseMove(canvas, QPoint(40, 35));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(40, 35));
    QVERIFY(firstPromptSeen);
    QCOMPARE(labelList->count(), 1);
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("beta"));

    bool secondPromptSeen = false;
    QString secondDefault;
    QTimer::singleShot(50, [&]() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            return;
        }
        auto *combo = dialog->findChild<QComboBox *>();
        QVERIFY(combo);
        secondDefault = combo->currentText();
        secondPromptSeen = true;
        dialog->accept();
    });
    createModeAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 20));
    QTest::mouseMove(canvas, QPoint(80, 55));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 55));

    QVERIFY(secondPromptSeen);
    QCOMPARE(secondDefault, QStringLiteral("beta"));
    QCOMPARE(canvas->shapes().last().label, QStringLiteral("beta"));
    QCOMPARE(settings.value("lastUsedLabel").toString(), QStringLiteral("beta"));
}

void UiTests::mainWindowCanDisableNewShapeLabelPopup() {
    resetTestSettings("disable-new-shape-label-popup");
    QSettings settings;
    settings.setValue(QStringLiteral("labelme/displayLabelPopup"), false);
    settings.setValue(QStringLiteral("lastUsedLabel"), QStringLiteral("dog"));
    settings.setValue(QStringLiteral("labelHistory"), QStringList{QStringLiteral("dog")});

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(120, 90, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("popup-off.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QAction *createAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    Canvas *canvas = window.findChild<Canvas *>();
    QVERIFY(createAction);
    QVERIFY(canvas);

    bool popupSeen = false;
    QTimer::singleShot(100, [&popupSeen]() {
        if (QApplication::activeModalWidget()) {
            popupSeen = true;
            QApplication::activeModalWidget()->close();
        }
    });
    createAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(15, 15));
    QTest::mouseMove(canvas, QPoint(55, 45));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(55, 45));
    QTRY_COMPARE(canvas->shapes().size(), 1);
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("dog"));
    QVERIFY(!popupSeen);
    QVERIFY(QApplication::activeModalWidget() == nullptr);
}

void UiTests::mainWindowShowsLabelPopupWhenPopupDisabledButNoPreferredLabel() {
    resetTestSettings("popup-off-without-preferred-label");
    QSettings settings;
    settings.setValue(QStringLiteral("labelme/displayLabelPopup"), false);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(120, 90, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("popup-off-empty.jpg"));
    QVERIFY(image.save(imagePath));
    const QString classFilePath = dir.filePath(QStringLiteral("empty-classes.txt"));
    QFile classFile(classFilePath);
    QVERIFY(classFile.open(QIODevice::WriteOnly | QIODevice::Text));
    classFile.close();

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath, classFilePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QAction *createAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    Canvas *canvas = window.findChild<Canvas *>();
    QVERIFY(createAction);
    QVERIFY(canvas);

    bool popupSeen = false;
    auto *acceptPrompt = new AcceptNextLabelPromptOnShow(QStringLiteral("fallback"), qApp);
    qApp->installEventFilter(acceptPrompt);
    createAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(15, 15));
    QTest::mouseMove(canvas, QPoint(55, 45));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(55, 45));

    // This assignment is intentionally checked through the resulting shape;
    // the event filter only accepts a label dialog if one was shown.
    popupSeen = canvas->shapes().size() == 1 &&
                canvas->shapes().first().label == QStringLiteral("fallback");
    QVERIFY(popupSeen);
}

void UiTests::mainWindowCancelledLabelPromptRemovesDraft() {
    resetTestSettings("cancel-label-prompt-reopens-draft");
    QSettings settings;
    settings.setValue(QStringLiteral("labelme/displayLabelPopup"), true);
    settings.setValue(QStringLiteral("lastUsedLabel"), QStringLiteral("polygon"));
    settings.setValue(QStringLiteral("labelHistory"), QStringList{QStringLiteral("polygon")});

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(160, 120, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("cancel-popup.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *polygonAction = actionByShortcut(&window, QKeySequence(QStringLiteral("P")));
    QVERIFY(canvas);
    QVERIFY(polygonAction);

    bool promptCancelled = false;
    RejectDialogOnShow rejectDialog(&promptCancelled, qApp);
    qApp->installEventFilter(&rejectDialog);

    polygonAction->trigger();
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(90, 20));
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 80));
    QTest::keyClick(canvas, Qt::Key_Return);
    qApp->removeEventFilter(&rejectDialog);

    QVERIFY(promptCancelled);
    QCOMPARE(canvas->shapes().size(), 0);
    QVERIFY(!canvas->isDrawing());
    QVERIFY(!window.windowTitle().contains(QLatin1Char('*')));
}

void UiTests::mainWindowCancelledLabelPromptPreservesExistingDirtyState() {
    resetTestSettings("cancel-label-prompt-preserves-dirty-state");
    QSettings settings;
    settings.setValue(QStringLiteral("labelme/displayLabelPopup"), true);
    settings.setValue(QStringLiteral("lastUsedLabel"), QStringLiteral("polygon"));
    settings.setValue(QStringLiteral("labelHistory"), QStringList{QStringLiteral("polygon")});

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(160, 120, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("cancel-popup-dirty.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QAction *verifyAction = window.findChild<QAction *>(QStringLiteral("verifyAction"));
    Canvas *canvas = window.findChild<Canvas *>();
    QAction *polygonAction = actionByShortcut(&window, QKeySequence(QStringLiteral("P")));
    QVERIFY(verifyAction);
    QVERIFY(canvas);
    QVERIFY(polygonAction);

    // Make the document dirty before starting the transient creation.
    verifyAction->trigger();
    QVERIFY(window.windowTitle().contains(QLatin1Char('*')));

    bool promptCancelled = false;
    RejectDialogOnShow rejectDialog(&promptCancelled, qApp);
    qApp->installEventFilter(&rejectDialog);
    polygonAction->trigger();
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(90, 20));
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 80));
    QTest::keyClick(canvas, Qt::Key_Return);
    qApp->removeEventFilter(&rejectDialog);

    QVERIFY(promptCancelled);
    QCOMPARE(canvas->shapes().size(), 0);
    QVERIFY(window.windowTitle().contains(QLatin1Char('*')));
}

void UiTests::mainWindowCancelledOrientedLabelPromptRemovesDraft() {
    resetTestSettings("cancel-oriented-label-prompt-restores-draft");
    QSettings settings;
    settings.setValue(QStringLiteral("labelme/displayLabelPopup"), true);
    settings.setValue(QStringLiteral("lastUsedLabel"), QStringLiteral("rotated"));
    settings.setValue(QStringLiteral("labelHistory"), QStringList{QStringLiteral("rotated")});

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(180, 140, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("cancel-oriented.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *orientedAction = window.findChild<QAction *>(QStringLiteral("createOrientedRectangleModeAction"));
    QVERIFY(canvas);
    QVERIFY(orientedAction);
    // The geometry assertions below use image coordinates directly; pin the
    // canvas to 100% so they remain independent of the window's default
    // LabelMe fit-window mode.
    canvas->setScale(1.0);

    bool promptCancelled = false;
    RejectDialogOnShow rejectDialog(&promptCancelled, qApp);
    qApp->installEventFilter(&rejectDialog);

    orientedAction->trigger();
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(90, 20));
    QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(90, 80));
    qApp->removeEventFilter(&rejectDialog);

    QVERIFY(promptCancelled);
    QCOMPARE(canvas->shapes().size(), 0);
    QVERIFY(!canvas->isDrawing());
}

void UiTests::mainWindowCancelledMaskLabelPromptRemovesDraft() {
    resetTestSettings("cancel-mask-label-prompt-restores-draft");
    QSettings settings;
    settings.setValue(QStringLiteral("labelme/displayLabelPopup"), true);
    settings.setValue(QStringLiteral("lastUsedLabel"), QStringLiteral("mask"));
    settings.setValue(QStringLiteral("labelHistory"), QStringList{QStringLiteral("mask")});

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(180, 140, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("cancel-mask.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *maskAction = window.findChild<QAction *>(QStringLiteral("createMaskModeAction"));
    QVERIFY(canvas);
    QVERIFY(maskAction);

    bool promptCancelled = false;
    RejectDialogOnShow rejectDialog(&promptCancelled, qApp);
    qApp->installEventFilter(&rejectDialog);

    maskAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseMove(canvas, QPoint(55, 40));
    QTest::mouseMove(canvas, QPoint(85, 60));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(85, 60));
    qApp->removeEventFilter(&rejectDialog);

    QVERIFY(promptCancelled);
    QCOMPARE(canvas->shapes().size(), 0);
    QVERIFY(!canvas->isDrawing());
}

void UiTests::mainWindowCreateBoxPromptPrefersLastUsedOverDefaultLabel() {
    resetTestSettings("create-box-last-used-over-default");
    QSettings settings;
    settings.setValue("labelHistory", QStringList{QStringLiteral("first"), QStringLiteral("last")});
    settings.setValue("useDefaultLabel", true);
    settings.setValue("defaultLabel", QStringLiteral("first"));
    settings.setValue("lastUsedLabel", QStringLiteral("last"));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QVERIFY(canvas);
    QVERIFY(createModeAction);

    bool promptSeen = false;
    QString observedDefault;
    AcceptLabelDialogOnShow acceptPrompt(&promptSeen, &observedDefault, qApp);
    qApp->installEventFilter(&acceptPrompt);

    createModeAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
    QTest::mouseMove(canvas, QPoint(40, 35));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(40, 35));
    qApp->removeEventFilter(&acceptPrompt);

    QVERIFY(promptSeen);
    QCOMPARE(observedDefault, QStringLiteral("last"));
    QCOMPARE(canvas->shapes().last().label, QStringLiteral("last"));
}

void UiTests::mainWindowUniqueLabelListSelectsLabelForNewShapeAndEscClears() {
    resetTestSettings("unique-label-list-new-shape");
    QSettings settings;
    settings.setValue(QStringLiteral("labelHistory"), QStringList{QStringLiteral("dog"), QStringLiteral("cat")});
    settings.setValue(QStringLiteral("lastUsedLabel"), QStringLiteral("dog"));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("unique-label.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *uniqueLabelList = window.findChild<QListWidget *>(QStringLiteral("uniqueLabelList"));
    auto *canvas = window.findChild<Canvas *>();
    auto *createAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QVERIFY(uniqueLabelList);
    QVERIFY(canvas);
    QVERIFY(createAction);

    int catRow = -1;
    for (int row = 0; row < uniqueLabelList->count(); ++row) {
        if (uniqueLabelList->item(row)->data(Qt::UserRole).toString() == QStringLiteral("cat")) {
            catRow = row;
            break;
        }
    }
    QVERIFY(catRow >= 0);
    uniqueLabelList->setCurrentRow(catRow);
    QCOMPARE(uniqueLabelList->currentItem()->data(Qt::UserRole).toString(), QStringLiteral("cat"));

    bool promptSeen = false;
    QString observedDefault;
    AcceptLabelDialogOnShow acceptPrompt(&promptSeen, &observedDefault, qApp);
    qApp->installEventFilter(&acceptPrompt);

    createAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
    QTest::mouseMove(canvas, QPoint(40, 35));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(40, 35));
    qApp->removeEventFilter(&acceptPrompt);

    QVERIFY(promptSeen);
    QCOMPARE(observedDefault, QStringLiteral("cat"));
    QCOMPARE(canvas->shapes().last().label, QStringLiteral("cat"));

    uniqueLabelList->setFocus();
    QTest::keyClick(uniqueLabelList, Qt::Key_Escape);
    QVERIFY(uniqueLabelList->selectedItems().isEmpty());
}

void UiTests::mainWindowCreatedEmptyShapePromptFallsBackToLastUsedLabel() {
    resetTestSettings("created-empty-shape-last-used");
    QSettings settings;
    settings.setValue("labelHistory", QStringList{QStringLiteral("alpha"), QStringLiteral("beta")});
    settings.setValue("lastUsedLabel", QStringLiteral("beta"));

    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QListWidget *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    QVERIFY(canvas);
    QVERIFY(labelList);

    QVector<Shape> shapes;
    shapes.push_back(Shape::fromRect(QString(), QRectF(10, 10, 30, 24), false));
    canvas->setPixmap(QPixmap(100, 80));
    canvas->setShapes(shapes);
    labelList->addItem(QString());
    labelList->setCurrentRow(0);

    bool promptSeen = false;
    QString defaultLabel;
    AcceptLabelDialogOnShow acceptPrompt(&promptSeen, &defaultLabel, qApp);
    qApp->installEventFilter(&acceptPrompt);

    QVERIFY(QMetaObject::invokeMethod(&window, "editCreatedShapeLabel", Qt::DirectConnection, Q_ARG(int, 0)));
    qApp->removeEventFilter(&acceptPrompt);
    QVERIFY(promptSeen);
    QCOMPARE(defaultLabel, QStringLiteral("beta"));
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("beta"));
    QCOMPARE(settings.value("lastUsedLabel").toString(), QStringLiteral("beta"));
}

void UiTests::mainWindowKeepPreviousAnnotationCopiesShapesToEmptyImage() {
    resetTestSettings("keep-previous-annotation");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));
    QVERIFY(image.save(dir.filePath("b.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *keepPreviousAction = window.findChild<QAction *>(QStringLiteral("keepPreviousAction"));
    QVERIFY(canvas);
    QVERIFY(keepPreviousAction);
    keepPreviousAction->setChecked(true);
    QCOMPARE(QSettings().value(QStringLiteral("keepPrevious")).toBool(), true);

    Shape previous = Shape::fromRect(QStringLiteral("defect"), QRectF(10, 12, 20, 18), false);
    canvas->setShapes({previous});
    QVERIFY(QMetaObject::invokeMethod(&window, "openNextImage", Qt::DirectConnection));
    QApplication::processEvents();

    QCOMPARE(canvas->shapes().size(), 1);
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("defect"));
    QCOMPARE(canvas->shapes().first().boundingRect(), previous.boundingRect());
}

void UiTests::mainWindowKeepPreviousZoomPreservesManualScale() {
    resetTestSettings("keep-previous-zoom");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(120, 90, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("a.jpg")));
    QVERIFY(image.save(dir.filePath("b.jpg")));

    MainWindow window;
    window.resize(500, 400);
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *keepPreviousZoomAction = window.findChild<QAction *>(QStringLiteral("keepPreviousZoomAction"));
    QAction *zoomOriginalAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+0")));
    QVERIFY(canvas);
    QVERIFY(keepPreviousZoomAction);
    QVERIFY(zoomOriginalAction);
    keepPreviousZoomAction->setChecked(true);

    zoomOriginalAction->trigger();
    canvas->setScale(2.0);
    QVERIFY(QMetaObject::invokeMethod(&window, "openNextImage", Qt::DirectConnection));
    QApplication::processEvents();
    QCOMPARE(canvas->scale(), 2.0);
}

void UiTests::canvasHiddenShapesAreNotEditableOrSelectable() {
    Canvas canvas;
    canvas.resize(100, 80);
    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    canvas.setPixmap(QPixmap::fromImage(image));

    Shape hidden = Shape::fromPoints(QStringLiteral("hidden"), QStringLiteral("polygon"),
                                     {QPointF(10, 10), QPointF(30, 10), QPointF(30, 30)}, false);
    hidden.visible = false;
    Shape visible = Shape::fromPoints(QStringLiteral("visible"), QStringLiteral("polygon"),
                                      {QPointF(50, 10), QPointF(70, 10), QPointF(70, 30)}, false);
    canvas.setShapes({hidden, visible});
    canvas.setEditMode();
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QTest::mouseMove(&canvas, QPoint(20, 10));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 10));
    QVERIFY(canvas.selectedIndices().isEmpty());

    QTest::mouseMove(&canvas, QPoint(60, 10));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 10));
    QCOMPARE(canvas.selectedIndices(), QVector<int>({1}));

    canvas.shapesRef()[1].visible = false;
    canvas.update();
    QTest::mouseMove(&canvas, QPoint(60, 10));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(60, 10));
    QVERIFY(canvas.selectedIndices().isEmpty());

    Shape hiddenPoint = Shape::fromPoints(QStringLiteral("hidden-point"),
                                          QStringLiteral("points"),
                                          {QPointF(20, 50)},
                                          false);
    hiddenPoint.visible = false;
    canvas.setShapes({hiddenPoint});
    canvas.setCurrentIndex(-1);
    QTest::mouseMove(&canvas, QPoint(20, 50));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 50));
    QVERIFY2(canvas.selectedIndices().isEmpty(),
             "Hidden point containers must not be selected through shapeAt");
}

void UiTests::canvasDirectionKeysOnlyMoveShapesInEditMode() {
    Canvas canvas;
    canvas.resize(100, 80);
    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    canvas.setPixmap(QPixmap::fromImage(image));
    canvas.setShapes({Shape::fromRect(QStringLiteral("box"), QRectF(20, 20, 20, 16), false)});
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    const QRectF original = canvas.shapes().first().boundingRect();
    canvas.setCreateMode(true);
    canvas.setSelectedIndices({0});
    QTest::keyClick(&canvas, Qt::Key_Right);
    QCOMPARE(canvas.shapes().first().boundingRect(), original);

    canvas.setEditMode();
    canvas.setSelectedIndices({0});
    QTest::keyClick(&canvas, Qt::Key_Right);
    QCOMPARE(canvas.shapes().first().boundingRect().left(), original.left() + 5.0);
}

void UiTests::mainWindowDefaultsZoomShortcutsLikeLabelMe() {
    resetTestSettings("default-zoom-shortcuts-labelme");

    MainWindow window;
    QAction *zoomInAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+=")));
    QAction *zoomOriginalAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+0")));
    QVERIFY(zoomInAction);
    QVERIFY(zoomOriginalAction);
    QVERIFY(zoomInAction->shortcuts().contains(QKeySequence(QStringLiteral("Ctrl++"))));
    QVERIFY(zoomInAction->shortcuts().contains(QKeySequence(QStringLiteral("Ctrl+="))));
    QCOMPARE(zoomOriginalAction->shortcut(), QKeySequence(QStringLiteral("Ctrl+0")));
}

void UiTests::mainWindowSavePromptCanEnableAutoSave() {
    resetTestSettings("save-prompt-auto-save");
    QSettings settings;
    settings.setValue(QStringLiteral("autosave"), false);
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage first(80, 60, QImage::Format_RGB32);
    first.fill(Qt::white);
    QImage second(80, 60, QImage::Format_RGB32);
    second.fill(Qt::black);
    QVERIFY(first.save(dir.filePath("a.jpg")));
    QVERIFY(second.save(dir.filePath("b.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QAction *nextAction = actionByShortcut(&window, QKeySequence(QStringLiteral("D")));
    QAction *autoSaveAction = window.findChild<QAction *>(QStringLiteral("autoSaveAction"));
    QVERIFY(canvas);
    QVERIFY(createModeAction);
    QVERIFY(nextAction);
    QVERIFY(autoSaveAction);

    acceptNextLabelPrompt();
    createModeAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
    QTest::mouseMove(canvas, QPoint(40, 35));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(40, 35));
    QVERIFY(!autoSaveAction->isChecked());

    QTimer::singleShot(0, []() {
        QWidget *activeModal = QApplication::activeModalWidget();
        auto *box = qobject_cast<QMessageBox *>(activeModal);
        QVERIFY(box);
        QPushButton *autoSaveButton = box->findChild<QPushButton *>(QStringLiteral("autoSavePromptButton"));
        QVERIFY(autoSaveButton);
        QTest::mouseClick(autoSaveButton, Qt::LeftButton);
    });
    nextAction->trigger();

    QVERIFY(autoSaveAction->isChecked());
    QVERIFY(window.windowTitle().contains(QStringLiteral("b.jpg")));
    QSettings debugSettings;
    QTRY_VERIFY2(QFileInfo::exists(dir.filePath("a.xml")),
                 qPrintable(QStringLiteral("files=%1 savedir=%2")
                                .arg(QDir(dir.path()).entryList(QDir::Files).join(QStringLiteral(",")))
                                .arg(debugSettings.value(QStringLiteral("savedir")).toString())));
}

void UiTests::mainWindowDefaultsAutoSaveLikeLabelMe() {
    resetTestSettings("default-autosave-labelme");
    MainWindow window;
    QAction *autoSaveAction = window.findChild<QAction *>(QStringLiteral("autoSaveAction"));
    QVERIFY(autoSaveAction);
    QVERIFY(autoSaveAction->isChecked());
}

void UiTests::mainWindowAutoSaveWritesDuringEditing() {
    resetTestSettings("autosave-during-edit");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("auto.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QAction *autoSaveAction = window.findChild<QAction *>(QStringLiteral("autoSaveAction"));
    QVERIFY(canvas);
    QVERIFY(createModeAction);
    QVERIFY(autoSaveAction);
    autoSaveAction->setChecked(true);

    acceptNextLabelPrompt(QStringLiteral("defect"));
    createModeAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
    QTest::mouseMove(canvas, QPoint(40, 35));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(40, 35));

    QTRY_VERIFY2(QFileInfo::exists(dir.filePath(QStringLiteral("auto.xml"))),
                 qPrintable(QStringLiteral("files=%1").arg(QDir(dir.path()).entryList(QDir::Files).join(','))));
}

void UiTests::mainWindowStatusBarShowsLoadedImageMessage() {
    resetTestSettings("status-loaded-image");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("loaded.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    QAction *zhAction = languageAction(&window, QStringLiteral("zh-CN"));
    QVERIFY(zhAction);
    zhAction->trigger();
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QVERIFY(window.statusBar()->currentMessage().contains(QString::fromUtf8("已加载图像")));
    QVERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("loaded.jpg")));
}

void UiTests::mainWindowAutomaticAnnotationFailureReportsError() {
    resetTestSettings("automatic-annotation-load-failure");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString imagePath = dir.filePath(QStringLiteral("broken.jpg"));
    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(imagePath));

    QFile annotation(dir.filePath(QStringLiteral("broken.json")));
    QVERIFY(annotation.open(QIODevice::WriteOnly | QIODevice::Text));
    annotation.write("{\"version\":\"5.7.0\",\"imagePath\":\"broken.jpg\",\"imageData\":null,\"shapes\":[");
    annotation.close();

    MainWindow window;
    QAction *english = languageAction(&window, QStringLiteral("en"));
    QVERIFY(english);
    english->trigger();
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});

    QVERIFY2(window.statusBar()->currentMessage().contains(QStringLiteral("Failed to load")),
             qPrintable(window.statusBar()->currentMessage()));
    QVERIFY2(window.statusBar()->currentMessage().contains(QStringLiteral("broken.json")),
             qPrintable(window.statusBar()->currentMessage()));
    auto *canvas = window.findChild<Canvas *>();
    QVERIFY(canvas);
    QCOMPARE(canvas->shapes().size(), 0);
}

void UiTests::mainWindowAutomaticAnnotationFailureBlocksOverwriteSave() {
    resetTestSettings("automatic-annotation-save-guard");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString imagePath = dir.filePath(QStringLiteral("broken.jpg"));
    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(imagePath));

    const QString annotationPath = dir.filePath(QStringLiteral("broken.json"));
    const QByteArray original =
        "{\"version\":\"5.7.0\",\"imagePath\":\"broken.jpg\",\"imageData\":null,\"shapes\":[";
    QFile annotation(annotationPath);
    QVERIFY(annotation.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(annotation.write(original) == original.size());
    annotation.close();

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});

    QAction *saveAction = window.findChild<QAction *>(QStringLiteral("saveAction"));
    QAction *saveAsAction = window.findChild<QAction *>(QStringLiteral("saveAsAction"));
    QVERIFY(saveAction);
    QVERIFY(saveAsAction);
    QVERIFY2(!saveAction->isEnabled(), "save must not overwrite an annotation that failed to parse");
    QVERIFY(saveAsAction->isEnabled());

    saveAction->trigger();
    QFile unchanged(annotationPath);
    QVERIFY(unchanged.open(QIODevice::ReadOnly | QIODevice::Text));
    QCOMPARE(unchanged.readAll(), original);
}

void UiTests::mainWindowValidAnnotationClearsAutomaticLoadFailure() {
    resetTestSettings("automatic-annotation-recovery");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString imagePath = dir.filePath(QStringLiteral("recover.jpg"));
    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(imagePath));

    QFile broken(dir.filePath(QStringLiteral("recover.json")));
    QVERIFY(broken.open(QIODevice::WriteOnly | QIODevice::Text));
    broken.write("{\"version\":\"5.7.0\",\"imagePath\":\"recover.jpg\",\"imageData\":null,\"shapes\":[");
    broken.close();

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    QAction *saveAction = window.findChild<QAction *>(QStringLiteral("saveAction"));
    QAction *saveAsAction = window.findChild<QAction *>(QStringLiteral("saveAsAction"));
    QAction *verifyAction = window.findChild<QAction *>(QStringLiteral("verifyAction"));
    QVERIFY(saveAction);
    QVERIFY(saveAsAction);
    QVERIFY(verifyAction);
    QVERIFY(!saveAction->isEnabled());

    const QString validPath = dir.filePath(QStringLiteral("recovered.json"));
    QFile valid(validPath);
    QVERIFY(valid.open(QIODevice::WriteOnly | QIODevice::Text));
    const QByteArray validJson =
        "{\"version\":\"5.7.0\",\"flags\":{},\"shapes\":[],"
        "\"imagePath\":\"recover.jpg\",\"imageData\":null,"
        "\"imageHeight\":60,\"imageWidth\":80}";
    QVERIFY(valid.write(validJson) == validJson.size());
    valid.close();

    QVERIFY(window.openAnnotation(validPath));
    QVERIFY2(saveAsAction->isEnabled(), "a valid annotation load must allow Save As");
    QVERIFY2(!saveAction->isEnabled(), "a valid annotation load must remain clean");
    verifyAction->trigger();
    QVERIFY2(saveAction->isEnabled(), "a valid annotation load must enable Save after editing");
}

void UiTests::mainWindowStatusBarShowsErrorForCorruptLabelMeFile() {
    resetTestSettings("status-corrupt-labelme");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString annotationPath = dir.filePath(QStringLiteral("corrupt.json"));
    QFile annotation(annotationPath);
    QVERIFY(annotation.open(QIODevice::WriteOnly | QIODevice::Text));
    annotation.write("{\"version\": \"5.0\", \"shapes\": [");
    annotation.close();

    MainWindow window;
    QAction *english = languageAction(&window, QStringLiteral("en"));
    QVERIFY(english);
    english->trigger();
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), annotationPath});

    QVERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("Failed to load")));
    QVERIFY(window.statusBar()->currentMessage().contains(annotationPath));
}

void UiTests::mainWindowStatusBarShowsErrorForSaveFailure() {
    resetTestSettings("status-save-failure");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString imagePath = dir.filePath(QStringLiteral("save.jpg"));
    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(imagePath));

    const QString annotationDir = dir.filePath(QStringLiteral("annotations"));
    QVERIFY(QDir().mkpath(annotationDir));
    const QString annotationPath = QDir(annotationDir).filePath(QStringLiteral("save.json"));
    QJsonObject root;
    root.insert(QStringLiteral("version"), QStringLiteral("5.0"));
    root.insert(QStringLiteral("flags"), QJsonObject());
    root.insert(QStringLiteral("shapes"), QJsonArray());
    root.insert(QStringLiteral("imagePath"), QStringLiteral("../save.jpg"));
    root.insert(QStringLiteral("imageData"), QJsonValue(QJsonValue::Null));
    root.insert(QStringLiteral("imageHeight"), image.height());
    root.insert(QStringLiteral("imageWidth"), image.width());
    QFile annotation(annotationPath);
    QVERIFY(annotation.open(QIODevice::WriteOnly | QIODevice::Text));
    annotation.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    annotation.close();

    MainWindow window;
    QAction *english = languageAction(&window, QStringLiteral("en"));
    QVERIFY(english);
    english->trigger();
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), annotationPath});
    QVERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("Loaded annotation")));
    QVERIFY(QDir(annotationDir).removeRecursively());

    QVERIFY(QMetaObject::invokeMethod(&window, "saveFile", Qt::DirectConnection));
    QVERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("Failed to save")));
    QVERIFY(window.statusBar()->currentMessage().contains(annotationPath));
}

void UiTests::mainWindowTitleShowsDirtyMarkerUntilSave() {
    resetTestSettings("title-dirty-marker");
    QSettings settings;
    settings.setValue(QStringLiteral("autosave"), false);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("title.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *createAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QVERIFY(canvas);
    QVERIFY(createAction);

    acceptNextLabelPrompt(QStringLiteral("title_label"));
    createAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
    QTest::mouseMove(canvas, QPoint(40, 35));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(40, 35));
    QVERIFY(window.windowTitle().endsWith(QLatin1Char('*')));

    QVERIFY(QMetaObject::invokeMethod(&window, "saveFile", Qt::DirectConnection));
    QVERIFY(!window.windowTitle().endsWith(QLatin1Char('*')));
}

void UiTests::mainWindowDisablesSaveUntilDirty() {
    resetTestSettings("save-action-dirty-state");
    QSettings settings;
    settings.setValue(QStringLiteral("autosave"), false);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("clean.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    auto *saveAction = window.findChild<QAction *>(QStringLiteral("saveAction"));
    auto *verifyAction = window.findChild<QAction *>(QStringLiteral("verifyAction"));
    QVERIFY(saveAction);
    QVERIFY(verifyAction);
    QVERIFY2(!saveAction->isEnabled(), "Save must be disabled while the loaded image is clean");

    verifyAction->trigger();
    QVERIFY2(saveAction->isEnabled(), "Save must be enabled after the image becomes dirty");
}

void UiTests::mainWindowMissingLabelMeImageReportsErrorAndKeepsState() {
    resetTestSettings("missing-labelme-image");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("existing.jpg"));
    QVERIFY(image.save(imagePath));

    const QString annotationPath = dir.filePath(QStringLiteral("missing.json"));
    QJsonObject root;
    root.insert(QStringLiteral("version"), QStringLiteral("6.0.0"));
    root.insert(QStringLiteral("flags"), QJsonObject());
    root.insert(QStringLiteral("shapes"), QJsonArray());
    root.insert(QStringLiteral("imagePath"), QStringLiteral("does_not_exist.jpg"));
    root.insert(QStringLiteral("imageData"), QJsonValue(QJsonValue::Null));
    root.insert(QStringLiteral("imageHeight"), 100);
    root.insert(QStringLiteral("imageWidth"), 100);
    QFile annotation(annotationPath);
    QVERIFY(annotation.open(QIODevice::WriteOnly | QIODevice::Text));
    annotation.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    annotation.close();

    MainWindow window;
    QAction *english = languageAction(&window, QStringLiteral("en"));
    QVERIFY(english);
    english->trigger();
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    Canvas *canvas = window.findChild<Canvas *>();
    QVERIFY(canvas);
    QCOMPARE(canvas->pixmapSize(), image.size());
    QVERIFY(window.windowTitle().contains(QStringLiteral("existing.jpg")));

    window.loadStartupArgs({QStringLiteral("labelImgCpp"), annotationPath});

    QVERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("Failed to load")));
    QVERIFY(window.statusBar()->currentMessage().contains(annotationPath));
    QCOMPARE(canvas->pixmapSize(), image.size());
    QVERIFY(window.windowTitle().contains(QStringLiteral("existing.jpg")));
}

void UiTests::mainWindowOpenAnnotationReportsLoadFailure() {
    resetTestSettings("open-annotation-load-failure");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("source.jpg"));
    QVERIFY(image.save(imagePath));
    const QString annotationPath = dir.filePath(QStringLiteral("broken.json"));
    QFile annotation(annotationPath);
    QVERIFY(annotation.open(QIODevice::WriteOnly | QIODevice::Text));
    annotation.write("{");
    annotation.close();

    MainWindow window;
    QAction *english = languageAction(&window, QStringLiteral("en"));
    QVERIFY(english);
    english->trigger();
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    QVERIFY(!window.openAnnotation(annotationPath));
    QVERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("Failed to load")));
    QVERIFY(window.statusBar()->currentMessage().contains(annotationPath));
}

void UiTests::mainWindowSaveFailureBlocksCloseAndKeepsDirty() {
    resetTestSettings("save-failure-blocks-close");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("source.jpg"));
    QVERIFY(image.save(imagePath));
    const QString annotationDir = dir.filePath(QStringLiteral("annotations"));
    QVERIFY(QDir().mkpath(annotationDir));
    const QString annotationPath = QDir(annotationDir).filePath(QStringLiteral("source.json"));

    QJsonObject root;
    root.insert(QStringLiteral("version"), QStringLiteral("5.0"));
    root.insert(QStringLiteral("flags"), QJsonObject());
    root.insert(QStringLiteral("shapes"), QJsonArray());
    root.insert(QStringLiteral("imagePath"), QStringLiteral("../source.jpg"));
    root.insert(QStringLiteral("imageData"), QJsonValue(QJsonValue::Null));
    root.insert(QStringLiteral("imageHeight"), image.height());
    root.insert(QStringLiteral("imageWidth"), image.width());
    QFile annotation(annotationPath);
    QVERIFY(annotation.open(QIODevice::WriteOnly | QIODevice::Text));
    annotation.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    annotation.close();

    MainWindow window;
    QAction *english = languageAction(&window, QStringLiteral("en"));
    QVERIFY(english);
    english->trigger();
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), annotationPath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QAction *verifyAction = window.findChild<QAction *>(QStringLiteral("verifyAction"));
    QVERIFY(verifyAction);
    verifyAction->trigger();
    QVERIFY(window.windowTitle().endsWith(QLatin1Char('*')));
    QVERIFY(QDir(annotationDir).removeRecursively());

    QTimer::singleShot(50, [&window]() {
        auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (box && box->button(QMessageBox::Yes)) {
            box->button(QMessageBox::Yes)->click();
        }
    });
    window.close();

    QVERIFY(window.isVisible());
    QVERIFY(window.windowTitle().endsWith(QLatin1Char('*')));
    QVERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("Failed to save")));
}

void UiTests::mainWindowCreatesMissingLabelMeConfigFile() {
    resetTestSettings("labelme-create-missing-config");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString configPath = dir.filePath(QStringLiteral("nested/.labelmerc"));
    QVERIFY(!QFileInfo::exists(configPath));
    MainWindow window(nullptr, configPath);

    QVERIFY2(QFileInfo::exists(configPath), qPrintable(configPath));
    QFile config(configPath);
    QVERIFY(config.open(QIODevice::ReadOnly | QIODevice::Text));
    QCOMPARE(config.readAll(), QByteArray());
}

void UiTests::mainWindowLoadsDefaultConfigPathAndMigratesLegacyKeys() {
    resetTestSettings("labelme-default-config-path");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString configPath = dir.filePath(QStringLiteral(".labelmerc"));
    QFile config(configPath);
    QVERIFY(config.open(QIODevice::WriteOnly | QIODevice::Text));
    config.write("auto_save: false\n"
                 "store_data: true\n"
                 "keep_prev_brightness: true\n"
                 "keep_prev_contrast: false\n"
                 "labels: [legacy]\n");
    config.close();

    MainWindow window(nullptr, configPath);
    QAction *autoSaveAction = window.findChild<QAction *>(QStringLiteral("autoSaveAction"));
    QAction *embedImageDataAction = window.findChild<QAction *>(QStringLiteral("embedImageDataAction"));
    QAction *keepBrightnessAction =
        window.findChild<QAction *>(QStringLiteral("keepPreviousBrightnessContrastAction"));
    QComboBox *defaultLabel = window.findChild<QComboBox *>(QStringLiteral("defaultLabelCombo"));
    QVERIFY(autoSaveAction);
    QVERIFY(embedImageDataAction);
    QVERIFY(keepBrightnessAction);
    QVERIFY(defaultLabel);
    QVERIFY(!autoSaveAction->isChecked());
    QVERIFY(embedImageDataAction->isChecked());
    QVERIFY(keepBrightnessAction->isChecked());
    QCOMPARE(defaultLabel->findText(QStringLiteral("legacy")) >= 0, true);
}

void UiTests::mainWindowResetConfigClearsOnlyWindowState() {
    resetTestSettings("labelme-reset-config");
    QSettings settings;
    settings.setValue(QStringLiteral("window/size"), QSize(321, 123));
    settings.setValue(QStringLiteral("window/position"), QPoint(45, 67));
    settings.setValue(QStringLiteral("window/state"), QByteArray("stale-dock-state"));
    settings.setValue(QStringLiteral("autosave"), false);

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), QStringLiteral("--reset-config")});

    QVERIFY(!settings.contains(QStringLiteral("window/size")));
    QVERIFY(!settings.contains(QStringLiteral("window/position")));
    QVERIFY(!settings.contains(QStringLiteral("window/state")));
    QVERIFY(settings.contains(QStringLiteral("autosave")));
    QCOMPARE(settings.value(QStringLiteral("autosave")).toBool(), false);
}

void UiTests::mainWindowAppliesLabelMeCliCanvasOptions() {
    resetTestSettings("labelme-cli-canvas-options");
    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"),
                            QStringLiteral("--epsilon"), QStringLiteral("3.5"),
                            QStringLiteral("--no-sort-labels")});

    Canvas *canvas = window.findChild<Canvas *>();
    QVERIFY(canvas);
    QCOMPARE(canvas->epsilon(), 3.5);
}

void UiTests::mainWindowMigratesLegacyAiModelConfig() {
    resetTestSettings("labelme-ai-model-migration");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString configPath = dir.filePath(QStringLiteral("labelmerc.yaml"));
    QFile config(configPath);
    QVERIFY(config.open(QIODevice::WriteOnly | QIODevice::Text));
    config.write("ai:\n  default: SegmentAnything (balanced)\n");
    config.close();

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"),
                            QStringLiteral("--config"),
                            configPath});

    QComboBox *aiModel = window.findChild<QComboBox *>(QStringLiteral("aiModelCombo"));
    QVERIFY(aiModel);
    QCOMPARE(aiModel->currentData().toString(), QStringLiteral("sam:300m"));
}

void UiTests::mainWindowAppliesLabelMeConfigFileAndCliLabels() {
    resetTestSettings("labelme-config-startup");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage catImage(40, 30, QImage::Format_RGB32);
    catImage.fill(Qt::white);
    QImage dogImage(40, 30, QImage::Format_RGB32);
    dogImage.fill(Qt::black);
    QVERIFY(catImage.save(dir.filePath(QStringLiteral("cat.jpg"))));
    QVERIFY(dogImage.save(dir.filePath(QStringLiteral("dog.jpg"))));

    const QString configPath = dir.filePath(QStringLiteral("labelmerc.yaml"));
    QFile config(configPath);
    QVERIFY(config.open(QIODevice::WriteOnly | QIODevice::Text));
    config.write("auto_save: true\n"
                 "display_label_popup: false\n"
                 "epsilon: 18\n"
                 "labels: [cat, dog]\n"
                 "flags: [reviewed, approved]\n"
                 "label_flags:\n"
                 "  bird: [occluded, truncated]\n"
                 "canvas:\n"
                 "  double_click: none\n"
                 "  crosshair:\n"
                 "    rectangle: false\n"
                 "    polygon: true\n"
                 "ai:\n"
                 "  default: Sam2 (accuracy)\n"
                 "shape:\n"
                 "  point_size: 12\n"
                 "  vertex_fill_color: [10, 20, 30, 40]\n"
                 "  hvertex_fill_color: [50, 60, 70, 80]\n"
                 "  select_line_color: [90, 100, 110, 120]\n"
                 "  select_fill_color: [130, 140, 150, 160]\n"
                 "file_search: dog\n");
    config.close();

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"),
                            QStringLiteral("--config"), configPath,
                            QStringLiteral("--labels"), QStringLiteral("bird"),
                            dir.filePath(QStringLiteral("cat.jpg"))});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QAction *autoSaveAction = window.findChild<QAction *>(QStringLiteral("autoSaveAction"));
    QLineEdit *fileSearch = window.findChild<QLineEdit *>(QStringLiteral("fileSearchEdit"));
    QComboBox *defaultLabel = window.findChild<QComboBox *>(QStringLiteral("defaultLabelCombo"));
    QComboBox *aiModel = window.findChild<QComboBox *>(QStringLiteral("aiModelCombo"));
    QListWidget *fileList = window.findChild<QListWidget *>(QStringLiteral("fileList"));
    QListWidget *flagList = window.findChild<QListWidget *>(QStringLiteral("flagList"));
    Canvas *canvas = window.findChild<Canvas *>();
    QVERIFY(autoSaveAction);
    QVERIFY(fileSearch);
    QVERIFY(defaultLabel);
    QVERIFY(aiModel);
    QVERIFY(fileList);
    QVERIFY(flagList);
    QVERIFY(canvas);
    QVERIFY(autoSaveAction->isChecked());
    QCOMPARE(fileSearch->text(), QStringLiteral("dog"));
    QCOMPARE(defaultLabel->count(), 1);
    QCOMPARE(defaultLabel->itemText(0), QStringLiteral("bird"));
    QCOMPARE(aiModel->currentData().toString(), QStringLiteral("sam2:large"));
    QCOMPARE(fileList->count(), 1);
    QCOMPARE(flagList->count(), 2);
    QCOMPARE(flagList->item(0)->text(), QStringLiteral("approved"));
    QCOMPARE(flagList->item(1)->text(), QStringLiteral("reviewed"));
    QVERIFY(!canvas->crosshairEnabledForShapeType(QStringLiteral("rectangle")));
    QVERIFY(!canvas->doubleClickClose());
    QVERIFY(canvas->crosshairEnabledForShapeType(QStringLiteral("polygon")));
    QCOMPARE(canvas->epsilon(), 18.0);
    QCOMPARE(canvas->pointSize(), 12);
    QCOMPARE(canvas->vertexFillColor(), QColor(10, 20, 30, 40));
    QCOMPARE(canvas->hoverVertexFillColor(), QColor(50, 60, 70, 80));
    QCOMPARE(canvas->selectedLineColor(), QColor(90, 100, 110, 120));
    QCOMPARE(canvas->selectedFillColor(), QColor(130, 140, 150, 160));
    QCOMPARE(QSettings().value(QStringLiteral("labelme/labelFlags")).toString(),
             QStringLiteral("bird=occluded,truncated"));
    QCOMPARE(QSettings().value(QStringLiteral("labelme/displayLabelPopup")).toBool(), false);
}

void UiTests::mainWindowAppliesLabelMeLabelColorConfig() {
    resetTestSettings("labelme-label-color-config");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("color.jpg"));
    QVERIFY(image.save(imagePath));

    QJsonObject shapeObject;
    shapeObject[QStringLiteral("label")] = QStringLiteral("cat");
    shapeObject[QStringLiteral("points")] = QJsonArray{
        QJsonArray{10, 10}, QJsonArray{40, 10}, QJsonArray{40, 35}, QJsonArray{10, 35}};
    shapeObject[QStringLiteral("group_id")] = QJsonValue(QJsonValue::Null);
    shapeObject[QStringLiteral("description")] = QString();
    shapeObject[QStringLiteral("shape_type")] = QStringLiteral("rectangle");
    shapeObject[QStringLiteral("flags")] = QJsonObject();
    QJsonObject annotationObject;
    annotationObject[QStringLiteral("version")] = QStringLiteral("5.4.1");
    annotationObject[QStringLiteral("flags")] = QJsonObject();
    annotationObject[QStringLiteral("shapes")] = QJsonArray{shapeObject};
    annotationObject[QStringLiteral("imagePath")] = QStringLiteral("color.jpg");
    annotationObject[QStringLiteral("imageData")] = QJsonValue(QJsonValue::Null);
    annotationObject[QStringLiteral("imageHeight")] = 80;
    annotationObject[QStringLiteral("imageWidth")] = 100;
    QFile annotation(dir.filePath(QStringLiteral("color.json")));
    QVERIFY(annotation.open(QIODevice::WriteOnly | QIODevice::Text));
    annotation.write(QJsonDocument(annotationObject).toJson(QJsonDocument::Compact));
    annotation.close();

    const QString configPath = dir.filePath(QStringLiteral("labelmerc.yaml"));
    QFile config(configPath);
    QVERIFY(config.open(QIODevice::WriteOnly | QIODevice::Text));
    config.write("display_label_popup: false\n"
                 "labels: [cat]\n"
                 "shape_color: manual\n"
                 "default_shape_color: [0, 255, 0]\n"
                 "label_colors:\n"
                 "  cat: [255, 0, 0]\n");
    config.close();

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"),
                            QStringLiteral("--config"), configPath,
                            imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *createModeAction = actionByShortcut(&window, QKeySequence(QStringLiteral("W")));
    QVERIFY(canvas);
    QVERIFY(createModeAction);

    createModeAction->trigger();
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
    QTest::mouseMove(canvas, QPoint(40, 35));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(40, 35));

    QCOMPARE(canvas->shapes().size(), 2);
    QCOMPARE(canvas->shapes().first().lineColor, QColor(255, 0, 0));
    QCOMPARE(canvas->shapes().first().fillColor, QColor(255, 0, 0, 128));
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("cat"));
    QCOMPARE(canvas->shapes().last().label, QStringLiteral("cat"));
    QCOMPARE(canvas->shapes().last().lineColor, QColor(255, 0, 0));
    QCOMPARE(canvas->shapes().last().fillColor, QColor(255, 0, 0, 128));
}

void UiTests::mainWindowAutoShapeColorUsesImgvizColormap() {
    resetTestSettings("labelme-auto-shape-color-colormap");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(200, 120, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("colormap.jpg"));
    QVERIFY(image.save(imagePath));

    AnnotationDocument document;
    document.imagePath = imagePath;
    document.imageSize = image.size();
    document.labelMeVersion = QStringLiteral("5.7.0");
    for (int index = 0; index < 24; ++index) {
        document.shapes.push_back(Shape::fromRect(
            QStringLiteral("label-%1").arg(index),
            QRectF(5.0 + index * 10.0, 10.0, 6.0, 6.0),
            false));
    }
    const QString annotationPath = dir.filePath(QStringLiteral("colormap.json"));
    QVERIFY(AnnotationIO::saveLabelMe(annotationPath, document));

    QFile config(dir.filePath(QStringLiteral("labelmerc.yaml")));
    QVERIFY(config.open(QIODevice::WriteOnly | QIODevice::Text));
    QStringList configuredLabels;
    for (int index = 0; index < 24; ++index) {
        configuredLabels.append(QStringLiteral("label-%1").arg(index));
    }
    config.write(QStringLiteral("shape_color: auto\nlabels: [%1]\n")
                    .arg(configuredLabels.join(QStringLiteral(", ")))
                    .toUtf8());
    config.close();

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"),
                            QStringLiteral("--config"), config.fileName(),
                            imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QVERIFY(canvas);
    QCOMPARE(canvas->shapes().size(), 24);
    // The old 24-entry palette wrapped id 24 to black. imgviz continues the
    // bit-coded colormap, where id 24 is (64, 64, 0).
    QCOMPARE(canvas->shapes().at(23).lineColor, QColor(64, 64, 0, 255));
}

void UiTests::mainWindowShapeColorActionsUseCanvasSelection() {
    resetTestSettings("shape-color-canvas-selection");
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    auto *labelList = window.findChild<QListWidget *>(QStringLiteral("labelList"));
    auto *shapeLineColorAction = window.findChild<QAction *>(QStringLiteral("shapeLineColorAction"));
    QVERIFY(canvas);
    QVERIFY(labelList);
    QVERIFY(shapeLineColorAction);

    Shape first = Shape::fromRect(QStringLiteral("first"), QRectF(10, 10, 20, 20), false);
    first.lineColor = QColor(220, 20, 20);
    Shape second = Shape::fromRect(QStringLiteral("second"), QRectF(40, 10, 20, 20), false);
    second.lineColor = QColor(20, 20, 220);
    canvas->setShapes({first, second});
    canvas->setCurrentIndex(1);
    QCoreApplication::processEvents();
    labelList->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);

    QVERIFY(shapeLineColorAction->isEnabled());
    bool dialogSeen = false;
    const QColor replacement(20, 180, 80);
    AcceptColorDialogOnShow dialogHandler(&dialogSeen, replacement);
    qApp->installEventFilter(&dialogHandler);
    shapeLineColorAction->trigger();
    qApp->removeEventFilter(&dialogHandler);

    QVERIFY(dialogSeen);
    QCOMPARE(canvas->shapes().at(0).lineColor, QColor(220, 20, 20));
    QCOMPARE(canvas->shapes().at(1).lineColor, replacement);
}

void UiTests::mainWindowAppliesLabelMeShortcutConfig() {
    resetTestSettings("labelme-shortcut-config");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("shortcuts.jpg"));
    QVERIFY(image.save(imagePath));

    const QString configPath = dir.filePath(QStringLiteral("labelmerc.yaml"));
    QFile config(configPath);
    QVERIFY(config.open(QIODevice::WriteOnly | QIODevice::Text));
    config.write("shortcuts:\n"
                 "  quit: Ctrl+Alt+Q\n"
                 "  create_rectangle: Ctrl+Alt+R\n"
                 "  edit_shape: Ctrl+Alt+J\n"
                 "  open_next: [Alt+D, Alt+Shift+D]\n"
                 "  copy_shape: Ctrl+Alt+C\n");
    config.close();

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"),
                            QStringLiteral("--config"), configPath,
                            imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QAction *rectangleAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+Alt+R")));
    QAction *editAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+Alt+J")));
    QAction *nextPrimary = actionByShortcut(&window, QKeySequence(QStringLiteral("Alt+D")));
    QAction *nextSecondary = actionByShortcut(&window, QKeySequence(QStringLiteral("Alt+Shift+D")));
    QAction *copyAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+Alt+C")));
    QAction *quitAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+Alt+Q")));
    QVERIFY(rectangleAction);
    QVERIFY(editAction);
    QVERIFY(nextPrimary);
    QVERIFY(nextSecondary);
    QVERIFY(copyAction);
    QVERIFY(quitAction);
    QCOMPARE(quitAction->objectName(), QStringLiteral("quitAction"));
    QVERIFY(!actionByShortcut(&window, QKeySequence(QStringLiteral("W"))));
}

void UiTests::mainWindowAppliesLabelMeLabelDialogConfig() {
    resetTestSettings("labelme-label-dialog-config");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("dialog.jpg"));
    QVERIFY(image.save(imagePath));

    const QString configPath = dir.filePath(QStringLiteral("labelmerc.yaml"));
    QFile config(configPath);
    QVERIFY(config.open(QIODevice::WriteOnly | QIODevice::Text));
    config.write("labels: [zebra, cat]\n"
                 "sort_labels: false\n"
                 "label_completion: contains\n"
                 "show_label_text_field: false\n");
    config.close();

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"),
                            QStringLiteral("--config"), configPath,
                            imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *editLabelAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+E")));
    QVERIFY(canvas);
    QVERIFY(editLabelAction);
    auto *defaultLabel = window.findChild<QComboBox *>(QStringLiteral("defaultLabelCombo"));
    QVERIFY(defaultLabel);
    QCOMPARE(defaultLabel->itemText(0), QStringLiteral("zebra"));
    QCOMPARE(defaultLabel->itemText(1), QStringLiteral("cat"));
    canvas->setShapes({Shape::fromRect(QStringLiteral("cat"), QRectF(10, 10, 30, 20), false)});
    canvas->setCurrentIndex(0);
    QTest::qWait(20);

    bool inspected = false;
    QStringList observedLabels;
    QStringList observedComboLabels;
    bool labelFieldVisible = true;
    bool groupFieldVisible = true;
    bool listSortingEnabled = true;
    int currentLabelRow = -1;
    QCompleter::CompletionMode completionMode = QCompleter::PopupCompletion;
    Qt::MatchFlags completionFilter = Qt::MatchExactly;
    QTimer::singleShot(60, [&]() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            return;
        }
        auto *labelList = dialog->findChild<QListWidget *>(QStringLiteral("labelEditList"));
        auto *labelCombo = dialog->findChild<QComboBox *>(QStringLiteral("labelEditCombo"));
        auto *groupSpin = dialog->findChild<QSpinBox *>(QStringLiteral("labelGroupIdSpin"));
        if (!labelList || !labelCombo || !groupSpin || !labelCombo->completer()) {
            dialog->reject();
            return;
        }
        for (int row = 0; row < labelList->count(); ++row) {
            observedLabels.append(labelList->item(row)->text());
        }
        for (int row = 0; row < labelCombo->count(); ++row) {
            observedComboLabels.append(labelCombo->itemText(row));
        }
        labelFieldVisible = labelCombo->isVisible();
        groupFieldVisible = groupSpin->isVisible();
        listSortingEnabled = labelList->isSortingEnabled();
        currentLabelRow = labelList->currentRow();
        completionMode = labelCombo->completer()->completionMode();
        completionFilter = labelCombo->completer()->filterMode();
        inspected = true;
        dialog->reject();
    });
    editLabelAction->trigger();

    QVERIFY(inspected);
    QVERIFY2(observedLabels.size() == 2, qPrintable(observedLabels.join(QStringLiteral(","))));
    QVERIFY2(observedComboLabels.size() == 2, qPrintable(observedComboLabels.join(QStringLiteral(","))));
    QCOMPARE(observedComboLabels, QStringList({QStringLiteral("zebra"), QStringLiteral("cat")}));
    QCOMPARE(observedLabels, QStringList({QStringLiteral("zebra"), QStringLiteral("cat")}));
    QVERIFY(!labelFieldVisible);
    QVERIFY(!groupFieldVisible);
    QVERIFY(!listSortingEnabled);
    QCOMPARE(currentLabelRow, 1);
    QCOMPARE(completionMode, QCompleter::PopupCompletion);
    QCOMPARE(completionFilter, Qt::MatchContains);
}

void UiTests::mainWindowAppliesLabelMeFitToContentConfig() {
    resetTestSettings("labelme-fit-to-content");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("fit-dialog.jpg"));
    QVERIFY(image.save(imagePath));

    const QString configPath = dir.filePath(QStringLiteral("labelmerc.yaml"));
    QFile config(configPath);
    QVERIFY(config.open(QIODevice::WriteOnly | QIODevice::Text));
    config.write("labels: [a_very_long_label_that_must_remain_visible, cat, dog, person, car, tree, road, sky, metal, scratch]\n"
                 "fit_to_content:\n"
                 "  column: true\n"
                 "  row: true\n");
    config.close();

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"),
                            QStringLiteral("--config"), configPath,
                            imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *editLabelAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+E")));
    QVERIFY(canvas);
    QVERIFY(editLabelAction);
    canvas->setShapes({Shape::fromRect(QStringLiteral("cat"), QRectF(10, 10, 30, 20), false)});
    canvas->setCurrentIndex(0);
    QApplication::processEvents();

    bool inspected = false;
    int observedMinimumWidth = 0;
    int observedMinimumHeight = 0;
    int expectedWidth = 0;
    int expectedHeight = 0;
    QTimer::singleShot(60, [&]() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            return;
        }
        auto *labelList = dialog->findChild<QListWidget *>(QStringLiteral("labelEditList"));
        if (!labelList || labelList->count() == 0) {
            dialog->reject();
            return;
        }
        inspected = true;
        expectedWidth = labelList->sizeHintForColumn(0) + 2;
        expectedHeight = labelList->sizeHintForRow(0) * labelList->count() + 2;
        observedMinimumWidth = labelList->minimumWidth();
        observedMinimumHeight = labelList->minimumHeight();
        dialog->reject();
    });
    editLabelAction->trigger();

    QVERIFY(inspected);
    QVERIFY2(observedMinimumWidth >= expectedWidth,
             qPrintable(QStringLiteral("width=%1 expected=%2").arg(observedMinimumWidth).arg(expectedWidth)));
    QVERIFY2(observedMinimumHeight >= expectedHeight,
             qPrintable(QStringLiteral("height=%1 expected=%2").arg(observedMinimumHeight).arg(expectedHeight)));
}

void UiTests::mainWindowAppliesLabelMeUndoBackupLimit() {
    resetTestSettings("labelme-undo-backup-limit");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("undo.jpg"));
    QVERIFY(image.save(imagePath));

    const QString configPath = dir.filePath(QStringLiteral("labelmerc.yaml"));
    QFile config(configPath);
    QVERIFY(config.open(QIODevice::WriteOnly | QIODevice::Text));
    config.write("canvas:\n"
                 "  num_backups: 1\n");
    config.close();

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"),
                            QStringLiteral("--config"), configPath,
                            imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *undoAction = actionByShortcut(&window, QKeySequence::Undo);
    QVERIFY(canvas);
    QVERIFY(undoAction);

    Shape shape = Shape::fromRect(QStringLiteral("label-0"), QRectF(5, 5, 20, 15), false);
    canvas->setShapes({shape});
    QVERIFY(QMetaObject::invokeMethod(&window, "onCanvasShapesChanged", Qt::DirectConnection));
    for (int index = 1; index <= 3; ++index) {
        shape.label = QStringLiteral("label-%1").arg(index);
        canvas->setShapes({shape});
        QVERIFY(QMetaObject::invokeMethod(&window, "onCanvasShapesChanged", Qt::DirectConnection));
    }
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("label-3"));

    QVERIFY(undoAction->isEnabled());
    QVERIFY(QMetaObject::invokeMethod(&window, "undoShapeOperation", Qt::DirectConnection));
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("label-2"));
    QVERIFY(QMetaObject::invokeMethod(&window, "undoShapeOperation", Qt::DirectConnection));
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("label-2"));
}

void UiTests::mainWindowAppliesLabelMeDockConfig() {
    resetTestSettings("labelme-dock-config");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("dock.jpg"));
    QVERIFY(image.save(imagePath));

    const QString configPath = dir.filePath(QStringLiteral("labelmerc.yaml"));
    QFile config(configPath);
    QVERIFY(config.open(QIODevice::WriteOnly | QIODevice::Text));
    config.write("file_dock:\n"
                 "  show: false\n"
                 "  closable: false\n"
                 "  movable: false\n"
                 "  floatable: false\n"
                 "flag_dock:\n"
                 "  show: false\n"
                 "label_dock:\n"
                 "  closable: false\n");
    config.close();

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"),
                            QStringLiteral("--config"), configPath,
                            imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QDockWidget *fileDock = window.findChild<QDockWidget *>(QStringLiteral("files"));
    QDockWidget *flagDock = window.findChild<QDockWidget *>(QStringLiteral("flags"));
    QDockWidget *labelDock = window.findChild<QDockWidget *>(QStringLiteral("labels"));
    QVERIFY(fileDock);
    QVERIFY(flagDock);
    QVERIFY(labelDock);
    QVERIFY(!fileDock->isVisible());
    QVERIFY(!flagDock->isVisible());
    QVERIFY(!fileDock->features().testFlag(QDockWidget::DockWidgetClosable));
    QVERIFY(!fileDock->features().testFlag(QDockWidget::DockWidgetMovable));
    QVERIFY(!fileDock->features().testFlag(QDockWidget::DockWidgetFloatable));
    QVERIFY(!labelDock->features().testFlag(QDockWidget::DockWidgetClosable));
}

void UiTests::mainWindowSeparatesLabelAndAnnotationDocks() {
    resetTestSettings("labelme-separate-label-annotation-docks");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("dock.jpg"));
    QVERIFY(image.save(imagePath));

    const QString configPath = dir.filePath(QStringLiteral("labelmerc.yaml"));
    QFile config(configPath);
    QVERIFY(config.open(QIODevice::WriteOnly | QIODevice::Text));
    config.write("label_dock:\n"
                 "  show: true\n"
                 "shape_dock:\n"
                 "  show: false\n");
    config.close();

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"),
                            QStringLiteral("--config"), configPath,
                            imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QDockWidget *labelDock = window.findChild<QDockWidget *>(QStringLiteral("labels"));
    QDockWidget *shapeDock = window.findChild<QDockWidget *>(QStringLiteral("shapeLabels"));
    QAction *shapeDockAction = window.findChild<QAction *>(QStringLiteral("showShapeDockAction"));
    QVERIFY(labelDock);
    QVERIFY(shapeDock);
    QVERIFY(shapeDockAction);
    QVERIFY(labelDock->isVisible());
    QVERIFY(!shapeDock->isVisible());

    QAction *english = languageAction(&window, QStringLiteral("en"));
    QVERIFY(english);
    english->trigger();
    QCOMPARE(shapeDock->windowTitle(), QStringLiteral("Annotation List"));
    QCOMPARE(shapeDockAction->text(), QStringLiteral("Annotation List"));

    QAction *zhAction = languageAction(&window, QStringLiteral("zh-CN"));
    QVERIFY(zhAction);
    zhAction->trigger();
    QCOMPARE(shapeDock->windowTitle(), QString::fromUtf8("标注列表"));
    QCOMPARE(shapeDockAction->text(), QString::fromUtf8("标注列表"));
}

void UiTests::mainWindowEditableConfigPersistsSettings() {
    resetTestSettings("labelme-config-editable");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(40, 30, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("image.jpg"));
    QVERIFY(image.save(imagePath));

    const QString configPath = dir.filePath(QStringLiteral("labelmerc.yaml"));
    QFile config(configPath);
    QVERIFY(config.open(QIODevice::WriteOnly | QIODevice::Text));
    config.write("auto_save: true\nlabels: [cat]\n");
    config.close();

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"),
                            QStringLiteral("--config"), configPath,
                            imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QAction *settingsAction = window.findChild<QAction *>(QStringLiteral("settingsAction"));
    QVERIFY(settingsAction);
    QTimer::singleShot(60, []() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QVERIFY(dialog);
        auto *autoSave = dialog->findChild<QCheckBox *>(QStringLiteral("settingsAutoSave"));
        QVERIFY(autoSave);
        autoSave->setChecked(false);
        dialog->accept();
    });
    settingsAction->trigger();

    QVariantMap values;
    QString error;
    QVERIFY2(LabelMeConfig::loadFile(configPath, &values, &error), qPrintable(error));
    QCOMPARE(values.value(QStringLiteral("auto_save")).toBool(), false);
}

void UiTests::mainWindowAppliesLabelMeCliFlagsAndLabelFlags() {
    resetTestSettings("labelme-cli-flags");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(40, 30, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("image.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"),
                            QStringLiteral("--flags"), QStringLiteral("reviewed,approved"),
                            QStringLiteral("--label-flags"), QStringLiteral("bird=occluded,truncated"),
                            QStringLiteral("--labels"), QStringLiteral("bird"), imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListWidget *flagList = window.findChild<QListWidget *>(QStringLiteral("flagList"));
    QVERIFY(flagList);
    QCOMPARE(flagList->count(), 2);
    QCOMPARE(QSettings().value(QStringLiteral("labelme/labelFlags")).toString(),
             QStringLiteral("bird=occluded,truncated"));
}

void UiTests::mainWindowAcceptsLabelMeConfigStringAndCliAliases() {
    resetTestSettings("labelme-cli-aliases");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(40, 30, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("image.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"),
                            QStringLiteral("--config"),
                            QStringLiteral("auto_save: false\nlabels: [inline-cat]\n"),
                            QStringLiteral("--nosortlabels"),
                            QStringLiteral("--labelflags"),
                            QStringLiteral("inline-cat=occluded"),
                            QStringLiteral("--validatelabel"),
                            QStringLiteral("exact"),
                            imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QAction *autoSave = window.findChild<QAction *>(QStringLiteral("autoSaveAction"));
    QListWidget *labelList = window.findChild<QListWidget *>(QStringLiteral("uniqueLabelList"));
    QVERIFY(autoSave);
    QVERIFY(labelList);
    QVERIFY(!autoSave->isChecked());
    QVERIFY(labelList->findItems(QStringLiteral("inline-cat"), Qt::MatchExactly).size() == 1);
    QCOMPARE(QSettings().value(QStringLiteral("labelme/validateLabel")).toString(),
             QStringLiteral("exact"));
    QCOMPARE(QSettings().value(QStringLiteral("labelme/labelFlags")).toString(),
             QStringLiteral("inline-cat=occluded"));
}

void UiTests::mainWindowAppliesExtendedLabelMeDrawingShortcutsAndCrosshair() {
    resetTestSettings("labelme-extended-drawing-config");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(160, 120, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("extended-drawing.jpg"));
    QVERIFY(image.save(imagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"),
                            QStringLiteral("--config"),
                            QStringLiteral("shortcuts:\n"
                                           "  create_points: K\n"
                                           "  create_mask: M\n"
                                           "  create_ai_points: J\n"
                                           "  create_ai_box: B\n"
                                           "canvas:\n"
                                           "  crosshair:\n"
                                           "    points: true\n"
                                           "    mask: true\n"),
                            imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QAction *pointsAction = window.findChild<QAction *>(QStringLiteral("createPointsModeAction"));
    QAction *maskAction = window.findChild<QAction *>(QStringLiteral("createMaskModeAction"));
    QAction *aiPointsAction = window.findChild<QAction *>(QStringLiteral("createAiPointsModeAction"));
    QAction *aiBoxAction = window.findChild<QAction *>(QStringLiteral("createAiBoxModeAction"));
    Canvas *canvas = window.findChild<Canvas *>();
    QVERIFY(pointsAction);
    QVERIFY(maskAction);
    QVERIFY(aiPointsAction);
    QVERIFY(aiBoxAction);
    QVERIFY(canvas);
    QVERIFY(pointsAction->shortcuts().contains(QKeySequence(QStringLiteral("K"))));
    QVERIFY(maskAction->shortcuts().contains(QKeySequence(QStringLiteral("M"))));
    QVERIFY(aiPointsAction->shortcuts().contains(QKeySequence(QStringLiteral("J"))));
    QVERIFY(aiBoxAction->shortcuts().contains(QKeySequence(QStringLiteral("B"))));
    QVERIFY(canvas->crosshairEnabledForShapeType(QStringLiteral("points")));
    QVERIFY(canvas->crosshairEnabledForShapeType(QStringLiteral("mask")));
}

void UiTests::mainWindowOutputJsonUsesFixedLabelMePath() {
    resetTestSettings("labelme-output-json");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(80, 60, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("source.jpg"));
    const QString otherImagePath = dir.filePath(QStringLiteral("other.jpg"));
    const QString outputPath = dir.filePath(QStringLiteral("annotations.json"));
    QVERIFY(image.save(imagePath));
    QVERIFY(image.save(otherImagePath));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"),
                            QStringLiteral("--output"),
                            outputPath,
                            imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    Canvas *canvas = window.findChild<Canvas *>();
    QAction *formatAction = window.findChild<QAction *>(QStringLiteral("formatAction"));
    QVERIFY(canvas);
    QVERIFY(formatAction);
    QCOMPARE(formatAction->text(), QStringLiteral("LabelMe"));
    canvas->setShapes({Shape::fromRect(QStringLiteral("cat"), QRectF(4, 5, 20, 12), false)});
    QVERIFY(QMetaObject::invokeMethod(&window, "saveFile", Qt::DirectConnection));
    QVERIFY(QFileInfo::exists(outputPath));

    AnnotationDocument document;
    QString error;
    QVERIFY2(AnnotationIO::loadLabelMe(outputPath, &document, &error), qPrintable(error));
    QCOMPARE(document.shapes.size(), 1);
    QCOMPARE(document.shapes.first().label, QStringLiteral("cat"));
    QVERIFY(!QFileInfo::exists(QDir(dir.path()).filePath(QStringLiteral("annotations.json/source.json"))));

    window.close();
    MainWindow reloaded;
    reloaded.loadStartupArgs({QStringLiteral("labelImgCpp"),
                              QStringLiteral("--output"),
                              outputPath,
                              imagePath});
    Canvas *reloadedCanvas = reloaded.findChild<Canvas *>();
    QVERIFY(reloadedCanvas);
    QTRY_COMPARE(reloadedCanvas->shapes().size(), 1);
    QCOMPARE(reloadedCanvas->shapes().first().label, QStringLiteral("cat"));

    MainWindow otherWindow;
    otherWindow.loadStartupArgs({QStringLiteral("labelImgCpp"),
                                 QStringLiteral("--output"),
                                 outputPath,
                                 otherImagePath});
    Canvas *otherCanvas = otherWindow.findChild<Canvas *>();
    QVERIFY(otherCanvas);
    QCOMPARE(otherCanvas->shapes().size(), 0);
}

void UiTests::mainWindowSettingsCanEditLabelMeLabels() {
    resetTestSettings("labelme-settings-labels");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage image(40, 30, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath(QStringLiteral("image.jpg"));
    QVERIFY(image.save(imagePath));

    const QString configPath = dir.filePath(QStringLiteral("labelmerc.yaml"));
    QFile config(configPath);
    QVERIFY(config.open(QIODevice::WriteOnly | QIODevice::Text));
    config.write("labels: [cat]\n");
    config.close();

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), QStringLiteral("--config"), configPath, imagePath});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QAction *settingsAction = window.findChild<QAction *>(QStringLiteral("settingsAction"));
    QComboBox *defaultLabel = window.findChild<QComboBox *>(QStringLiteral("defaultLabelCombo"));
    QVERIFY(settingsAction);
    QVERIFY(defaultLabel);
    QTimer::singleShot(60, []() {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QVERIFY(dialog);
        auto *labels = dialog->findChild<QPlainTextEdit *>(QStringLiteral("settingsLabels"));
        QVERIFY(labels);
        labels->setPlainText(QStringLiteral("bird\ncat\n"));
        dialog->accept();
    });
    settingsAction->trigger();

    QCOMPARE(defaultLabel->count(), 2);
    QCOMPARE(defaultLabel->itemText(0), QStringLiteral("bird"));
    QCOMPARE(defaultLabel->itemText(1), QStringLiteral("cat"));
    QVariantMap values;
    QString error;
    QVERIFY2(LabelMeConfig::loadFile(configPath, &values, &error), qPrintable(error));
    QCOMPARE(LabelMeConfig::stringList(values.value(QStringLiteral("labels"))),
             QStringList({QStringLiteral("bird"), QStringLiteral("cat")}));
}

void UiTests::mainWindowStartupDirectoryPopulatesFileList() {
    resetTestSettings("startup-directory");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage first(10, 10, QImage::Format_RGB32);
    first.fill(Qt::white);
    QImage second(10, 10, QImage::Format_RGB32);
    second.fill(Qt::black);
    QVERIFY(first.save(dir.filePath("b.jpg")));
    QVERIFY(second.save(dir.filePath("a.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    QListWidget *fileList = window.findChild<QListWidget *>("fileList");
    QVERIFY(fileList);

    QCOMPARE(fileList->count(), 2);
    QVERIFY(fileList->currentItem());
    QCOMPARE(QFileInfo(fileList->currentItem()->text()).fileName(), QString("a.jpg"));
}

void UiTests::mainWindowStartupDirectoryUsesSupportedFormatsAndNaturalSort() {
    resetTestSettings("startup-directory-natural-sort");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(10, 10, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath("img10.jpg")));
    QVERIFY(image.save(dir.filePath("img2.jpg")));
    QVERIFY(image.save(dir.filePath("img1.ppm")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    QListWidget *fileList = window.findChild<QListWidget *>("fileList");
    QVERIFY(fileList);

    QCOMPARE(fileList->count(), 3);
    QCOMPARE(QFileInfo(fileList->item(0)->text()).fileName(), QString("img1.ppm"));
    QCOMPARE(QFileInfo(fileList->item(1)->text()).fileName(), QString("img2.jpg"));
    QCOMPARE(QFileInfo(fileList->item(2)->text()).fileName(), QString("img10.jpg"));
}

void UiTests::mainWindowLoadsExifOrientationForCanvas() {
    resetTestSettings("exif-orientation-canvas");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    // Keep the original embedded fixture disabled; generate the same EXIF case
    // from a Qt JPEG so the test source stays readable and the bytes stay valid.
#if 0
    const QByteArray jpeg = QByteArray::fromBase64(QByteArrayLiteral(
        "/9j/4AAQSkZJRgABAQAAAQABAAD/4QAiRXhpZgAATU0AKgAAAAgAAQESAAMAAAABAAYAAAAAAAD/2wBDAAEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQH/2wBDAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQH/wAARCAADAAIDAREAAhEBAxEB/8QAHwAAAQUBAQEBAQEAAAAAAAAAAAECAwQFBgcICQoL/8QAtRAAAgEDAwIEAwUFBAQAAAF9AQIDAAQRBRIhMUEGE1FhByJxFDKBkaEII0KxwRVS0fAkM2JyggkKFhcYGRolJicoKSo0NTY3ODk6Q0RFRkdISUpTVVldWV1hZWVpjZGVmZ2hqcqKjpKWmp6ipqrKztLW2t7i5usLDxMXGx8jJytLT1NXW19jZ2uHi4+Tl5ebn6Onq8vP09fb3+Pn6/9oADAMBAAIRAxEAPwD9UfgGmlaN8Cvgto9n4U8BTWmlfCb4c6bazar8PPA2vapLb2Pg/RrWCTUtc1zw9qOtazfvFErXmq6xqF9qmo3BkvNQvLq7mmnf9y8Xv2Uv0C+LPFnxQ4pzTwRzHC5nxL4ica8QZjheGvGTx44L4cw2PzniXM8xxdDIODuDvE7IuEeE8ko4jE1KeVcNcLZHk3DmRYCNDK8jyrLsswuFwlH/AJcf2i30pPFngb9oN9OvgrJaHhDjcm4P+mR9J3hbKcZxh9HH6OviHxbi8s4f8beN8pwGJ4p4/wCP/CvibjvjjiOvhcJSq53xhxrxJxBxbxNmc8VnXEmd5rnONxmPxH//2Q=="));
#endif
    QImage source(QSize(2, 3), QImage::Format_RGB32);
    source.fill(Qt::black);
    QByteArray jpegWithExif;
    QBuffer jpegBuffer(&jpegWithExif);
    QVERIFY(jpegBuffer.open(QIODevice::WriteOnly));
    QVERIFY(source.save(&jpegBuffer, "JPEG"));
    const QByteArray exifSegment = QByteArray::fromHex(
        "ffe100224578696600004d4d002a00000008000101120003000000010006000000000000");
    jpegWithExif.insert(2, exifSegment);

    const QString imagePath = dir.filePath(QStringLiteral("oriented.jpg"));
    QFile imageFile(imagePath);
    QVERIFY(imageFile.open(QIODevice::WriteOnly));
    QCOMPARE(imageFile.write(jpegWithExif), jpegWithExif.size());
    imageFile.close();

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), imagePath});
    Canvas *canvas = window.findChild<Canvas *>();
    QVERIFY(canvas);
    QCOMPARE(canvas->pixmapSize(), QSize(3, 2));
}

void UiTests::mainWindowAcceptsImageDropAndImportsFiles() {
    resetTestSettings("image-drop-import");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage first(32, 24, QImage::Format_RGB32);
    first.fill(Qt::white);
    QImage second(48, 36, QImage::Format_RGB32);
    second.fill(Qt::black);
    const QString firstPath = dir.filePath(QStringLiteral("drop-a.jpg"));
    const QString secondPath = dir.filePath(QStringLiteral("drop-b.png"));
    QVERIFY(first.save(firstPath));
    QVERIFY(second.save(secondPath));

    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QMimeData mimeData;
    mimeData.setUrls({QUrl::fromLocalFile(firstPath), QUrl::fromLocalFile(secondPath),
                      QUrl::fromLocalFile(firstPath)});
    QDragEnterEvent dragEnter(QPoint(40, 40), Qt::CopyAction, &mimeData, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&window, &dragEnter);
    QVERIFY(dragEnter.isAccepted());

    QDropEvent drop(QPointF(40, 40), Qt::CopyAction, &mimeData, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&window, &drop);
    QVERIFY(drop.isAccepted());

    QListWidget *fileList = window.findChild<QListWidget *>(QStringLiteral("fileList"));
    Canvas *canvas = window.findChild<Canvas *>();
    QVERIFY(fileList);
    QVERIFY(canvas);
    QCOMPARE(fileList->count(), 2);
    QCOMPARE(QFileInfo(fileList->currentItem()->text()).fileName(), QStringLiteral("drop-a.jpg"));
    QCOMPARE(canvas->pixmapSize(), QSize(32, 24));
    QVERIFY(window.windowTitle().contains(QStringLiteral("drop-a.jpg")));

    QMimeData invalidMimeData;
    invalidMimeData.setUrls({QUrl::fromLocalFile(dir.filePath(QStringLiteral("not-an-image.txt")))});
    QDragEnterEvent invalidDrag(QPoint(40, 40), Qt::CopyAction, &invalidMimeData, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&window, &invalidDrag);
    QVERIFY(!invalidDrag.isAccepted());
}

void UiTests::mainWindowNextPreviousKeepsFileListSelection() {
    resetTestSettings("next-previous-selection");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage first(10, 10, QImage::Format_RGB32);
    first.fill(Qt::white);
    QImage second(10, 10, QImage::Format_RGB32);
    second.fill(Qt::black);
    QVERIFY(first.save(dir.filePath("a.jpg")));
    QVERIFY(second.save(dir.filePath("b.jpg")));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    QListWidget *fileList = window.findChild<QListWidget *>("fileList");
    QVERIFY(fileList);
    QAction *nextAction = actionByShortcut(&window, QKeySequence("D"));
    QAction *prevAction = actionByShortcut(&window, QKeySequence("A"));
    QVERIFY(nextAction);
    QVERIFY(prevAction);

    nextAction->trigger();
    QVERIFY(fileList->currentItem());
    QCOMPARE(QFileInfo(fileList->currentItem()->text()).fileName(), QString("b.jpg"));

    prevAction->trigger();
    QVERIFY(fileList->currentItem());
    QCOMPARE(QFileInfo(fileList->currentItem()->text()).fileName(), QString("a.jpg"));
}

void UiTests::mainWindowCtrlShiftNavigationCopiesPreviousShapes() {
    resetTestSettings("ctrl-shift-navigation-copy-shapes");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage first(100, 80, QImage::Format_RGB32);
    first.fill(Qt::white);
    QImage second(100, 80, QImage::Format_RGB32);
    second.fill(Qt::black);
    QVERIFY(first.save(dir.filePath(QStringLiteral("a.jpg"))));
    QVERIFY(second.save(dir.filePath(QStringLiteral("b.jpg"))));

    MainWindow window;
    window.loadStartupArgs({QStringLiteral("labelImgCpp"), dir.path()});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    Canvas *canvas = window.findChild<Canvas *>();
    QAction *nextCopyAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+Shift+D")));
    QAction *prevCopyAction = actionByShortcut(&window, QKeySequence(QStringLiteral("Ctrl+Shift+A")));
    QAction *autoSaveAction = window.findChild<QAction *>(QStringLiteral("autoSaveAction"));
    QVERIFY(canvas);
    QVERIFY(nextCopyAction);
    QVERIFY(prevCopyAction);
    QVERIFY(autoSaveAction);
    autoSaveAction->setChecked(true);

    canvas->setShapes({Shape::fromRect(QStringLiteral("carry"), QRectF(12, 14, 24, 18), false)});
    nextCopyAction->trigger();

    QString title = window.windowTitle();
    if (title.endsWith(QLatin1Char('*'))) {
        title.chop(1);
    }
    QCOMPARE(QFileInfo(title).fileName(), QStringLiteral("b.jpg"));
    QCOMPARE(canvas->shapes().size(), 1);
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("carry"));

    prevCopyAction->trigger();
    title = window.windowTitle();
    if (title.endsWith(QLatin1Char('*'))) {
        title.chop(1);
    }
    QCOMPARE(QFileInfo(title).fileName(), QStringLiteral("a.jpg"));
    QCOMPARE(canvas->shapes().size(), 1);
    QCOMPARE(canvas->shapes().first().label, QStringLiteral("carry"));
}

void UiTests::mainWindowRestoresPersistedLabelHistory() {
    resetTestSettings("label-history");
    QSettings settings;
    settings.setValue("labelHistory", QStringList{QString::fromUtf8("自定义缺陷")});

    MainWindow window;
    QComboBox *defaultLabelCombo = window.findChild<QComboBox *>("defaultLabelCombo");
    QVERIFY(defaultLabelCombo);

    QVERIFY(defaultLabelCombo->findText(QString::fromUtf8("自定义缺陷")) >= 0);
}

void UiTests::mainWindowRestoresDefaultLabelSettings() {
    resetTestSettings("default-label");
    QSettings settings;
    settings.setValue("labelHistory", QStringList{QString::fromUtf8("缺陷A")});
    settings.setValue("useDefaultLabel", true);
    settings.setValue("defaultLabel", QString::fromUtf8("缺陷A"));

    MainWindow window;
    QCheckBox *useDefaultLabel = window.findChild<QCheckBox *>("useDefaultLabel");
    QComboBox *defaultLabelCombo = window.findChild<QComboBox *>("defaultLabelCombo");
    QVERIFY(useDefaultLabel);
    QVERIFY(defaultLabelCombo);

    QVERIFY(useDefaultLabel->isChecked());
    QCOMPARE(defaultLabelCombo->currentText(), QString::fromUtf8("缺陷A"));
}

void UiTests::mainWindowInlineLabelEditUpdatesLabelHistory() {
    resetTestSettings("inline-label-history");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(40, 40, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath("a.jpg");
    QVERIFY(image.save(imagePath));

    AnnotationDocument doc;
    doc.imagePath = imagePath;
    doc.imageSize = image.size();
    doc.depth = 3;
    doc.shapes = {Shape::fromRect("old", QRectF(5, 5, 10, 10), false)};
    QVERIFY(AnnotationIO::savePascalVoc(dir.filePath("a.xml"), doc));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    QListWidget *labelList = window.findChild<QListWidget *>("labelList");
    QComboBox *defaultLabelCombo = window.findChild<QComboBox *>("defaultLabelCombo");
    QVERIFY(labelList);
    QVERIFY(defaultLabelCombo);
    QCOMPARE(labelList->count(), 1);

    labelList->item(0)->setText(QString::fromUtf8("新标签"));

    QVERIFY(defaultLabelCombo->findText(QString::fromUtf8("新标签")) >= 0);
    QSettings settings;
    QVERIFY(settings.value("labelHistory").toStringList().contains(QString::fromUtf8("新标签")));
}

void UiTests::mainWindowLabelFilterOnlyTogglesShapeVisibility() {
    resetTestSettings("label-filter");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(40, 40, QImage::Format_RGB32);
    image.fill(Qt::white);
    const QString imagePath = dir.filePath("a.jpg");
    QVERIFY(image.save(imagePath));

    AnnotationDocument doc;
    doc.imagePath = imagePath;
    doc.imageSize = image.size();
    doc.depth = 3;
    doc.shapes = {Shape::fromRect("keep", QRectF(5, 5, 10, 10), false),
                  Shape::fromRect("hide", QRectF(20, 20, 10, 10), false)};
    QVERIFY(AnnotationIO::savePascalVoc(dir.filePath("a.xml"), doc));

    MainWindow window;
    window.loadStartupArgs({"labelImgCpp", dir.path()});
    QComboBox *filterCombo = window.findChild<QComboBox *>("filterCombo");
    QListWidget *labelList = window.findChild<QListWidget *>("labelList");
    Canvas *canvas = window.findChild<Canvas *>();
    QVERIFY(filterCombo);
    QVERIFY(labelList);
    QVERIFY(canvas);
    QCOMPARE(labelList->count(), 2);

    filterCombo->setCurrentText("keep");

    QCOMPARE(labelList->item(0)->checkState(), Qt::Checked);
    QCOMPARE(labelList->item(1)->checkState(), Qt::Unchecked);
    QVERIFY(canvas->shapes().at(0).visible);
    QVERIFY(!canvas->shapes().at(1).visible);
    QCOMPARE(filterCombo->currentText(), QString("keep"));
}

QTEST_MAIN(UiTests)
#include "test_ui.moc"
