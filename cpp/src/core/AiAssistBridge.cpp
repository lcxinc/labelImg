#include "core/AiAssistBridge.h"

#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

#include <cmath>

namespace {
bool isFinitePoint(const QPointF &point) {
    return std::isfinite(point.x()) && std::isfinite(point.y());
}

bool setError(QString *error, const QString &message) {
    if (error) {
        *error = message;
    }
    return false;
}

bool parsePointArray(const QJsonValue &value, QVector<QPointF> *points, QString *error) {
    if (!value.isArray()) {
        return setError(error, QStringLiteral("AI shape points must be an array"));
    }
    const QJsonArray pointArray = value.toArray();
    QVector<QPointF> parsed;
    parsed.reserve(pointArray.size());
    for (const QJsonValue &pointValue : pointArray) {
        if (!pointValue.isArray()) {
            return setError(error, QStringLiteral("AI shape point must be an array"));
        }
        const QJsonArray pair = pointValue.toArray();
        if (pair.size() != 2 || !pair.at(0).isDouble() || !pair.at(1).isDouble()) {
            return setError(error, QStringLiteral("AI shape point must contain two numbers"));
        }
        const QPointF point(pair.at(0).toDouble(), pair.at(1).toDouble());
        if (!isFinitePoint(point)) {
            return setError(error, QStringLiteral("AI shape point must be finite"));
        }
        parsed.push_back(point);
    }
    *points = parsed;
    return true;
}

bool parseFlags(const QJsonValue &value, QMap<QString, bool> *flags, QString *error) {
    if (value.isUndefined() || value.isNull()) {
        return true;
    }
    if (!value.isObject()) {
        return setError(error, QStringLiteral("AI shape flags must be an object"));
    }
    QMap<QString, bool> parsed;
    const QJsonObject object = value.toObject();
    for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
        if (!it.value().isBool()) {
            return setError(error, QStringLiteral("AI shape flags must contain booleans"));
        }
        parsed.insert(it.key(), it.value().toBool());
    }
    *flags = parsed;
    return true;
}

bool validatePointCount(const QString &shapeType, int pointCount, QString *error) {
    int expectedCount = -1;
    int minimumCount = -1;
    if (shapeType == QStringLiteral("rectangle") ||
        shapeType == QStringLiteral("mask") ||
        shapeType == QStringLiteral("circle")) {
        expectedCount = 2;
    } else if (shapeType == QStringLiteral("oriented_rectangle")) {
        expectedCount = 4;
    } else if (shapeType == QStringLiteral("polygon")) {
        // LabelMe's AI polygon builder can emit a two-point degenerate path;
        // preserve that boundary while still rejecting an empty polygon.
        minimumCount = 2;
    }

    const bool valid = (expectedCount >= 0 && pointCount == expectedCount) ||
                       (minimumCount >= 0 && pointCount >= minimumCount);
    if (valid) {
        return true;
    }

    const QString expected = expectedCount >= 0
                                 ? QString::number(expectedCount)
                                 : QStringLiteral("at least %1").arg(minimumCount);
    return setError(error,
                    QStringLiteral("AI %1 point count must be %2 (got %3)")
                        .arg(shapeType, expected)
                        .arg(pointCount));
}
}

QJsonObject AiAssistBridge::requestObject(const AiPrompt &prompt) {
    QJsonObject object;
    object[QStringLiteral("image_path")] = prompt.imagePath;
    object[QStringLiteral("model")] = prompt.modelName;
    object[QStringLiteral("output_format")] = prompt.outputFormat;

    if (prompt.textPrompt) {
        object[QStringLiteral("prompt_type")] = QStringLiteral("text");
        QJsonArray texts;
        for (const QString &text : prompt.texts) {
            texts.append(text);
        }
        object[QStringLiteral("texts")] = texts;
        object[QStringLiteral("score_threshold")] = prompt.scoreThreshold;
        object[QStringLiteral("iou_threshold")] = prompt.iouThreshold;
    }

    QJsonArray points;
    for (const QPointF &point : prompt.points) {
        points.append(QJsonArray{point.x(), point.y()});
    }
    object[QStringLiteral("points")] = points;

    QJsonArray pointLabels;
    for (int label : prompt.pointLabels) {
        pointLabels.append(label);
    }
    object[QStringLiteral("point_labels")] = pointLabels;
    return object;
}

QByteArray AiAssistBridge::requestJson(const AiPrompt &prompt) {
    return QJsonDocument(requestObject(prompt)).toJson(QJsonDocument::Compact);
}

