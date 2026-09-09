#include "ui/Canvas.h"

#include <QAbstractScrollArea>
#include <QApplication>
#include <QByteArray>
#include <QBuffer>
#include <QCursor>
#include <QElapsedTimer>
#include <QEnterEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QToolTip>
#include <QWheelEvent>

#include <cmath>
#include <limits>

Canvas::Canvas(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus);
    // Match LabelMe's default_config.yaml: a crosshair is useful for boxes,
    // while point/line/polygon modes stay visually unobstructed by default.
    m_crosshairByShapeType = {
        {QStringLiteral("polygon"), false},
        {QStringLiteral("rectangle"), true},
        {QStringLiteral("oriented_rectangle"), false},
        {QStringLiteral("circle"), false},
        {QStringLiteral("line"), false},
        {QStringLiteral("point"), false},
        {QStringLiteral("linestrip"), false},
        {QStringLiteral("points"), false},
        {QStringLiteral("mask"), false},
        {QStringLiteral("ai_points_to_shape"), false},
        {QStringLiteral("ai_box_to_shape"), true},
    };
}

void Canvas::setLanguage(const QString &language) {
    m_strings = StringBundle(language);
    update();
}

QString Canvas::language() const {
    return m_strings.language();
}

QString Canvas::canvasPositionStatus(const QPointF &position) const {
    return m_strings.get(QStringLiteral("canvasPositionStatus"))
        .arg(qRound(position.x()))
        .arg(qRound(position.y()));
}

QString Canvas::canvasSizeStatus(const QRectF &box, const QPointF &position) const {
    return m_strings.get(QStringLiteral("canvasSizeStatus"))
        .arg(qRound(box.width()))
        .arg(qRound(box.height()))
        .arg(qRound(position.x()))
        .arg(qRound(position.y()));
}

QString Canvas::canvasPointsStatus(int count, const QString &type, const QPointF &position) const {
    return m_strings.get(QStringLiteral("canvasPointsStatus"))
        .arg(type)
        .arg(count)
        .arg(qRound(position.x()))
        .arg(qRound(position.y()));
}

QString Canvas::canvasOrientedRectangleStatus(int count, const QPointF &position) const {
    return m_strings.get(QStringLiteral("canvasOrientedStatus"))
        .arg(count)
        .arg(qRound(position.x()))
        .arg(qRound(position.y()));
}

QString Canvas::canvasEdgeStatus(const QPointF &position) const {
    return m_strings.get(QStringLiteral("canvasEdgeStatus"))
        .arg(qRound(position.x()))
        .arg(qRound(position.y()));
}

namespace {
constexpr qreal kLabelMeKeyboardMoveSpeed = 5.0;

QSize scaledSize(const QSize &size, double scale) {
    return QSize(qMax(1, qRound(size.width() * scale)),
                 qMax(1, qRound(size.height() * scale)));
}

int overscrollSlack(int scaled, int viewport) {
    if (scaled <= viewport || viewport <= 0) {
        return 0;
    }
    return qMax(viewport / 8, qMin(viewport / 2, scaled - viewport));
}

bool isTooSmallToKeep(const QRectF &rect) {
    constexpr qreal minimumBoxSize = 3.0;
    return rect.width() < minimumBoxSize || rect.height() < minimumBoxSize;
}

bool isTooSmallToKeepCreatedShape(const Shape &shape) {
    if (shape.shapeType == QStringLiteral("point")) {
        return false;
    }
    if ((shape.shapeType == QStringLiteral("line") ||
         shape.shapeType == QStringLiteral("linestrip") ||
         shape.shapeType == QStringLiteral("circle")) && shape.points.size() >= 2) {
        const QPointF delta = shape.points[1] - shape.points[0];
        return std::hypot(delta.x(), delta.y()) < 3.0;
    }
    return isTooSmallToKeep(shape.boundingRect());
}

bool hasMinimumDistinctPoints(const QVector<QPointF> &points, int minimum) {
    QVector<QPointF> distinct;
    distinct.reserve(qMin(minimum, points.size()));
    for (const QPointF &point : points) {
        bool alreadySeen = false;
        for (const QPointF &candidate : distinct) {
            if (candidate == point) {
                alreadySeen = true;
                break;
            }
        }
        if (!alreadySeen) {
            distinct.push_back(point);
            if (distinct.size() >= minimum) {
                return true;
            }
        }
    }
    return false;
}

qreal vertexHitRadius(double scale) {
    return 16.0 / scale;
}

qreal distanceToSegment(const QPointF &point, const QPointF &start, const QPointF &end) {
    const QPointF segment = end - start;
    const qreal lengthSquared = segment.x() * segment.x() + segment.y() * segment.y();
    if (lengthSquared <= 0.0) {
        const QPointF delta = point - start;
        return std::hypot(delta.x(), delta.y());
    }

    const QPointF fromStart = point - start;
    const qreal projection = (fromStart.x() * segment.x() + fromStart.y() * segment.y()) / lengthSquared;
    const qreal clampedProjection = qBound(0.0, projection, 1.0);
    const QPointF closest = start + segment * clampedProjection;
    const QPointF delta = point - closest;
    return std::hypot(delta.x(), delta.y());
}

bool isPointBackedShape(const Shape &shape) {
    return shape.shapeType == QStringLiteral("polygon") ||
           shape.shapeType == QStringLiteral("point") ||
           shape.shapeType == QStringLiteral("points") ||
           shape.shapeType == QStringLiteral("line") ||
           shape.shapeType == QStringLiteral("linestrip") ||
           shape.shapeType == QStringLiteral("circle") ||
           shape.shapeType == QStringLiteral("oriented_rectangle") ||
           (shape.shapeType != QStringLiteral("rectangle") &&
            shape.shapeType != QStringLiteral("mask") &&
            !shape.points.isEmpty());
}

bool isPointMarkerShape(const Shape &shape) {
    return shape.shapeType == QStringLiteral("point") ||
           shape.shapeType == QStringLiteral("points");
}

bool isSeedCompatibleCreateMode(const QString &shapeType) {
    return shapeType == QStringLiteral("polygon") || shapeType == QStringLiteral("rectangle") ||
           shapeType == QStringLiteral("points") || shapeType == QStringLiteral("line") ||
           shapeType == QStringLiteral("linestrip") || shapeType == QStringLiteral("circle") ||
           shapeType == QStringLiteral("oriented_rectangle");
}

QPointF projectToPerpendicularThrough(const QPointF &point, const QPointF &lineStart, const QPointF &lineEnd) {
    const QPointF axis = lineEnd - lineStart;
    const qreal lengthSquared = axis.x() * axis.x() + axis.y() * axis.y();
    if (lengthSquared <= 0.0) {
        return lineEnd;
    }
    const QPointF normal(-axis.y(), axis.x());
    const QPointF delta = point - lineEnd;
    const qreal projection = (delta.x() * normal.x() + delta.y() * normal.y()) / lengthSquared;
    return lineEnd + normal * projection;
}

QPointF projectToLine(const QPointF &point, const QPointF &lineStart, const QPointF &lineEnd) {
    const QPointF direction = lineStart - lineEnd;
    const qreal lengthSquared = QPointF::dotProduct(direction, direction);
    if (lengthSquared <= 0.0) {
        return lineEnd;
    }
    const qreal projection = QPointF::dotProduct(direction, point - lineEnd) / lengthSquared;
    return lineEnd + direction * projection;
}

bool isOutsidePixmap(const QPointF &point, const QSize &imageSize) {
    return point.x() < 0.0 || point.y() < 0.0 ||
           point.x() > imageSize.width() || point.y() > imageSize.height();
}

QPointF intersectionWithPixmap(const QPointF &p1, const QPointF &p2, const QSize &imageSize) {
    const qreal width = imageSize.width();
    const qreal height = imageSize.height();
    const qreal startX = qBound<qreal>(0.0, p1.x(), width);
    const qreal startY = qBound<qreal>(0.0, p1.y(), height);
    const qreal deltaX = p2.x() - startX;
    const qreal deltaY = p2.y() - startY;

    qreal exitT = 1.0;
    const QVector<QPair<qreal, qreal>> boundaries = {
        {startX, -deltaX},
        {width - startX, deltaX},
        {startY, -deltaY},
        {height - startY, deltaY},
    };
    for (const auto &boundary : boundaries) {
        if (boundary.second > 0.0) {
            exitT = qMin(exitT, boundary.first / boundary.second);
        }
    }
    if (exitT > 0.0) {
        return QPointF(startX + exitT * deltaX, startY + exitT * deltaY);
    }

    // When the segment starts on an edge and points outward, keep the
    // boundary coordinate fixed and slide the other coordinate into range.
    if (startX <= 0.0 || startX >= width) {
        return QPointF(startX, qBound<qreal>(0.0, p2.y(), height));
    }
    return QPointF(qBound<qreal>(0.0, p2.x(), width), startY);
}

QVector<QPointF> orientedRectangleCorners(const QPointF &first, const QPointF &second, const QPointF &third) {
    const QPointF projectedThird = projectToPerpendicularThrough(third, first, second);
    const QPointF fourth = first + projectedThird - second;
    return {first, second, projectedThird, fourth};
}

QVector<QPointF> reprojectOrientedRectangleCorners(const QVector<QPointF> &corners,
                                                    int vertexIndex,
                                                    const QPointF &moving,
                                                    const QSize &imageSize) {
    if (corners.size() != 4 || vertexIndex < 0 || vertexIndex >= 4) {
        return corners;
    }
    const QPointF anchor = corners[(vertexIndex + 2) % 4];
    const QPointF edgeAxis = corners[(vertexIndex + 3) % 4];
    QPointF boundedMoving = moving;
    QPointF adjacentPerpendicular = projectToPerpendicularThrough(boundedMoving, edgeAxis, anchor);
    QPointF adjacentParallel = anchor + boundedMoving - adjacentPerpendicular;

    if (isOutsidePixmap(boundedMoving, imageSize)) {
        const QPointF edgeA = intersectionWithPixmap(adjacentPerpendicular, boundedMoving, imageSize);
        const QPointF edgeB = intersectionWithPixmap(adjacentParallel, boundedMoving, imageSize);
        boundedMoving = projectToLine(boundedMoving, edgeA, edgeB);
        adjacentPerpendicular = projectToPerpendicularThrough(boundedMoving, adjacentParallel, anchor);
        adjacentParallel = anchor + boundedMoving - adjacentPerpendicular;
    }

    if (isOutsidePixmap(adjacentPerpendicular, imageSize)) {
        adjacentPerpendicular = intersectionWithPixmap(anchor, adjacentPerpendicular, imageSize);
        boundedMoving = adjacentPerpendicular + adjacentParallel - anchor;
    }

    if (isOutsidePixmap(adjacentParallel, imageSize)) {
        adjacentParallel = intersectionWithPixmap(anchor, adjacentParallel, imageSize);
        boundedMoving = adjacentPerpendicular + adjacentParallel - anchor;
    }

    QVector<QPointF> updated = corners;
    updated[vertexIndex] = boundedMoving;
    updated[(vertexIndex + 1) % 4] = adjacentPerpendicular;
    updated[(vertexIndex + 3) % 4] = adjacentParallel;
    return updated;
}

QVector<QPointF> rotationHandlePoints(const Shape &shape) {
    if (shape.shapeType != QStringLiteral("oriented_rectangle") || shape.points.size() != 4) {
        return {};
    }
    QVector<QPointF> handles;
    handles.reserve(4);
    for (int i = 0; i < 4; ++i) {
        handles.push_back((shape.points[i] + shape.points[(i + 3) % 4]) / 2.0);
    }
    return handles;
}

QVector<QPointF> orientedRectangleArrowPoints(const Shape &shape) {
    if (shape.shapeType != QStringLiteral("oriented_rectangle") || shape.points.size() != 4) {
        return {};
    }

    const QPointF center = (shape.points[0] + shape.points[2]) / 2.0;
    const QPointF direction = shape.points[1] - shape.points[0];
    const qreal angle = std::atan2(direction.y(), direction.x());
    const qreal cosAngle = std::cos(angle);
    const qreal sinAngle = std::sin(angle);
    const QVector<QPointF> templatePoints = {
        QPointF(1.1, -2.5),
        QPointF(5.0, 0.0),
        QPointF(1.1, 2.5),
        QPointF(-5.0, 0.0)};

    QVector<QPointF> points;
    points.reserve(templatePoints.size());
    for (const QPointF &point : templatePoints) {
        points.append(QPointF(center.x() + point.x() * cosAngle - point.y() * sinAngle,
                              center.y() + point.x() * sinAngle + point.y() * cosAngle));
    }
    return points;
}

qreal selectedOutlineWidth(double scale, const QRectF &box) {
    const qreal maxScreenWidth = qMin<qreal>(2.0, qMax<qreal>(1.0, qMin(box.width(), box.height()) * scale / 5.0));
    return maxScreenWidth / scale;
}

qreal vertexHandleRadius(double scale, const QRectF &box) {
    const qreal minScreenSide = qMin(qAbs(box.width() * scale), qAbs(box.height() * scale));
    const qreal screenRadius = qBound<qreal>(1.0, qMin<qreal>(5.0, minScreenSide / 6.0), 5.0);
    return screenRadius / scale;
}

qreal pointMarkerRadius(int pointSize, double scale) {
    Q_UNUSED(scale);
    // LabelMe stores point_size in image pixels. Letting the world transform
    // apply the zoom keeps markers small in overview/low-zoom views instead
    // of expanding them back to a fixed screen-space diameter.
    return qMax<qreal>(0.5, pointSize / 2.0);
}

QColor vertexColorForShape(const Shape &shape, const QColor &fallback) {
    QColor color = shape.lineColor.isValid() ? shape.lineColor : fallback;
    color.setAlpha(255);
    return color;
}

QVector<QPointF> resizeHandlePoints(const QRectF &box, double scale) {
    const qreal minScreenSide = qMin(qAbs(box.width() * scale), qAbs(box.height() * scale));
    if (minScreenSide < 32.0) {
        return {box.topLeft(), box.topRight(), box.bottomRight(), box.bottomLeft()};
    }
    return {
        box.topLeft(),
        QPointF(box.center().x(), box.top()),
        box.topRight(),
        QPointF(box.right(), box.center().y()),
        box.bottomRight(),
        QPointF(box.center().x(), box.bottom()),
        box.bottomLeft(),
        QPointF(box.left(), box.center().y()),
    };
}

QImage maskOverlayImage(const Shape &shape, int alpha) {
    if (shape.maskData.isEmpty()) {
        return {};
    }
    QImage mask;
    mask.loadFromData(QByteArray::fromBase64(shape.maskData.toLatin1()), "PNG");
    if (mask.isNull()) {
        return {};
    }

    QImage overlay(mask.size(), QImage::Format_ARGB32);
    overlay.fill(Qt::transparent);
    const QColor fill(shape.fillColor.red(), shape.fillColor.green(), shape.fillColor.blue(), alpha);
    for (int y = 0; y < mask.height(); ++y) {
        for (int x = 0; x < mask.width(); ++x) {
            const QColor pixel = mask.pixelColor(x, y);
            if (pixel.alpha() > 0 && qGray(pixel.rgb()) > 0) {
                overlay.setPixelColor(x, y, fill);
            }
        }
    }
    return overlay;
}

QImage maskOverlayImage(const QImage &mask, const QColor &color, int alpha) {
    if (mask.isNull()) {
        return {};
    }
    QImage overlay(mask.size(), QImage::Format_ARGB32);
    overlay.fill(Qt::transparent);
    const QColor fill(color.red(), color.green(), color.blue(), alpha);
    for (int y = 0; y < mask.height(); ++y) {
        for (int x = 0; x < mask.width(); ++x) {
            if (qGray(mask.pixelColor(x, y).rgb()) > 0) {
                overlay.setPixelColor(x, y, fill);
            }
        }
    }
    return overlay;
}

QImage decodeMaskData(const QString &maskData) {
    if (maskData.isEmpty()) {
        return {};
    }
    QImage mask;
    if (!mask.loadFromData(QByteArray::fromBase64(maskData.toLatin1()), "PNG")) {
        return {};
    }
    return mask.convertToFormat(QImage::Format_Grayscale8);
}

QString encodeMaskData(const QImage &mask) {
    if (mask.isNull()) {
        return {};
    }
    QByteArray pngData;
    QBuffer buffer(&pngData);
    if (!buffer.open(QIODevice::WriteOnly) || !mask.save(&buffer, "PNG")) {
        return {};
    }
    return QString::fromLatin1(pngData.toBase64());
}

bool maskPixelIsSet(const QImage &mask, int x, int y) {
    if (x < 0 || y < 0 || x >= mask.width() || y >= mask.height()) {
        return false;
    }
    const QColor pixel = mask.pixelColor(x, y);
    return pixel.alpha() > 0 && qGray(pixel.rgb()) > 0;
}

void drawMaskBoundary(QPainter &painter,
                      const Shape &shape,
                      const QImage &mask,
                      const QPen &pen) {
    if (mask.isNull() || shape.points.isEmpty()) {
        return;
    }

    const QPointF origin = shape.points.first();
    painter.save();
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    for (int y = 0; y < mask.height(); ++y) {
        for (int x = 0; x < mask.width(); ++x) {
            if (!maskPixelIsSet(mask, x, y)) {
                continue;
            }
            const qreal left = origin.x() + x;
            const qreal top = origin.y() + y;
            if (!maskPixelIsSet(mask, x, y - 1)) {
                painter.drawLine(QPointF(left, top), QPointF(left + 1.0, top));
            }
            if (!maskPixelIsSet(mask, x + 1, y)) {
                painter.drawLine(QPointF(left + 1.0, top), QPointF(left + 1.0, top + 1.0));
            }
            if (!maskPixelIsSet(mask, x, y + 1)) {
                painter.drawLine(QPointF(left + 1.0, top + 1.0), QPointF(left, top + 1.0));
            }
            if (!maskPixelIsSet(mask, x - 1, y)) {
                painter.drawLine(QPointF(left, top + 1.0), QPointF(left, top));
            }
        }
    }
    painter.restore();
}
}

