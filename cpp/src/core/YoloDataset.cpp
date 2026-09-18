#include "core/YoloDataset.h"
#include "core/LabelMeConfig.h"

#include <QDir>
#include <QFileInfo>
#include <QSet>

QString YoloDataset::annotationPath(const QString &imagePath) const {
    if (!isOpen()) return {};
    const QString relative = QDir(QDir(root).filePath("images")).relativeFilePath(
        QFileInfo(imagePath).absoluteFilePath());
    if (QDir::isAbsolutePath(relative) || relative == ".." || relative.startsWith("../")) return {};
    const QFileInfo info(relative);
    return QDir::cleanPath(QDir(root).filePath("labels/" + info.path() + "/" + info.completeBaseName() + ".txt"));
}

bool YoloDataset::load(const QString &path, YoloDataset *dataset, QString *error) {
    const auto fail = [error](const QString &message) {
        if (error) *error = message;
        return false;
    };
    QFileInfo config(path);
    if (config.isDir()) {
        config = QFileInfo(QDir(path).filePath("data.yaml"));
        if (!config.exists()) config = QFileInfo(QDir(path).filePath("data.yml"));
    }
    QVariantMap values;
    if (!LabelMeConfig::loadFile(config.absoluteFilePath(), &values, error)) return false;
    YoloDataset result;
    result.configPath = config.absoluteFilePath();
    result.root = config.absolutePath();
    // Prefer the selected local dataset so copied datasets with an old absolute
    // `path` keep working. Honor `path` when images are stored elsewhere.
    if (!QDir(QDir(result.root).filePath("images")).exists()) {
        const QString configured = values.value("path").toString();
        if (!configured.isEmpty()) result.root = QDir::cleanPath(QDir(result.root).absoluteFilePath(configured));
    }
    if (!QDir(QDir(result.root).filePath("images")).exists()) {
        return fail(QStringLiteral("YOLO: images directory not found: %1").arg(result.root));
    }
    if (values.value("names").typeId() == QMetaType::QVariantList) {
        for (const QVariant &name : values.value("names").toList()) {
            result.classes.append(name.toString());
        }
    } else {
        QVariantMap names = values.value("names").toMap();
        for (auto it = values.cbegin(); it != values.cend(); ++it) {
            if (it.key().startsWith("names.")) names.insert(it.key().mid(6), it.value());
        }
        for (int index = 0; index < names.size(); ++index) {
            const QString key = QString::number(index);
            if (!names.contains(key)) return fail(QStringLiteral("YOLO: names IDs must be consecutive, starting at 0"));
            result.classes.append(names.value(key).toString());
        }
    }
    QSet<QString> seen;
    for (QString &name : result.classes) {
        name = name.trimmed();
        if (name.isEmpty() || seen.contains(name)) return fail(QStringLiteral("YOLO: class names must be nonempty and unique"));
        seen.insert(name);
    }
    if (result.classes.isEmpty()) return fail(QStringLiteral("YOLO: missing names in dataset YAML"));
    if (values.contains("nc") && values.value("nc").toInt() != result.classes.size()) {
        return fail(QStringLiteral("YOLO: nc does not match names"));
    }
    if (!dataset) return false;
    *dataset = result;
    if (error) error->clear();
    return true;
}