bool AiAssistBridge::parseResponse(const QByteArray &json, QVector<Shape> *shapes, QString *error) {
    if (!shapes) {
        return setError(error, QStringLiteral("AI output target is null"));
    }
    shapes->clear();

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return setError(error, QStringLiteral("AI response is not a JSON object"));
    }
    const QJsonObject root = document.object();
    if (root.contains(QStringLiteral("ok")) && !root.value(QStringLiteral("ok")).toBool()) {
        const QString message = root.value(QStringLiteral("error")).toString(QStringLiteral("AI inference failed"));
        return setError(error, message);
    }
    if (!root.value(QStringLiteral("shapes")).isArray()) {
        return setError(error, QStringLiteral("AI response does not contain shapes"));
    }

    const QJsonArray shapeArray = root.value(QStringLiteral("shapes")).toArray();
    const QStringList supported = supportedOutputFormats();
    QVector<Shape> parsedShapes;
    parsedShapes.reserve(shapeArray.size());
    for (const QJsonValue &shapeValue : shapeArray) {
        if (!shapeValue.isObject()) {
            return setError(error, QStringLiteral("AI shape must be an object"));
        }
        const QJsonObject object = shapeValue.toObject();
        const QString shapeType = object.value(QStringLiteral("shape_type")).toString();
        if (!supported.contains(shapeType)) {
            return setError(error, QStringLiteral("Unsupported AI shape type: %1").arg(shapeType));
        }

        Shape shape;
        shape.shapeType = shapeType;
        // LabelMe materializes every AI detection as a closed runtime shape,
        // including polygon-like plugin output that is not part of the core
        // shape-type list.
        shape.closed = true;
        shape.label = object.value(QStringLiteral("label")).toString();
        if (!parsePointArray(object.value(QStringLiteral("points")), &shape.points, error)) {
            return false;
        }
        if (shape.points.isEmpty()) {
            return setError(error, QStringLiteral("AI shape points must not be empty"));
        }
        // Match LabelMe Shape.__post_init__: ordinary runtime shapes get a
        // positive point label for every geometry point unless the transient
        // AI prompt explicitly supplies a different label set.
        shape.pointLabels.fill(1, shape.points.size());
        if (!validatePointCount(shapeType, shape.points.size(), error)) {
            return false;
        }

        const QJsonValue groupValue = object.value(QStringLiteral("group_id"));
        if (!groupValue.isUndefined() && !groupValue.isNull()) {
            if (!groupValue.isDouble() || std::floor(groupValue.toDouble()) != groupValue.toDouble()) {
                return setError(error, QStringLiteral("AI group_id must be an integer or null"));
            }
            shape.groupId = groupValue.toInt();
        }

        const QJsonValue descriptionValue = object.value(QStringLiteral("description"));
        shape.descriptionPresent = true;
        shape.descriptionIsNull = false;
        if (!descriptionValue.isUndefined() && !descriptionValue.isNull()) {
            if (!descriptionValue.isString()) {
                return setError(error, QStringLiteral("AI description must be a string or null"));
            }
            shape.description = descriptionValue.toString();
            shape.descriptionIsNull = false;
        }
        if (!parseFlags(object.value(QStringLiteral("flags")), &shape.flags, error)) {
            return false;
        }
        const QJsonValue maskValue = object.value(QStringLiteral("mask_data"));
        shape.maskPresent = object.contains(QStringLiteral("mask_data"));
        if (shapeType == QStringLiteral("mask") &&
            (maskValue.isUndefined() || maskValue.isNull() ||
             (maskValue.isString() && maskValue.toString().trimmed().isEmpty()))) {
            return setError(error, QStringLiteral("AI mask shape requires mask_data"));
        }
        if (!maskValue.isUndefined() && !maskValue.isNull()) {
            if (!maskValue.isString()) {
                return setError(error, QStringLiteral("AI mask_data must be a base64 string or null"));
            }
            const QString encoded = maskValue.toString();
            const QByteArray decoded = QByteArray::fromBase64(encoded.toLatin1());
            QImage mask;
            if (decoded.isEmpty() || !mask.loadFromData(decoded, "PNG")) {
                return setError(error, QStringLiteral("AI mask_data must be a base64-encoded PNG"));
            }
            if (shapeType == QStringLiteral("mask")) {
                const QRectF box = shape.boundingRect();
                const QSize expectedSize(qMax(1, qRound(box.width()) + 1),
                                         qMax(1, qRound(box.height()) + 1));
                if (mask.size() != expectedSize) {
                    return setError(error,
                                    QStringLiteral("AI mask dimensions %1x%2 do not match "
                                                   "bbox dimensions %3x%4")
                                        .arg(mask.width())
                                        .arg(mask.height())
                                        .arg(expectedSize.width())
                                        .arg(expectedSize.height()));
                }
            }
            shape.maskData = encoded;
        }
        if (object.value(QStringLiteral("other_data")).isObject()) {
            shape.labelMeOtherData = object.value(QStringLiteral("other_data")).toObject();
        }
        shape.difficult = shape.flags.value(QStringLiteral("difficult"), false);
        parsedShapes.push_back(shape);
    }

    *shapes = parsedShapes;
    return true;
}

QVector<Shape> AiAssistBridge::suppressOverlappingShapes(const QVector<Shape> &inferredShapes,
                                                         const QVector<Shape> &existingShapes,
                                                         double iouThreshold) {
    const double threshold = qBound(0.0, iouThreshold, 1.0);
    QVector<Shape> accepted;
    accepted.reserve(inferredShapes.size());

    for (const Shape &shape : inferredShapes) {
        bool duplicate = false;
        for (const Shape &existing : existingShapes) {
            if (existing.isRedundantWith(shape, threshold, 0.85)) {
                duplicate = true;
                break;
            }
        }
        if (!duplicate) {
            for (const Shape &peer : accepted) {
                if (peer.label != shape.label) {
                    continue;
                }
                if (peer.isRedundantWith(shape, threshold, 0.85)) {
                    duplicate = true;
                    break;
                }
            }
        }
        if (!duplicate) {
            accepted.push_back(shape);
        }
    }
    return accepted;
}

QStringList AiAssistBridge::supportedOutputFormats() {
    return {QStringLiteral("rectangle"),
            QStringLiteral("polygon"),
            QStringLiteral("mask"),
            QStringLiteral("oriented_rectangle"),
            QStringLiteral("circle")};
}