void Canvas::setPixmap(const QPixmap &pixmap) {
    m_cachedOverview = {};
    m_pixmap = pixmap;
    m_imageSize = pixmap.size();
    m_promptPointLabels.clear();
    m_maskCanvas = {};
    m_maskCanvasOrigin = {};
    m_maskDrawing = false;
    m_maskEditingExisting = false;
    m_maskErase = false;
    m_shapeEditActive = false;
    m_hoverShape = -1;
    m_hoverVertex = -1;
    m_hoverEdge = -1;
    emit vertexSelectionChanged(false);
    emit edgeSelectionChanged(false);
    m_previewImage = overviewImage(1600);
    rebuildAdjustedImage();
    setMinimumSize(scrollableSize());
    resize(sizeHint());
    updateGeometry();
    update();
}

void Canvas::setPreviewPixmap(const QPixmap &preview, const QSize &imageSize) {
    m_cachedOverview = {};
    if (preview.isNull() || !imageSize.isValid() || imageSize.isEmpty()) {
        setPixmap(preview);
        return;
    }
    m_pixmap = preview;
    m_imageSize = imageSize;
    m_promptPointLabels.clear();
    m_maskCanvas = {};
    m_maskCanvasOrigin = {};
    m_maskDrawing = false;
    m_maskEditingExisting = false;
    m_maskErase = false;
    m_shapeEditActive = false;
    m_hoverShape = -1;
    m_hoverVertex = -1;
    m_hoverEdge = -1;
    emit vertexSelectionChanged(false);
    emit edgeSelectionChanged(false);
    m_previewImage = overviewImage(1600);
    rebuildAdjustedImage();
    setMinimumSize(scrollableSize());
    resize(sizeHint());
    updateGeometry();
    update();
}

bool Canvas::isPreviewImage() const {
    return !m_pixmap.isNull() && m_imageSize.isValid() && m_pixmap.size() != m_imageSize;
}

void Canvas::setShapes(const QVector<Shape> &shapes) {
    m_shapes = shapes;
    m_promptPointLabels.clear();
    m_currentIndex = m_shapes.isEmpty() ? -1 : m_shapes.size() - 1;
    for (Shape &shape : m_shapes) {
        shape.selected = false;
    }
    if (m_currentIndex >= 0) {
        m_shapes[m_currentIndex].selected = true;
    }
    m_rotating = false;
    m_hoverShape = -1;
    m_hoverVertex = -1;
    m_hoverEdge = -1;
    emit vertexSelectionChanged(false);
    m_hoverRotationHandle = -1;
    m_rotationOriginalPoints.clear();
    m_shapeEditActive = false;
    update();
}

QVector<Shape> Canvas::shapes() const {
    return m_shapes;
}

QVector<Shape> &Canvas::shapesRef() {
    return m_shapes;
}

QSize Canvas::pixmapSize() const {
    return coordinateImageSize();
}

QSize Canvas::coordinateImageSize() const {
    return m_imageSize.isValid() && !m_imageSize.isEmpty() ? m_imageSize : m_pixmap.size();
}

QPointF Canvas::imageOriginOffset() const {
    return imageOriginOffsetForScale(m_scale);
}

QPointF Canvas::imageOriginOffsetForScale(double scale) const {
    if (m_pixmap.isNull()) {
        return {};
    }
    const double safeScale = qBound(0.005, scale, 16.0);
    const QSize scaled = scaledSize(coordinateImageSize(), safeScale);
    const QSize viewport = scrollViewportSize();
    if (viewport.isEmpty()) {
        return {};
    }
    return QPointF(overscrollSlack(scaled.width(), viewport.width()) / (2.0 * safeScale),
                   overscrollSlack(scaled.height(), viewport.height()) / (2.0 * safeScale));
}

QSize Canvas::scrollViewportSize() const {
    QWidget *ancestor = parentWidget();
    while (ancestor) {
        if (auto *scrollArea = qobject_cast<QAbstractScrollArea *>(ancestor)) {
            return scrollArea->viewport()->size();
        }
        ancestor = ancestor->parentWidget();
    }
    return {};
}

QSize Canvas::scrollableSize() const {
    const QSize scaled = scaledSize(coordinateImageSize(), m_scale);
    const QSize viewport = scrollViewportSize();
    if (viewport.isEmpty()) {
        return scaled;
    }
    return QSize(scaled.width() + overscrollSlack(scaled.width(), viewport.width()),
                 scaled.height() + overscrollSlack(scaled.height(), viewport.height()));
}

QSize Canvas::sizeHint() const {
    return scrollableSize();
}

void Canvas::setScale(double scale) {
    const double oldScale = m_scale;
    m_scale = qBound(0.005, scale, 16.0);
    setMinimumSize(scrollableSize());
    resize(sizeHint());
    updateGeometry();
    update();
    if (!qFuzzyCompare(oldScale, m_scale)) {
        emit scaleValueChanged(m_scale);
    }
}

void Canvas::addScale(double delta) {
    setScale(m_scale + delta);
}

double Canvas::scale() const {
    return m_scale;
}

void Canvas::setSamplingMode(SamplingMode mode) {
    if (m_samplingMode == mode) {
        return;
    }
    m_samplingMode = mode;
    update();
}

Canvas::SamplingMode Canvas::samplingMode() const {
    return m_samplingMode;
}

QImage Canvas::overviewImage(int maxSide) const {
    if (m_pixmap.isNull() || maxSide <= 0) {
        return {};
    }
    if (!m_cachedOverview.isNull() && m_cachedOverviewMaxSide == maxSide) {
        return m_cachedOverview;
    }
    const QSize sourceSize = m_pixmap.size();
    const double scale = qMin(static_cast<double>(maxSide) / sourceSize.width(),
                              static_cast<double>(maxSide) / sourceSize.height());
    QSize targetSize(qMax(1, qRound(sourceSize.width() * scale)),
                     qMax(1, qRound(sourceSize.height() * scale)));
    m_cachedOverview = m_pixmap.toImage().scaled(targetSize, Qt::KeepAspectRatio, Qt::FastTransformation);
    m_cachedOverviewMaxSide = maxSide;
    return m_cachedOverview;
}

void Canvas::rebuildAdjustedImage() {
    if (m_pixmap.isNull() || (m_brightness == 50 && m_contrast == 50)) {
        m_adjustedImage = {};
        m_adjustedPreviewImage = {};
        return;
    }

    const QImage source = m_pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
    m_adjustedImage = QImage(source.size(), QImage::Format_ARGB32);
    const double brightnessFactor = m_brightness / 50.0;
    const double contrastFactor = m_contrast / 50.0;

    // LabelMe delegates these operations to PIL.ImageEnhance: brightness is
    // applied first, then contrast blends the result with a flat image whose
    // value is the whole-image grayscale mean. Keeping the intermediate
    // image also preserves the original alpha channel just like LabelMe.
    int brightnessLut[256];
    for (int channel = 0; channel < 256; ++channel) {
        brightnessLut[channel] = qBound(0, static_cast<int>(std::floor(channel * brightnessFactor)), 255);
    }

    for (int y = 0; y < source.height(); ++y) {
        const QRgb *sourceLine = reinterpret_cast<const QRgb *>(source.constScanLine(y));
        QRgb *targetLine = reinterpret_cast<QRgb *>(m_adjustedImage.scanLine(y));
        for (int x = 0; x < source.width(); ++x) {
            const QRgb pixel = sourceLine[x];
            targetLine[x] = qRgba(brightnessLut[qRed(pixel)],
                                  brightnessLut[qGreen(pixel)],
                                  brightnessLut[qBlue(pixel)],
                                  qAlpha(pixel));
        }
    }

    if (!qFuzzyCompare(contrastFactor, 1.0)) {
        long double graySum = 0.0L;
        const qint64 pixelCount = static_cast<qint64>(source.width()) * source.height();
        for (int y = 0; y < source.height(); ++y) {
            const QRgb *line = reinterpret_cast<const QRgb *>(m_adjustedImage.constScanLine(y));
            for (int x = 0; x < source.width(); ++x) {
                const QRgb pixel = line[x];
                // PIL's RGB -> L conversion uses 299/587/114 coefficients.
                graySum += (299 * qRed(pixel) + 587 * qGreen(pixel) + 114 * qBlue(pixel)) / 1000.0L;
            }
        }
        const double grayMean = pixelCount > 0
                                    ? std::floor(static_cast<double>(graySum / pixelCount) + 0.5)
                                    : 0.0;
        int contrastLut[256];
        for (int channel = 0; channel < 256; ++channel) {
            const double blended = grayMean * (1.0 - contrastFactor) + channel * contrastFactor;
            contrastLut[channel] = qBound(0, static_cast<int>(std::floor(blended)), 255);
        }
        for (int y = 0; y < source.height(); ++y) {
            QRgb *line = reinterpret_cast<QRgb *>(m_adjustedImage.scanLine(y));
            for (int x = 0; x < source.width(); ++x) {
                const QRgb pixel = line[x];
                line[x] = qRgba(contrastLut[qRed(pixel)],
                                contrastLut[qGreen(pixel)],
                                contrastLut[qBlue(pixel)],
                                qAlpha(pixel));
            }
        }
    }

    const QSize sourceSize = m_adjustedImage.size();
    const double scale = qMin(1600.0 / sourceSize.width(), 1600.0 / sourceSize.height());
    const QSize targetSize(qMax(1, qRound(sourceSize.width() * scale)),
                           qMax(1, qRound(sourceSize.height() * scale)));
    m_adjustedPreviewImage = m_adjustedImage.scaled(targetSize,
                                                    Qt::KeepAspectRatio,
                                                    Qt::FastTransformation);
}

void Canvas::setBrightness(int brightness) {
    const int bounded = qBound(0, brightness, 150);
    if (m_brightness == bounded) {
        return;
    }
    m_brightness = bounded;
    rebuildAdjustedImage();
    update();
}

void Canvas::addBrightness(int delta) {
    setBrightness(m_brightness + delta);
}

int Canvas::brightness() const {
    return m_brightness;
}

void Canvas::setContrast(int contrast) {
    const int bounded = qBound(0, contrast, 150);
    if (m_contrast == bounded) {
        return;
    }
    m_contrast = bounded;
    rebuildAdjustedImage();
    update();
}

void Canvas::addContrast(int delta) {
    setContrast(m_contrast + delta);
}

int Canvas::contrast() const {
    return m_contrast;
}

void Canvas::setCurrentIndex(int index) {
    selectIndex(index);
}

int Canvas::currentIndex() const {
    return m_currentIndex;
}

QVector<int> Canvas::selectedIndices() const {
    return selectionIndices();
}

void Canvas::setSelectedIndices(const QVector<int> &indices) {
    for (Shape &shape : m_shapes) {
        shape.selected = false;
    }
    m_currentIndex = -1;
    for (int index : indices) {
        if (index < 0 || index >= m_shapes.size()) {
            continue;
        }
        m_shapes[index].selected = true;
        m_currentIndex = index;
    }
    emit selectionChanged(m_currentIndex);
    emit selectionSetChanged(selectionIndices());
    update();
}

bool Canvas::duplicateSelected(const QPointF &offset) {
    const QVector<int> selected = selectionIndices();
    if (selected.isEmpty()) {
        return false;
    }

    QVector<int> duplicated;
    duplicated.reserve(selected.size());
    for (int index : selected) {
        if (index < 0 || index >= m_shapes.size()) {
            continue;
        }
        Shape copy = m_shapes[index].copy();
        copy.moveBy(offset);
        copy.selected = false;
        m_shapes.push_back(copy);
        duplicated.push_back(m_shapes.size() - 1);
    }
    setSelectedIndices(duplicated);
    emit shapesChanged();
    return !duplicated.isEmpty();
}

void Canvas::deleteSelected() {
    const QVector<int> selected = selectionIndices();
    if (selected.isEmpty()) {
        return;
    }
    QVector<int> descending = selected;
    std::sort(descending.begin(), descending.end(), std::greater<int>());
    for (int index : descending) {
        if (index >= 0 && index < m_shapes.size()) {
            m_shapes.removeAt(index);
        }
    }
    m_currentIndex = -1;
    emit shapesChanged();
    emit selectionChanged(m_currentIndex);
    emit selectionSetChanged(selectionIndices());
    update();
}

void Canvas::setDrawSquare(bool enabled) {
    m_drawSquare = enabled;
}

void Canvas::setCreateShapeType(const QString &shapeType) {
    const QString oldShapeType = m_createShapeType;
    QString nextShapeType = shapeType;
    if (shapeType == QStringLiteral("polygon") ||
        shapeType == QStringLiteral("point") ||
        shapeType == QStringLiteral("points") ||
        shapeType == QStringLiteral("ai_points_to_shape") ||
        shapeType == QStringLiteral("ai_box_to_shape") ||
        shapeType == QStringLiteral("line") ||
        shapeType == QStringLiteral("linestrip") ||
        shapeType == QStringLiteral("circle") ||
        shapeType == QStringLiteral("oriented_rectangle") ||
        shapeType == QStringLiteral("mask")) {
        nextShapeType = shapeType;
    } else {
        nextShapeType = QStringLiteral("rectangle");
    }
    if (nextShapeType == oldShapeType) {
        return;
    }

    bool hasSeed = false;
    bool hasMovedDraft = false;
    QPointF seed;
    QPointF draftCursor;
    if (m_creating && hasSelection() && m_currentIndex >= 0 && m_currentIndex < m_shapes.size()) {
        const Shape &draft = m_shapes.at(m_currentIndex);
        if (draft.points.size() >= 2 &&
            (draft.shapeType == QStringLiteral("rectangle") ||
             draft.shapeType == QStringLiteral("line") ||
             draft.shapeType == QStringLiteral("circle"))) {
            hasSeed = true;
            seed = draft.points.first();
            draftCursor = draft.points.last();
            hasMovedDraft = !isTooSmallToKeepCreatedShape(draft);
        }
    } else if (m_polygonDrawing && m_polygonPoints.size() == 1) {
        hasSeed = true;
        seed = m_polygonPoints.first();
    } else if (m_orientedRectangleDrawing && m_orientedRectanglePoints.size() == 1) {
        hasSeed = true;
        seed = m_orientedRectanglePoints.first();
    }

    const bool preserveSeed = hasSeed && isSeedCompatibleCreateMode(oldShapeType) &&
                              isSeedCompatibleCreateMode(nextShapeType);
    m_createShapeType = nextShapeType;
    m_retypedDraft = false;
    cancelDrawing();
    if (!preserveSeed) {
        return;
    }

    m_promptPointLabels.clear();
    m_polygonPreviewPos = hasMovedDraft ? draftCursor : seed;
    if (nextShapeType == QStringLiteral("oriented_rectangle")) {
        m_orientedRectangleDrawing = true;
        m_orientedRectanglePoints = {seed};
        emit drawingStateChanged(true);
    } else if (nextShapeType == QStringLiteral("polygon") ||
               nextShapeType == QStringLiteral("points") ||
               nextShapeType == QStringLiteral("linestrip")) {
        m_polygonDrawing = true;
        m_polygonPoints = {seed};
        emit drawingStateChanged(true);
    } else {
        m_creating = true;
        Shape draft;
        const QPointF endPoint = hasMovedDraft ? draftCursor : seed;
        if (nextShapeType == QStringLiteral("line") || nextShapeType == QStringLiteral("circle")) {
            draft = Shape::fromPoints(QString(), nextShapeType, {seed, endPoint}, false);
        } else {
            draft = Shape::fromRect(QString(), QRectF(seed, endPoint), false);
        }
        draft.lineColor = m_lineColor;
        draft.fillColor = m_fillColor;
        draft.paintLabel = m_paintLabels;
        m_shapes.push_back(draft);
        selectIndex(m_shapes.size() - 1);
        m_retypedDraft = hasMovedDraft;
        emit drawingStateChanged(true);
    }
    update();
}

