#pragma once

#include <QImage>
#include <QString>

class ImageIO {
public:
    static QImage readForDisplay(const QString &path, QString *errorMessage = nullptr);
    static QImage readPreview(const QString &path,
                              int maxSide,
                              QSize *sourceSize = nullptr,
                              QString *errorMessage = nullptr);
    static QImage normalizeForDisplay(const QImage &image);
};
