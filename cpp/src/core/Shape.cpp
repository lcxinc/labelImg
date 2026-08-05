#include "core/Shape.h"

#include <QByteArray>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>

#include <algorithm>
#include <cmath>
#include <limits>

namespace {
constexpr qint64 kMaxRasterPixels = 64LL * 1024LL * 1024LL;

bool maskPixelIsSet(const QImage &mask, int x, int y) {
    if (mask.isNull() || x < 0 || y < 0 || x >= mask.width() || y >= mask.height()) {
        return false;
    }
    const QColor color = mask.pixelColor(x, y);
    if (color.alpha() == 0) {
        return false;
    }
    return qGray(color.rgb()) > 0;
}

bool isAreaShape(const Shape &shape) {
    if (shape.shapeType == QStringLiteral("rectangle")) {
        return shape.points.size() >= 2;
    }
    if (shape.shapeType == QStringLiteral("polygon")) {
        return shape.points.size() >= 3;
    }
    if (shape.shapeType == QStringLiteral("oriented_rectangle")) {
        return shape.points.size() >= 4;
    }
    if (shape.shapeType == QStringLiteral("circle")) {
        return shape.points.size() >= 2;
    }
    if (shape.shapeType == QStringLiteral("mask")) {
        return shape.points.size() >= 2;
    }
    return false;
}

QPainterPath areaPath(const Shape &shape) {
    QPainterPath path;
    if (shape.shapeType == QStringLiteral("rectangle")) {
        path.addRect(shape.boundingRect());
    } else if ((shape.shapeType == QStringLiteral("polygon") ||
                shape.shapeType == QStringLiteral("oriented_rectangle")) &&
               shape.points.size() >= 3) {
        path.addPolygon(QPolygonF(shape.points));
    } else if (shape.shapeType == QStringLiteral("circle") && shape.points.size() >= 2) {
        const QPointF radiusVector = shape.points[1] - shape.points[0];
        const qreal radius = std::hypot(radiusVector.x(), radiusVector.y());
        path.addEllipse(shape.points[0], radius, radius);
    } else if (shape.shapeType == QStringLiteral("mask")) {
        // A LabelMe mask without bitmap data is treated as its bounding box,
        // matching the reference suppression fallback.
        path.addRect(shape.boundingRect());
    } else if (shape.closed && shape.points.size() >= 3) {
        path.moveTo(shape.points.first());
        for (int i = 1; i < shape.points.size(); ++i) {
            path.lineTo(shape.points.at(i));
        }
        path.closeSubpath();
    }
    return path;
}

double polygonArea(const QPolygonF &polygon) {
    if (polygon.size() < 3) {
        return 0.0;
    }
    double twiceArea = 0.0;
    for (int i = 0; i < polygon.size(); ++i) {
        const QPointF &left = polygon.at(i);
        const QPointF &right = polygon.at((i + 1) % polygon.size());
        twiceArea += left.x() * right.y() - right.x() * left.y();
    }
    return std::abs(twiceArea) * 0.5;
}

double pathArea(const QPainterPath &path) {
    double area = 0.0;
    for (const QPolygonF &polygon : path.toFillPolygons()) {
        area += polygonArea(polygon);
    }
    return area;
}

QRect coverageRect(const Shape &shape) {
    const QRectF box = shape.boundingRect().normalized();
    const int left = qRound(box.left());
    const int top = qRound(box.top());
    const int right = qRound(box.right());
    const int bottom = qRound(box.bottom());
    return QRect(QPoint(left, top), QPoint(right, bottom));
}

bool decodeMask(const Shape &shape, QImage *mask) {
    if (!mask || shape.maskData.isEmpty()) {
        return false;
    }
    QImage decoded;
    if (!decoded.loadFromData(QByteArray::fromBase64(shape.maskData.toLatin1()), "PNG")) {
        return false;
    }
    *mask = decoded.convertToFormat(QImage::Format_Grayscale8);
    return !mask->isNull();
}

bool maskHasPixels(const QImage &image, int x, int y) {
    return image.pixelColor(x, y).value() > 0;
}

qint64 countMaskPixels(const QImage &image) {
    qint64 count = 0;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            count += maskHasPixels(image, x, y) ? 1 : 0;
        }
    }
    return count;
}