QString Canvas::createShapeType() const {
    return m_createShapeType;
}

QVector<int> Canvas::promptPointLabels() const {
    return m_promptPointLabels;
}

void Canvas::syncCrosshairPosition(const QPointF &widgetPos) {
    m_hasHoverImagePos = !m_pixmap.isNull() && rect().contains(widgetPos.toPoint());
    if (m_hasHoverImagePos) {
        m_hoverImagePos = clampToPixmap(imagePos(widgetPos));
    }
}

void Canvas::setCreateMode(bool enabled) {
    m_createMode = enabled;
    m_editing = !enabled;
    if (enabled) {
        const bool hadVertexSelection = m_hoverVertex >= 0;
        const bool hadEdgeSelection = m_hoverEdge >= 0;
        m_hoverShape = -1;
        m_hoverVertex = -1;
        m_hoverEdge = -1;
        m_hoverRotationHandle = -1;
        m_hoverResizeHandle = ResizeHandle::None;
        if (hadVertexSelection) {
            emit vertexSelectionChanged(false);
        }
        if (hadEdgeSelection) {
            emit edgeSelectionChanged(false);
        }
        setSelectedIndices({});
        setCursor(Qt::CrossCursor);
        syncCrosshairPosition(mapFromGlobal(QCursor::pos()));
    } else {
        unsetCursor();
    }
    update();
}

void Canvas::setEditing(bool enabled) {
    m_editing = enabled;
    m_createMode = !enabled;
    if (!enabled) {
        m_hoverEdge = -1;
        emit edgeSelectionChanged(false);
    }
    if (enabled) {
        unsetCursor();
    } else {
        setCursor(Qt::CrossCursor);
    }
    update();
}

void Canvas::setEditMode() {
    setEditing(true);
}

bool Canvas::isDrawing() const {
    return m_creating || m_polygonDrawing || m_orientedRectangleDrawing || m_maskDrawing;
}

void Canvas::setViewMode() {
    cancelDrawing();
    m_editing = false;
    m_createMode = false;
    m_creating = false;
    m_moving = false;
    m_movingVertex = false;
    m_rotating = false;
    m_hoverShape = -1;
    m_hoverVertex = -1;
    m_hoverEdge = -1;
    emit edgeSelectionChanged(false);
    setCursor(Qt::ArrowCursor);
    update();
}

bool Canvas::undoLastDrawingPoint() {
    if (m_polygonDrawing && !m_polygonPoints.isEmpty()) {
        m_polygonPoints.removeLast();
        if (!m_promptPointLabels.isEmpty()) {
            m_promptPointLabels.removeLast();
        }
        if (m_polygonPoints.isEmpty()) {
            m_polygonDrawing = false;
            emit drawingStateChanged(false);
        } else {
            m_polygonPreviewPos = m_polygonPoints.last();
        }
        update();
        return true;
    }
    if (m_orientedRectangleDrawing && !m_orientedRectanglePoints.isEmpty()) {
        m_orientedRectanglePoints.removeLast();
        if (m_orientedRectanglePoints.isEmpty()) {
            m_orientedRectangleDrawing = false;
        } else {
            m_polygonPreviewPos = m_orientedRectanglePoints.last();
        }
        update();
        return true;
    }
    if (m_creating && hasSelection() && m_currentIndex >= 0 && m_currentIndex < m_shapes.size()) {
        Shape &draft = m_shapes[m_currentIndex];
        if ((draft.shapeType == QStringLiteral("rectangle") ||
             draft.shapeType == QStringLiteral("line") ||
             draft.shapeType == QStringLiteral("circle")) &&
            draft.points.size() >= 2) {
            // LabelMe's undo-last-point action reopens a two-point draft at
            // its anchor instead of discarding the in-progress shape. Keep
            // the draft active so the next drag can finish the same gesture.
            const QPointF anchor = draft.points.first();
            draft.points = {anchor, anchor};
            draft.pointLabels.fill(1, draft.points.size());
            update();
            return true;
        }
    }
    return false;
}

void Canvas::cancelDrawing() {
    const bool wasDrawing = isDrawing();
    m_altSnappingDisabled = false;
    m_retypedDraft = false;
    m_promptPointLabels.clear();
    if (m_maskDrawing) {
        m_maskDrawing = false;
        m_maskEditingExisting = false;
        m_maskErase = false;
        m_maskCanvas = {};
        m_maskCanvasOrigin = {};
        if (wasDrawing) {
            emit drawingStateChanged(false);
        }
        update();
        return;
    }
    m_rotating = false;
    m_hoverRotationHandle = -1;
    m_rotationOriginalPoints.clear();
    if (m_polygonDrawing) {
        m_polygonDrawing = false;
        m_polygonPoints.clear();
        if (wasDrawing) {
            emit drawingStateChanged(false);
        }
        update();
        return;
    }
    if (m_orientedRectangleDrawing) {
        m_orientedRectangleDrawing = false;
        m_orientedRectanglePoints.clear();
        if (wasDrawing) {
            emit drawingStateChanged(false);
        }
        update();
        return;
    }
    if (m_creating && hasSelection()) {
        const bool wasShapeEditActive = m_shapeEditActive;
        m_shapes.removeAt(m_currentIndex);
        m_currentIndex = -1;
        m_creating = false;
        if (wasDrawing) {
            emit drawingStateChanged(false);
        }
        emit selectionChanged(m_currentIndex);
        if (wasShapeEditActive) {
            emit shapeEditFinished(false);
            m_shapeEditActive = false;
        }
        update();
    }
}

void Canvas::setLineColor(const QColor &color) {
    m_lineColor = color;
}

void Canvas::setFillColor(const QColor &color) {
    m_fillColor = color;
}

void Canvas::setVertexFillColor(const QColor &color) {
    if (!color.isValid()) {
        return;
    }
    m_vertexFillColor = color;
    update();
}

QColor Canvas::vertexFillColor() const {
    return m_vertexFillColor;
}

void Canvas::setHoverVertexFillColor(const QColor &color) {
    if (!color.isValid()) {
        return;
    }
    m_hoverVertexFillColor = color;
    update();
}

QColor Canvas::hoverVertexFillColor() const {
    return m_hoverVertexFillColor;
}

void Canvas::setSelectedLineColor(const QColor &color) {
    if (!color.isValid()) {
        return;
    }
    m_selectedLineColor = color;
    update();
}

QColor Canvas::selectedLineColor() const {
    return m_selectedLineColor;
}

void Canvas::setSelectedFillColor(const QColor &color) {
    m_selectedFillColor = color;
    update();
}

QColor Canvas::selectedFillColor() const {
    return m_selectedFillColor;
}

void Canvas::setCrosshairEnabled(bool enabled) {
    if (m_crosshairEnabled == enabled) {
        return;
    }
    m_crosshairEnabled = enabled;
    update();
}

bool Canvas::crosshairEnabled() const {
    return m_crosshairEnabled;
}

void Canvas::setCrosshairEnabledForShapeType(const QString &shapeType, bool enabled) {
    const QString key = shapeType.trimmed();
    if (key.isEmpty()) {
        setCrosshairEnabled(enabled);
        return;
    }
    if (m_crosshairByShapeType.contains(key) && m_crosshairByShapeType.value(key) == enabled) {
        return;
    }
    m_crosshairByShapeType.insert(key, enabled);
    update();
}

bool Canvas::crosshairEnabledForShapeType(const QString &shapeType) const {
    const QString key = shapeType.trimmed();
    if (key.isEmpty()) {
        return m_crosshairEnabled;
    }
    return m_crosshairEnabled && m_crosshairByShapeType.value(key, true);
}

void Canvas::setDoubleClickClose(bool enabled) {
    m_doubleClickClose = enabled;
}

bool Canvas::doubleClickClose() const {
    return m_doubleClickClose;
}

void Canvas::setSnapping(bool enabled) {
    m_snapping = enabled;
    update();
}

bool Canvas::snapping() const {
    return m_snapping;
}

void Canvas::setPointSize(int pointSize) {
    const int boundedSize = qBound(1, pointSize, 64);
    if (m_pointSize == boundedSize) {
        return;
    }
    m_pointSize = boundedSize;
    update();
}

int Canvas::pointSize() const {
    return m_pointSize;
}

void Canvas::setEpsilon(double epsilon) {
    m_epsilon = qMax(0.0, epsilon);
}

double Canvas::epsilon() const {
    return m_epsilon;
}

void Canvas::setFillDrawing(bool enabled) {
    m_fillDrawing = enabled;
    update();
}

bool Canvas::fillDrawing() const {
    return m_fillDrawing;
}

void Canvas::setMaskEditing(bool enabled) {
    if (m_maskEditing == enabled) {
        return;
    }
    m_maskEditing = enabled;
    if (!enabled && m_maskDrawing && m_maskEditingExisting) {
        cancelDrawing();
    }
    update();
}

bool Canvas::maskEditing() const {
    return m_maskEditing;
}

void Canvas::setPaintLabels(bool enabled) {
    m_paintLabels = enabled;
    for (Shape &shape : m_shapes) {
        shape.paintLabel = enabled;
    }
    update();
}

void Canvas::setAllShapesVisible(bool visible) {
    for (Shape &shape : m_shapes) {
        shape.visible = visible;
    }
    emit shapesChanged();
    update();
}

bool Canvas::hasSelection() const {
    return m_currentIndex >= 0 && m_currentIndex < m_shapes.size();
}

void Canvas::deleteCurrent() {
    if (!hasSelection()) {
        return;
    }
    m_shapes.removeAt(m_currentIndex);
    if (m_shapes.isEmpty()) {
        m_currentIndex = -1;
    } else if (m_currentIndex >= m_shapes.size()) {
        m_currentIndex = 0;
    }
    for (Shape &shape : m_shapes) {
        shape.selected = false;
    }
    if (m_currentIndex >= 0 && m_currentIndex < m_shapes.size()) {
        m_shapes[m_currentIndex].selected = true;
    }
    emit shapesChanged();
    emit selectionChanged(m_currentIndex);
    emit selectionSetChanged(selectionIndices());
    update();
}

bool Canvas::discardShapeAt(int index) {
    if (index < 0 || index >= m_shapes.size()) {
        return false;
    }

    m_shapes.removeAt(index);
    if (m_shapes.isEmpty()) {
        m_currentIndex = -1;
    } else if (m_currentIndex == index) {
        m_currentIndex = -1;
    } else if (m_currentIndex > index) {
        --m_currentIndex;
    }

    for (Shape &shape : m_shapes) {
        shape.selected = false;
    }
    // This is the rollback path for a just-created, still-unlabeled shape.
    // LabelMe removes that transient line without publishing a document
    // change, so MainWindow can restore the previous dirty state and history.
    emit selectionChanged(m_currentIndex);
    emit selectionSetChanged(selectionIndices());
    update();
    return true;
}

bool Canvas::copyCurrentTo(const QPointF &imagePoint) {
    if (!hasSelection()) {
        return false;
    }
    const QVector<int> selected = selectionIndices();
    if (selected.size() > 1) {
        QRectF bounds;
        for (int index : selected) {
            bounds = bounds.isValid() ? bounds.united(m_shapes[index].boundingRect())
                                      : m_shapes[index].boundingRect();
        }
        QPointF delta(imagePoint.x() - bounds.center().x(),
                      imagePoint.y() - bounds.center().y());
        delta = boundedDeltaForSelection(delta);

        QVector<int> duplicated;
        duplicated.reserve(selected.size());
        for (int index : selected) {
            Shape copy = m_shapes[index].copy();
            copy.moveBy(delta);
            copy.selected = false;
            m_shapes.push_back(copy);
            duplicated.push_back(m_shapes.size() - 1);
        }
        setSelectedIndices(duplicated);
        emit shapesChanged();
        update();
        return true;
    }
    Shape copy = m_shapes[m_currentIndex].copy();
    QPointF topLeft = boundedTopLeftForCenter(copy, imagePoint);
    copy.moveBy(topLeft - copy.boundingRect().topLeft());
    m_shapes.push_back(copy);
    selectIndex(m_shapes.size() - 1);
    emit shapesChanged();
    update();
    return true;
}

bool Canvas::moveCurrentTo(const QPointF &imagePoint) {
    if (!hasSelection()) {
        return false;
    }
    const QVector<int> selected = selectionIndices();
    if (selected.size() > 1) {
        QRectF bounds;
        for (int index : selected) {
            bounds = bounds.isValid() ? bounds.united(m_shapes[index].boundingRect())
                                      : m_shapes[index].boundingRect();
        }
        QPointF delta(imagePoint.x() - bounds.center().x(),
                      imagePoint.y() - bounds.center().y());
        delta = boundedDeltaForSelection(delta);
        for (int index : selected) {
            m_shapes[index].moveBy(delta);
        }
        emit shapesChanged();
        emit selectionChanged(m_currentIndex);
        emit selectionSetChanged(selectionIndices());
        update();
        return true;
    }
    QPointF topLeft = boundedTopLeftForCenter(m_shapes[m_currentIndex], imagePoint);
    m_shapes[m_currentIndex].moveBy(topLeft - m_shapes[m_currentIndex].boundingRect().topLeft());
    emit shapesChanged();
    emit selectionChanged(m_currentIndex);
    emit selectionSetChanged(selectionIndices());
    update();
    return true;
}

bool Canvas::hasPendingRightDrag() const {
    return m_rightDragCandidate && m_rightDragged && !m_rightDragIndices.isEmpty() &&
           m_rightDragShapes.size() == m_rightDragIndices.size();
}

bool Canvas::finishRightDrag(bool copy) {
    if (!hasPendingRightDrag()) {
        return false;
    }

    if (copy) {
        QVector<int> duplicated;
        duplicated.reserve(m_rightDragShapes.size());
        for (const Shape &draggedShape : m_rightDragShapes) {
            Shape committed = draggedShape.copy();
            committed.selected = false;
            m_shapes.push_back(committed);
            duplicated.push_back(m_shapes.size() - 1);
        }
        setSelectedIndices(duplicated);
    } else {
        for (int i = 0; i < m_rightDragIndices.size(); ++i) {
            const int shapeIndex = m_rightDragIndices.at(i);
            if (shapeIndex >= 0 && shapeIndex < m_shapes.size()) {
                Shape committed = m_rightDragShapes.at(i).copy();
                committed.selected = false;
                m_shapes[shapeIndex] = committed;
            }
        }
        setSelectedIndices(m_rightDragIndices);
    }

    m_rightDragCandidate = false;
    m_rightDragged = false;
    m_rightDragIndices.clear();
    m_rightDragShapes.clear();
    m_rightDragAppliedDelta = {};
    emit shapesChanged();
    update();
    return true;
}

void Canvas::cancelRightDrag() {
    const bool hadPreview = m_rightDragCandidate || !m_rightDragShapes.isEmpty();
    m_rightDragCandidate = false;
    m_rightDragged = false;
    m_rightDragIndices.clear();
    m_rightDragShapes.clear();
    m_rightDragAppliedDelta = {};
    if (hadPreview) {
        update();
    }
}

bool Canvas::insertPointAt(const QPointF &imagePoint) {
    const qreal topologyTolerance = qMax(4.0 / m_scale, m_epsilon / m_scale);
    for (int i = m_shapes.size() - 1; i >= 0; --i) {
        Shape &shape = m_shapes[i];
        if (!shape.visible) {
            continue;
        }
        if (shape.shapeType != QStringLiteral("polygon") && shape.shapeType != QStringLiteral("linestrip")) {
            continue;
        }
        const int insertionIndex = nearestPolygonEdge(shape, imagePoint, topologyTolerance);
        if (insertionIndex < 0 || !shape.insertPoint(insertionIndex, clampToPixmap(imagePoint))) {
            continue;
        }
        selectIndex(i);
        m_hoverVertex = insertionIndex;
        emit shapesChanged();
        emit selectionChanged(m_currentIndex);
        update();
        return true;
    }
    return false;
}

bool Canvas::canAddPointToEdge() const {
    return m_editing && m_hoverShape >= 0 && m_hoverShape < m_shapes.size() &&
           m_hoverEdge >= 0;
}

bool Canvas::canRemoveSelectedPoint() const {
    if (!m_editing || m_hoverShape < 0 || m_hoverShape >= m_shapes.size() || m_hoverVertex < 0) {
        return false;
    }
    const Shape &shape = m_shapes.at(m_hoverShape);
    return (shape.shapeType == QStringLiteral("polygon") ||
            shape.shapeType == QStringLiteral("linestrip")) &&
           shape.canRemovePoint();
}

