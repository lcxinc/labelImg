#pragma once

#include <QColor>
#include <QImage>
#include <QJsonObject>
#include <QMap>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QVector>

class Shape {
public:
    QString label;
    QString shapeType = QStringLiteral("rectangle");
    int groupId = -1;
    QString description;
    bool descriptionPresent = true;
    bool descriptionIsNull = false;
    QMap<QString, bool> flags;
    QVector<QPointF> points;
    QVector<int> pointLabels;
    QString maskData;
    bool maskPresent = true;
    QJsonObject labelMeOtherData;
    // LabelMe keeps this runtime flag separate from the serialized shape_type.
    // Loaded shapes are closed, while line and linestrip remain open.
    bool closed = false;
    QColor lineColor = QColor(0, 255, 0, 128);
    QColor fillColor = QColor(0, 0, 0, 64);
    bool selected = false;
    bool visible = true;
    bool difficult = false;
    bool paintLabel = false;

    static Shape fromRect(const QString &label, const QRectF &rect, bool difficult);
    static Shape fromPolygon(const QString &label, const QVector<QPointF> &points, bool difficult);
    static Shape fromPoints(const QString &label, const QString &shapeType, const QVector<QPointF> &points, bool difficult);

    QRectF boundingRect() const;
    // Rasterize the supported LabelMe primitive into an image-sized mask.
    // The result is grayscale 8-bit and null for unsupported or malformed
    // shape types (for example the multi-point "points" container).
    QImage toMask(const QSize &imageSize, int lineWidth = 10, int pointSize = 5) const;
    // Returns the larger of bounding-box IoU and intersection-over-smaller.
    // The second term catches nested detections that have low IoU.
    double overlapScore(const Shape &other) const;
    // LabelMe's duplicate rule uses separate IoU and containment thresholds.
    bool isRedundantWith(const Shape &other,
                         double iouThreshold,
                         double containmentThreshold = 0.85) const;
    bool contains(const QPointF &point) const;
    bool hitTest(const QPointF &point, qreal scale, qreal epsilon, int pointSize) const;
    int nearestVertex(const QPointF &point, qreal epsilon) const;
    bool rotate(const QPointF &center, qreal angle);
    bool canInsertPoint() const;
    bool insertPoint(int index, const QPointF &point, int pointLabel = 1);
    bool canRemovePoint() const;
    bool removePoint(int index);
    void moveBy(const QPointF &delta);
    void moveVertexBy(int index, const QPointF &delta);
    Shape copy() const;
    void setVisible(bool value);
};