QImage rasterizeShape(const Shape &shape, const QRect &targetRect, const QImage &decodedMask) {
    const QSize size = targetRect.size();
    QImage result(size, QImage::Format_Grayscale8);
    result.fill(0);
    if (!decodedMask.isNull()) {
        const QRect sourceRect = coverageRect(shape);
        const QRect visibleSource = sourceRect.intersected(targetRect);
        if (!visibleSource.isEmpty()) {
            const QRect sourcePixels(visibleSource.topLeft() - sourceRect.topLeft(),
                                     visibleSource.size());
            QPainter painter(&result);
            painter.drawImage(visibleSource.topLeft() - targetRect.topLeft(),
                              decodedMask,
                              sourcePixels);
        }
        return result;
    }

    if (shape.shapeType == QStringLiteral("mask")) {
        // LabelMe treats a mask without decoded pixels as a full local bbox
        // for automation overlap checks.
        result.fill(255);
        return result;
    }

    Shape localShape = shape.copy();
    localShape.moveBy(-QPointF(targetRect.left(), targetRect.top()));
    const QImage localMask = localShape.toMask(size, 1, 5);
    if (!localMask.isNull()) {
        return localMask;
    }

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.translate(-targetRect.left(), -targetRect.top());
    painter.drawPath(areaPath(shape));
    return result;
}

struct OverlapMetrics {
    double iou = 0.0;
    double containment = 0.0;
};

OverlapMetrics boundingOverlapMetrics(const Shape &left, const Shape &right) {
    const QRectF leftRect = left.boundingRect().normalized();
    const QRectF rightRect = right.boundingRect().normalized();
    const double leftArea = leftRect.width() * leftRect.height();
    const double rightArea = rightRect.width() * rightRect.height();
    if (leftArea <= 0.0 || rightArea <= 0.0) {
        return {};
    }
    const QRectF intersection = leftRect.intersected(rightRect);
    const double intersectionArea = intersection.width() * intersection.height();
    if (intersectionArea <= 0.0) {
        return {};
    }
    const double unionArea = leftArea + rightArea - intersectionArea;
    const double iou = unionArea > 0.0 ? intersectionArea / unionArea : 0.0;
    const double containment = intersectionArea / std::min(leftArea, rightArea);
    return {iou, containment};
}

OverlapMetrics shapeOverlapMetrics(const Shape &left, const Shape &right) {
    if (!isAreaShape(left) || !isAreaShape(right)) {
        return {};
    }

    QImage leftMask;
    QImage rightMask;
    decodeMask(left, &leftMask);
    decodeMask(right, &rightMask);
    const QRect leftRect = coverageRect(left);
    const QRect rightRect = coverageRect(right);
    const QRect intersectionRect = leftRect.intersected(rightRect);
    if (intersectionRect.isEmpty()) {
        return {};
    }
    const qint64 leftPixels = static_cast<qint64>(leftRect.width()) * leftRect.height();
    const qint64 rightPixels = static_cast<qint64>(rightRect.width()) * rightRect.height();
    const qint64 intersectionPixels = static_cast<qint64>(intersectionRect.width()) *
                                      intersectionRect.height();
    if (leftPixels <= 0 || rightPixels <= 0 || intersectionPixels > kMaxRasterPixels ||
        leftPixels > kMaxRasterPixels || rightPixels > kMaxRasterPixels) {
        return boundingOverlapMetrics(left, right);
    }

    const QImage leftRaster = rasterizeShape(left, leftRect, leftMask);
    const QImage rightRaster = rasterizeShape(right, rightRect, rightMask);
    const QImage leftIntersection = rasterizeShape(left, intersectionRect, leftMask);
    const QImage rightIntersection = rasterizeShape(right, intersectionRect, rightMask);
    const qint64 leftArea = countMaskPixels(leftRaster);
    const qint64 rightArea = countMaskPixels(rightRaster);
    if (leftArea <= 0 || rightArea <= 0) {
        return {};
    }

    qint64 intersectionArea = 0;
    for (int y = 0; y < intersectionRect.height(); ++y) {
        for (int x = 0; x < intersectionRect.width(); ++x) {
            intersectionArea += maskHasPixels(leftIntersection, x, y) &&
                                         maskHasPixels(rightIntersection, x, y)
                                     ? 1
                                     : 0;
        }
    }
    if (intersectionArea <= 0) {
        return {};
    }
    const qint64 unionArea = leftArea + rightArea - intersectionArea;
    const double iou = unionArea > 0 ? static_cast<double>(intersectionArea) / unionArea : 0.0;
    const double containment = static_cast<double>(intersectionArea) /
                               static_cast<double>(std::min(leftArea, rightArea));
    return {iou, containment};
}