bool Canvas::addPointToEdge() {
    if (!canAddPointToEdge()) {
        return false;
    }

    const int shapeIndex = m_hoverShape;
    const int insertionIndex = m_hoverEdge;
    Shape &shape = m_shapes[shapeIndex];
    if (!shape.insertPoint(insertionIndex, clampToPixmap(m_hoverImagePos))) {
        return false;
    }

    selectIndex(shapeIndex);
    m_hoverVertex = insertionIndex;
    m_hoverEdge = -1;
    emit edgeSelectionChanged(false);
    emit shapesChanged();
    emit selectionChanged(m_currentIndex);
    update();
    return true;
}

bool Canvas::removePointAt(const QPointF &imagePoint) {
    const qreal topologyTolerance = qMax(4.0 / m_scale, m_epsilon / m_scale);
    for (int i = m_shapes.size() - 1; i >= 0; --i) {
        Shape &shape = m_shapes[i];
        if (!shape.visible) {
            continue;
        }
        if (shape.shapeType != QStringLiteral("polygon") && shape.shapeType != QStringLiteral("linestrip")) {
            continue;
        }
        const int vertex = shape.nearestVertex(imagePoint, topologyTolerance);
        if (vertex < 0 || !shape.removePoint(vertex)) {
            continue;
        }
        selectIndex(i);
        m_hoverVertex = -1;
        emit shapesChanged();
        emit selectionChanged(m_currentIndex);
        update();
        return true;
    }
    return false;
}

bool Canvas::removeSelectedPoint() {
    const int shapeIndex = m_hoverShape;
    const int vertexIndex = m_hoverVertex;
    if (shapeIndex < 0 || shapeIndex >= m_shapes.size() || vertexIndex < 0) {
        return false;
    }

    Shape &shape = m_shapes[shapeIndex];
    if ((shape.shapeType != QStringLiteral("polygon") && shape.shapeType != QStringLiteral("linestrip")) ||
        !shape.removePoint(vertexIndex)) {
        return false;
    }

    selectIndex(shapeIndex);
    m_hoverShape = -1;
    m_hoverVertex = -1;
    emit shapesChanged();
    emit selectionChanged(m_currentIndex);
    update();
    return true;
}

void Canvas::paintEvent(QPaintEvent *) {
    QElapsedTimer frameTimer;
    frameTimer.start();
    QPainter painter(this);
    painter.fillRect(rect(), palette().base());
    if (m_pixmap.isNull()) {
        return;
    }

    painter.save();
    painter.scale(m_scale, m_scale);
    painter.translate(imageOriginOffset());
    painter.setRenderHint(QPainter::SmoothPixmapTransform, m_samplingMode == SamplingMode::Smooth);
    const bool usePreview = m_samplingMode == SamplingMode::FastNearest && m_scale < 0.25;
    if (usePreview && !m_adjustedPreviewImage.isNull()) {
        painter.drawImage(QRectF(QPointF(0, 0), coordinateImageSize()), m_adjustedPreviewImage);
    } else if (usePreview && !m_previewImage.isNull()) {
        painter.drawImage(QRectF(QPointF(0, 0), coordinateImageSize()), m_previewImage);
    } else if (!m_adjustedImage.isNull()) {
        painter.drawImage(QRectF(QPointF(0, 0), coordinateImageSize()), m_adjustedImage);
    } else {
        painter.drawPixmap(QRectF(QPointF(0, 0), coordinateImageSize()), m_pixmap, m_pixmap.rect());
    }

    for (int i = 0; i < m_shapes.size(); ++i) {
        const Shape &shape = m_shapes[i];
        if (!shape.visible) {
            continue;
        }
        QRectF box = shape.boundingRect();
        const bool selected = shape.selected;
        const bool hovered = m_hoverShape == i;
        QPen pen(selected ? (i == m_currentIndex ? m_selectedLineColor : shape.lineColor) : shape.lineColor,
                 selected ? selectedOutlineWidth(m_scale, box) : 1.5 / m_scale);
        painter.setPen(pen);
        if (!selected && !hovered) {
            painter.setBrush(Qt::NoBrush);
        } else {
            QColor fill = shape.fillColor;
            if (selected && m_selectedFillColor.isValid()) {
                fill = m_selectedFillColor;
            } else if (selected) {
                // LabelMe's committed-shape palette uses a stronger fill for
                // selected shapes; draft colors are configured separately.
                fill.setAlpha(155);
            } else if (fill.alpha() <= 0) {
                fill.setAlpha(128);
            }
            painter.setBrush(fill);
        }
        if (shape.shapeType == QStringLiteral("polygon") && shape.points.size() >= 3) {
            painter.drawPolygon(QPolygonF(shape.points));
        } else if (shape.shapeType == QStringLiteral("oriented_rectangle") && shape.points.size() >= 4) {
            painter.drawPolygon(QPolygonF(shape.points));
            const QVector<QPointF> arrow = orientedRectangleArrowPoints(shape);
            if (arrow.size() == 4) {
                QPainterPath arrowPath;
                arrowPath.moveTo(arrow[0]);
                arrowPath.lineTo(arrow[1]);
                arrowPath.lineTo(arrow[2]);
                arrowPath.moveTo(arrow[3]);
                arrowPath.lineTo(arrow[1]);
                painter.save();
                painter.setPen(QPen(vertexColorForShape(shape, m_vertexFillColor), 2.0 / m_scale));
                painter.setBrush(Qt::NoBrush);
                painter.drawPath(arrowPath);
                painter.restore();
            }
        } else if ((shape.shapeType == QStringLiteral("point") ||
                    shape.shapeType == QStringLiteral("points")) && !shape.points.isEmpty()) {
            const qreal radius = pointMarkerRadius(m_pointSize, m_scale);
            // LabelMe's point vertices use the shape palette color, while
            // selection changes the outline/fill of the shape itself. Keep
            // selected point markers on the label color instead of turning
            // them into the white selected-line color.
            const QColor normalPointColor = vertexColorForShape(shape, m_vertexFillColor);
            for (int pointIndex = 0; pointIndex < shape.points.size(); ++pointIndex) {
                const QPointF &point = shape.points.at(pointIndex);
                const bool negativePrompt = pointIndex >= 0 && pointIndex < shape.pointLabels.size() &&
                                             shape.pointLabels.at(pointIndex) == 0;
                const bool hoveredPoint = selected && m_hoverShape == i && m_hoverVertex == pointIndex;
                const QColor pointColor = hoveredPoint
                                               ? m_hoverVertexFillColor
                                               : (negativePrompt ? QColor(255, 0, 0, 255) : normalPointColor);
                painter.setPen(QPen(pointColor,
                                    selected ? selectedOutlineWidth(m_scale, box) : 1.5 / m_scale));
                painter.setBrush(pointColor);
                painter.drawEllipse(point, radius, radius);
            }
        } else if (shape.shapeType == QStringLiteral("line") && shape.points.size() >= 2) {
            painter.drawLine(shape.points[0], shape.points[1]);
        } else if (shape.shapeType == QStringLiteral("linestrip") && shape.points.size() >= 2) {
            painter.drawPolyline(QPolygonF(shape.points));
        } else if (shape.shapeType == QStringLiteral("circle") && shape.points.size() >= 2) {
            const QPointF radiusVector = shape.points[1] - shape.points[0];
            const qreal radius = std::hypot(radiusVector.x(), radiusVector.y());
            painter.drawEllipse(shape.points[0], radius, radius);
        } else if (shape.shapeType == QStringLiteral("mask") && !shape.points.isEmpty()) {
            const QImage mask = decodeMaskData(shape.maskData);
            const QColor outlineColor = selected
                                            ? (i == m_currentIndex ? m_selectedLineColor : shape.lineColor)
                                            : shape.lineColor;
            const QPen outlinePen(outlineColor,
                                  selected ? selectedOutlineWidth(m_scale, box) : 1.5 / m_scale);
            if (mask.isNull()) {
                // LabelMe renders a mask with no bitmap payload as its
                // two-point bounding box so legacy/plugin shapes remain
                // visible and selectable.
                painter.setBrush(Qt::NoBrush);
                painter.setPen(outlinePen);
                painter.drawRect(box);
            } else {
                const QImage overlay = maskOverlayImage(shape, i == m_currentIndex ? 28 : 14);
                if (!overlay.isNull()) {
                    painter.drawImage(QRectF(shape.points.first(),
                                             QSizeF(overlay.width(), overlay.height())),
                                      overlay);
                }
                drawMaskBoundary(painter, shape, mask, outlinePen);
            }
        } else if (shape.shapeType != QStringLiteral("rectangle") && shape.points.size() >= 2) {
            QPainterPath path;
            path.moveTo(shape.points.first());
            for (int pointIndex = 1; pointIndex < shape.points.size(); ++pointIndex) {
                path.lineTo(shape.points.at(pointIndex));
            }
            if (shape.closed) {
                path.closeSubpath();
            }
            painter.drawPath(path);
        } else {
            painter.drawRect(box);
        }
        if (m_editing && i == m_currentIndex) {
            const bool hoveredVertex = m_hoverShape == i && m_hoverVertex >= 0;
            const QColor normalVertexColor = vertexColorForShape(shape, m_vertexFillColor);
            painter.setBrush(hoveredVertex ? m_hoverVertexFillColor : normalVertexColor);
            painter.setPen(QPen(Qt::white, 1.0 / m_scale));
            const qreal handle = vertexHandleRadius(m_scale, box);
            QVector<QPointF> handles;
            if (shape.shapeType != QStringLiteral("mask") && !isPointMarkerShape(shape)) {
                handles = isPointBackedShape(shape)
                              ? shape.points
                              : resizeHandlePoints(box, m_scale);
            }
            for (const QPointF &point : handles) {
                QRectF handleRect(point.x() - handle, point.y() - handle, handle * 2, handle * 2);
                painter.drawRect(handleRect);
            }
            if (shape.shapeType == QStringLiteral("oriented_rectangle")) {
                const bool hoveredRotation = m_hoverShape == i && m_hoverRotationHandle >= 0;
                painter.setBrush(hoveredRotation ? m_hoverVertexFillColor : normalVertexColor);
                painter.setPen(QPen(Qt::white, 1.0 / m_scale));
                const qreal rotationHandle = qMax(2.0 / m_scale, 3.0);
                for (const QPointF &point : rotationHandlePoints(shape)) {
                    painter.drawEllipse(point, rotationHandle, rotationHandle);
                }
            }
        }
        if (shape.paintLabel || m_paintLabels) {
            painter.setPen(Qt::yellow);
            painter.drawText(box.topLeft() + QPointF(2, -2), shape.label);
        }
    }
    if (hasPendingRightDrag()) {
        // Keep the LabelMe-style right-drag preview separate from committed
        // shapes. The menu decides whether this preview is copied or moved.
        QPen previewPen(QColor(255, 255, 255, 220), qMax(1.0, 1.5 / m_scale), Qt::DashLine);
        painter.setPen(previewPen);
        painter.setBrush(Qt::NoBrush);
        for (const Shape &shape : m_rightDragShapes) {
            if (!shape.visible) {
                continue;
            }
            const QRectF box = shape.boundingRect();
            if (shape.shapeType == QStringLiteral("polygon") && shape.points.size() >= 3) {
                painter.drawPolygon(QPolygonF(shape.points));
            } else if (shape.shapeType == QStringLiteral("oriented_rectangle") && shape.points.size() >= 4) {
                painter.drawPolygon(QPolygonF(shape.points));
            } else if ((shape.shapeType == QStringLiteral("point") ||
                        shape.shapeType == QStringLiteral("points")) && !shape.points.isEmpty()) {
                const qreal radius = pointMarkerRadius(m_pointSize, m_scale);
                for (const QPointF &point : shape.points) {
                    painter.drawEllipse(point, radius, radius);
                }
            } else if (shape.shapeType == QStringLiteral("line") && shape.points.size() >= 2) {
                painter.drawLine(shape.points[0], shape.points[1]);
            } else if (shape.shapeType == QStringLiteral("linestrip") && shape.points.size() >= 2) {
                painter.drawPolyline(QPolygonF(shape.points));
            } else if (shape.shapeType == QStringLiteral("circle") && shape.points.size() >= 2) {
                const QPointF radiusVector = shape.points[1] - shape.points[0];
                painter.drawEllipse(shape.points[0],
                                    std::hypot(radiusVector.x(), radiusVector.y()),
                                    std::hypot(radiusVector.x(), radiusVector.y()));
            } else {
                painter.drawRect(box);
            }
        }
    }
    if (m_polygonDrawing && !m_polygonPoints.isEmpty()) {
        painter.setPen(QPen(m_lineColor, 1.5 / m_scale, Qt::DashLine));
        painter.setBrush(Qt::NoBrush);
        QPolygonF preview(m_polygonPoints);
        if (m_fillDrawing && !isPointsCreateMode() && m_polygonPoints.size() >= 2) {
            preview << m_polygonPreviewPos;
            QColor fill = m_fillColor;
            fill.setAlpha(qMax(24, fill.alpha()));
            painter.setBrush(fill);
            painter.drawPolygon(preview);
            painter.setBrush(Qt::NoBrush);
        }
        if (m_polygonPoints.size() > 1 && !isPointsCreateMode()) {
            painter.drawPolyline(preview);
        }
        if (!isPointsCreateMode()) {
            painter.drawLine(m_polygonPoints.last(), m_polygonPreviewPos);
        }
        const qreal radius = pointMarkerRadius(m_pointSize, m_scale);
        const bool aiPoints = m_createShapeType == QStringLiteral("ai_points_to_shape");
        for (int i = 0; i < m_polygonPoints.size(); ++i) {
            QColor pointColor = m_vertexFillColor;
            if (aiPoints && i < m_promptPointLabels.size() && m_promptPointLabels[i] == 0) {
                pointColor = QColor(255, 0, 0, 255);
            }
            painter.setPen(QPen(pointColor, 1.5 / m_scale));
            painter.setBrush(pointColor);
            const QPointF &point = m_polygonPoints[i];
            painter.drawEllipse(point, radius, radius);
        }
    }
    if (m_orientedRectangleDrawing && !m_orientedRectanglePoints.isEmpty()) {
        painter.setPen(QPen(m_lineColor, 1.5 / m_scale, Qt::DashLine));
        painter.setBrush(Qt::NoBrush);
        if (m_orientedRectanglePoints.size() == 1) {
            painter.drawLine(m_orientedRectanglePoints.first(), m_polygonPreviewPos);
        } else {
            painter.drawPolygon(QPolygonF(orientedRectangleCorners(m_orientedRectanglePoints[0],
                                                                    m_orientedRectanglePoints[1],
                                                                    m_polygonPreviewPos)));
        }
        const qreal radius = pointMarkerRadius(m_pointSize, m_scale);
        painter.setBrush(m_vertexFillColor);
        for (const QPointF &point : m_orientedRectanglePoints) {
            painter.drawEllipse(point, radius, radius);
        }
    }
    if (m_maskDrawing && !m_maskCanvas.isNull()) {
        const QImage overlay = maskOverlayImage(m_maskCanvas, m_fillColor, 90);
        painter.drawImage(QRectF(QPointF(m_maskCanvasOrigin), QSizeF(overlay.size())), overlay);
    }
    if (m_createMode && crosshairEnabledForShapeType(m_createShapeType) && m_hasHoverImagePos) {
        const QRectF imageRect(QPointF(0, 0), QSizeF(coordinateImageSize()));
        const QPointF point = imageRect.contains(m_hoverImagePos)
                                  ? m_hoverImagePos
                                  : imageRect.center();
        QPen crosshairPen(QColor(255, 255, 255, 160), 1.0 / m_scale, Qt::DashLine);
        painter.setPen(crosshairPen);
        painter.drawLine(QPointF(imageRect.left(), point.y()), QPointF(imageRect.right(), point.y()));
        painter.drawLine(QPointF(point.x(), imageRect.top()), QPointF(point.x(), imageRect.bottom()));
    }
    painter.restore();
    emit frameRendered(frameTimer.nsecsElapsed() / 1000000.0);
}

void Canvas::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_polygonDrawing && m_doubleClickClose) {
        const bool aiPoints = m_createShapeType == QStringLiteral("ai_points_to_shape");
        const bool linestrip = m_createShapeType == QStringLiteral("linestrip");
        const bool points = m_createShapeType == QStringLiteral("points");
        const int minimumPoints = aiPoints ? 1 : (linestrip ? 2 : (points ? 3 : 3));
        const QPointF doubleClickPoint = clampToPixmap(imagePos(event->position()));
        if (m_polygonPoints.isEmpty() || m_polygonPoints.last() != doubleClickPoint) {
            m_polygonPoints.push_back(doubleClickPoint);
            if (aiPoints) {
                m_promptPointLabels.push_back(event->modifiers().testFlag(Qt::ShiftModifier) ? 0 : 1);
            }
        }
        if (m_polygonPoints.size() >= minimumPoints) {
            finishPolygonDrawing();
            event->accept();
            return;
        }
    }
    if (event->button() == Qt::LeftButton && m_orientedRectangleDrawing && m_doubleClickClose) {
        if (m_orientedRectanglePoints.size() == 2 &&
            m_polygonPreviewPos != m_orientedRectanglePoints.at(1)) {
            m_orientedRectanglePoints.push_back(m_polygonPreviewPos);
        }
        if (m_orientedRectanglePoints.size() >= 3) {
            finishOrientedRectangleDrawing();
            event->accept();
            return;
        }
    }
    QWidget::mouseDoubleClickEvent(event);
}

