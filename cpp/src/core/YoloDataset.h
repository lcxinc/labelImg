#pragma once

#include <QStringList>

// A standard images/... -> labels/... detection dataset. Class order is the
// numeric order from YAML and must never be taken from the UI label history.
struct YoloDataset {
    QString root;
    QString configPath;
    QStringList classes;

    bool isOpen() const { return !root.isEmpty(); }
    QString annotationPath(const QString &imagePath) const;
    static bool load(const QString &path, YoloDataset *dataset, QString *error);
};
