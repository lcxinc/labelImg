#include "core/AnnotationIO.h"

#include "core/ImageIO.h"

#include <QDir>
#include <QBuffer>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageIOHandler>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStringConverter>
#include <QTextStream>

#include <cmath>
#include <limits>

namespace {
QByteArray labelMeJsonWithTwoSpaceIndent(const QJsonObject &root) {
    const QByteArray qtIndented = QJsonDocument(root).toJson(QJsonDocument::Indented);
    QList<QByteArray> lines = qtIndented.split('\n');
    for (QByteArray &line : lines) {
        int leadingSpaces = 0;
        while (leadingSpaces < line.size() && line.at(leadingSpaces) == ' ') {
            ++leadingSpaces;
        }
        if (leadingSpaces >= 4 && leadingSpaces % 4 == 0) {
            line = QByteArray(leadingSpaces / 2, ' ') + line.mid(leadingSpaces);
        }
    }
    QByteArray serialized = lines.join('\n');
    while (serialized.endsWith('\n') || serialized.endsWith('\r')) {
        serialized.chop(1);
    }
    return serialized;
}
}

QRect AnnotationIO::boundedRectFromShape(const Shape &shape) {
    QRect rect = shape.boundingRect().toAlignedRect();
    if (rect.left() < 1) rect.setLeft(1);
    if (rect.top() < 1) rect.setTop(1);
    return rect;
}