void Canvas::mousePressEvent(QMouseEvent *event) {
    setFocus();
    m_shapeDragStarted = false;
    m_shapeGeometryChanged = false;
    m_clickSelectedShape = -1;
    m_viewClickShape = -1;
    m_lastMousePos = event->position().toPoint();
    const QPointF rawImagePos = imagePos(event->position());
    m_startImagePos = clampToPixmap(rawImagePos);
    if (event->button() == Qt::LeftButton) {
        // LabelMe ignores a first drawing click in the scrollable slack around
        // an overflowing image. Keep clamping for an already active draft so
        // dragging from inside the image to its edge still finishes cleanly.
        const bool hasActiveDraft = m_creating || m_polygonDrawing ||
                                    m_orientedRectangleDrawing || m_maskDrawing;
        const bool outsidePixmap = !m_pixmap.isNull() &&
                                   (rawImagePos.x() < 0.0 || rawImagePos.y() < 0.0 ||
                                    rawImagePos.x() > coordinateImageSize().width() ||
                                    rawImagePos.y() > coordinateImageSize().height());
        if (m_createMode && !hasActiveDraft && outsidePixmap) {
            return;
        }
        if (m_editing && m_maskEditing && hasSelection() &&
            m_shapes[m_currentIndex].shapeType == QStringLiteral("mask")) {
            const Shape &shape = m_shapes[m_currentIndex];
            const QRectF box = shape.boundingRect();
            const QPoint origin(qRound(box.left()), qRound(box.top()));
            const QSize maskSize(qMax(1, qRound(box.width()) + 1),
                                 qMax(1, qRound(box.height()) + 1));
            QImage existingMask = decodeMaskData(shape.maskData);
            if (existingMask.isNull()) {
                existingMask = QImage(maskSize, QImage::Format_Grayscale8);
                existingMask.fill(0);
            } else if (existingMask.size() != maskSize) {
                existingMask = existingMask.scaled(maskSize,
                                                   Qt::IgnoreAspectRatio,
                                                   Qt::FastTransformation);
            }
            m_maskCanvas = existingMask;
            m_maskCanvasOrigin = origin;
            m_maskLastPoint = m_startImagePos;
            m_maskErase = event->modifiers().testFlag(Qt::ShiftModifier);
            m_maskEditingExisting = true;
            m_maskDrawing = true;
            drawMaskStroke(m_startImagePos, m_startImagePos, m_maskErase);
            update();
        } else if (m_editing) {
            const bool altPressed = event->modifiers().testFlag(Qt::AltModifier);
            const bool shiftPressed = event->modifiers().testFlag(Qt::ShiftModifier);
            if (altPressed) {
                bool vertexHit = false;
                if (!shiftPressed) {
                    const qreal vertexTolerance = qMax(4.0 / m_scale, m_epsilon / m_scale);
                    for (int i = m_shapes.size() - 1; i >= 0; --i) {
                        if (m_shapes[i].visible &&
                            m_shapes[i].nearestVertex(m_startImagePos, vertexTolerance) >= 0) {
                            // LabelMe resolves vertex hover for all shapes
                            // before considering an editable polygon edge.
                            vertexHit = true;
                            break;
                        }
                    }
                }
                if ((shiftPressed ? removePointAt(m_startImagePos)
                                  : (!vertexHit && insertPointAt(m_startImagePos)))) {
                    return;
                }
            }
            int index = -1;
            int pressedVertex = -1;
            int pressedRotationHandle = -1;
            ResizeHandle pressedHandle = ResizeHandle::None;
            const qreal vertexTolerance = qMax(4.0 / m_scale, m_epsilon / m_scale);

            // LabelMe resolves all vertices before considering any rotation,
            // resize, or shape hit. Keep that ordering global so a vertex on
            // a lower shape is not swallowed by a resize edge on a higher one.
            for (int i = m_shapes.size() - 1; i >= 0; --i) {
                if (!m_shapes[i].visible || m_shapes[i].shapeType == QStringLiteral("mask")) {
                    continue;
                }
                const int vertex = m_shapes[i].nearestVertex(m_startImagePos, vertexTolerance);
                if (vertex >= 0) {
                    index = i;
                    pressedVertex = vertex;
                    break;
                }
            }
            if (index < 0) {
                for (int i = m_shapes.size() - 1; i >= 0; --i) {
                    if (!m_shapes[i].visible || m_shapes[i].shapeType != QStringLiteral("oriented_rectangle")) {
                        continue;
                    }
                    const int rotationHandle = nearestRotationHandle(m_shapes[i],
                                                                     m_startImagePos,
                                                                     vertexTolerance);
                    if (rotationHandle >= 0) {
                        index = i;
                        pressedRotationHandle = rotationHandle;
                        break;
                    }
                }
            }
            if (index < 0) {
                for (int i = m_shapes.size() - 1; i >= 0; --i) {
                    if (!m_shapes[i].visible || isPointBackedShape(m_shapes[i])) {
                        continue;
                    }
                    pressedHandle = resizeHandleAt(m_shapes[i], m_startImagePos);
                    if (pressedHandle != ResizeHandle::None) {
                        index = i;
                        break;
                    }
                }
            }
            if (index < 0) {
                index = shapeAt(m_startImagePos);
            }
            if (index >= 0) {
                const bool additiveSelection = event->modifiers().testFlag(Qt::ControlModifier);
                if (additiveSelection) {
                    toggleSelectedIndex(index);
                    return;
                }
                m_shapeEditActive = true;
                emit shapeEditStarted();
                const bool wasSelected = m_shapes[index].selected;
                if (wasSelected) {
                    // Keep LabelMe's existing multi-selection intact while a
                    // selected shape is being clicked or dragged.
                    m_currentIndex = index;
                    emit selectionChanged(m_currentIndex);
                    emit selectionSetChanged(selectionIndices());
                } else {
                    selectIndex(index);
                }
                if (pressedVertex >= 0) {
                    m_hoverVertex = pressedVertex;
                    m_movingVertex = true;
                    setCursor(Qt::SizeAllCursor);
                } else if (pressedRotationHandle >= 0 && m_shapes[index].points.size() == 4) {
                    m_hoverRotationHandle = pressedRotationHandle;
                    m_rotating = true;
                    m_rotationCenter = (m_shapes[index].points[0] + m_shapes[index].points[2]) / 2.0;
                    const QPointF startVector = m_startImagePos - m_rotationCenter;
                    m_rotationInitialAngle = std::atan2(startVector.y(), startVector.x());
                    m_rotationOriginalPoints = m_shapes[index].points;
                    setCursor(Qt::SizeAllCursor);
                } else if (isPointBackedShape(m_shapes[index])) {
                    m_moving = true;
                    captureMoveAnchor(m_startImagePos);
                    if (wasSelected) {
                        m_clickSelectedShape = index;
                    }
                    setCursor(Qt::ClosedHandCursor);
                } else {
                    m_resizeHandle = pressedHandle != ResizeHandle::None ? pressedHandle : resizeHandleAt(m_shapes[index], m_startImagePos);
                    if (m_resizeHandle != ResizeHandle::None) {
                        m_resizing = true;
                        setResizeCursor(m_resizeHandle);
                    } else {
                        m_moving = true;
                        captureMoveAnchor(m_startImagePos);
                        if (wasSelected) {
                            m_clickSelectedShape = index;
                        }
                        setCursor(Qt::ClosedHandCursor);
                    }
                }
            } else {
                m_leftPanning = true;
                m_leftDragged = false;
                m_leftPressPos = event->position().toPoint();
                m_leftPressGlobalPos = event->globalPosition().toPoint();
            }
        } else if (m_createMode && (isPolygonCreateMode() || isPointsCreateMode() || isLinestripCreateMode())) {
            const bool aiPoints = m_createShapeType == QStringLiteral("ai_points_to_shape");
            const bool closePolygon = isPolygonCreateMode() &&
                                      m_snapping &&
                                      !m_altSnappingDisabled &&
                                      !event->modifiers().testFlag(Qt::AltModifier) &&
                                      m_polygonPoints.size() >= 3 &&
                                      std::hypot(m_startImagePos.x() - m_polygonPoints.first().x(),
                                                 m_startImagePos.y() - m_polygonPoints.first().y()) <
                                          (m_epsilon / qMax(0.001, m_scale));
            if (closePolygon) {
                m_polygonPreviewPos = m_polygonPoints.first();
                finishPolygonDrawing();
                return;
            }
            const bool wasDrawing = m_polygonDrawing;
            m_polygonDrawing = true;
            if (!wasDrawing) {
                emit drawingStateChanged(true);
            }
            m_polygonPreviewPos = m_startImagePos;
            m_polygonPoints.push_back(m_startImagePos);
            if (aiPoints) {
                m_promptPointLabels.push_back(event->modifiers().testFlag(Qt::ShiftModifier) ? 0 : 1);
            }
            const QString type = isPointsCreateMode()
                                     ? m_strings.get(QStringLiteral("canvasPoints"))
                                     : (isLinestripCreateMode()
                                            ? m_strings.get(QStringLiteral("canvasLinestrip"))
                                            : m_strings.get(QStringLiteral("canvasPolygon")));
            emit statusTextChanged(canvasPointsStatus(m_polygonPoints.size(), type, m_startImagePos));
            if ((aiPoints || isLinestripCreateMode()) &&
                event->modifiers().testFlag(Qt::ControlModifier)) {
                finishPolygonDrawing();
                return;
            }
            update();
        } else if (m_createMode && isOrientedRectangleCreateMode()) {
            const bool wasDrawing = m_orientedRectangleDrawing;
            m_orientedRectangleDrawing = true;
            if (!wasDrawing) {
                emit drawingStateChanged(true);
            }
            m_polygonPreviewPos = m_startImagePos;
            m_orientedRectanglePoints.push_back(m_startImagePos);
            if (m_orientedRectanglePoints.size() >= 3 && m_orientedRectangleDrawing) {
                finishOrientedRectangleDrawing();
            } else if (m_orientedRectangleDrawing) {
                emit statusTextChanged(canvasOrientedRectangleStatus(m_orientedRectanglePoints.size(), m_startImagePos));
                update();
            }
        } else if (m_createMode && m_createShapeType == QStringLiteral("mask")) {
            const bool wasDrawing = m_maskDrawing;
            m_maskDrawing = true;
            if (!wasDrawing) {
                emit drawingStateChanged(true);
            }
            m_maskCanvas = {};
            m_maskCanvasOrigin = {};
            m_maskLastPoint = m_startImagePos;
            drawMaskStroke(m_startImagePos, m_startImagePos);
            update();
        } else if (m_createMode && m_createShapeType == QStringLiteral("point")) {
            m_shapeEditActive = true;
            emit shapeEditStarted();
            Shape shape = Shape::fromPoints(QString(), QStringLiteral("point"), {m_startImagePos}, false);
            shape.lineColor = m_lineColor;
            shape.fillColor = m_fillColor;
            shape.paintLabel = m_paintLabels;
            m_shapes.push_back(shape);
            selectIndex(m_shapes.size() - 1);
            emit shapesChanged();
            emit shapeCreated(m_currentIndex);
            emit shapeEditFinished(true);
            m_shapeEditActive = false;
            emit selectionChanged(m_currentIndex);
            m_createMode = false;
            m_editing = true;
            emit editModeRequested();
            unsetCursor();
            update();
        } else if (m_createMode && m_retypedDraft && m_creating && hasSelection() &&
                   (m_createShapeType == QStringLiteral("rectangle") ||
                    m_createShapeType == QStringLiteral("line") ||
                    m_createShapeType == QStringLiteral("circle"))) {
            Shape &draft = m_shapes[m_currentIndex];
            const QPointF anchor = draft.points.isEmpty() ? m_startImagePos : draft.points.first();
            draft.points = {anchor, m_startImagePos};
            draft.pointLabels.fill(1, draft.points.size());
            draft.closed = m_createShapeType != QStringLiteral("line");
            m_retypedDraft = false;
            if (!m_shapeEditActive) {
                m_shapeEditActive = true;
                emit shapeEditStarted();
            }
            emit shapesChanged();
            update();
        } else if (m_createMode && (m_createShapeType == QStringLiteral("line") || m_createShapeType == QStringLiteral("circle"))) {
            const bool wasDrawing = m_creating;
            if (!wasDrawing) {
                m_shapeEditActive = true;
                emit shapeEditStarted();
            }
            m_creating = true;
            if (!wasDrawing) {
                emit drawingStateChanged(true);
            }
            Shape shape = Shape::fromPoints(QString(), m_createShapeType, {m_startImagePos, m_startImagePos}, false);
            shape.lineColor = m_lineColor;
            shape.fillColor = m_fillColor;
            shape.paintLabel = m_paintLabels;
            m_shapes.push_back(shape);
            selectIndex(m_shapes.size() - 1);
        } else if (m_createMode) {
            const bool wasDrawing = m_creating;
            if (!wasDrawing) {
                m_shapeEditActive = true;
                emit shapeEditStarted();
            }
            m_creating = true;
            if (!wasDrawing) {
                emit drawingStateChanged(true);
            }
            Shape shape = Shape::fromRect(QString(), QRectF(m_startImagePos, QSizeF(1, 1)), false);
            shape.lineColor = m_lineColor;
            shape.fillColor = m_fillColor;
            shape.paintLabel = m_paintLabels;
            m_shapes.push_back(shape);
            selectIndex(m_shapes.size() - 1);
        } else if (!m_editing && !m_createMode) {
            // View mode is read-only, but a shape can still be selected for
            // inspection. Keep the pan state separate so a drag never edits
            // geometry and a blank click clears the current selection.
            const int viewShape = shapeAt(m_startImagePos);
            if (viewShape >= 0) {
                selectIndex(viewShape);
                m_viewClickShape = viewShape;
            } else {
                selectIndex(-1);
            }
            m_leftPanning = true;
            m_leftDragged = false;
            m_leftPressPos = event->position().toPoint();
            m_leftPressGlobalPos = event->globalPosition().toPoint();
        } else {
            m_leftPanning = true;
            m_leftDragged = false;
            m_leftPressPos = event->position().toPoint();
            m_leftPressGlobalPos = event->globalPosition().toPoint();
        }
    } else if (event->button() == Qt::RightButton) {
        cancelRightDrag();
        m_rightPanning = true;
        m_rightDragged = false;
        m_rightPressPos = event->position().toPoint();
        m_rightPressGlobalPos = event->globalPosition().toPoint();
        if (m_editing && !m_createMode) {
            const int pressedShape = shapeAt(m_startImagePos);
            if (pressedShape >= 0) {
                if (!m_shapes.at(pressedShape).selected) {
                    selectIndex(pressedShape);
                } else {
                    m_currentIndex = pressedShape;
                    emit selectionChanged(m_currentIndex);
                    emit selectionSetChanged(selectionIndices());
                }
                m_rightDragIndices = selectionIndices();
                m_rightDragShapes.reserve(m_rightDragIndices.size());
                for (int index : m_rightDragIndices) {
                    if (index >= 0 && index < m_shapes.size()) {
                        Shape copy = m_shapes.at(index).copy();
                        copy.selected = false;
                        m_rightDragShapes.push_back(copy);
                    }
                }
                if (!m_rightDragIndices.isEmpty() &&
                    m_rightDragShapes.size() == m_rightDragIndices.size()) {
                    m_rightDragCandidate = true;
                    m_rightDragStartImagePos = m_startImagePos;
                    m_rightDragAppliedDelta = {};
                } else {
                    cancelRightDrag();
                }
            }
        }
    } else if (event->button() == Qt::MiddleButton) {
        m_middlePanning = true;
        m_middleDragged = false;
        m_middlePressGlobalPos = event->globalPosition().toPoint();
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
    QPoint current = event->position().toPoint();
    QPoint currentGlobal = event->globalPosition().toPoint();
    const QPointF rawCurrentImagePos = imagePos(event->position());
    QPointF currentImagePos = clampToPixmap(rawCurrentImagePos);
    const bool hoverPositionChanged = !m_hasHoverImagePos || m_hoverImagePos != currentImagePos;
    m_hoverImagePos = currentImagePos;
    m_hasHoverImagePos = true;

    if (m_polygonDrawing) {
        m_polygonPreviewPos = currentImagePos;
        if (isPolygonCreateMode() && m_snapping && !m_altSnappingDisabled &&
            !event->modifiers().testFlag(Qt::AltModifier) &&
            m_polygonPoints.size() >= 3 &&
            std::hypot(currentImagePos.x() - m_polygonPoints.first().x(),
                       currentImagePos.y() - m_polygonPoints.first().y()) <
                (m_epsilon / qMax(0.001, m_scale))) {
            m_polygonPreviewPos = m_polygonPoints.first();
        }
        const QString type = isPointsCreateMode()
                                 ? m_strings.get(QStringLiteral("canvasPoints"))
                                 : (isLinestripCreateMode()
                                        ? m_strings.get(QStringLiteral("canvasLinestrip"))
                                        : m_strings.get(QStringLiteral("canvasPolygon")));
        emit statusTextChanged(canvasPointsStatus(m_polygonPoints.size(), type, currentImagePos));
        update();
        return;
    }

    if (m_orientedRectangleDrawing) {
        m_polygonPreviewPos = currentImagePos;
        emit statusTextChanged(canvasOrientedRectangleStatus(m_orientedRectanglePoints.size(), currentImagePos));
        update();
        return;
    }

    if (m_maskDrawing) {
        const QPointF previousPoint = m_maskLastPoint;
        m_maskLastPoint = currentImagePos;
        drawMaskStroke(previousPoint, currentImagePos, m_maskErase);
        update();
        return;
    }

    if (m_rotating && hasSelection()) {
        const QVector<QPointF> previousPoints = m_shapes[m_currentIndex].points;
        rotateCurrentShape(currentImagePos);
        m_shapeDragStarted = true;
        if (m_shapes[m_currentIndex].points != previousPoints) {
            m_shapeGeometryChanged = true;
            emit shapesChanged();
        }
        update();
        return;
    }

    if (m_rightPanning) {
        if (!m_rightDragged && (currentGlobal - m_rightPressGlobalPos).manhattanLength() < QApplication::startDragDistance()) {
            return;
        }
        // LabelMe leaves a right-drag preview at its last valid image
        // position while the cursor is in the scrollable slack or outside
        // the pixmap. Clamping here would make Copy/Move previews jump to
        // the image edge and then jump back when the cursor re-enters.
        if (m_rightDragCandidate && !m_pixmap.isNull() &&
            (rawCurrentImagePos.x() < 0.0 || rawCurrentImagePos.y() < 0.0 ||
             rawCurrentImagePos.x() > coordinateImageSize().width() ||
             rawCurrentImagePos.y() > coordinateImageSize().height())) {
            update();
            return;
        }
        if (m_rightDragCandidate) {
            m_rightDragged = true;
            setCursor(Qt::ClosedHandCursor);
            const QPointF requestedDelta = currentImagePos - m_rightDragStartImagePos;
            const QPointF boundedDelta = boundedDeltaForSelection(requestedDelta);
            const QPointF incrementalDelta = boundedDelta - m_rightDragAppliedDelta;
            if (!qFuzzyIsNull(incrementalDelta.x()) || !qFuzzyIsNull(incrementalDelta.y())) {
                for (Shape &shape : m_rightDragShapes) {
                    shape.moveBy(incrementalDelta);
                }
                m_rightDragAppliedDelta = boundedDelta;
            }
            update();
            return;
        }
        if (!m_rightDragged) {
            m_rightDragged = true;
            m_lastMousePos = current;
            m_lastPanGlobalPos = currentGlobal;
            return;
        }
        QPoint delta = currentGlobal - m_lastPanGlobalPos;
        emit scrollRequested(delta.x(), delta.y());
        m_lastMousePos = current;
        m_lastPanGlobalPos = currentGlobal;
        return;
    }

    if (m_middlePanning) {
        if (!m_middleDragged && (currentGlobal - m_middlePressGlobalPos).manhattanLength() < QApplication::startDragDistance()) {
            return;
        }
        if (!m_middleDragged) {
            m_middleDragged = true;
            m_lastMousePos = current;
            m_lastPanGlobalPos = currentGlobal;
            setCursor(Qt::ClosedHandCursor);
            return;
        }
        const QPoint delta = currentGlobal - m_lastPanGlobalPos;
        emit scrollRequested(delta.x(), delta.y());
        m_lastMousePos = current;
        m_lastPanGlobalPos = currentGlobal;
        return;
    }

    if (m_leftPanning) {
        if (!m_leftDragged && (currentGlobal - m_leftPressGlobalPos).manhattanLength() < QApplication::startDragDistance()) {
            return;
        }
        if (!m_leftDragged) {
            m_leftDragged = true;
            m_lastMousePos = current;
            m_lastPanGlobalPos = currentGlobal;
            setCursor((!m_editing && !m_createMode) ? Qt::ArrowCursor : Qt::ClosedHandCursor);
            return;
        }
        QPoint delta = currentGlobal - m_lastPanGlobalPos;
        emit scrollRequested(delta.x(), delta.y());
        m_lastMousePos = current;
        m_lastPanGlobalPos = currentGlobal;
        return;
    }

    if (m_creating && hasSelection()) {
        const QString createdType = m_shapes[m_currentIndex].shapeType;
        QRectF rect(m_startImagePos, currentImagePos);
        if (createdType == QStringLiteral("line") || createdType == QStringLiteral("circle")) {
            m_shapes[m_currentIndex] = Shape::fromPoints(m_shapes[m_currentIndex].label,
                                                         createdType,
                                                         {m_startImagePos, currentImagePos},
                                                         m_shapes[m_currentIndex].difficult);
        } else {
            // Match LabelMe's creation gesture while retaining the persisted
            // draw-square setting used by the native workflow.
            if (m_drawSquare || event->modifiers().testFlag(Qt::ShiftModifier)) {
                qreal side = qMin(qAbs(rect.width()), qAbs(rect.height()));
                rect.setWidth(rect.width() < 0 ? -side : side);
                rect.setHeight(rect.height() < 0 ? -side : side);
            }
            m_shapes[m_currentIndex] = Shape::fromRect(m_shapes[m_currentIndex].label, rect.normalized(), m_shapes[m_currentIndex].difficult);
        }
        m_shapes[m_currentIndex].lineColor = m_lineColor;
        m_shapes[m_currentIndex].fillColor = m_fillColor;
        m_shapes[m_currentIndex].paintLabel = m_paintLabels;
        emit statusTextChanged(canvasSizeStatus(QRectF(0, 0, qAbs(rect.width()), qAbs(rect.height())), currentImagePos));
        if (!isTooSmallToKeep(m_shapes[m_currentIndex].boundingRect())) {
            emit shapesChanged();
        }
        update();
    } else if (m_resizing && hasSelection()) {
        const QVector<QPointF> previousPoints = m_shapes[m_currentIndex].points;
        resizeCurrentShape(m_resizeHandle,
                           currentImagePos,
                           event->modifiers().testFlag(Qt::ShiftModifier));
        m_shapeDragStarted = true;
        m_lastMousePos = current;
        QRectF box = m_shapes[m_currentIndex].boundingRect();
        emit statusTextChanged(canvasSizeStatus(box, currentImagePos));
        if (m_shapes[m_currentIndex].points != previousPoints) {
            m_shapeGeometryChanged = true;
            emit shapesChanged();
        }
        update();
    } else if (m_movingVertex && hasSelection() && m_hoverVertex >= 0) {
        const QVector<QPointF> previousPoints = m_shapes[m_currentIndex].points;
        Shape moved = m_shapes[m_currentIndex];
        // LabelMe projects an out-of-image vertex drag onto the image edge
        // along the segment from the original vertex to the cursor. Separate
        // X/Y clamping would place diagonal drags at a different point.
        const bool outsidePixmap = !m_pixmap.isNull() &&
                                    (rawCurrentImagePos.x() < 0.0 ||
                                     rawCurrentImagePos.y() < 0.0 ||
                                     rawCurrentImagePos.x() > coordinateImageSize().width() ||
                                     rawCurrentImagePos.y() > coordinateImageSize().height());
        QPointF target = outsidePixmap
                             ? intersectionWithPixmap(moved.points[m_hoverVertex],
                                                       rawCurrentImagePos,
                                                       coordinateImageSize())
                             : clampToPixmap(rawCurrentImagePos);
        QPointF delta = target - moved.points[m_hoverVertex];
        if (moved.shapeType == QStringLiteral("oriented_rectangle") && moved.points.size() == 4) {
            QVector<QPointF> corners = reprojectOrientedRectangleCorners(moved.points,
                                                                           m_hoverVertex,
                                                                           outsidePixmap ? rawCurrentImagePos : target,
                                                                           coordinateImageSize());
            m_shapes[m_currentIndex].points = corners;
        } else {
            if ((m_drawSquare || event->modifiers().testFlag(Qt::ShiftModifier)) &&
                moved.shapeType == QStringLiteral("rectangle") &&
                moved.points.size() == 2) {
                const int oppositeIndex = 1 - m_hoverVertex;
                const QPointF opposite = moved.points[oppositeIndex];
                const qreal side = qMin(qAbs(target.x() - opposite.x()),
                                        qAbs(target.y() - opposite.y()));
                target.setX(opposite.x() + (target.x() < opposite.x() ? -side : side));
                target.setY(opposite.y() + (target.y() < opposite.y() ? -side : side));
                delta = target - moved.points[m_hoverVertex];
            }
            m_shapes[m_currentIndex].moveVertexBy(m_hoverVertex, delta);
        }
        m_shapeDragStarted = true;
        m_lastMousePos = current;
        QRectF box = m_shapes[m_currentIndex].boundingRect();
        emit statusTextChanged(canvasSizeStatus(box, currentImagePos));
        if (m_shapes[m_currentIndex].points != previousPoints) {
            m_shapeGeometryChanged = true;
            emit shapesChanged();
        }
        update();
    } else if (m_moving && hasSelection()) {
        m_shapeDragStarted = true;
        // LabelMe keeps the grabbed point anchored while the union of the
        // selected shapes is clamped to the image. A raw per-frame delta would
        // release that anchor immediately after touching an image boundary.
        if (!m_pixmap.isNull() &&
            (rawCurrentImagePos.x() < 0.0 || rawCurrentImagePos.y() < 0.0 ||
             rawCurrentImagePos.x() > coordinateImageSize().width() ||
             rawCurrentImagePos.y() > coordinateImageSize().height())) {
            m_lastMousePos = current;
            update();
            return;
        }
        QPointF targetTopLeft = rawCurrentImagePos + m_moveAnchorOffset;
        if (!m_moveBounds.isValid()) {
            captureMoveAnchor(m_startImagePos);
        }
        if (!m_pixmap.isNull()) {
            const qreal maxX = qMax<qreal>(0.0, coordinateImageSize().width() - m_moveBounds.width());
            const qreal maxY = qMax<qreal>(0.0, coordinateImageSize().height() - m_moveBounds.height());
            targetTopLeft.setX(qBound<qreal>(0.0, targetTopLeft.x(), maxX));
            targetTopLeft.setY(qBound<qreal>(0.0, targetTopLeft.y(), maxY));
        }
        const QPointF effectiveCursor = targetTopLeft - m_moveAnchorOffset;
        QPointF delta = effectiveCursor - m_moveEffectiveCursor;
        const QVector<int> selected = selectionIndices();
        if (!delta.isNull()) {
            if (selected.size() > 1) {
                for (int index : selected) {
                    m_shapes[index].moveBy(delta);
                }
            } else {
                m_shapes[m_currentIndex].moveBy(delta);
            }
            m_shapeGeometryChanged = true;
            emit shapesChanged();
        }
        m_moveEffectiveCursor = effectiveCursor;
        m_lastMousePos = current;
        QRectF box = m_shapes[m_currentIndex].boundingRect();
        emit statusTextChanged(canvasSizeStatus(box, currentImagePos));
        emit shapesChanged();
        update();
    } else {
        if (!m_editing && !m_createMode) {
            setCursor(Qt::ArrowCursor);
            emit statusTextChanged(canvasPositionStatus(currentImagePos));
            return;
        }
        if (m_createMode && !m_editing) {
            setCursor(Qt::CrossCursor);
            emit statusTextChanged(canvasPositionStatus(currentImagePos));
            if (hoverPositionChanged && crosshairEnabledForShapeType(m_createShapeType)) {
                update();
            }
            return;
        }
        m_hoverShape = -1;
        const bool hadVertexSelection = m_hoverVertex >= 0;
        const bool hadEdgeSelection = m_hoverEdge >= 0;
        m_hoverVertex = -1;
        m_hoverEdge = -1;
        m_hoverRotationHandle = -1;
        m_hoverResizeHandle = ResizeHandle::None;
        const qreal vertexTolerance = qMax(4.0 / m_scale, m_epsilon / m_scale);

        // Keep hover priority identical to LabelMe: resolve every vertex
        // before looking at rotation handles or resize borders.
        for (int i = m_shapes.size() - 1; i >= 0; --i) {
            if (!m_shapes[i].visible || m_shapes[i].shapeType == QStringLiteral("mask")) {
                continue;
            }
            const int vertex = m_shapes[i].nearestVertex(currentImagePos, vertexTolerance);
            if (vertex >= 0) {
                m_hoverShape = i;
                m_hoverVertex = vertex;
                break;
            }
        }
        if (m_hoverShape < 0) {
            for (int i = m_shapes.size() - 1; i >= 0; --i) {
                if (!m_shapes[i].visible || m_shapes[i].shapeType != QStringLiteral("oriented_rectangle")) {
                    continue;
                }
                const int rotationHandle = nearestRotationHandle(m_shapes[i],
                                                                 currentImagePos,
                                                                 vertexTolerance);
                if (rotationHandle >= 0) {
                    m_hoverShape = i;
                    m_hoverRotationHandle = rotationHandle;
                    break;
                }
            }
        }
        if (m_hoverShape < 0) {
            for (int i = m_shapes.size() - 1; i >= 0; --i) {
                if (!m_shapes[i].visible || isPointBackedShape(m_shapes[i])) {
                    continue;
                }
                m_hoverResizeHandle = resizeHandleAt(m_shapes[i], currentImagePos);
                if (m_hoverResizeHandle != ResizeHandle::None) {
                    m_hoverShape = i;
                    break;
                }
            }
        }
        if (m_hoverShape < 0) {
            const qreal edgeTolerance = qMax(4.0 / m_scale, m_epsilon / m_scale);
            for (int i = m_shapes.size() - 1; i >= 0; --i) {
                const Shape &shape = m_shapes[i];
                if (!shape.visible) {
                    continue;
                }
                if (shape.shapeType != QStringLiteral("polygon") &&
                    shape.shapeType != QStringLiteral("linestrip")) {
                    continue;
                }
                const int edge = nearestPolygonEdge(shape, currentImagePos, edgeTolerance);
                if (edge < 0) {
                    continue;
                }
                m_hoverShape = i;
                m_hoverEdge = edge;
                setCursor(Qt::SizeAllCursor);
                setToolTip(m_strings.get(QStringLiteral("canvasEdgeTooltip")));
                emit statusTextChanged(canvasEdgeStatus(currentImagePos));
                break;
            }
        }
        if (m_hoverShape < 0) {
            m_hoverShape = shapeAt(currentImagePos);
        }
        if (m_hoverShape >= 0) {
            if (m_hoverVertex >= 0) {
                setCursor(Qt::SizeAllCursor);
                setToolTip(m_strings.get(QStringLiteral("canvasVertexTooltip")));
            } else if (m_hoverEdge >= 0) {
                setCursor(Qt::SizeAllCursor);
                setToolTip(m_strings.get(QStringLiteral("canvasEdgeTooltip")));
                emit statusTextChanged(canvasEdgeStatus(currentImagePos));
            } else if (m_hoverRotationHandle >= 0) {
                setCursor(Qt::OpenHandCursor);
                setToolTip(m_strings.get(QStringLiteral("canvasRotationTooltip")));
            } else if (m_hoverResizeHandle != ResizeHandle::None) {
                setResizeCursor(m_hoverResizeHandle);
                setToolTip(m_strings.get(QStringLiteral("canvasResizeTooltip")));
            } else {
                setCursor(Qt::OpenHandCursor);
                QRectF box = m_shapes[m_hoverShape].boundingRect();
                emit statusTextChanged(canvasSizeStatus(box, currentImagePos));
            }
        } else {
            setCursor(Qt::OpenHandCursor);
            emit statusTextChanged(canvasPositionStatus(currentImagePos));
        }
        if (hadEdgeSelection != (m_hoverEdge >= 0)) {
            emit edgeSelectionChanged(m_hoverEdge >= 0);
        }
        if (hadVertexSelection != (m_hoverVertex >= 0)) {
            emit vertexSelectionChanged(m_hoverVertex >= 0);
        }
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton && m_middlePanning) {
        m_middlePanning = false;
        m_middleDragged = false;
        if (m_createMode && !m_editing) {
            setCursor(Qt::CrossCursor);
        } else if (!m_createMode && !m_editing) {
            setCursor(Qt::ArrowCursor);
        } else {
            unsetCursor();
        }
        update();
        return;
    }
    if (m_polygonDrawing) {
        if (event->button() == Qt::RightButton) {
            finishPolygonDrawing();
        }
        return;
    }
    if (m_orientedRectangleDrawing) {
        return;
    }
    if (m_maskDrawing) {
        if (event->button() == Qt::LeftButton) {
            if (m_maskEditingExisting) {
                finishMaskEditing();
            } else {
                finishMaskDrawing();
            }
        }
        return;
    }
    m_rotating = false;
    m_hoverRotationHandle = -1;
    m_rotationOriginalPoints.clear();
    const bool wasCreating = m_creating;
    const bool shouldClearSelection = event->button() == Qt::LeftButton &&
                                      m_leftPanning && !m_leftDragged && !m_createMode &&
                                      m_viewClickShape < 0;
    const bool shouldToggleSelectedShape = event->button() == Qt::LeftButton &&
                                           m_clickSelectedShape >= 0 &&
                                           !m_shapeDragStarted;
    bool removedTinyCreate = false;
    bool changed = m_creating || m_shapeGeometryChanged;
    if (m_creating && hasSelection() && isTooSmallToKeepCreatedShape(m_shapes[m_currentIndex])) {
        m_shapes.removeAt(m_currentIndex);
        m_currentIndex = -1;
        removedTinyCreate = true;
        changed = false;
    }
    if (event->button() == Qt::RightButton && m_rightPanning) {
        const QPointF contextImagePosition = clampToPixmap(imagePos(event->position()));
        if (m_rightDragCandidate && m_rightDragged) {
            // Keep the dragged copies alive while MainWindow's synchronous
            // context menu decides between Copy here and Move here.
            emit contextMenuRequested(event->globalPosition().toPoint(), contextImagePosition);
        } else if (!m_rightDragged) {
            cancelRightDrag();
            const int contextShape = shapeAt(contextImagePosition);
            if (contextShape >= 0) {
                selectIndex(contextShape);
            }
            emit contextMenuRequested(event->globalPosition().toPoint(), contextImagePosition);
        }
    }
    m_creating = false;
    if (wasCreating) {
        emit drawingStateChanged(false);
    }
    m_moving = false;
    m_movingVertex = false;
    m_resizing = false;
    m_resizeHandle = ResizeHandle::None;
    m_leftPanning = false;
    m_leftDragged = false;
    m_viewClickShape = -1;
    m_rightPanning = false;
    m_middlePanning = false;
    m_middleDragged = false;
    if (changed) {
        emit shapesChanged();
    }
    if (wasCreating && !removedTinyCreate && hasSelection()) {
        emit shapeCreated(m_currentIndex);
    }
    if (shouldClearSelection) {
        selectIndex(-1);
    }
    if (shouldToggleSelectedShape && m_clickSelectedShape < m_shapes.size()) {
        toggleSelectedIndex(m_clickSelectedShape);
    }
    if (m_shapeEditActive) {
        emit shapeEditFinished(changed);
    }
    emit selectionChanged(m_currentIndex);
    if (wasCreating && !removedTinyCreate && hasSelection()) {
        m_createMode = false;
        m_editing = true;
        emit editModeRequested();
    }
    if (m_createMode && !m_editing) {
        setCursor(Qt::CrossCursor);
    } else if (!m_createMode && !m_editing) {
        setCursor(Qt::ArrowCursor);
    } else {
        unsetCursor();
    }
    m_shapeDragStarted = false;
    m_shapeGeometryChanged = false;
    m_clickSelectedShape = -1;
    m_shapeEditActive = false;
    update();
}

void Canvas::enterEvent(QEnterEvent *event) {
    syncCrosshairPosition(event->position());
    if (m_createMode) update();
    if (m_createMode && !m_editing) {
        setCursor(Qt::CrossCursor);
    } else if (!m_editing) {
        setCursor(Qt::ArrowCursor);
    } else {
        unsetCursor();
    }
    QWidget::enterEvent(event);
}

void Canvas::leaveEvent(QEvent *event) {
    m_hasHoverImagePos = false;
    const bool hadVertexSelection = m_hoverVertex >= 0;
    const bool hadEdgeSelection = m_hoverEdge >= 0;
    m_hoverShape = -1;
    m_hoverVertex = -1;
    m_hoverEdge = -1;
    m_hoverRotationHandle = -1;
    m_hoverResizeHandle = ResizeHandle::None;
    if (hadEdgeSelection) {
        emit edgeSelectionChanged(false);
    }
    if (hadVertexSelection) {
        emit vertexSelectionChanged(false);
    }
    QToolTip::hideText();
    if (m_createMode && !m_editing) {
        setCursor(Qt::CrossCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
    emit statusTextChanged(QString());
    update();
    QWidget::leaveEvent(event);
}

void Canvas::focusOutEvent(QFocusEvent *event) {
    m_altSnappingDisabled = false;
    setCursor(Qt::ArrowCursor);
    emit statusTextChanged(QString());
    QWidget::focusOutEvent(event);
}

void Canvas::wheelEvent(QWheelEvent *event) {
    if (event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) {
        setBrightness(m_brightness + event->angleDelta().y() / 120 * 5);
    } else if (event->modifiers() == Qt::ControlModifier) {
        const double oldScale = m_scale;
        // LabelMe applies wheel zoom through the integer zoom widget rather
        // than a continuous factor. Keep the same ceil/floor behavior as
        // the shortcut actions so wheel and keyboard zoom never diverge.
        const int currentPercent = qBound(1, qRound(m_scale * 100.0), 1600);
        const int targetPercent = event->angleDelta().y() > 0
                                      ? qBound(1, static_cast<int>(std::ceil(currentPercent * 1.1)), 1600)
                                      : qBound(1, static_cast<int>(std::floor(currentPercent * 0.9)), 1600);
        setScale(targetPercent / 100.0);
        emit scaleChanged(oldScale, m_scale, event->position().toPoint());
    } else if (event->modifiers() == Qt::ShiftModifier && event->angleDelta().x() == 0) {
        emit wheelScrollRequested(event->angleDelta().y(), 0);
    } else {
        emit wheelScrollRequested(event->angleDelta().x(), 0);
        emit wheelScrollRequested(0, event->angleDelta().y());
    }
    event->accept();
}

void Canvas::keyPressEvent(QKeyEvent *event) {
    if (m_createMode && event->key() == Qt::Key_Alt) {
        m_altSnappingDisabled = true;
        update();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        setViewMode();
        emit viewModeRequested();
        event->accept();
        return;
    }
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Z && undoLastDrawingPoint()) {
        return;
    }
    if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Backspace &&
        undoLastDrawingPoint()) {
        return;
    }
    if (!m_createMode && m_editing && event->modifiers() == Qt::ControlModifier &&
        event->key() == Qt::Key_A) {
        QVector<int> allIndices;
        allIndices.reserve(m_shapes.size());
        for (int i = 0; i < m_shapes.size(); ++i) {
            allIndices.push_back(i);
        }
        setSelectedIndices(allIndices);
        return;
    }
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ||
        event->key() == Qt::Key_Space) {
        if (m_polygonDrawing) {
            finishPolygonDrawing();
            return;
        }
        if (m_orientedRectangleDrawing) {
            if (m_orientedRectanglePoints.size() == 2 &&
                m_polygonPreviewPos != m_orientedRectanglePoints.at(1)) {
                m_orientedRectanglePoints.push_back(m_polygonPreviewPos);
            }
            if (m_orientedRectanglePoints.size() >= 3) {
                finishOrientedRectangleDrawing();
            }
            return;
        }
        if (m_creating) {
            if (!hasSelection() || isTooSmallToKeepCreatedShape(m_shapes[m_currentIndex])) {
                cancelDrawing();
                return;
            }
            m_creating = false;
            emit drawingStateChanged(false);
            emit shapesChanged();
            emit selectionChanged(m_currentIndex);
            update();
        }
        return;
    }
    if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Q) {
        emit previousShapeRequested();
        return;
    }
    if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_E) {
        emit nextShapeRequested();
        return;
    }
    if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_X) {
        emit deleteShapeRequested();
        return;
    }
    if (!m_createMode && m_editing &&
        ((event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Backspace) ||
         (event->modifiers() == Qt::MetaModifier && event->key() == Qt::Key_H))) {
        emit removeSelectedPointRequested();
        return;
    }
    // LabelMe only nudges selected geometry while the canvas is in edit mode.
    // A stale selection must not let arrow keys move an existing shape while
    // the user is creating a new one or viewing the image.
    if (m_createMode || !m_editing) {
        return;
    }
    if (!hasSelection()) {
        return;
    }
    QPointF delta;
    if (event->key() == Qt::Key_Left) delta = QPointF(-kLabelMeKeyboardMoveSpeed, 0);
    if (event->key() == Qt::Key_Right) delta = QPointF(kLabelMeKeyboardMoveSpeed, 0);
    if (event->key() == Qt::Key_Up) delta = QPointF(0, -kLabelMeKeyboardMoveSpeed);
    if (event->key() == Qt::Key_Down) delta = QPointF(0, kLabelMeKeyboardMoveSpeed);
    if (!delta.isNull()) {
        const QVector<int> selected = selectionIndices();
        delta = selected.size() > 1 ? boundedDeltaForSelection(delta)
                                    : boundedDeltaForMove(m_shapes[m_currentIndex], delta);
        if (!delta.isNull()) {
            for (const int index : selected) {
                if (index >= 0 && index < m_shapes.size()) {
                    m_shapes[index].moveBy(delta);
                }
            }
            emit shapesChanged();
        }
        update();
    }
}