qreal distanceToSegment(const QPointF &point, const QPointF &start, const QPointF &end) {
    const QPointF segment = end - start;
    const qreal lengthSquared = QPointF::dotProduct(segment, segment);
    if (lengthSquared <= 0.0) {
        return std::hypot(point.x() - start.x(), point.y() - start.y());
    }
    const QPointF fromStart = point - start;
    const qreal projection = qBound<qreal>(
        0.0, QPointF::dotProduct(fromStart, segment) / lengthSquared, 1.0);
    const QPointF closest = start + segment * projection;
    return std::hypot(point.x() - closest.x(), point.y() - closest.y());
}
}

Shape Shape::fromRect(const QString &label, const QRectF &rect, bool difficult) {
    Shape shape;
    shape.label = label;
    shape.shapeType = QStringLiteral("rectangle");
    shape.difficult = difficult;
    shape.closed = true;
    // LabelMe stores axis-aligned rectangles as the two diagonal corners.
    // Keep that representation in memory as well; legacy four-corner input
    // remains supported by the editing and serialization paths below.
    const QRectF normalized = rect.normalized();
    shape.points = {normalized.topLeft(), normalized.bottomRight()};
    shape.pointLabels.fill(1, shape.points.size());
    return shape;
}

Shape Shape::fromPolygon(const QString &label, const QVector<QPointF> &points, bool difficult) {
    return Shape::fromPoints(label, QStringLiteral("polygon"), points, difficult);
}

Shape Shape::fromPoints(const QString &label, const QString &shapeType, const QVector<QPointF> &points, bool difficult) {
    Shape shape;
    shape.label = label;
    shape.shapeType = shapeType;
    shape.difficult = difficult;
    shape.closed = shapeType == QStringLiteral("polygon") ||
                   shapeType == QStringLiteral("rectangle") ||
                   shapeType == QStringLiteral("oriented_rectangle") ||
                   shapeType == QStringLiteral("circle") ||
                   shapeType == QStringLiteral("mask");
    shape.points = points;
    shape.pointLabels.fill(1, shape.points.size());
    return shape;
}

QRectF Shape::boundingRect() const {
    if (points.isEmpty()) {
        return {};
    }

    qreal minX = points[0].x();
    qreal minY = points[0].y();
    qreal maxX = points[0].x();
    qreal maxY = points[0].y();
    for (const QPointF &point : points) {
        minX = std::min(minX, point.x());
        minY = std::min(minY, point.y());
        maxX = std::max(maxX, point.x());
        maxY = std::max(maxY, point.y());
    }
    return QRectF(QPointF(minX, minY), QPointF(maxX, maxY)).normalized();
}

