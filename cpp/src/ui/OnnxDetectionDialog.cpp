#include "ui/OnnxDetectionDialog.h"

#include "core/AiAssistBridge.h"
#include "core/AiAssistSession.h"
#include "core/ResourcePaths.h"
#include "core/StringBundle.h"
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>
#include <memory>

OnnxDetectionDialog::OnnxDetectionDialog(const QString &imagePath, const QStringList &datasetClasses,
                                         QSettings &settings, const StringBundle &strings, QWidget *parent)
    : QDialog(parent) {
    setObjectName(QStringLiteral("onnxDetectionDialog"));
    setWindowTitle(strings.get("onnxDetection"));
    resize(620, 510);
    auto *layout = new QVBoxLayout(this);
    auto *hint = new QLabel(strings.get("onnxHint"), this);
    hint->setWordWrap(true);
    layout->addWidget(hint);
    auto *form = new QFormLayout;
    layout->addLayout(form);
    auto *pathRow = new QHBoxLayout;
    auto *path = new QLineEdit(settings.value("onnx/modelPath").toString(), this);
    path->setObjectName(QStringLiteral("onnxModelPath"));
    auto *browse = new QPushButton(strings.get("onnxBrowse"), this);
    pathRow->addWidget(path, 1);
    pathRow->addWidget(browse);
    form->addRow(strings.get("onnxModelFile"), pathRow);
    auto *format = new QComboBox(this);
    format->setObjectName(QStringLiteral("onnxLayout"));
    format->addItem(QStringLiteral("YOLOv8 / YOLO11 [1, 4+C, N]"), "yolo8");
    format->addItem(QStringLiteral("YOLOv5 [1, N, 5+C]"), "yolo5");
    format->addItem(QStringLiteral("End-to-end / NMS [1, N, 6] (xyxy, score, class)"), "xyxy");
    format->setCurrentIndex(qMax(0, format->findData(settings.value("onnx/layout", "yolo8"))));
    form->addRow(strings.get("onnxLayout"), format);
    auto *size = new QSpinBox(this);
    size->setRange(32, 4096);
    size->setValue(settings.value("onnx/inputSize", 640).toInt());
    form->addRow(strings.get("onnxInputSize"), size);
    auto *score = new QDoubleSpinBox(this);
    score->setRange(0.0, 1.0);
    score->setSingleStep(0.05);
    score->setValue(settings.value("onnx/score", 0.25).toDouble());
    form->addRow(strings.get("score"), score);
    auto *iou = new QDoubleSpinBox(this);
    iou->setRange(0.0, 1.0);
    iou->setSingleStep(0.05);
    iou->setValue(settings.value("onnx/iou", 0.45).toDouble());
    form->addRow(strings.get("iou"), iou);
    auto *classes = new QPlainTextEdit(this);
    classes->setObjectName(QStringLiteral("onnxClasses"));
    classes->setPlaceholderText(strings.get("onnxClassesHint"));
    classes->setPlainText(datasetClasses.join('\n'));
    classes->setReadOnly(!datasetClasses.isEmpty());
    form->addRow(strings.get("onnxClasses"), classes);
    auto *status = new QLabel(strings.get("onnxNotLoaded"), this);
    status->setObjectName(QStringLiteral("onnxStatus"));
    status->setWordWrap(true);
    status->setTextFormat(Qt::PlainText);
    status->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(status);
    auto *progress = new QProgressBar(this);
    progress->setRange(0, 0);
    progress->hide();
    layout->addWidget(progress);
    auto *buttons = new QHBoxLayout;
    auto *load = new QPushButton(strings.get("onnxLoad"), this);
    load->setObjectName(QStringLiteral("onnxLoadButton"));
    auto *run = new QPushButton(strings.get("onnxRun"), this);
    run->setObjectName(QStringLiteral("onnxRunButton"));
    run->setEnabled(false);
    auto *cancel = new QPushButton(strings.get("onnxCancel"), this);
    buttons->addWidget(load);
    buttons->addWidget(run);
    buttons->addStretch();
    buttons->addWidget(cancel);
    layout->addLayout(buttons);

    auto *session = new AiAssistSession(this);
    auto *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(180000);
    struct State { bool loaded = false; bool loading = false; };
    auto state = std::make_shared<State>();
    const auto setBusy = [=](bool busy) {
        for (QWidget *widget : QList<QWidget *>{path, browse, format, size, score, iou, classes, load})
            widget->setEnabled(!busy);
        run->setEnabled(!busy && state->loaded && !imagePath.isEmpty());
        progress->setVisible(busy);
        if (busy) timer->start(); else timer->stop();
    };
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(this, &QDialog::finished, session, &AiAssistSession::stop);
    connect(browse, &QPushButton::clicked, this, [=, &strings] {
        const QString selected = QFileDialog::getOpenFileName(this, strings.get("onnxModelFile"),
                                                              path->text(), QStringLiteral("ONNX (*.onnx)"));
        if (!selected.isEmpty()) path->setText(selected);
    });
    connect(path, &QLineEdit::textChanged, this, [=, &strings] {
        state->loaded = false;
        run->setEnabled(false);
        if (datasetClasses.isEmpty()) classes->clear();
        status->setText(strings.get("onnxNotLoaded"));
    });
    const auto start = [=, &strings](bool loading) {
        QString script = QDir(QCoreApplication::applicationDirPath()).filePath("onnx_detection_bridge.py");
        if (!QFileInfo::exists(script))
            script = ResourcePaths::filePath("cpp/tools/onnx_detection_bridge.py");
        if (!QFileInfo::exists(script)) {
            status->setText(strings.get("aiBridgeMissing"));
            return;
        }
        state->loading = loading;
        if (loading) state->loaded = false;
        QJsonObject request{{"operation", loading ? "load" : "infer"}, {"model_path", path->text()},
                            {"image_path", imagePath}, {"layout", format->currentData().toString()},
                            {"input_size", size->value()}, {"score_threshold", score->value()},
                            {"iou_threshold", iou->value()}};
        QJsonArray names;
        // Preserve blank lines: silently dropping one would shift class IDs.
        if (!classes->toPlainText().trimmed().isEmpty()) {
            for (const QString &name : classes->toPlainText().trimmed().split('\n')) names.append(name.trimmed());
        }
        request.insert("classes", names);
        setBusy(true);
        status->setText(strings.get("aiInferenceProgress"));
        if (!session->request(script, QJsonDocument(request).toJson(QJsonDocument::Compact))) {
            setBusy(false);
            status->setText(strings.get("aiPythonStartFailed"));
        }
    };
    connect(load, &QPushButton::clicked, this, [=] { start(true); });
    connect(run, &QPushButton::clicked, this, [=] { start(false); });
    connect(timer, &QTimer::timeout, this, [=, &strings] {
        session->stop();
        state->loaded = false;
        setBusy(false);
        status->setText(strings.get("onnxTimeout"));
    });
    connect(session, &AiAssistSession::requestFailed, this, [=](const QString &error) {
        state->loaded = false;
        setBusy(false);
        status->setText(error);
    });
    connect(session, &AiAssistSession::responseReady, this, [=, &strings, &settings](const QByteArray &payload) {
        const QJsonObject response = QJsonDocument::fromJson(payload).object();
        if (!response.value("ok").toBool()) {
            setBusy(false);
            status->setText(response.value("error").toString(strings.get("aiBridgeReturnedError")));
            return;
        }
        if (state->loading) {
            QStringList names;
            for (const QJsonValue &name : response.value("classes").toArray()) names.append(name.toString());
            if (!datasetClasses.isEmpty() && !names.isEmpty() && names != datasetClasses) {
                setBusy(false);
                status->setText(strings.get("onnxClassMismatch"));
                return;
            }
            if (datasetClasses.isEmpty() && !names.isEmpty()) classes->setPlainText(names.join('\n'));
            state->loaded = true;
            settings.setValue("onnx/modelPath", path->text());
            setBusy(false);
            status->setText(strings.get("onnxLoaded") + QString::fromUtf8(
                QJsonDocument(response.value("input_shape").toArray()).toJson(QJsonDocument::Compact)));
            return;
        }
        QString error;
        if (!AiAssistBridge::parseResponse(payload, &m_shapes, &error)) {
            setBusy(false);
            status->setText(error);
            return;
        }
        for (const Shape &shape : m_shapes) {
            if (!datasetClasses.isEmpty() && !datasetClasses.contains(shape.label)) {
                m_shapes.clear();
                setBusy(false);
                status->setText(strings.get("onnxClassMismatch"));
                return;
            }
        }
        m_iouThreshold = iou->value();
        settings.setValue("onnx/layout", format->currentData());
        settings.setValue("onnx/inputSize", size->value());
        settings.setValue("onnx/score", score->value());
        settings.setValue("onnx/iou", iou->value());
        setBusy(false);
        accept();
    });
}
