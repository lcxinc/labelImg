#pragma once

#include "core/Shape.h"
#include <QDialog>

class QSettings;
class StringBundle;

// Modal UI keeps the target image stable while the subprocess runs asynchronously.
class OnnxDetectionDialog : public QDialog {
public:
    OnnxDetectionDialog(const QString &imagePath, const QStringList &datasetClasses,
                        QSettings &settings, const StringBundle &strings, QWidget *parent = nullptr);
    QVector<Shape> shapes() const { return m_shapes; }
    double iouThreshold() const { return m_iouThreshold; }

private:
    QVector<Shape> m_shapes;
    double m_iouThreshold = 0.45;
};