void Canvas::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Alt) {
        m_altSnappingDisabled = false;
        update();
        event->accept();
        return;
    }
    QWidget::keyReleaseEvent(event);
}

QPointF Canvas::boundedTopLeftForCenter(const Shape &shape, const QPointF &center) const {
    QRectF box = shape.boundingRect();
    QPointF topLeft(center.x() - box.width() / 2.0, center.y() - box.height() / 2.0);
    if (!m_pixmap.isNull()) {
        topLeft.setX(qBound(0.0, topLeft.x(), static_cast<double>(coordinateImageSize().width()) - box.width()));
        topLeft.setY(qBound(0.0, topLeft.y(), static_cast<double>(coordinateImageSize().height()) - box.height()));
    }
    return topLeft;
}

QPointF Canvas::boundedDeltaForMove(const Shape &shape, const QPointF &delta) const {
    if (m_pixmap.isNull()) {
        return delta;
    }
    QRectF moved = shape.boundingRect().translated(delta);
    QPointF bounded = delta;
    if (moved.left() < 0) bounded.rx() -= moved.left();
    if (moved.top() < 0) bounded.ry() -= moved.top();
    if (moved.right() > coordinateImageSize().width()) bounded.rx() -= moved.right() - coordinateImageSize().width();
    if (moved.bottom() > coordinateImageSize().height()) bounded.ry() -= moved.bottom() - coordinateImageSize().height();
    return bounded;
}