bool AnnotationIO::savePascalVoc(const QString &path, const AnnotationDocument &document) {
    QDomDocument xml;
    QDomElement root = xml.createElement("annotation");
    if (document.verified) {
        root.setAttribute("verified", "yes");
    }
    xml.appendChild(root);

    QFileInfo imageInfo(document.imagePath);
    auto appendText = [&](QDomElement parent, const QString &name, const QString &text) {
        QDomElement element = xml.createElement(name);
        element.appendChild(xml.createTextNode(text));
        parent.appendChild(element);
        return element;
    };

    appendText(root, "folder", imageInfo.dir().dirName());
    appendText(root, "filename", imageInfo.fileName());
    appendText(root, "path", document.imagePath);
    QDomElement source = xml.createElement("source");
    root.appendChild(source);
    appendText(source, "database", "Unknown");

    QDomElement size = xml.createElement("size");
    root.appendChild(size);
    appendText(size, "width", QString::number(document.imageSize.width()));
    appendText(size, "height", QString::number(document.imageSize.height()));
    appendText(size, "depth", QString::number(document.depth));
    appendText(root, "segmented", "0");

    for (const Shape &shape : document.shapes) {
        QRect rect = boundedRectFromShape(shape);
        QDomElement object = xml.createElement("object");
        root.appendChild(object);
        appendText(object, "name", shape.label);
        appendText(object, "pose", "Unspecified");
        bool truncated = rect.left() == 1 || rect.top() == 1 ||
                         rect.right() == document.imageSize.width() ||
                         rect.bottom() == document.imageSize.height();
        appendText(object, "truncated", truncated ? "1" : "0");
        appendText(object, "difficult", shape.difficult ? "1" : "0");
        QDomElement box = xml.createElement("bndbox");
        object.appendChild(box);
        appendText(box, "xmin", QString::number(rect.left()));
        appendText(box, "ymin", QString::number(rect.top()));
        appendText(box, "xmax", QString::number(rect.right()));
        appendText(box, "ymax", QString::number(rect.bottom()));
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    xml.save(stream, 1);
    return true;
}

bool AnnotationIO::loadPascalVoc(const QString &path, AnnotationDocument *document) {
    QFile file(path);
    if (!document || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QDomDocument xml;
    if (!xml.setContent(&file)) {
        return false;
    }

    QDomElement root = xml.documentElement();
    document->verified = root.attribute("verified") == "yes";
    document->imagePath = root.firstChildElement("path").text();
    int width = root.firstChildElement("size").firstChildElement("width").text().toInt();
    int height = root.firstChildElement("size").firstChildElement("height").text().toInt();
    document->depth = root.firstChildElement("size").firstChildElement("depth").text().toInt();
    document->imageSize = QSize(width, height);
    document->shapes.clear();

    QDomNodeList objects = root.elementsByTagName("object");
    for (int i = 0; i < objects.size(); ++i) {
        QDomElement object = objects.at(i).toElement();
        QString label = object.firstChildElement("name").text();
        bool difficult = object.firstChildElement("difficult").text().toInt() != 0;
        QDomElement box = object.firstChildElement("bndbox");
        int xMin = box.firstChildElement("xmin").text().toInt();
        int yMin = box.firstChildElement("ymin").text().toInt();
        int xMax = box.firstChildElement("xmax").text().toInt();
        int yMax = box.firstChildElement("ymax").text().toInt();
        document->shapes.push_back(Shape::fromRect(label, QRectF(xMin, yMin, xMax - xMin + 1, yMax - yMin + 1), difficult));
    }
    return true;
}

bool AnnotationIO::saveYolo(const QString &path, const AnnotationDocument &document, QStringList classList) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    for (const Shape &shape : document.shapes) {
        if (!classList.contains(shape.label)) {
            classList.append(shape.label);
        }
        int classIndex = classList.indexOf(shape.label);
        QRectF rect = shape.boundingRect();
        double xCenter = rect.center().x() / document.imageSize.width();
        double yCenter = rect.center().y() / document.imageSize.height();
        double width = rect.width() / document.imageSize.width();
        double height = rect.height() / document.imageSize.height();
        stream << classIndex << ' '
               << QString::number(xCenter, 'f', 6) << ' '
               << QString::number(yCenter, 'f', 6) << ' '
               << QString::number(width, 'f', 6) << ' '
               << QString::number(height, 'f', 6) << '\n';
    }

    QFile classFile(QFileInfo(path).dir().filePath("classes.txt"));
    if (!classFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream classStream(&classFile);
    classStream.setEncoding(QStringConverter::Utf8);
    for (const QString &className : classList) {
        classStream << className << '\n';
    }
    return true;
}

bool AnnotationIO::loadYolo(const QString &path, const QSize &imageSize, AnnotationDocument *document, const QString &classListPath) {
    if (!document) {
        return false;
    }
    QFile classFile(classListPath.isEmpty() ? QFileInfo(path).dir().filePath("classes.txt") : classListPath);
    QFile file(path);
    if (!classFile.open(QIODevice::ReadOnly | QIODevice::Text) || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QStringList classes = QString::fromUtf8(classFile.readAll()).split('\n', Qt::SkipEmptyParts);
    document->imageSize = imageSize;
    document->shapes.clear();
    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QStringList parts = stream.readLine().split(' ', Qt::SkipEmptyParts);
        if (parts.size() != 5) {
            continue;
        }
        int classIndex = parts[0].toInt();
        double xCenter = parts[1].toDouble();
        double yCenter = parts[2].toDouble();
        double width = parts[3].toDouble();
        double height = parts[4].toDouble();
        QRectF rect((xCenter - width / 2.0) * imageSize.width(),
                    (yCenter - height / 2.0) * imageSize.height(),
                    width * imageSize.width(),
                    height * imageSize.height());
        document->shapes.push_back(Shape::fromRect(classes.value(classIndex), rect, false));
    }
    return true;
}

bool AnnotationIO::saveCreateMl(const QString &path, const AnnotationDocument &document) {
    QJsonArray root;
    QFile existing(path);
    if (existing.open(QIODevice::ReadOnly | QIODevice::Text)) {
        root = QJsonDocument::fromJson(existing.readAll()).array();
    }

    QFileInfo imageInfo(document.imagePath);
    QJsonObject imageObject;
    imageObject["image"] = imageInfo.fileName();
    imageObject["verified"] = document.verified;
    QJsonArray annotations;
    for (const Shape &shape : document.shapes) {
        QRectF rect = shape.boundingRect();
        QJsonObject coordinates;
        coordinates["x"] = rect.center().x();
        coordinates["y"] = rect.center().y();
        coordinates["width"] = rect.width();
        coordinates["height"] = rect.height();
        QJsonObject annotation;
        annotation["label"] = shape.label;
        annotation["coordinates"] = coordinates;
        annotations.append(annotation);
    }
    imageObject["annotations"] = annotations;

    bool replaced = false;
    for (int i = 0; i < root.size(); ++i) {
        if (root[i].toObject().value("image").toString() == imageInfo.fileName()) {
            root.replace(i, imageObject);
            replaced = true;
            break;
        }
    }
    if (!replaced) {
        root.append(imageObject);
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    return true;
}

bool AnnotationIO::loadCreateMl(const QString &path, const QString &imagePath, AnnotationDocument *document) {
    QFile file(path);
    if (!document || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QJsonParseError parseError;
    const QJsonDocument parsed = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !parsed.isArray()) {
        return false;
    }
    const QJsonArray root = parsed.array();
    QString fileName = QFileInfo(imagePath).fileName();
    document->imagePath = imagePath;
    document->shapes.clear();
    for (const QJsonValue &value : root) {
        QJsonObject imageObject = value.toObject();
        if (imageObject.value("image").toString() != fileName) {
            continue;
        }
        document->verified = imageObject.value("verified").toBool(false);
        for (const QJsonValue &annotationValue : imageObject.value("annotations").toArray()) {
            QJsonObject annotation = annotationValue.toObject();
            QJsonObject coordinates = annotation.value("coordinates").toObject();
            double width = coordinates.value("width").toDouble();
            double height = coordinates.value("height").toDouble();
            QRectF rect(coordinates.value("x").toDouble() - width / 2.0,
                        coordinates.value("y").toDouble() - height / 2.0,
                        width,
                        height);
            document->shapes.push_back(Shape::fromRect(annotation.value("label").toString(), rect, true));
        }
        return true;
    }
    return true;
}

bool AnnotationIO::loadImageData(const QString &path, QByteArray *data, QString *errorMessage) {
    if (errorMessage) {
        errorMessage->clear();
    }
    auto fail = [errorMessage](const QString &message) {
        if (errorMessage) {
            *errorMessage = message;
        }
        return false;
    };
    if (!data) {
        return fail(QStringLiteral("image data output is null"));
    }
    data->clear();

    QImageReader reader(path);
    reader.setAutoTransform(true);
    const QImageIOHandler::Transformations transformation = reader.transformation();
    const QString suffix = QFileInfo(path).suffix().toLower();
    const bool isTiff = suffix == QStringLiteral("tif") || suffix == QStringLiteral("tiff");
    QString displayError;
    const QImage decodedImage = isTiff ? ImageIO::readForDisplay(path, &displayError) : reader.read();
    if (decodedImage.isNull()) {
        const QString detail = displayError.isEmpty() ? reader.errorString() : displayError;
        return fail(QStringLiteral("cannot decode image: %1").arg(detail));
    }
    const QImage image = ImageIO::normalizeForDisplay(decodedImage);

    const bool canKeepRaw = (suffix == QStringLiteral("jpg") || suffix == QStringLiteral("jpeg") ||
                             suffix == QStringLiteral("png")) &&
                            transformation == QImageIOHandler::TransformationNone;
    if (canKeepRaw) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            return fail(QStringLiteral("cannot read image: %1").arg(path));
        }
        *data = file.readAll();
        return true;
    }

    const bool hasAlpha = image.hasAlphaChannel();
    const QImage normalized = image.convertToFormat(hasAlpha ? QImage::Format_RGBA8888
                                                              : QImage::Format_RGB888);
    QBuffer buffer(data);
    if (!buffer.open(QIODevice::WriteOnly)) {
        return fail(QStringLiteral("cannot open encoded image buffer"));
    }
    const char *format = hasAlpha ? "PNG" : "JPEG";
    const int quality = hasAlpha ? -1 : 95;
    if (!normalized.save(&buffer, format, quality)) {
        return fail(QStringLiteral("cannot encode image as %1").arg(QString::fromLatin1(format)));
    }
    return true;
}

bool AnnotationIO::saveLabelMe(const QString &path, const AnnotationDocument &document) {
    const QStringList reservedTopLevelKeys = {
        QStringLiteral("version"),
        QStringLiteral("imageData"),
        QStringLiteral("imagePath"),
        QStringLiteral("shapes"),
        QStringLiteral("flags"),
        QStringLiteral("imageHeight"),
        QStringLiteral("imageWidth")};
    for (const QString &key : reservedTopLevelKeys) {
        if (document.labelMeOtherData.contains(key)) {
            return false;
        }
    }

    if (!document.imageData.isEmpty()) {
        QImage embeddedImage;
        if (!embeddedImage.loadFromData(QByteArray::fromBase64(document.imageData.toLatin1()))) {
            return false;
        }
        if ((document.imageSize.width() > 0 &&
             embeddedImage.width() != document.imageSize.width()) ||
            (document.imageSize.height() > 0 &&
             embeddedImage.height() != document.imageSize.height())) {
            return false;
        }
    }

    QJsonObject root;
    root["version"] = document.labelMeVersion.isEmpty() ? QStringLiteral("5.7.0") : document.labelMeVersion;
    QJsonObject flags;
    for (auto it = document.topLevelFlags.cbegin(); it != document.topLevelFlags.cend(); ++it) {
        if (it.key() == QStringLiteral("verified")) {
            continue;
        }
        flags[it.key()] = it.value();
    }
    if (document.verified) {
        flags["verified"] = true;
    }
    root["flags"] = flags;

    QJsonArray shapes;
    for (const Shape &shape : document.shapes) {
        QJsonArray points;
        const QString shapeType = shape.shapeType.isEmpty() ? QStringLiteral("rectangle") : shape.shapeType;
        // LabelMe keeps the source point list, including the order of a
        // rectangle's two diagonal corners. The legacy C++ shape model uses
        // four points for newly created axis-aligned boxes, so collapse that
        // internal representation to LabelMe's two-corner form.
        if (shapeType != QStringLiteral("rectangle") || shape.points.size() == 2) {
            for (const QPointF &point : shape.points) {
                QJsonArray pointArray;
                pointArray.append(point.x());
                pointArray.append(point.y());
                points.append(pointArray);
            }
        }
        if (points.isEmpty()) {
            const QRectF rect = shape.boundingRect().normalized();
            QJsonArray topLeft;
            topLeft.append(rect.left());
            topLeft.append(rect.top());
            QJsonArray bottomRight;
            bottomRight.append(rect.right());
            bottomRight.append(rect.bottom());
            points.append(topLeft);
            points.append(bottomRight);
        }

        QJsonObject shapeFlags;
        for (auto it = shape.flags.cbegin(); it != shape.flags.cend(); ++it) {
            shapeFlags[it.key()] = it.value();
        }
        if (shape.difficult) {
            shapeFlags["difficult"] = true;
        }

        QJsonObject shapeObject = shape.labelMeOtherData;
        for (const QString &key : {QStringLiteral("label"),
                                   QStringLiteral("points"),
                                   QStringLiteral("group_id"),
                                   QStringLiteral("description"),
                                   QStringLiteral("shape_type"),
                                   QStringLiteral("flags"),
                                   QStringLiteral("mask")}) {
            shapeObject.remove(key);
        }
        shapeObject["label"] = shape.label;
        shapeObject["points"] = points;
        shapeObject["group_id"] = shape.groupId >= 0 ? QJsonValue(shape.groupId) : QJsonValue(QJsonValue::Null);
        // LabelMe's in-memory description is always editable text. A legacy
        // null marker in a Shape therefore serializes as the normalized empty
        // string rather than reintroducing JSON null into the file.
        shapeObject["description"] = shape.descriptionIsNull ? QString() : shape.description;
        shapeObject["shape_type"] = shapeType;
        shapeObject["flags"] = shapeFlags;
        shapeObject["mask"] = shape.maskData.isEmpty() ? QJsonValue(QJsonValue::Null)
                                                        : QJsonValue(shape.maskData);
        shapes.append(shapeObject);
    }
    root["shapes"] = shapes;
    QString serializedImagePath = document.imagePath;
    if (QDir::isAbsolutePath(document.imagePath)) {
        const QDir outputDir(QFileInfo(path).absolutePath());
        serializedImagePath = QDir::fromNativeSeparators(outputDir.relativeFilePath(document.imagePath));
    } else {
        serializedImagePath = QDir::fromNativeSeparators(serializedImagePath);
    }
    root["imagePath"] = serializedImagePath;
    root["imageData"] = document.imageData.isEmpty() ? QJsonValue(QJsonValue::Null) : QJsonValue(document.imageData);
    root["imageHeight"] = document.imageSize.height() > 0
                               ? QJsonValue(document.imageSize.height())
                               : QJsonValue(QJsonValue::Null);
    root["imageWidth"] = document.imageSize.width() > 0
                              ? QJsonValue(document.imageSize.width())
                              : QJsonValue(QJsonValue::Null);
    for (auto it = document.labelMeOtherData.constBegin(); it != document.labelMeOtherData.constEnd(); ++it) {
        if (it.key() == QStringLiteral("version") ||
            it.key() == QStringLiteral("flags") ||
            it.key() == QStringLiteral("shapes") ||
            it.key() == QStringLiteral("imagePath") ||
            it.key() == QStringLiteral("imageData") ||
            it.key() == QStringLiteral("imageHeight") ||
            it.key() == QStringLiteral("imageWidth")) {
            continue;
        }
        root[it.key()] = it.value();
    }

    QSaveFile file(path);
    file.setDirectWriteFallback(true);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    const QByteArray serialized = labelMeJsonWithTwoSpaceIndent(root);
    auto writeDirect = [&]() {
        QFile direct(path);
        if (!direct.open(QIODevice::WriteOnly | QIODevice::Text) ||
            direct.write(serialized) != serialized.size()) {
            return false;
        }
        const bool flushed = direct.flush();
        direct.close();
        return flushed;
    };
    if (file.write(serialized) != serialized.size()) {
        return writeDirect();
    }
    if (file.commit()) {
        return true;
    }
    return writeDirect();
}

bool AnnotationIO::loadLabelMe(const QString &path, AnnotationDocument *document,
                               QString *errorMessage, bool repairImageData) {
    if (errorMessage) {
        errorMessage->clear();
    }
    auto fail = [errorMessage](const QString &message) {
        if (errorMessage) {
            *errorMessage = message;
        }
        return false;
    };

    QFile file(path);
    if (!document || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return fail(QStringLiteral("Cannot open LabelMe file"));
    }

    QJsonParseError parseError;
    const QJsonDocument json = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !json.isObject()) {
        return fail(QStringLiteral("Invalid LabelMe JSON: %1").arg(parseError.errorString()));
    }
    const QJsonObject root = json.object();
    document->imageDataRepaired = false;
    document->imageDataRepairMessage.clear();

    const QStringList requiredFields = {
        QStringLiteral("imagePath"),
        QStringLiteral("imageData"),
        QStringLiteral("shapes")};
    for (const QString &field : requiredFields) {
        if (!root.contains(field)) {
            return fail(QStringLiteral("Missing required field: %1").arg(field));
        }
    }
    const QJsonValue imagePathValue = root.value(QStringLiteral("imagePath"));
    if (!imagePathValue.isString()) {
        return fail(QStringLiteral("imagePath must be a string"));
    }
    const QJsonValue imageDataValue = root.value(QStringLiteral("imageData"));
    if (!imageDataValue.isNull() && !imageDataValue.isString()) {
        return fail(QStringLiteral("imageData must be a base64 string or null"));
    }
    if (imageDataValue.isString() && imageDataValue.toString().isEmpty() && !repairImageData) {
        return fail(QStringLiteral("imageData must not be an empty string"));
    }
    const QJsonValue shapesValue = root.value(QStringLiteral("shapes"));
    if (!shapesValue.isArray()) {
        return fail(QStringLiteral("shapes must be an array"));
    }

    const QJsonValue topLevelFlagsValue = root.value(QStringLiteral("flags"));
    if (!topLevelFlagsValue.isUndefined() && !topLevelFlagsValue.isNull() &&
        !topLevelFlagsValue.isObject()) {
        return fail(QStringLiteral("flags must be an object or null"));
    }
    if (topLevelFlagsValue.isObject()) {
        const QJsonObject topLevelFlagsObject = topLevelFlagsValue.toObject();
        for (auto it = topLevelFlagsObject.constBegin(); it != topLevelFlagsObject.constEnd(); ++it) {
            if (!it.value().isBool()) {
                return fail(QStringLiteral("top-level flag %1 must be boolean").arg(it.key()));
            }
        }
    }

    int declaredWidth = 0;
    int declaredHeight = 0;
    bool widthPresent = false;
    bool heightPresent = false;
    auto readOptionalDimension = [&](const QString &key, int *dimension, bool *present) {
        const QJsonValue value = root.value(key);
        if (value.isUndefined() || value.isNull()) {
            return true;
        }
        if (!value.isDouble()) {
            return fail(QStringLiteral("%1 must be an integer or null").arg(key));
        }
        const double number = value.toDouble();
        if (!std::isfinite(number) || std::floor(number) != number || number < 0.0 ||
            number > static_cast<double>(std::numeric_limits<int>::max())) {
            return fail(QStringLiteral("%1 must be a non-negative integer or null").arg(key));
        }
        *dimension = static_cast<int>(number);
        *present = true;
        return true;
    };
    if (!readOptionalDimension(QStringLiteral("imageWidth"), &declaredWidth, &widthPresent) ||
        !readOptionalDimension(QStringLiteral("imageHeight"), &declaredHeight, &heightPresent)) {
        return false;
    }

    document->imagePath = imagePathValue.toString();
    document->imagePath.replace(QLatin1Char('\\'), QLatin1Char('/'));
    document->labelMeVersion = root.value(QStringLiteral("version")).toString();
    document->imageData = imageDataValue.isString() && !imageDataValue.toString().isEmpty()
                              ? imageDataValue.toString()
                              : QString();
    document->imageSize = QSize(widthPresent ? declaredWidth : 0,
                                heightPresent ? declaredHeight : 0);
    auto externalImagePath = [&]() {
        return QDir::isAbsolutePath(document->imagePath)
                   ? document->imagePath
                   : QFileInfo(path).dir().filePath(document->imagePath);
    };
    auto loadExternalImage = [&](QImage *image) {
        if (!image || document->imagePath.isEmpty()) {
            return false;
        }
        *image = ImageIO::readForDisplay(externalImagePath());
        return !image->isNull();
    };
    auto markImageDataRepair = [&](const QString &message) {
        document->imageDataRepaired = true;
        if (!document->imageDataRepairMessage.isEmpty()) {
            document->imageDataRepairMessage += QLatin1Char(' ');
        }
        document->imageDataRepairMessage += message;
    };
    auto dimensionsMismatch = [&](const QSize &actualSize) {
        return (widthPresent && actualSize.width() != declaredWidth) ||
               (heightPresent && actualSize.height() != declaredHeight);
    };
    if (!document->imageData.isEmpty()) {
        QImage embeddedImage;
        if (!embeddedImage.loadFromData(QByteArray::fromBase64(document->imageData.toLatin1()))) {
            if (!repairImageData) {
                return fail(QStringLiteral("imageData is not a decodable image"));
            }
            QImage externalImage;
            if (!loadExternalImage(&externalImage)) {
                return fail(QStringLiteral("imageData is invalid and external image cannot be loaded: %1")
                                .arg(externalImagePath()));
            }
            document->imageData.clear();
            document->imageSize = externalImage.size();
            markImageDataRepair(QStringLiteral("Invalid embedded imageData was replaced with the external image."));
        } else if (dimensionsMismatch(embeddedImage.size())) {
            if (!repairImageData) {
                return fail(QStringLiteral("imageData dimensions do not match imageWidth/imageHeight"));
            }
            document->imageSize = embeddedImage.size();
            markImageDataRepair(QStringLiteral("Declared image dimensions were repaired from embedded imageData."));
        } else {
            document->imageSize = embeddedImage.size();
        }
    } else {
        if (document->imagePath.isEmpty()) {
            return fail(QStringLiteral("imagePath is required when imageData is null"));
        }
        QImage externalImage;
        if (!loadExternalImage(&externalImage)) {
            return fail(QStringLiteral("external image cannot be loaded: %1").arg(externalImagePath()));
        }
        if (dimensionsMismatch(externalImage.size())) {
            if (!repairImageData) {
                return fail(QStringLiteral("external image dimensions do not match imageWidth/imageHeight"));
            }
            document->imageSize = externalImage.size();
            markImageDataRepair(QStringLiteral("Declared image dimensions were repaired from the external image."));
        } else {
            document->imageSize = externalImage.size();
        }
    }
    document->topLevelFlags.clear();
    document->labelMeOtherData = QJsonObject();
    const QJsonObject topLevelFlags = root.value(QStringLiteral("flags")).toObject();
    document->verified = topLevelFlags.value(QStringLiteral("verified")).toBool(false);
    for (auto it = topLevelFlags.constBegin(); it != topLevelFlags.constEnd(); ++it) {
        if (it.key() == QStringLiteral("verified") || !it.value().isBool()) {
            continue;
        }
        document->topLevelFlags.insert(it.key(), it.value().toBool());
    }
    for (auto it = root.constBegin(); it != root.constEnd(); ++it) {
        if (it.key() == QStringLiteral("version") ||
            it.key() == QStringLiteral("flags") ||
            it.key() == QStringLiteral("shapes") ||
            it.key() == QStringLiteral("imagePath") ||
            it.key() == QStringLiteral("imageData") ||
            it.key() == QStringLiteral("imageHeight") ||
            it.key() == QStringLiteral("imageWidth")) {
            continue;
        }
        document->labelMeOtherData.insert(it.key(), it.value());
    }
    document->shapes.clear();

    for (const QJsonValue &value : shapesValue.toArray()) {
        if (!value.isObject()) {
            return fail(QStringLiteral("shape %1 must be an object").arg(document->shapes.size()));
        }
        const QJsonObject shapeObject = value.toObject();
        const QJsonValue labelValue = shapeObject.value(QStringLiteral("label"));
        if (!labelValue.isString()) {
            return fail(QStringLiteral("shape %1 label must be a string").arg(document->shapes.size()));
        }
        const QString label = labelValue.toString();

        const QJsonValue pointsValue = shapeObject.value(QStringLiteral("points"));
        if (!pointsValue.isArray()) {
            return fail(QStringLiteral("shape %1 points must be an array").arg(document->shapes.size()));
        }
        const QJsonArray points = pointsValue.toArray();
        if (points.isEmpty()) {
            return fail(QStringLiteral("shape %1 points must not be empty").arg(document->shapes.size()));
        }
        for (const QJsonValue &pointValue : points) {
            if (!pointValue.isArray()) {
                return fail(QStringLiteral("shape %1 contains a non-array point").arg(document->shapes.size()));
            }
            const QJsonArray point = pointValue.toArray();
            if (point.size() != 2 || !point.at(0).isDouble() || !point.at(1).isDouble()) {
                return fail(QStringLiteral("shape %1 points must be [x, y] numbers").arg(document->shapes.size()));
            }
        }

        const QJsonValue shapeTypeValue = shapeObject.value(QStringLiteral("shape_type"));
        if (!shapeTypeValue.isString()) {
            return fail(QStringLiteral("shape %1 shape_type must be a string").arg(document->shapes.size()));
        }
        const QString shapeType = shapeTypeValue.toString();
        QVector<QPointF> shapePoints;
        for (const QJsonValue &pointValue : points) {
            const QJsonArray point = pointValue.toArray();
            shapePoints.push_back(QPointF(point.at(0).toDouble(), point.at(1).toDouble()));
        }
        if (shapeType == QStringLiteral("rectangle")) {
            qreal minX = shapePoints.first().x();
            qreal minY = shapePoints.first().y();
            qreal maxX = minX;
            qreal maxY = minY;
            for (const QPointF &point : shapePoints) {
                minX = qMin(minX, point.x());
                minY = qMin(minY, point.y());
                maxX = qMax(maxX, point.x());
                maxY = qMax(maxY, point.y());
            }
        }

        const QJsonValue shapeFlagsValue = shapeObject.value(QStringLiteral("flags"));
        if (!shapeFlagsValue.isUndefined() && !shapeFlagsValue.isNull() && !shapeFlagsValue.isObject()) {
            return fail(QStringLiteral("shape %1 flags must be an object")
                            .arg(document->shapes.size()));
        }
        const QJsonObject shapeFlagsObject = shapeFlagsValue.toObject();
        QMap<QString, bool> shapeFlags;
        for (auto it = shapeFlagsObject.constBegin(); it != shapeFlagsObject.constEnd(); ++it) {
            if (!it.value().isBool()) {
                return fail(QStringLiteral("shape %1 flag %2 must be boolean")
                                .arg(document->shapes.size())
                                .arg(it.key()));
            }
            shapeFlags.insert(it.key(), it.value().toBool());
        }
        const bool difficult = shapeFlags.value(QStringLiteral("difficult"), false);
        Shape loadedShape;
        if (shapeType == QStringLiteral("rectangle")) {
            // LabelMe stores rectangles as the original two diagonal corners.
            // Keep that representation in memory; boundingRect() still gives
            // the normalized box required by the other formats and the UI.
            loadedShape = Shape::fromPoints(label, shapeType, shapePoints, difficult);
        } else {
            // LabelMe plugins may define additional shape types. Keep the
            // original type and points; the runtime shape is closed just like
            // the reference loader's Shape instance.
            loadedShape = Shape::fromPoints(label, shapeType, shapePoints, difficult);
        }
        loadedShape.closed = true;
        loadedShape.flags = shapeFlags;
        const QJsonValue descriptionValue = shapeObject.value(QStringLiteral("description"));
        if (shapeObject.contains(QStringLiteral("description")) &&
            !descriptionValue.isNull() && !descriptionValue.isString()) {
            return fail(QStringLiteral("shape %1 description must be a string or null")
                            .arg(document->shapes.size()));
        }
        // LabelMe's loader represents both an omitted and an explicit-null
        // description as an empty editable string; its serializer then emits
        // the normalized empty string.
        loadedShape.descriptionPresent = true;
        loadedShape.descriptionIsNull = false;
        loadedShape.description = descriptionValue.isString() ? descriptionValue.toString() : QString();
        const QJsonValue groupIdValue = shapeObject.value(QStringLiteral("group_id"));
        if (!groupIdValue.isUndefined() && !groupIdValue.isNull() && !groupIdValue.isDouble()) {
            return fail(QStringLiteral("shape %1 group_id must be an integer or null")
                            .arg(document->shapes.size()));
        }
        if (groupIdValue.isDouble()) {
            const double groupId = groupIdValue.toDouble();
            if (!std::isfinite(groupId) || std::floor(groupId) != groupId ||
                groupId < std::numeric_limits<int>::min() ||
                groupId > std::numeric_limits<int>::max()) {
                return fail(QStringLiteral("shape %1 group_id is out of range")
                                .arg(document->shapes.size()));
            }
            loadedShape.groupId = groupIdValue.toInt();
        }
        loadedShape.maskPresent = true;
        const QJsonValue maskValue = shapeObject.value(QStringLiteral("mask"));
        if (!maskValue.isUndefined() && !maskValue.isNull()) {
            if (!maskValue.isString()) {
                return fail(QStringLiteral("shape %1 mask must be a base64 PNG string")
                                .arg(document->shapes.size()));
            }
            const QString maskData = maskValue.toString();
            if (maskData.isEmpty() ||
                !QImage().loadFromData(QByteArray::fromBase64(maskData.toLatin1()))) {
                return fail(QStringLiteral("shape %1 mask is not a decodable PNG")
                                .arg(document->shapes.size()));
            }
            loadedShape.maskData = maskData;
        }
        loadedShape.labelMeOtherData = QJsonObject();
        for (auto it = shapeObject.constBegin(); it != shapeObject.constEnd(); ++it) {
            if (it.key() == QStringLiteral("label") ||
                it.key() == QStringLiteral("points") ||
                it.key() == QStringLiteral("group_id") ||
                it.key() == QStringLiteral("description") ||
                it.key() == QStringLiteral("shape_type") ||
                it.key() == QStringLiteral("flags") ||
                it.key() == QStringLiteral("mask")) {
                continue;
            }
            loadedShape.labelMeOtherData.insert(it.key(), it.value());
        }
        document->shapes.push_back(loadedShape);
    }
    return true;
}
