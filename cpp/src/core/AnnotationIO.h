#pragma once

#include "core/Shape.h"

#include <QRect>
#include <QByteArray>
#include <QJsonObject>
#include <QMap>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QVector>

struct AnnotationDocument {
    QString imagePath;
    QString imageData;
    QString labelMeVersion;
    bool imageDataRepaired = false;
    QString imageDataRepairMessage;
    QSize imageSize;
    int depth = 3;
    bool verified = false;
    QMap<QString, bool> topLevelFlags;
    QJsonObject labelMeOtherData;
    QVector<Shape> shapes;
};

class AnnotationIO {
public:
    static bool loadPascalVoc(const QString &path, AnnotationDocument *document);
    static bool savePascalVoc(const QString &path, const AnnotationDocument &document);

    static bool loadYolo(const QString &path, const QSize &imageSize, AnnotationDocument *document, const QString &classListPath = {});
    static bool saveYolo(const QString &path, const AnnotationDocument &document, QStringList classList = {});

    static bool loadCreateMl(const QString &path, const QString &imagePath, AnnotationDocument *document);
    static bool saveCreateMl(const QString &path, const AnnotationDocument &document);

    static bool loadLabelMe(const QString &path, AnnotationDocument *document,
                            QString *errorMessage = nullptr, bool repairImageData = false);
    static bool saveLabelMe(const QString &path, const AnnotationDocument &document);
    static bool loadImageData(const QString &path, QByteArray *data, QString *errorMessage = nullptr);

private:
    static QRect boundedRectFromShape(const Shape &shape);
};