QPointF Canvas::clampToPixmap(const QPointF &point) const {
    if (m_pixmap.isNull()) {
        return point;
    }
    return QPointF(qBound(0.0, point.x(), static_cast<double>(coordinateImageSize().width())),
                   qBound(0.0, point.y(), static_cast<double>(coordinateImageSize().height())));
}

void Canvas::drawMaskStroke(const QPointF &from, const QPointF &to, bool erase) {
    if (m_pixmap.isNull()) {
        return;
    }
    const QRect imageRect(QPoint(0, 0), coordinateImageSize());
    const QRect strokeRect = QRectF(from, QSizeF(0, 0)).toAlignedRect()
                                 .united(QRectF(to, QSizeF(0, 0)).toAlignedRect())
                                 .adjusted(-m_maskBrushRadius, -m_maskBrushRadius,
                                           m_maskBrushRadius, m_maskBrushRadius)
                                 .intersected(imageRect);
    if (strokeRect.isEmpty()) {
        return;
    }

    const QRect currentRect = m_maskCanvas.isNull()
                                  ? QRect()
                                  : QRect(m_maskCanvasOrigin, m_maskCanvas.size());
    const QRect targetRect = currentRect.isEmpty() ? strokeRect : currentRect.united(strokeRect);
    if (m_maskCanvas.isNull() || targetRect != currentRect) {
        QImage expanded(targetRect.size(), QImage::Format_Grayscale8);
        expanded.fill(0);
        if (!m_maskCanvas.isNull()) {
            QPainter copyPainter(&expanded);
            copyPainter.drawImage(m_maskCanvasOrigin - targetRect.topLeft(), m_maskCanvas);
        }
        m_maskCanvas = expanded;
        m_maskCanvasOrigin = targetRect.topLeft();
    }

    QPainter painter(&m_maskCanvas);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(QPen(erase ? Qt::black : Qt::white,
                        m_maskBrushRadius * 2.0,
                        Qt::SolidLine,
                        Qt::RoundCap,
                        Qt::RoundJoin));
    painter.drawLine(from - QPointF(m_maskCanvasOrigin), to - QPointF(m_maskCanvasOrigin));
    painter.drawEllipse(QPointF(to - QPointF(m_maskCanvasOrigin)), m_maskBrushRadius, m_maskBrushRadius);
}

void Canvas::finishMaskDrawing() {
    if (!m_maskDrawing || m_maskCanvas.isNull()) {
        cancelDrawing();
        return;
    }

    QRect content;
    for (int y = 0; y < m_maskCanvas.height(); ++y) {
        for (int x = 0; x < m_maskCanvas.width(); ++x) {
            if (qGray(m_maskCanvas.pixelColor(x, y).rgb()) > 0) {
                content = content.isValid() ? content.united(QRect(x, y, 1, 1)) : QRect(x, y, 1, 1);
            }
        }
    }
    if (!content.isValid()) {
        cancelDrawing();
        return;
    }

    const QImage cropped = m_maskCanvas.copy(content);
    QByteArray pngData;
    QBuffer buffer(&pngData);
    if (!buffer.open(QIODevice::WriteOnly) || !cropped.save(&buffer, "PNG")) {
        cancelDrawing();
        return;
    }

    const QPoint origin = m_maskCanvasOrigin + content.topLeft();
    const QPoint bottomRight = origin + QPoint(cropped.width() - 1, cropped.height() - 1);
    Shape shape = Shape::fromPoints(QString(),
                                    QStringLiteral("mask"),
                                    {QPointF(origin), QPointF(bottomRight)},
                                    false);
    shape.maskData = QString::fromLatin1(pngData.toBase64());
    shape.maskPresent = true;
    shape.lineColor = m_lineColor;
    shape.fillColor = m_fillColor;
    shape.paintLabel = m_paintLabels;
    m_shapes.push_back(shape);
    selectIndex(m_shapes.size() - 1);
    m_maskDrawing = false;
    m_maskCanvas = {};
    m_maskCanvasOrigin = {};
    emit drawingStateChanged(false);
    emit shapesChanged();
    emit shapeCreated(m_currentIndex);
    emit selectionChanged(m_currentIndex);
    m_createMode = false;
    m_editing = true;
    emit editModeRequested();
    unsetCursor();
    update();
}

void Canvas::finishMaskEditing() {
    if (!m_maskDrawing || !m_maskEditingExisting || !hasSelection() || m_maskCanvas.isNull()) {
        cancelDrawing();
        return;
    }

    QRect content;
    for (int y = 0; y < m_maskCanvas.height(); ++y) {
        for (int x = 0; x < m_maskCanvas.width(); ++x) {
            if (qGray(m_maskCanvas.pixelColor(x, y).rgb()) > 0) {
                content = content.isValid() ? content.united(QRect(x, y, 1, 1)) : QRect(x, y, 1, 1);
            }
        }
    }

    QImage cropped;
    QPoint origin = m_maskCanvasOrigin;
    if (content.isValid()) {
        cropped = m_maskCanvas.copy(content);
        origin += content.topLeft();
    } else {
        cropped = QImage(1, 1, QImage::Format_Grayscale8);
        cropped.fill(0);
    }

    const QString encoded = encodeMaskData(cropped);
    if (encoded.isEmpty()) {
        cancelDrawing();
        return;
    }

    Shape &shape = m_shapes[m_currentIndex];
    shape.maskData = encoded;
    shape.maskPresent = true;
    const QPoint bottomRight = origin + QPoint(cropped.width() - 1, cropped.height() - 1);
    shape.points = {QPointF(origin), QPointF(bottomRight)};

    m_maskDrawing = false;
    m_maskEditingExisting = false;
    m_maskErase = false;
    m_maskCanvas = {};
    m_maskCanvasOrigin = {};
    emit drawingStateChanged(false);
    emit shapesChanged();
    emit selectionChanged(m_currentIndex);
    unsetCursor();
    update();
}

