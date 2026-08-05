#pragma once

#include "core/Shape.h"

#include <QByteArray>
#include <QJsonObject>
#include <QPointF>
#include <QString>
#include <QStringList>
#include <QVector>

struct AiPrompt {
    QString imagePath;
    QString modelName = QStringLiteral("sam2:latest");
    QString outputFormat = QStringLiteral("polygon");
    QVector<QPointF> points;
    QVector<int> pointLabels;
    bool textPrompt = false;
    QStringList texts;
    double scoreThreshold = 0.1;
    double iouThreshold = 0.5;
};

class AiAssistBridge {
public:
    static QJsonObject requestObject(const AiPrompt &prompt);
    static QByteArray requestJson(const AiPrompt &prompt);
    static bool parseResponse(const QByteArray &json, QVector<Shape> *shapes, QString *error = nullptr);
    static QVector<Shape> suppressOverlappingShapes(const QVector<Shape> &inferredShapes,
                                                    const QVector<Shape> &existingShapes,
                                                    double iouThreshold = 0.5);
    static QStringList supportedOutputFormats();
};
