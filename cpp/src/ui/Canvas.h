#pragma once

#include "core/Shape.h"
#include "core/StringBundle.h"

#include <QColor>
#include <QImage>
#include <QMap>
#include <QPoint>
#include <QPixmap>
#include <QRectF>
#include <QWidget>
#include <QVector>

class QEnterEvent;

class Canvas : public QWidget {
    Q_OBJECT

public:
    enum class SamplingMode { FastNearest, Smooth };

    explicit Canvas(QWidget *parent = nullptr);

    void setPixmap(const QPixmap &pixmap);
    // Keep annotation coordinates in the original image space while using a
    // bounded preview bitmap for very large images.
    void setPreviewPixmap(const QPixmap &preview, const QSize &imageSize);
    bool isPreviewImage() const;
    QPointF imageOriginOffset() const;
    QPointF imageOriginOffsetForScale(double scale) const;
    void setLanguage(const QString &language);
    QString language() const;
    void setShapes(const QVector<Shape> &shapes);
    QVector<Shape> shapes() const;
    QVector<Shape> &shapesRef();
    QSize pixmapSize() const;
    void setScale(double scale);
    void addScale(double delta);
    double scale() const;
    void setSamplingMode(SamplingMode mode);
    SamplingMode samplingMode() const;
    QImage overviewImage(int maxSide) const;
    void setBrightness(int brightness);
    void addBrightness(int delta);
    int brightness() const;
    Q_INVOKABLE void setContrast(int contrast);
    Q_INVOKABLE void addContrast(int delta);
    Q_INVOKABLE int contrast() const;
    void setCurrentIndex(int index);
    int currentIndex() const;
    QVector<int> selectedIndices() const;
    void setSelectedIndices(const QVector<int> &indices);
    bool duplicateSelected(const QPointF &offset = QPointF(5, 5));
    void deleteSelected();
    void setDrawSquare(bool enabled);
    void setCreateShapeType(const QString &shapeType);
    QString createShapeType() const;
    QVector<int> promptPointLabels() const;
    void setCreateMode(bool enabled);
    void setEditing(bool enabled);
    bool isDrawing() const;
    void setEditMode();
    void setViewMode();
    void cancelDrawing();
    void setLineColor(const QColor &color);
    void setFillColor(const QColor &color);
    void setVertexFillColor(const QColor &color);
    QColor vertexFillColor() const;
    void setHoverVertexFillColor(const QColor &color);
    QColor hoverVertexFillColor() const;
    void setSelectedLineColor(const QColor &color);
    QColor selectedLineColor() const;
    void setSelectedFillColor(const QColor &color);
    QColor selectedFillColor() const;
    void setCrosshairEnabled(bool enabled);
    bool crosshairEnabled() const;
    void setCrosshairEnabledForShapeType(const QString &shapeType, bool enabled);
    bool crosshairEnabledForShapeType(const QString &shapeType) const;
    void setDoubleClickClose(bool enabled);
    bool doubleClickClose() const;
    void setSnapping(bool enabled);
    bool snapping() const;
    void setPointSize(int pointSize);
    int pointSize() const;
    void setEpsilon(double epsilon);
    double epsilon() const;
    Q_INVOKABLE void setFillDrawing(bool enabled);
    Q_INVOKABLE bool fillDrawing() const;
    void setMaskEditing(bool enabled);
    bool maskEditing() const;
    bool undoLastDrawingPoint();
    void setPaintLabels(bool enabled);
    void setAllShapesVisible(bool visible);
    bool hasSelection() const;
    void deleteCurrent();
    // Discard a just-created shape when its label transaction is cancelled.
    // Unlike deleteCurrent(), this does not select an unrelated next shape.
    bool discardShapeAt(int index);
    bool copyCurrentTo(const QPointF &imagePoint);
    bool moveCurrentTo(const QPointF &imagePoint);
    bool hasPendingRightDrag() const;
    bool finishRightDrag(bool copy);
    void cancelRightDrag();
    bool insertPointAt(const QPointF &imagePoint);
    bool canAddPointToEdge() const;
    bool addPointToEdge();
    bool canRemoveSelectedPoint() const;
    bool removePointAt(const QPointF &imagePoint);
    bool removeSelectedPoint();
    QSize sizeHint() const override;

signals:
    void shapesChanged();
    void selectionChanged(int index);
    void selectionSetChanged(const QVector<int> &indices);
    void scrollRequested(int dx, int dy);
    // Wheel scrolling uses LabelMe's natural-scroll units. Mouse panning
    // keeps scrollRequested's one-to-one pixel delta semantics.
    void wheelScrollRequested(int dx, int dy);
    void statusTextChanged(const QString &text);
    void contextMenuRequested(const QPoint &globalPosition, const QPointF &imagePosition);
    void scaleChanged(double oldScale, double newScale, const QPoint &widgetPosition);
    void scaleValueChanged(double scale);
    void frameRendered(double frameMs);
    void shapeEditStarted();
    void shapeEditFinished(bool changed);
    void drawingStateChanged(bool drawing);
    void editModeRequested();
    void viewModeRequested();
    void shapeCreated(int index);
    void previousShapeRequested();
    void nextShapeRequested();
    void deleteShapeRequested();
    void removeSelectedPointRequested();
    void edgeSelectionChanged(bool selected);
    void vertexSelectionChanged(bool selected);

protected:
    void enterEvent(QEnterEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    enum class ResizeHandle { None, Left, Right, Top, Bottom, TopLeft, TopRight, BottomLeft, BottomRight };

    QPointF imagePos(const QPointF &widgetPos) const;
    void syncCrosshairPosition(const QPointF &widgetPos);
    QSize coordinateImageSize() const;
    QSize scrollableSize() const;
    QSize scrollViewportSize() const;
    bool isPolygonCreateMode() const;
    bool isPointsCreateMode() const;
    bool isLinestripCreateMode() const;
    bool isOrientedRectangleCreateMode() const;
    void finishPolygonDrawing();
    void finishOrientedRectangleDrawing();
    void drawMaskStroke(const QPointF &from, const QPointF &to, bool erase = false);
    void finishMaskDrawing();
    void finishMaskEditing();
    QRectF widgetRect(const Shape &shape) const;
    int shapeAt(const QPointF &imagePoint) const;
    void selectIndex(int index);
    QPointF boundedTopLeftForCenter(const Shape &shape, const QPointF &center) const;
    QPointF boundedDeltaForMove(const Shape &shape, const QPointF &delta) const;
    void captureMoveAnchor(const QPointF &cursor);
    QPointF clampToPixmap(const QPointF &point) const;
    int nearestPolygonEdge(const Shape &shape, const QPointF &imagePoint, qreal epsilon) const;
    int nearestRotationHandle(const Shape &shape, const QPointF &imagePoint, qreal epsilon) const;
    ResizeHandle resizeHandleAt(const Shape &shape, const QPointF &imagePoint) const;
    void resizeCurrentShape(ResizeHandle handle, const QPointF &imagePoint, bool square = false);
    void setResizeCursor(ResizeHandle handle);
    void rotateCurrentShape(const QPointF &imagePoint);
    void toggleSelectedIndex(int index);
    QVector<int> selectionIndices() const;
    QPointF boundedDeltaForSelection(const QPointF &delta) const;
    void rebuildAdjustedImage();
    QString canvasPositionStatus(const QPointF &position) const;
    QString canvasSizeStatus(const QRectF &box, const QPointF &position) const;
    QString canvasPointsStatus(int count, const QString &type, const QPointF &position) const;
    QString canvasOrientedRectangleStatus(int count, const QPointF &position) const;
    QString canvasEdgeStatus(const QPointF &position) const;

    QPixmap m_pixmap;
    mutable QImage m_cachedOverview;
    mutable int m_cachedOverviewMaxSide = 0;
    QSize m_imageSize;
    StringBundle m_strings;
    QImage m_previewImage;
    QImage m_adjustedImage;
    QImage m_adjustedPreviewImage;
    QVector<Shape> m_shapes;
    int m_currentIndex = -1;
    double m_scale = 1.0;
    int m_brightness = 50;
    int m_contrast = 50;
    SamplingMode m_samplingMode = SamplingMode::FastNearest;
    QColor m_lineColor = QColor(0, 255, 0, 128);
    QColor m_fillColor = QColor(0, 0, 0, 64);
    QColor m_vertexFillColor = QColor(0, 255, 0, 255);
    QColor m_hoverVertexFillColor = QColor(255, 255, 255, 255);
    QColor m_selectedLineColor = QColor(255, 255, 255, 255);
    QColor m_selectedFillColor;
    QString m_createShapeType = QStringLiteral("rectangle");
    bool m_drawSquare = false;
    bool m_fillDrawing = false;
    bool m_crosshairEnabled = true;
    QMap<QString, bool> m_crosshairByShapeType;
    bool m_doubleClickClose = true;
    bool m_snapping = true;
    bool m_altSnappingDisabled = false;
    int m_pointSize = 8;
    bool m_hasHoverImagePos = false;
    double m_epsilon = 10.0;
    // LabelMe constructs Canvas in EDIT mode. Creation is entered explicitly
    // through a mode action, so an empty click immediately after startup can
    // clear selection instead of being interpreted as a new rectangle.
    bool m_createMode = false;
    bool m_editing = true;
    bool m_paintLabels = false;
    bool m_creating = false;
    bool m_moving = false;
    bool m_movingVertex = false;
    bool m_resizing = false;
    bool m_rotating = false;
    bool m_shapeDragStarted = false;
    bool m_shapeGeometryChanged = false;
    int m_clickSelectedShape = -1;
    int m_viewClickShape = -1;
    bool m_shapeEditActive = false;
    bool m_leftPanning = false;
    bool m_leftDragged = false;
    bool m_rightPanning = false;
    bool m_rightDragged = false;
    bool m_rightDragCandidate = false;
    bool m_middlePanning = false;
    bool m_middleDragged = false;
    bool m_polygonDrawing = false;
    bool m_orientedRectangleDrawing = false;
    bool m_maskDrawing = false;
    bool m_maskEditing = false;
    bool m_maskEditingExisting = false;
    bool m_maskErase = false;
    int m_hoverVertex = -1;
    int m_hoverEdge = -1;
    int m_hoverRotationHandle = -1;
    int m_hoverShape = -1;
    ResizeHandle m_resizeHandle = ResizeHandle::None;
    ResizeHandle m_hoverResizeHandle = ResizeHandle::None;
    QPointF m_rotationCenter;
    qreal m_rotationInitialAngle = 0.0;
    QVector<QPointF> m_rotationOriginalPoints;
    QPoint m_lastMousePos;
    QPoint m_lastPanGlobalPos;
    QPoint m_leftPressPos;
    QPoint m_leftPressGlobalPos;
    QPoint m_rightPressPos;
    QPoint m_rightPressGlobalPos;
    QPointF m_rightDragStartImagePos;
    QPointF m_rightDragAppliedDelta;
    QVector<int> m_rightDragIndices;
    QVector<Shape> m_rightDragShapes;
    QPoint m_middlePressGlobalPos;
    QPointF m_startImagePos;
    QPointF m_moveAnchorOffset;
    QPointF m_moveEffectiveCursor;
    QRectF m_moveBounds;
    QPointF m_hoverImagePos;
    QVector<QPointF> m_polygonPoints;
    QVector<int> m_promptPointLabels;
    QVector<QPointF> m_orientedRectanglePoints;
    bool m_retypedDraft = false;
    QPointF m_polygonPreviewPos;
    QPoint m_maskCanvasOrigin;
    QImage m_maskCanvas;
    QPointF m_maskLastPoint;
    int m_maskBrushRadius = 6;
};