int Canvas::nearestPolygonEdge(const Shape &shape, const QPointF &imagePoint, qreal epsilon) const {
    const bool polygon = shape.shapeType == QStringLiteral("polygon");
    const bool linestrip = shape.shapeType == QStringLiteral("linestrip");
    if ((!polygon && !linestrip) || shape.points.size() < 2) {
        return -1;
    }

    int insertionIndex = -1;
    qreal bestDistance = std::numeric_limits<qreal>::infinity();
    // LabelMe rolls the point list for linestrip edge lookup as well. This
    // exposes the last -> first edge for hover and insertion without closing
    // the rendered polyline.
    const int segmentCount = (polygon || linestrip)
                                 ? shape.points.size()
                                 : shape.points.size() - 1;
    for (int i = 0; i < segmentCount; ++i) {
        const int next = (i + 1) % shape.points.size();
        const qreal distance = distanceToSegment(imagePoint, shape.points[i], shape.points[next]);
        // LabelMe indexes an edge by its destination vertex: the closing
        // polygon edge (last -> first) therefore inserts at index 0. Keep
        // the first edge on exact ties, matching numpy.argmin.
        if (distance < bestDistance) {
            bestDistance = distance;
            insertionIndex = next;
        }
    }
    return bestDistance <= epsilon ? insertionIndex : -1;
}

int Canvas::nearestRotationHandle(const Shape &shape, const QPointF &imagePoint, qreal epsilon) const {
    const QVector<QPointF> handles = rotationHandlePoints(shape);
    int nearest = -1;
    qreal bestDistance = epsilon;
    for (int i = 0; i < handles.size(); ++i) {
        const QPointF delta = handles[i] - imagePoint;
        const qreal distance = std::hypot(delta.x(), delta.y());
        if (distance <= bestDistance) {
            bestDistance = distance;
            nearest = i;
        }
    }
    return nearest;
}

QPointF Canvas::imagePos(const QPointF &widgetPos) const {
    return widgetPos / m_scale - imageOriginOffset();
}

bool Canvas::isPolygonCreateMode() const {
    return m_createMode && m_createShapeType == QStringLiteral("polygon");
}

bool Canvas::isPointsCreateMode() const {
    return m_createMode && (m_createShapeType == QStringLiteral("points") ||
                            m_createShapeType == QStringLiteral("ai_points_to_shape"));
}

bool Canvas::isLinestripCreateMode() const {
    return m_createMode && m_createShapeType == QStringLiteral("linestrip");
}

bool Canvas::isOrientedRectangleCreateMode() const {
    return m_createMode && m_createShapeType == QStringLiteral("oriented_rectangle");
}

void Canvas::finishPolygonDrawing() {
    if (!m_polygonDrawing) {
        return;
    }
    const bool linestrip = m_createShapeType == QStringLiteral("linestrip");
    const bool points = m_createShapeType == QStringLiteral("points") ||
                        m_createShapeType == QStringLiteral("ai_points_to_shape");
    const int minimumPoints = points ? 1 : (linestrip ? 2 : 3);
    if (m_polygonPoints.size() < minimumPoints) {
        m_polygonDrawing = false;
        m_polygonPoints.clear();
        emit drawingStateChanged(false);
        update();
        return;
    }
    if ((!points && !linestrip && !hasMinimumDistinctPoints(m_polygonPoints, 3)) ||
        (linestrip && !hasMinimumDistinctPoints(m_polygonPoints, 2))) {
        m_polygonDrawing = false;
        m_polygonPoints.clear();
        m_promptPointLabels.clear();
        emit drawingStateChanged(false);
        unsetCursor();
        update();
        return;
    }

    Shape shape = points
                      ? Shape::fromPoints(QString(), QStringLiteral("points"), m_polygonPoints, false)
                      : (linestrip
                             ? Shape::fromPoints(QString(), QStringLiteral("linestrip"), m_polygonPoints, false)
                             : Shape::fromPolygon(QString(), m_polygonPoints, false));
    if (m_createShapeType == QStringLiteral("ai_points_to_shape")) {
        shape.pointLabels = m_promptPointLabels;
        if (shape.pointLabels.size() != shape.points.size()) {
            shape.pointLabels.fill(1, shape.points.size());
        }
    }
    shape.lineColor = m_lineColor;
    shape.fillColor = m_fillColor;
    shape.paintLabel = m_paintLabels;
    m_shapes.push_back(shape);
    selectIndex(m_shapes.size() - 1);
    m_polygonDrawing = false;
    m_polygonPoints.clear();
    emit drawingStateChanged(false);
    emit shapesChanged();
    emit shapeCreated(m_currentIndex);
    m_promptPointLabels.clear();
    emit selectionChanged(m_currentIndex);
    m_createMode = false;
    m_editing = true;
    emit editModeRequested();
    unsetCursor();
    update();
}

void Canvas::finishOrientedRectangleDrawing() {
    if (!m_orientedRectangleDrawing) {
        return;
    }
    if (m_orientedRectanglePoints.size() < 3) {
        m_orientedRectangleDrawing = false;
        m_orientedRectanglePoints.clear();
        emit drawingStateChanged(false);
        update();
        return;
    }
    if (m_orientedRectanglePoints[0] == m_orientedRectanglePoints[1] ||
        m_orientedRectanglePoints[1] == m_orientedRectanglePoints[2]) {
        m_orientedRectangleDrawing = false;
        m_orientedRectanglePoints.clear();
        emit drawingStateChanged(false);
        unsetCursor();
        update();
        return;
    }

    const QVector<QPointF> corners = m_orientedRectanglePoints.size() >= 4
                                         ? m_orientedRectanglePoints.mid(0, 4)
                                         : orientedRectangleCorners(m_orientedRectanglePoints[0],
                                                                    m_orientedRectanglePoints[1],
                                                                    m_orientedRectanglePoints[2]);
    Shape shape = Shape::fromPoints(QString(),
                                    QStringLiteral("oriented_rectangle"),
                                    corners,
                                    false);
    shape.lineColor = m_lineColor;
    shape.fillColor = m_fillColor;
    shape.paintLabel = m_paintLabels;
    m_shapes.push_back(shape);
    selectIndex(m_shapes.size() - 1);
    m_orientedRectangleDrawing = false;
    m_orientedRectanglePoints.clear();
    emit drawingStateChanged(false);
    emit shapesChanged();
    emit shapeCreated(m_currentIndex);
    emit selectionChanged(m_currentIndex);
    m_createMode = false;
    m_editing = true;
    emit editModeRequested();
    unsetCursor();
    update();
}

QRectF Canvas::widgetRect(const Shape &shape) const {
    QRectF rect = shape.boundingRect();
    return QRectF(rect.topLeft() * m_scale, QSizeF(rect.width() * m_scale, rect.height() * m_scale));
}

int Canvas::shapeAt(const QPointF &imagePoint) const {
    for (int i = m_shapes.size() - 1; i >= 0; --i) {
        const Shape &shape = m_shapes[i];
        if (!shape.visible) {
            continue;
        }
        if (shape.shapeType == QStringLiteral("points")) {
            const qreal vertexTolerance = qMax<qreal>(1.0, m_pointSize) /
                                          (2.0 * qMax<qreal>(0.001, m_scale));
            if (shape.nearestVertex(imagePoint, vertexTolerance) >= 0) {
                return i;
            }
            continue;
        }
        if (shape.hitTest(imagePoint, m_scale, m_epsilon, m_pointSize)) {
            return i;
        }
    }
    return -1;
}

Canvas::ResizeHandle Canvas::resizeHandleAt(const Shape &shape, const QPointF &imagePoint) const {
    // LabelMe edits point-backed shapes through their actual vertices. Only
    // the native axis-aligned rectangle uses the window-style border handles
    // used by this port; applying that geometry to circles, lines, or
    // oriented rectangles would corrupt their point representation.
    if (shape.shapeType != QStringLiteral("rectangle") ||
        isPointBackedShape(shape) ||
        shape.shapeType == QStringLiteral("mask")) {
        return ResizeHandle::None;
    }

    const QRectF box = shape.boundingRect();
    // Keep the handles visually compact while leaving a predictable 14px screen
    // target for precise border dragging, especially at low zoom levels.
    const qreal tolerance = qMax(14.0 / m_scale, m_epsilon / m_scale);
    const bool nearLeft = qAbs(imagePoint.x() - box.left()) <= tolerance;
    const bool nearRight = qAbs(imagePoint.x() - box.right()) <= tolerance;
    const bool nearTop = qAbs(imagePoint.y() - box.top()) <= tolerance;
    const bool nearBottom = qAbs(imagePoint.y() - box.bottom()) <= tolerance;
    const bool withinX = imagePoint.x() >= box.left() - tolerance && imagePoint.x() <= box.right() + tolerance;
    const bool withinY = imagePoint.y() >= box.top() - tolerance && imagePoint.y() <= box.bottom() + tolerance;

    if (nearLeft && nearTop) return ResizeHandle::TopLeft;
    if (nearRight && nearTop) return ResizeHandle::TopRight;
    if (nearLeft && nearBottom) return ResizeHandle::BottomLeft;
    if (nearRight && nearBottom) return ResizeHandle::BottomRight;
    if (nearLeft && withinY) return ResizeHandle::Left;
    if (nearRight && withinY) return ResizeHandle::Right;
    if (nearTop && withinX) return ResizeHandle::Top;
    if (nearBottom && withinX) return ResizeHandle::Bottom;
    return ResizeHandle::None;
}

void Canvas::resizeCurrentShape(ResizeHandle handle, const QPointF &imagePoint, bool square) {
    if (!hasSelection()) {
        return;
    }

    const Shape oldShape = m_shapes[m_currentIndex];
    if (oldShape.shapeType == QStringLiteral("mask")) {
        return;
    }
    QRectF box = oldShape.boundingRect();
    const QPointF point = clampToPixmap(imagePoint);
    constexpr qreal minSize = 3.0;

    auto setLeft = [&]() { box.setLeft(qMin(point.x(), box.right() - minSize)); };
    auto setRight = [&]() { box.setRight(qMax(point.x(), box.left() + minSize)); };
    auto setTop = [&]() { box.setTop(qMin(point.y(), box.bottom() - minSize)); };
    auto setBottom = [&]() { box.setBottom(qMax(point.y(), box.top() + minSize)); };

    switch (handle) {
    case ResizeHandle::Left:
        setLeft();
        break;
    case ResizeHandle::Right:
        setRight();
        break;
    case ResizeHandle::Top:
        setTop();
        break;
    case ResizeHandle::Bottom:
        setBottom();
        break;
    case ResizeHandle::TopLeft:
        setLeft();
        setTop();
        break;
    case ResizeHandle::TopRight:
        setRight();
        setTop();
        break;
    case ResizeHandle::BottomLeft:
        setLeft();
        setBottom();
        break;
    case ResizeHandle::BottomRight:
        setRight();
        setBottom();
        break;
    case ResizeHandle::None:
        return;
    }

    if (square && (handle == ResizeHandle::TopLeft ||
                   handle == ResizeHandle::TopRight ||
                   handle == ResizeHandle::BottomLeft ||
                   handle == ResizeHandle::BottomRight)) {
        QPointF opposite;
        QPointF corner;
        switch (handle) {
        case ResizeHandle::TopLeft:
            opposite = box.bottomRight();
            corner = box.topLeft();
            break;
        case ResizeHandle::TopRight:
            opposite = box.bottomLeft();
            corner = box.topRight();
            break;
        case ResizeHandle::BottomLeft:
            opposite = box.topRight();
            corner = box.bottomLeft();
            break;
        case ResizeHandle::BottomRight:
            opposite = box.topLeft();
            corner = box.bottomRight();
            break;
        default:
            break;
        }
        const qreal side = qMax(minSize,
                                qMin(qAbs(corner.x() - opposite.x()),
                                     qAbs(corner.y() - opposite.y())));
        if (handle == ResizeHandle::TopLeft) {
            box.setTopLeft(QPointF(opposite.x() - side, opposite.y() - side));
        } else if (handle == ResizeHandle::TopRight) {
            box.setTopRight(QPointF(opposite.x() + side, opposite.y() - side));
        } else if (handle == ResizeHandle::BottomLeft) {
            box.setBottomLeft(QPointF(opposite.x() - side, opposite.y() + side));
        } else if (handle == ResizeHandle::BottomRight) {
            box.setBottomRight(QPointF(opposite.x() + side, opposite.y() + side));
        }
    }

    const QRectF normalizedBox = box.normalized();
    Shape resized = oldShape;
    resized.points = Shape::fromRect(oldShape.label, normalizedBox, oldShape.difficult).points;
    m_shapes[m_currentIndex] = resized;
}

void Canvas::rotateCurrentShape(const QPointF &imagePoint) {
    if (!hasSelection() || m_rotationOriginalPoints.size() != 4 ||
        m_shapes[m_currentIndex].shapeType != QStringLiteral("oriented_rectangle")) {
        return;
    }
    const QPointF vector = imagePoint - m_rotationCenter;
    const qreal angle = std::atan2(vector.y(), vector.x()) - m_rotationInitialAngle;
    const qreal cosAngle = std::cos(angle);
    const qreal sinAngle = std::sin(angle);
    QVector<QPointF> rotated;
    rotated.reserve(m_rotationOriginalPoints.size());
    for (const QPointF &point : m_rotationOriginalPoints) {
        const QPointF relative = point - m_rotationCenter;
        const QPointF transformed(m_rotationCenter.x() + relative.x() * cosAngle - relative.y() * sinAngle,
                                  m_rotationCenter.y() + relative.x() * sinAngle + relative.y() * cosAngle);
        rotated.push_back(transformed);
    }
    m_shapes[m_currentIndex].points = rotated;
}

void Canvas::setResizeCursor(ResizeHandle handle) {
    switch (handle) {
    case ResizeHandle::Left:
    case ResizeHandle::Right:
        setCursor(Qt::SizeHorCursor);
        break;
    case ResizeHandle::Top:
    case ResizeHandle::Bottom:
        setCursor(Qt::SizeVerCursor);
        break;
    case ResizeHandle::TopLeft:
    case ResizeHandle::BottomRight:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case ResizeHandle::TopRight:
    case ResizeHandle::BottomLeft:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case ResizeHandle::None:
        setCursor(Qt::ArrowCursor);
        break;
    }
}

QVector<int> Canvas::selectionIndices() const {
    QVector<int> indices;
    for (int i = 0; i < m_shapes.size(); ++i) {
        if (m_shapes[i].selected) {
            indices.push_back(i);
        }
    }
    return indices;
}

QPointF Canvas::boundedDeltaForSelection(const QPointF &delta) const {
    const QVector<int> selected = selectionIndices();
    if (selected.isEmpty() || m_pixmap.isNull()) {
        return delta;
    }
    QRectF bounds;
    for (int index : selected) {
        if (index < 0 || index >= m_shapes.size()) {
            continue;
        }
        bounds = bounds.isValid() ? bounds.united(m_shapes[index].boundingRect())
                                  : m_shapes[index].boundingRect();
    }
    if (!bounds.isValid()) {
        return delta;
    }
    const QRectF moved = bounds.translated(delta);
    QPointF bounded = delta;
    if (moved.left() < 0) bounded.rx() -= moved.left();
    if (moved.top() < 0) bounded.ry() -= moved.top();
    if (moved.right() > coordinateImageSize().width()) bounded.rx() -= moved.right() - coordinateImageSize().width();
    if (moved.bottom() > coordinateImageSize().height()) bounded.ry() -= moved.bottom() - coordinateImageSize().height();
    return bounded;
}

void Canvas::captureMoveAnchor(const QPointF &cursor) {
    m_moveBounds = {};
    for (const int index : selectionIndices()) {
        if (index < 0 || index >= m_shapes.size()) {
            continue;
        }
        m_moveBounds = m_moveBounds.isValid()
                           ? m_moveBounds.united(m_shapes.at(index).boundingRect())
                           : m_shapes.at(index).boundingRect();
    }
    if (!m_moveBounds.isValid()) {
        m_moveAnchorOffset = {};
        m_moveEffectiveCursor = cursor;
        return;
    }
    m_moveAnchorOffset = m_moveBounds.topLeft() - cursor;
    m_moveEffectiveCursor = cursor;
}

void Canvas::toggleSelectedIndex(int index) {
    if (index < 0 || index >= m_shapes.size()) {
        return;
    }
    if (m_shapes[index].selected) {
        m_shapes[index].selected = false;
        if (m_currentIndex == index) {
            const QVector<int> remaining = selectionIndices();
            m_currentIndex = remaining.isEmpty() ? -1 : remaining.last();
        }
    } else {
        m_shapes[index].selected = true;
        m_currentIndex = index;
    }
    emit selectionChanged(m_currentIndex);
    emit selectionSetChanged(selectionIndices());
    update();
}

void Canvas::selectIndex(int index) {
    for (Shape &shape : m_shapes) {
        shape.selected = false;
    }
    m_currentIndex = (index >= 0 && index < m_shapes.size()) ? index : -1;
    if (hasSelection()) {
        m_shapes[m_currentIndex].selected = true;
    }
    emit selectionChanged(m_currentIndex);
    emit selectionSetChanged(selectionIndices());
    update();
}