QImage Shape::toMask(const QSize &imageSize, int lineWidth, int pointSize) const {
    if (imageSize.isEmpty()) {
        return {};
    }

    const int safeLineWidth = qMax(1, lineWidth);
    const int safePointSize = qMax(1, pointSize);
    QImage mask(imageSize, QImage::Format_Grayscale8);
    mask.fill(0);

    QPainter painter(&mask);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(QPen(Qt::white, safeLineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::white);

    if (shapeType == QStringLiteral("rectangle")) {
        if (points.size() < 2) {
            return {};
        }
        const QRectF box = boundingRect();
        const QRect inclusiveBox(QPoint(qRound(box.left()), qRound(box.top())),
                                 QPoint(qRound(box.right()), qRound(box.bottom())));
        painter.fillRect(inclusiveBox, Qt::white);
        return mask;
    }

    if (shapeType == QStringLiteral("polygon")) {
        if (points.size() < 3) {
            return {};
        }
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(QPolygonF(points));
        return mask;
    }

    if (shapeType == QStringLiteral("oriented_rectangle")) {
        if (points.size() != 4) {
            return {};
        }
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(QPolygonF(points));
        return mask;
    }

    if (shapeType == QStringLiteral("circle")) {
        if (points.size() != 2) {
            return {};
        }
        const QPointF radiusVector = points.at(1) - points.at(0);
        const qreal radius = std::hypot(radiusVector.x(), radiusVector.y());
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(points.first(), radius, radius);
        return mask;
    }

    if (shapeType == QStringLiteral("line")) {
        if (points.size() != 2) {
            return {};
        }
        painter.drawLine(points.at(0), points.at(1));
        return mask;
    }

    if (shapeType == QStringLiteral("linestrip")) {
        if (points.size() < 2) {
            return {};
        }
        painter.drawPolyline(QPolygonF(points));
        return mask;
    }

    if (shapeType == QStringLiteral("point")) {
        if (points.size() != 1) {
            return {};
        }
        const qreal radius = safePointSize;
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(points.first(), radius, radius);
        return mask;
    }

    return {};
}

double Shape::overlapScore(const Shape &other) const {
    const OverlapMetrics metrics = shapeOverlapMetrics(*this, other);
    return std::max(metrics.iou, metrics.containment);
}

bool Shape::isRedundantWith(const Shape &other,
                            double iouThreshold,
                            double containmentThreshold) const {
    const OverlapMetrics metrics = shapeOverlapMetrics(*this, other);
    return (metrics.iou > 0.0 && metrics.iou >= iouThreshold) ||
           (metrics.containment > 0.0 && metrics.containment >= containmentThreshold);
}

bool Shape::contains(const QPointF &point) const {
    if (!visible) {
        return false;
    }
    if ((shapeType == QStringLiteral("polygon") && points.size() >= 3) ||
        (shapeType == QStringLiteral("oriented_rectangle") && points.size() >= 4)) {
        QPainterPath path;
        path.moveTo(points.first());
        for (int i = 1; i < points.size(); ++i) {
            path.lineTo(points[i]);
        }
        path.closeSubpath();
        return path.contains(point);
    }
    if ((shapeType == QStringLiteral("point") || shapeType == QStringLiteral("points")) && !points.isEmpty()) {
        for (const QPointF &candidate : points) {
            const QPointF delta = candidate - point;
            if (std::hypot(delta.x(), delta.y()) <= 6.0) {
                return true;
            }
        }
        return false;
    }
    if ((shapeType == QStringLiteral("line") || shapeType == QStringLiteral("linestrip")) && points.size() >= 2) {
        // LabelMe's nearest-edge helper rolls the point list for both line
        // and linestrip shapes. A linestrip is rendered open, but its final
        // edge (last point back to first point) is still part of hit testing.
        const int segmentCount = shapeType == QStringLiteral("linestrip")
                                     ? points.size()
                                     : points.size() - 1;
        for (int i = 0; i < segmentCount; ++i) {
            const int next = (i + 1) % points.size();
            const QPointF segment = points[next] - points[i];
            const qreal lengthSquared = segment.x() * segment.x() + segment.y() * segment.y();
            if (lengthSquared <= 0.0) {
                continue;
            }
            const QPointF fromStart = point - points[i];
            const qreal projection = std::clamp((fromStart.x() * segment.x() + fromStart.y() * segment.y()) / lengthSquared, 0.0, 1.0);
            const QPointF closest = points[i] + segment * projection;
            const QPointF delta = closest - point;
            if (std::hypot(delta.x(), delta.y()) <= 4.0) {
                return true;
            }
        }
        return false;
    }
    if (shapeType == QStringLiteral("circle") && points.size() >= 2) {
        const QPointF radiusVector = points[1] - points[0];
        const qreal radius = std::hypot(radiusVector.x(), radiusVector.y());
        const QPointF delta = point - points[0];
        const qreal distance = std::hypot(delta.x(), delta.y());
        return qAbs(distance - radius) <= 4.0 || distance < radius;
    }
    if (shapeType == QStringLiteral("mask") && !maskData.isEmpty() && !points.isEmpty()) {
        QImage mask;
        mask.loadFromData(QByteArray::fromBase64(maskData.toLatin1()), "PNG");
        const int rawX = qRound(point.x() - points.first().x());
        const int rawY = qRound(point.y() - points.first().y());
        return maskPixelIsSet(mask, rawX, rawY);
    }
    if (shapeType == QStringLiteral("mask") && points.size() == 2) {
        // LabelMe falls back to the mask bounding box when no bitmap payload
        // is present, which keeps legacy/plugin annotations selectable.
        return boundingRect().contains(point);
    }
    if (shapeType == QStringLiteral("rectangle")) {
        return points.size() >= 2 && boundingRect().contains(point);
    }
    if (points.size() >= 2 && closed) {
        QPainterPath path;
        path.moveTo(points.first());
        for (int i = 1; i < points.size(); ++i) {
            path.lineTo(points.at(i));
        }
        path.closeSubpath();
        return path.contains(point);
    }
    return false;
}

bool Shape::hitTest(const QPointF &point, qreal scale, qreal epsilon, int pointSize) const {
    if (!visible) {
        return false;
    }

    const qreal safeScale = qMax<qreal>(0.001, scale);
    const qreal screenEpsilon = qMax<qreal>(0.0, epsilon);
    if ((shapeType == QStringLiteral("line") || shapeType == QStringLiteral("linestrip")) &&
        points.size() >= 2) {
        const int segmentCount = shapeType == QStringLiteral("linestrip")
                                     ? points.size()
                                     : points.size() - 1;
        for (int i = 0; i < segmentCount; ++i) {
            const int next = (i + 1) % points.size();
            if (distanceToSegment(point, points[i], points[next]) * safeScale <= screenEpsilon) {
                return true;
            }
        }
        return false;
    }
    if (shapeType == QStringLiteral("points")) {
        // LabelMe uses the individual vertex hit test for points shapes and
        // never treats the multi-point container as a filled shape.
        return false;
    }
    if (shapeType == QStringLiteral("point")) {
        if (points.isEmpty()) {
            return false;
        }
        const qreal radius = qMax<qreal>(0.0, pointSize) / (2.0 * safeScale);
        const QPointF delta = points.first() - point;
        return std::hypot(delta.x(), delta.y()) <= radius;
    }
    return contains(point);
}

int Shape::nearestVertex(const QPointF &point, qreal epsilon) const {
    if (shapeType == QStringLiteral("mask")) {
        return -1;
    }
    int nearest = -1;
    qreal bestDistance = std::numeric_limits<qreal>::infinity();
    for (int i = 0; i < points.size(); ++i) {
        const QPointF delta = points[i] - point;
        const qreal distance = std::hypot(delta.x(), delta.y());
        if (distance < bestDistance) {
            bestDistance = distance;
            nearest = i;
        }
    }
    return bestDistance <= epsilon ? nearest : -1;
}

bool Shape::rotate(const QPointF &center, qreal angle) {
    if (shapeType != QStringLiteral("oriented_rectangle") || points.size() != 4) {
        return false;
    }

    const qreal cosAngle = std::cos(angle);
    const qreal sinAngle = std::sin(angle);
    for (QPointF &point : points) {
        const QPointF relative = point - center;
        point = QPointF(center.x() + relative.x() * cosAngle - relative.y() * sinAngle,
                        center.y() + relative.x() * sinAngle + relative.y() * cosAngle);
    }
    return true;
}

bool Shape::canInsertPoint() const {
    return shapeType == QStringLiteral("polygon") || shapeType == QStringLiteral("linestrip");
}

bool Shape::insertPoint(int index, const QPointF &point, int pointLabel) {
    if (!canInsertPoint()) {
        return false;
    }
    const int boundedIndex = std::clamp(index, 0, static_cast<int>(points.size()));
    if (pointLabels.size() != points.size()) {
        pointLabels.fill(1, points.size());
    }
    points.insert(points.begin() + boundedIndex, point);
    pointLabels.insert(boundedIndex, pointLabel);
    return true;
}

bool Shape::canRemovePoint() const {
    return (shapeType == QStringLiteral("polygon") && points.size() > 3) ||
           (shapeType == QStringLiteral("linestrip") && points.size() > 2);
}

bool Shape::removePoint(int index) {
    if (!canRemovePoint() || index < 0 || index >= points.size()) {
        return false;
    }
    points.removeAt(index);
    if (pointLabels.size() == points.size() + 1) {
        pointLabels.removeAt(index);
    } else {
        pointLabels.fill(1, points.size());
    }
    return true;
}

void Shape::moveBy(const QPointF &delta) {
    for (QPointF &point : points) {
        point += delta;
    }
}

void Shape::moveVertexBy(int index, const QPointF &delta) {
    if (index < 0 || index >= points.size()) {
        return;
    }

    if (shapeType == QStringLiteral("polygon") ||
        shapeType == QStringLiteral("point") ||
        shapeType == QStringLiteral("points") ||
        shapeType == QStringLiteral("line") ||
        shapeType == QStringLiteral("linestrip") ||
        shapeType == QStringLiteral("circle") ||
        shapeType == QStringLiteral("oriented_rectangle")) {
        points[index] += delta;
        return;
    }

    if (shapeType != QStringLiteral("rectangle") && shapeType != QStringLiteral("mask")) {
        points[index] += delta;
        return;
    }

    if (shapeType == QStringLiteral("rectangle") && points.size() == 2) {
        points[index] += delta;
        return;
    }

    if (points.size() != 4) {
        return;
    }

    points[index] += delta;
    const int leftIndex = (index + 1) % 4;
    const int rightIndex = (index + 3) % 4;
    if (index % 2 == 0) {
        points[leftIndex].ry() += delta.y();
        points[rightIndex].rx() += delta.x();
    } else {
        points[leftIndex].rx() += delta.x();
        points[rightIndex].ry() += delta.y();
    }
}

Shape Shape::copy() const {
    return *this;
}

void Shape::setVisible(bool value) {
    visible = value;
}
