#include "ui/MainWindow.h"

#include "core/ResourcePaths.h"
#include "core/AiAssistBridge.h"
#include "core/ImageIO.h"
#include "core/WindowChrome.h"

#include <QAbstractButton>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QCloseEvent>
#include <QColor>
#include <QColorDialog>
#include <QCompleter>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QDockWidget>
#include <QDir>
#include <QDirIterator>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHash>
#include <QHBoxLayout>
#include <QIcon>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListView>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QScreen>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QScrollBar>
#include <QSlider>
#include <QSpinBox>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTabWidget>
#include <QTimer>
#include <QThread>
#include <QToolButton>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>
#include <QWindow>
#include <QtMath>

#include <functional>
#include <cmath>
#include <memory>
#include <utility>

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#endif

#include <limits>

namespace {
constexpr int ActiveFileRole = Qt::UserRole + 1;
constexpr int VisitedFileRole = Qt::UserRole + 2;
constexpr int MarkedFileRole = Qt::UserRole + 3;
constexpr int DockStateVersion = 2;
constexpr int CanvasPreviewMaxSide = 4096;
constexpr double CanvasPreviewUpgradeScale = 0.5;

QIcon resourceIcon(const QString &fileName) {
    return QIcon(ResourcePaths::filePath(QStringLiteral("resources/icons/") + fileName));
}

QImage readImageWithAutoTransform(const QString &path) {
    return ImageIO::readForDisplay(path);
}

QJsonObject shapeToClipboardJson(const Shape &shape) {
    QJsonObject object = shape.labelMeOtherData;
    for (const QString &key : {QStringLiteral("label"),
                               QStringLiteral("points"),
                               QStringLiteral("group_id"),
                               QStringLiteral("description"),
                               QStringLiteral("shape_type"),
                               QStringLiteral("flags"),
                               QStringLiteral("mask")}) {
        object.remove(key);
    }
    object.insert(QStringLiteral("label"), shape.label);
    QJsonArray points;
    for (const QPointF &point : shape.points) {
        QJsonArray pair;
        pair.append(point.x());
        pair.append(point.y());
        points.append(pair);
    }
    object.insert(QStringLiteral("points"), points);
    object.insert(QStringLiteral("group_id"), shape.groupId >= 0
                                                    ? QJsonValue(shape.groupId)
                                                    : QJsonValue(QJsonValue::Null));
    object.insert(QStringLiteral("description"), shape.description);
    object.insert(QStringLiteral("shape_type"),
                  shape.shapeType.isEmpty() ? QStringLiteral("rectangle") : shape.shapeType);
    QJsonObject flags;
    for (auto it = shape.flags.cbegin(); it != shape.flags.cend(); ++it) {
        flags.insert(it.key(), it.value());
    }
    if (shape.difficult) {
        flags.insert(QStringLiteral("difficult"), true);
    }
    object.insert(QStringLiteral("flags"), flags);
    object.insert(QStringLiteral("mask"), shape.maskData.isEmpty()
                                              ? QJsonValue(QJsonValue::Null)
                                              : QJsonValue(shape.maskData));
    return object;
}

bool shapeFromClipboardJson(const QJsonObject &object, Shape *shape) {
    if (!shape || !object.value(QStringLiteral("label")).isString() ||
        !object.value(QStringLiteral("points")).isArray() ||
        !object.value(QStringLiteral("shape_type")).isString()) {
        return false;
    }
    QVector<QPointF> points;
    for (const QJsonValue &pointValue : object.value(QStringLiteral("points")).toArray()) {
        if (!pointValue.isArray()) {
            return false;
        }
        const QJsonArray pair = pointValue.toArray();
        if (pair.size() != 2 || !pair.at(0).isDouble() || !pair.at(1).isDouble()) {
            return false;
        }
        points.push_back(QPointF(pair.at(0).toDouble(), pair.at(1).toDouble()));
    }
    if (points.isEmpty()) {
        return false;
    }
    const QString shapeType = object.value(QStringLiteral("shape_type")).toString();
    Shape restored = Shape::fromPoints(object.value(QStringLiteral("label")).toString(),
                                        shapeType,
                                        points,
                                        false);
    restored.closed = true;
    const QJsonValue groupId = object.value(QStringLiteral("group_id"));
    if (groupId.isDouble()) {
        const double value = groupId.toDouble();
        if (!std::isfinite(value) || std::floor(value) != value ||
            value < (std::numeric_limits<int>::min)() || value > (std::numeric_limits<int>::max)()) {
            return false;
        }
        restored.groupId = groupId.toInt();
    }
    const QJsonValue description = object.value(QStringLiteral("description"));
    if (!description.isUndefined() && !description.isNull() && !description.isString()) {
        return false;
    }
    restored.description = description.isString() ? description.toString() : QString();
    restored.descriptionPresent = true;
    const QJsonValue flags = object.value(QStringLiteral("flags"));
    if (!flags.isUndefined() && !flags.isNull() && !flags.isObject()) {
        return false;
    }
    const QJsonObject flagObject = flags.toObject();
    for (auto it = flagObject.constBegin(); it != flagObject.constEnd(); ++it) {
        if (!it.value().isBool()) {
            return false;
        }
        restored.flags.insert(it.key(), it.value().toBool());
    }
    restored.difficult = restored.flags.value(QStringLiteral("difficult"), false);
    const QJsonValue mask = object.value(QStringLiteral("mask"));
    if (!mask.isUndefined() && !mask.isNull()) {
        if (!mask.isString()) {
            return false;
        }
        restored.maskData = mask.toString();
    }
    for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
        if (it.key() == QStringLiteral("label") || it.key() == QStringLiteral("points") ||
            it.key() == QStringLiteral("group_id") || it.key() == QStringLiteral("description") ||
            it.key() == QStringLiteral("shape_type") || it.key() == QStringLiteral("flags") ||
            it.key() == QStringLiteral("mask")) {
            continue;
        }
        restored.labelMeOtherData.insert(it.key(), it.value());
    }
    *shape = restored;
    return true;
}

QVector<Shape> shapesFromClipboardMime(const QMimeData *mimeData) {
    QVector<Shape> shapes;
    if (!mimeData) {
        return shapes;
    }
    QByteArray payload = mimeData->data(QStringLiteral("application/x-labelme-shapes"));
    if (payload.isEmpty() && mimeData->hasText()) {
        payload = mimeData->text().toUtf8();
    }
    if (payload.isEmpty()) {
        return shapes;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
    QJsonArray jsonShapes;
    if (parseError.error != QJsonParseError::NoError) {
        return shapes;
    }
    if (document.isArray()) {
        jsonShapes = document.array();
    } else if (document.isObject() &&
               document.object().value(QStringLiteral("shapes")).isArray()) {
        jsonShapes = document.object().value(QStringLiteral("shapes")).toArray();
    }
    for (const QJsonValue &value : jsonShapes) {
        Shape shape;
        if (value.isObject() && shapeFromClipboardJson(value.toObject(), &shape)) {
            shapes.push_back(shape);
        }
    }
    return shapes;
}

void setToolbarIcon(QAction *action, const QString &fileName) {
    action->setIcon(resourceIcon(fileName));
    action->setProperty("toolbarIconFile", fileName);
}

QColor fallbackColorForLabel(const QString &label) {
    uint hash = qHash(label);
    return QColor::fromHsv(hash % 359, 160, 230);
}

QColor imgvizLabelColor(int labelId) {
    // Keep this in sync with imgviz.label_colormap(): each three-bit group
    // contributes one bit to the high-to-low RGB channel positions.
    quint32 value = static_cast<quint32>(labelId);
    int red = 0;
    int green = 0;
    int blue = 0;
    for (int bit = 0; bit < 8; ++bit) {
        red |= static_cast<int>((value >> 0U) & 1U) << (7 - bit);
        green |= static_cast<int>((value >> 1U) & 1U) << (7 - bit);
        blue |= static_cast<int>((value >> 2U) & 1U) << (7 - bit);
        value >>= 3U;
    }
    return QColor(red, green, blue);
}

QColor colorFromConfig(const QVariant &value, const QColor &fallback) {
    const QVariantList components = value.toList();
    if (components.size() != 3 && components.size() != 4) {
        return fallback;
    }
    QVector<int> channels;
    channels.reserve(components.size());
    for (const QVariant &component : components) {
        bool ok = false;
        const int channel = component.toInt(&ok);
        if (!ok || channel < 0 || channel > 255) {
            return fallback;
        }
        channels.append(channel);
    }
    return components.size() == 3
               ? QColor(channels[0], channels[1], channels[2])
               : QColor(channels[0], channels[1], channels[2], channels[3]);
}

QStringList labelsFromCommandLine(const QString &value) {
    const QFileInfo info(value.trimmed());
    if (info.isFile()) {
        QFile file(info.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QStringList labels;
            while (!file.atEnd()) {
                const QString label = QString::fromUtf8(file.readLine()).trimmed();
                if (!label.isEmpty() && !labels.contains(label)) {
                    labels.append(label);
                }
            }
            return labels;
        }
    }
    return LabelMeConfig::stringList(QVariant(value));
}

QString labelFlagPresetSourceFromConfig(const QVariantMap &values) {
    QStringList lines;
    const QVariantMap inlinePresets = values.value(QStringLiteral("label_flags")).toMap();
    for (auto it = inlinePresets.cbegin(); it != inlinePresets.cend(); ++it) {
        const QString pattern = it.key().trimmed();
        const QStringList flags = LabelMeConfig::stringList(it.value());
        if (!pattern.isEmpty() && !flags.isEmpty()) {
            lines.append(pattern + QLatin1Char('=') + flags.join(QLatin1Char(',')));
        }
    }
    const QString prefix = QStringLiteral("label_flags.");
    for (auto it = values.cbegin(); it != values.cend(); ++it) {
        if (!it.key().startsWith(prefix)) {
            continue;
        }
        const QString pattern = it.key().mid(prefix.size()).trimmed();
        const QStringList flags = LabelMeConfig::stringList(it.value());
        if (!pattern.isEmpty() && !flags.isEmpty()) {
            lines.append(pattern + QLatin1Char('=') + flags.join(QLatin1Char(',')));
        }
    }
    return lines.join(QLatin1Char('\n'));
}

class LabelListKeyForwarder final : public QObject {
public:
    LabelListKeyForwarder(QListWidget *labelList, QComboBox *labelCombo, QObject *parent = nullptr)
        : QObject(parent), m_labelList(labelList), m_labelCombo(labelCombo) {}

protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        Q_UNUSED(watched);
        if (event->type() != QEvent::KeyPress || !m_labelList || !m_labelCombo || !m_labelList->isEnabled()) {
            return QObject::eventFilter(watched, event);
        }
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() != Qt::Key_Up && keyEvent->key() != Qt::Key_Down) {
            return QObject::eventFilter(watched, event);
        }

        const int count = m_labelList->count();
        if (count <= 0) {
            return true;
        }
        int row = m_labelList->currentRow();
        if (row < 0) {
            row = keyEvent->key() == Qt::Key_Down ? 0 : count - 1;
        } else {
            row += keyEvent->key() == Qt::Key_Down ? 1 : -1;
            row = qBound(0, row, count - 1);
        }
        m_labelList->setCurrentRow(row);
        if (QListWidgetItem *item = m_labelList->currentItem()) {
            m_labelCombo->setCurrentText(item->text());
        }
        return true;
    }

private:
    QListWidget *m_labelList = nullptr;
    QComboBox *m_labelCombo = nullptr;
};

class LabelListWidget final : public QListWidget {
public:
    using SelectionRestoreCallback = std::function<void(const QVector<int> &)>;

    explicit LabelListWidget(QWidget *parent = nullptr)
        : QListWidget(parent) {}

    void setSelectionRestoreCallback(SelectionRestoreCallback callback) {
        m_restoreSelection = std::move(callback);
    }

protected:
    void mousePressEvent(QMouseEvent *event) override {
        m_selectedRowsAtPress.clear();
        m_checkStatesAtPress.clear();
        for (QListWidgetItem *item : selectedItems()) {
            const int row = this->row(item);
            if (row < 0) {
                continue;
            }
            m_selectedRowsAtPress.append(row);
            m_checkStatesAtPress.insert(row, item->checkState());
        }
        QListWidget::mousePressEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        QListWidget::mouseReleaseEvent(event);

        bool checkStateChanged = false;
        for (auto it = m_checkStatesAtPress.cbegin(); it != m_checkStatesAtPress.cend(); ++it) {
            if (it.key() >= 0 && it.key() < count() && item(it.key())->checkState() != it.value()) {
                checkStateChanged = true;
                break;
            }
        }

        if (checkStateChanged && m_selectedRowsAtPress.size() > 1) {
            QSet<int> currentRows;
            for (QListWidgetItem *selectedItem : selectedItems()) {
                currentRows.insert(row(selectedItem));
            }
            QSet<int> pressedRows;
            for (const int selectedRow : m_selectedRowsAtPress) {
                pressedRows.insert(selectedRow);
            }
            if (currentRows != pressedRows && m_restoreSelection) {
                m_restoreSelection(m_selectedRowsAtPress);
            }
        }

        m_selectedRowsAtPress.clear();
        m_checkStatesAtPress.clear();
    }

private:
    QVector<int> m_selectedRowsAtPress;
    QHash<int, Qt::CheckState> m_checkStatesAtPress;
    SelectionRestoreCallback m_restoreSelection;
};

bool sameShapeForHistory(const Shape &left, const Shape &right) {
    return left.label == right.label &&
           left.shapeType == right.shapeType &&
           left.groupId == right.groupId &&
           left.description == right.description &&
           left.descriptionPresent == right.descriptionPresent &&
           left.descriptionIsNull == right.descriptionIsNull &&
           left.flags == right.flags &&
           left.points == right.points &&
           left.pointLabels == right.pointLabels &&
           left.labelMeOtherData == right.labelMeOtherData &&
           left.lineColor == right.lineColor &&
           left.fillColor == right.fillColor &&
           left.maskData == right.maskData &&
           left.maskPresent == right.maskPresent &&
           left.closed == right.closed &&
           left.difficult == right.difficult &&
           left.visible == right.visible &&
           left.paintLabel == right.paintLabel;
}

bool sameShapesForHistory(const QVector<Shape> &left, const QVector<Shape> &right) {
    if (left.size() != right.size()) {
        return false;
    }
    for (int i = 0; i < left.size(); ++i) {
        if (!sameShapeForHistory(left[i], right[i])) {
            return false;
        }
    }
    return true;
}

bool naturalPathLess(const QString &left, const QString &right) {
    const QString a = left.toLower();
    const QString b = right.toLower();
    int i = 0;
    int j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i].isDigit() && b[j].isDigit()) {
            int aStart = i;
            int bStart = j;
            while (i < a.size() && a[i].isDigit()) ++i;
            while (j < b.size() && b[j].isDigit()) ++j;

            QString aNumber = a.mid(aStart, i - aStart);
            QString bNumber = b.mid(bStart, j - bStart);
            while (aNumber.size() > 1 && aNumber.startsWith('0')) aNumber.remove(0, 1);
            while (bNumber.size() > 1 && bNumber.startsWith('0')) bNumber.remove(0, 1);

            if (aNumber.size() != bNumber.size()) return aNumber.size() < bNumber.size();
            int numericCompare = QString::compare(aNumber, bNumber);
            if (numericCompare != 0) return numericCompare < 0;

            int originalLengthCompare = (i - aStart) - (j - bStart);
            if (originalLengthCompare != 0) return originalLengthCompare < 0;
            continue;
        }

        if (a[i] != b[j]) return a[i] < b[j];
        ++i;
        ++j;
    }
    return a.size() < b.size();
}

QString flagsToEditorText(const QMap<QString, bool> &flags) {
    QStringList lines;
    for (auto it = flags.cbegin(); it != flags.cend(); ++it) {
        lines.append(QStringLiteral("%1=%2").arg(it.key(), it.value() ? QStringLiteral("true") : QStringLiteral("false")));
    }
    return lines.join(QLatin1Char('\n'));
}

QString labelListDisplayText(const Shape &shape) {
    QString text = shape.label;
    if (shape.groupId >= 0) {
        text += QStringLiteral(" (%1)").arg(shape.groupId);
    }
    QStringList enabledFlags;
    for (auto it = shape.flags.cbegin(); it != shape.flags.cend(); ++it) {
        if (it.value()) {
            enabledFlags.append(it.key());
        }
    }
    if (!enabledFlags.isEmpty()) {
        text += QStringLiteral(" [%1]").arg(enabledFlags.join(QStringLiteral(", ")));
    }
    return text;
}

QIcon labelColorDotIcon(const QColor &color) {
    QPixmap pixmap(12, 12);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QColor fill = color.isValid() ? color : QColor(128, 128, 128);
    fill.setAlpha(255);
    painter.setPen(QPen(fill.darker(145), 1));
    painter.setBrush(fill);
    painter.drawEllipse(QRectF(2.0, 2.0, 8.0, 8.0));
    return QIcon(pixmap);
}

QMap<QString, bool> flagsFromEditorText(const QString &text) {
    QMap<QString, bool> flags;
    const QStringList lines = text.split(QRegularExpression(QStringLiteral("[\\r\\n]+")), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty()) {
            continue;
        }
        const int separator = trimmed.indexOf(QLatin1Char('='));
        const QString key = (separator >= 0 ? trimmed.left(separator) : trimmed).trimmed();
        const QString valueText = (separator >= 0 ? trimmed.mid(separator + 1) : QStringLiteral("true")).trimmed().toLower();
        if (key.isEmpty()) {
            continue;
        }
        const bool value = valueText == QStringLiteral("1") ||
                           valueText == QStringLiteral("true") ||
                           valueText == QStringLiteral("yes") ||
                           valueText == QStringLiteral("on");
        flags.insert(key, value);
    }
    return flags;
}

QStringList splitTopLevel(const QString &text, QChar separator) {
    QStringList parts;
    QString current;
    int bracketDepth = 0;
    int braceDepth = 0;
    for (const QChar ch : text) {
        if (ch == QLatin1Char('[')) {
            ++bracketDepth;
        } else if (ch == QLatin1Char(']')) {
            bracketDepth = qMax(0, bracketDepth - 1);
        } else if (ch == QLatin1Char('{')) {
            ++braceDepth;
        } else if (ch == QLatin1Char('}')) {
            braceDepth = qMax(0, braceDepth - 1);
        }
        if (ch == separator && bracketDepth == 0 && braceDepth == 0) {
            parts.append(current.trimmed());
            current.clear();
            continue;
        }
        current.append(ch);
    }
    if (!current.trimmed().isEmpty()) {
        parts.append(current.trimmed());
    }
    return parts;
}

int indexOfTopLevelMappingSeparator(const QString &text) {
    int bracketDepth = 0;
    int braceDepth = 0;
    for (int i = 0; i < text.size(); ++i) {
        const QChar ch = text.at(i);
        if (ch == QLatin1Char('[')) {
            ++bracketDepth;
        } else if (ch == QLatin1Char(']')) {
            bracketDepth = qMax(0, bracketDepth - 1);
        } else if (ch == QLatin1Char('{')) {
            ++braceDepth;
        } else if (ch == QLatin1Char('}')) {
            braceDepth = qMax(0, braceDepth - 1);
        } else if ((ch == QLatin1Char('=') || ch == QLatin1Char(':')) && bracketDepth == 0 && braceDepth == 0) {
            return i;
        }
    }
    return -1;
}

QStringList labelFlagKeysFromText(const QString &text) {
    QString value = text.trimmed();
    if (value.startsWith(QLatin1Char('[')) && value.endsWith(QLatin1Char(']'))) {
        value = value.mid(1, value.size() - 2);
    }
    QStringList normalizedKeys;
    for (const QString &key : splitTopLevel(value, QLatin1Char(','))) {
        QString normalized = key.trimmed();
        if ((normalized.startsWith(QLatin1Char('"')) && normalized.endsWith(QLatin1Char('"'))) ||
            (normalized.startsWith(QLatin1Char('\'')) && normalized.endsWith(QLatin1Char('\'')))) {
            normalized = normalized.mid(1, normalized.size() - 2).trimmed();
        }
        if (!normalized.isEmpty() && !normalizedKeys.contains(normalized)) {
            normalizedKeys.append(normalized);
        }
    }
    return normalizedKeys;
}

QMap<QString, QStringList> labelFlagPresetsFromText(const QString &text) {
    QMap<QString, QStringList> presets;
    QString normalizedText = text.trimmed();
    if (normalizedText.startsWith(QLatin1Char('{')) && normalizedText.endsWith(QLatin1Char('}'))) {
        normalizedText = normalizedText.mid(1, normalizedText.size() - 2);
    }
    const QStringList lines = normalizedText.contains(QLatin1Char('\n'))
                                  ? normalizedText.split(QRegularExpression(QStringLiteral("[\\r\\n]+")), Qt::SkipEmptyParts)
                                  : splitTopLevel(normalizedText, QLatin1Char(','));
    for (QString line : lines) {
        QString trimmed = line.trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith(QLatin1Char('#'))) {
            continue;
        }
        if (trimmed.endsWith(QLatin1Char(','))) {
            trimmed.chop(1);
            trimmed = trimmed.trimmed();
        }
        const int separator = indexOfTopLevelMappingSeparator(trimmed);
        if (separator <= 0) {
            continue;
        }
        const QString pattern = trimmed.left(separator).trimmed();
        const QStringList normalizedKeys = labelFlagKeysFromText(trimmed.mid(separator + 1));
        if (!pattern.isEmpty() && !normalizedKeys.isEmpty()) {
            presets.insert(pattern, normalizedKeys);
        }
    }
    return presets;
}

QString labelFlagPresetSourceText(const QString &source) {
    const QString trimmed = source.trimmed();
    QFile file(trimmed);
    if (!trimmed.isEmpty() && file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString::fromUtf8(file.readAll());
    }
    return source;
}

bool labelFlagPresetSourceIsFile(const QString &source) {
    const QFileInfo info(source.trimmed());
    return info.exists() && info.isFile();
}

QMap<QString, QStringList> labelFlagPresetsFromSource(const QString &source) {
    return labelFlagPresetsFromText(labelFlagPresetSourceText(source));
}

QStringList supportedImageNameFilters() {
    QStringList filters;
    const QList<QByteArray> formats = QImageReader::supportedImageFormats();
    for (const QByteArray &format : formats) {
        QString suffix = QString::fromLatin1(format).toLower();
        if (!suffix.isEmpty()) {
            QString filter = "*." + suffix;
            if (!filters.contains(filter)) filters.append(filter);
        }
    }
    return filters;
}

bool isSupportedImagePath(const QString &path) {
    const QFileInfo info(path);
    if (!info.isFile()) {
        return false;
    }
    const QByteArray suffix = info.suffix().toLatin1();
    for (const QByteArray &format : QImageReader::supportedImageFormats()) {
        if (format.compare(suffix, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}

bool isAiTextCreateMode(const QString &shapeType) {
    return shapeType == QStringLiteral("rectangle") || shapeType == QStringLiteral("polygon") ||
           shapeType == QStringLiteral("ai_points_to_shape") || shapeType == QStringLiteral("ai_box_to_shape");
}

}

MainWindow::MainWindow(QWidget *parent, const QString &defaultConfigPath)
    : QMainWindow(parent),
      m_defaultConfigPath(defaultConfigPath) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground, false);
    setAcceptDrops(true);
    setWindowIcon(QIcon(ResourcePaths::filePath(QStringLiteral("resources/icons/app-cpp.png"))));
    loadSettings();
    loadPredefinedClasses();
    createUi();
    createActions();
    createMenusAndToolbars();
    installFramelessChrome();
    applyNativeWindowChrome();
    m_defaultDockState = saveState(DockStateVersion);
    connectSignals();
    m_aiSession = new AiAssistSession(this);
    connect(m_aiSession, &AiAssistSession::responseReady,
            this, &MainWindow::onAiSessionResponse);
    connect(m_aiSession, &AiAssistSession::progressUpdated,
            this, &MainWindow::onAiSessionProgress);
    connect(m_aiSession, &AiAssistSession::requestFailed,
            this, &MainWindow::onAiSessionFailed);
    setAdvancedMode(m_advancedModeAction->isChecked());
    refreshTexts();
    refreshActions();
    syncTopLevelFlagsEditor();
    resize(m_settings.value("window/size", QSize(1100, 700)).toSize());
    move(m_settings.value("window/position", QPoint(20, 20)).toPoint());
    restoreState(m_settings.value("window/state").toByteArray(), DockStateVersion);
    bool intersectsAvailableScreen = false;
    for (QScreen *screen : QGuiApplication::screens()) {
        if (screen && screen->availableGeometry().intersects(frameGeometry())) {
            intersectsAvailableScreen = true;
            break;
        }
    }
    if (!intersectsAvailableScreen) {
        if (QScreen *primaryScreen = QGuiApplication::primaryScreen()) {
            move(primaryScreen->availableGeometry().topLeft());
        }
    }
    // The application entry point supplies real positional arguments after
    // construction.  Keeping construction config-only prevents test-runner
    // options (for example QtTest's -o output path) from being treated as an
    // image or annotation file.
    loadStartupArgs({QCoreApplication::arguments().value(0)});
}

void MainWindow::loadStartupArgs(const QStringList &arguments) {
    m_configFilePath.clear();
    QStringList positional;
    if (!arguments.isEmpty()) {
        positional.append(arguments.first());
    }
    QVariantMap configValues;
    QString configPath;
    QStringList cliLabels;
    QString cliLanguage;
    QString cliLabelFlagsSource;
    bool hasCommandLineOverride = false;
    bool hasExplicitConfigOption = false;
    bool hasInlineConfig = false;
    bool resetConfig = false;
    auto consumeValue = [&](int *index, const QString &option) {
        if (*index + 1 >= arguments.size()) {
            statusBar()->showMessage(QStringLiteral("Missing value for %1").arg(option), 5000);
            return QString();
        }
        ++(*index);
        return arguments.at(*index);
    };

    for (int index = 1; index < arguments.size(); ++index) {
        const QString argument = arguments.at(index);
        if (argument == QStringLiteral("--reset-config")) {
            resetConfig = true;
            continue;
        }
        if (argument == QStringLiteral("--config")) {
            hasExplicitConfigOption = true;
            configPath = consumeValue(&index, argument);
            continue;
        }
        if (argument.startsWith(QStringLiteral("--config="))) {
            hasExplicitConfigOption = true;
            configPath = argument.mid(QStringLiteral("--config=").size());
            continue;
        }
        if (argument == QStringLiteral("--labels")) {
            cliLabels = labelsFromCommandLine(consumeValue(&index, argument));
            hasCommandLineOverride = true;
            continue;
        }
        if (argument.startsWith(QStringLiteral("--labels="))) {
            cliLabels = labelsFromCommandLine(argument.mid(QStringLiteral("--labels=").size()));
            hasCommandLineOverride = true;
            continue;
        }
        if (argument == QStringLiteral("--flags")) {
            configValues.insert(QStringLiteral("flags"), labelsFromCommandLine(consumeValue(&index, argument)));
            hasCommandLineOverride = true;
            continue;
        }
        if (argument.startsWith(QStringLiteral("--flags="))) {
            configValues.insert(QStringLiteral("flags"),
                                labelsFromCommandLine(argument.mid(QStringLiteral("--flags=").size())));
            hasCommandLineOverride = true;
            continue;
        }
        if (argument == QStringLiteral("--label-flags") ||
            argument == QStringLiteral("--labelflags")) {
            cliLabelFlagsSource = consumeValue(&index, argument);
            hasCommandLineOverride = true;
            continue;
        }
        if (argument.startsWith(QStringLiteral("--label-flags=")) ||
            argument.startsWith(QStringLiteral("--labelflags="))) {
            const int separator = argument.indexOf(QLatin1Char('='));
            cliLabelFlagsSource = argument.mid(separator + 1);
            hasCommandLineOverride = true;
            continue;
        }
        if (argument == QStringLiteral("--language")) {
            cliLanguage = consumeValue(&index, argument).trimmed();
            hasCommandLineOverride = true;
            continue;
        }
        if (argument.startsWith(QStringLiteral("--language="))) {
            cliLanguage = argument.mid(QStringLiteral("--language=").size()).trimmed();
            hasCommandLineOverride = true;
            continue;
        }
        if (argument == QStringLiteral("--output")) {
            const QString outputPath = consumeValue(&index, argument);
            if (!outputPath.isEmpty()) {
                const QFileInfo outputInfo(outputPath);
                if (outputInfo.suffix().compare(QStringLiteral("json"), Qt::CaseInsensitive) == 0) {
                    m_outputFilePath = outputInfo.absoluteFilePath();
                    m_saveDir.clear();
                    setFormat(SaveFormat::LabelMe);
                } else {
                    m_outputFilePath.clear();
                    m_saveDir = outputInfo.absoluteFilePath();
                }
            }
            hasCommandLineOverride = true;
            continue;
        }
        if (argument == QStringLiteral("--save-dir")) {
            const QString saveDir = consumeValue(&index, argument);
            if (!saveDir.isEmpty()) {
                m_outputFilePath.clear();
                m_saveDir = QFileInfo(saveDir).absoluteFilePath();
            }
            hasCommandLineOverride = true;
            continue;
        }
        if (argument.startsWith(QStringLiteral("--output="))) {
            const int separator = argument.indexOf(QLatin1Char('='));
            const QFileInfo outputInfo(argument.mid(separator + 1));
            if (outputInfo.suffix().compare(QStringLiteral("json"), Qt::CaseInsensitive) == 0) {
                m_outputFilePath = outputInfo.absoluteFilePath();
                m_saveDir.clear();
                setFormat(SaveFormat::LabelMe);
            } else {
                m_outputFilePath.clear();
                m_saveDir = outputInfo.absoluteFilePath();
            }
            hasCommandLineOverride = true;
            continue;
        }
        if (argument.startsWith(QStringLiteral("--save-dir="))) {
            const int separator = argument.indexOf(QLatin1Char('='));
            m_outputFilePath.clear();
            m_saveDir = QFileInfo(argument.mid(separator + 1)).absoluteFilePath();
            hasCommandLineOverride = true;
            continue;
        }
        if (argument == QStringLiteral("--no-auto-save")) {
            configValues.insert(QStringLiteral("auto_save"), false);
            hasCommandLineOverride = true;
            continue;
        }
        if (argument == QStringLiteral("--nodata") ||
            argument == QStringLiteral("--autosave")) {
            // These options are retained as deprecated no-ops by LabelMe.
            continue;
        }
        if (argument == QStringLiteral("--with-image-data")) {
            configValues.insert(QStringLiteral("with_image_data"), true);
            hasCommandLineOverride = true;
            continue;
        }
        if (argument == QStringLiteral("--keep-prev")) {
            configValues.insert(QStringLiteral("keep_prev"), true);
            hasCommandLineOverride = true;
            continue;
        }
        if (argument == QStringLiteral("--epsilon")) {
            bool ok = false;
            const double epsilon = consumeValue(&index, argument).toDouble(&ok);
            if (ok && epsilon >= 0.0) {
                configValues.insert(QStringLiteral("epsilon"), epsilon);
                hasCommandLineOverride = true;
            } else {
                statusBar()->showMessage(QStringLiteral("Invalid value for %1").arg(argument), 5000);
            }
            continue;
        }
        if (argument == QStringLiteral("--no-sort-labels") ||
            argument == QStringLiteral("--nosortlabels")) {
            configValues.insert(QStringLiteral("sort_labels"), false);
            hasCommandLineOverride = true;
            continue;
        }
        if (argument == QStringLiteral("--validate-label") ||
            argument == QStringLiteral("--validatelabel")) {
            configValues.insert(QStringLiteral("validate_label"), consumeValue(&index, argument));
            hasCommandLineOverride = true;
            continue;
        }
        if (argument.startsWith(QStringLiteral("--validate-label=")) ||
            argument.startsWith(QStringLiteral("--validatelabel="))) {
            const int separator = argument.indexOf(QLatin1Char('='));
            configValues.insert(QStringLiteral("validate_label"), argument.mid(separator + 1));
            hasCommandLineOverride = true;
            continue;
        }
        if (argument == QStringLiteral("--logger-level")) {
            consumeValue(&index, argument);
            continue;
        }
        if (argument.startsWith(QStringLiteral("--logger-level="))) {
            continue;
        }
        if (!argument.startsWith(QLatin1Char('-'))) {
            positional.append(argument);
        }
    }

    const QVariantMap commandLineValues = configValues;
    configValues.clear();
    if (!hasExplicitConfigOption && configPath.isEmpty()) {
        configPath = m_defaultConfigPath;
    }
    if (!configPath.isEmpty()) {
        const bool creatingUserConfig = !hasExplicitConfigOption &&
                                        configPath == m_defaultConfigPath &&
                                        !QFileInfo::exists(configPath);
        if (creatingUserConfig) {
            const QFileInfo configInfo(configPath);
            QDir().mkpath(configInfo.absolutePath());
            QFile configFile(configPath);
            if (configFile.open(QIODevice::WriteOnly)) {
                configFile.close();
            }
        }
        QString error;
        QVariantMap fileValues;
        const QRegularExpression inlineConfigPattern(
            QStringLiteral("(^|\\n)\\s*[^#\\s][^\\n:]*:\\s+"));
        const QString trimmedConfig = configPath.trimmed();
        const bool configFileExists = QFileInfo::exists(configPath);
        const bool looksLikeInlineConfig =
            !configFileExists &&
            (configPath.contains(QLatin1Char('\n')) ||
             trimmedConfig.startsWith(QLatin1Char('{')) ||
             inlineConfigPattern.match(configPath).hasMatch());
        const bool loadedConfig =
            (hasExplicitConfigOption && looksLikeInlineConfig &&
             LabelMeConfig::loadText(configPath, &fileValues, &error)) ||
            LabelMeConfig::loadFile(configPath, &fileValues, &error);
        if (loadedConfig) {
            hasInlineConfig = hasExplicitConfigOption && looksLikeInlineConfig;
            m_configFilePath = QFileInfo(configPath).absoluteFilePath();
            if (hasInlineConfig) {
                m_configFilePath.clear();
            }
            // Keep the migrations used by LabelMe's config loader for older
            // ~/.labelmerc files commonly created by pre-5.x releases.
            if (!fileValues.contains(QStringLiteral("with_image_data")) &&
                fileValues.contains(QStringLiteral("store_data"))) {
                fileValues.insert(QStringLiteral("with_image_data"),
                                  fileValues.value(QStringLiteral("store_data")));
            }
            if (!fileValues.contains(QStringLiteral("keep_prev_brightness_contrast")) &&
                (fileValues.contains(QStringLiteral("keep_prev_brightness")) ||
                 fileValues.contains(QStringLiteral("keep_prev_contrast")))) {
                fileValues.insert(QStringLiteral("keep_prev_brightness_contrast"),
                                  fileValues.value(QStringLiteral("keep_prev_brightness"), false).toBool() ||
                                      fileValues.value(QStringLiteral("keep_prev_contrast"), false).toBool());
            }
            const QHash<QString, QString> shortcutMigrations = {
                {QStringLiteral("shortcuts.edit_polygon"), QStringLiteral("shortcuts.edit_shape")},
                {QStringLiteral("shortcuts.delete_polygon"), QStringLiteral("shortcuts.delete_shape")},
                {QStringLiteral("shortcuts.duplicate_polygon"), QStringLiteral("shortcuts.duplicate_shape")},
                {QStringLiteral("shortcuts.copy_polygon"), QStringLiteral("shortcuts.copy_shape")},
                {QStringLiteral("shortcuts.paste_polygon"), QStringLiteral("shortcuts.paste_shape")},
                {QStringLiteral("shortcuts.show_all_polygons"), QStringLiteral("shortcuts.show_all_shapes")},
                {QStringLiteral("shortcuts.hide_all_polygons"), QStringLiteral("shortcuts.hide_all_shapes")},
                {QStringLiteral("shortcuts.toggle_all_polygons"), QStringLiteral("shortcuts.toggle_all_shapes")},
            };
            for (auto it = shortcutMigrations.cbegin(); it != shortcutMigrations.cend(); ++it) {
                if (!fileValues.contains(it.value()) && fileValues.contains(it.key())) {
                    fileValues.insert(it.value(), fileValues.value(it.key()));
                }
            }
            for (auto it = fileValues.cbegin(); it != fileValues.cend(); ++it) {
                configValues.insert(it.key(), it.value());
            }
        } else {
            m_configFilePath.clear();
            statusBar()->showMessage(error, 5000);
        }
    }
    for (auto it = commandLineValues.cbegin(); it != commandLineValues.cend(); ++it) {
        configValues.insert(it.key(), it.value());
    }
    if (!cliLanguage.isEmpty()) {
        configValues.insert(QStringLiteral("language"), cliLanguage);
    }
    applyLabelMeConfig(configValues, cliLabels);
    if (!cliLabelFlagsSource.trimmed().isEmpty()) {
        const QString source = labelFlagPresetSourceText(cliLabelFlagsSource);
        m_settings.setValue(QStringLiteral("labelme/labelFlags"), cliLabelFlagsSource);
        m_labelFlagPresets = labelFlagPresetsFromSource(source);
        applyLabelFlagDefaults(&m_canvas->shapesRef());
        refreshLabels();
    }
    m_configOverrides = hasCommandLineOverride || hasInlineConfig;

    if (resetConfig) {
        resetWindowConfig();
        return;
    }

    if (positional.size() > 1 && QFileInfo::exists(positional.at(1))) {
        if (positional.size() > 2 && QFileInfo::exists(positional.at(2))) {
            const QString savedDefaultLabel = m_settings.value("defaultLabel").toString();
            loadPredefinedClassesFromFile(positional.at(2));
            mergePersistedLabelHistory();
            QSignalBlocker blocker(m_defaultLabelCombo);
            m_defaultLabelCombo->clear();
            m_defaultLabelCombo->addItems(m_classList);
            int defaultLabelIndex = m_defaultLabelCombo->findText(savedDefaultLabel);
            if (defaultLabelIndex >= 0) {
                m_defaultLabelCombo->setCurrentIndex(defaultLabelIndex);
            }
        }
        if (positional.size() > 3) {
            m_saveDir = QFileInfo(positional.at(3)).absoluteFilePath();
        }
        QFileInfo startupInfo(positional.at(1));
        if (startupInfo.isDir()) {
            m_dirPath = startupInfo.absoluteFilePath();
            m_settings.setValue("lastOpenDir", m_dirPath);
            m_imageList = scanImages(m_dirPath);
            populateFileList();
            m_currentImageIndex = 0;
            addRecentDir(m_dirPath);
            const QString preferred = preferredImageForCurrentDir();
            if (!preferred.isEmpty()) loadImage(preferred);
        } else {
            openPath(startupInfo.absoluteFilePath());
        }
    } else {
        QString lastFile = m_settings.value("filename").toString();
        if (!lastFile.isEmpty() && QFileInfo::exists(lastFile)) {
            loadImage(lastFile);
        }
    }
}

void MainWindow::resetWindowConfig() {
    m_settings.remove(QStringLiteral("window/size"));
    m_settings.remove(QStringLiteral("window/position"));
    m_settings.remove(QStringLiteral("window/state"));
    restoreState(m_defaultDockState, DockStateVersion);
    resize(QSize(1100, 700));
    move(QPoint(20, 20));
    m_settings.sync();
}

void MainWindow::applyLabelMeConfig(const QVariantMap &values, const QStringList &cliLabels) {
    auto setActionFromConfig = [&](QAction *action, const QString &key) {
        if (action && values.contains(key)) {
            action->setChecked(values.value(key).toBool());
        }
    };

    auto applyDockConfig = [&](QDockWidget *dock, QAction *toggleAction, const QString &prefix, bool applyVisibility) {
        if (!dock) {
            return;
        }
        QDockWidget::DockWidgetFeatures features = dock->features();
        auto setFeature = [&](QDockWidget::DockWidgetFeature feature, const QString &name) {
            const QString key = prefix + QLatin1Char('.') + name;
            if (!values.contains(key)) {
                return;
            }
            if (values.value(key).toBool()) {
                features |= feature;
            } else {
                features &= ~feature;
            }
        };
        setFeature(QDockWidget::DockWidgetClosable, QStringLiteral("closable"));
        setFeature(QDockWidget::DockWidgetMovable, QStringLiteral("movable"));
        setFeature(QDockWidget::DockWidgetFloatable, QStringLiteral("floatable"));
        dock->setFeatures(features);
        const QString showKey = prefix + QStringLiteral(".show");
        if (applyVisibility && values.contains(showKey)) {
            const bool visible = values.value(showKey).toBool();
            dock->setVisible(visible);
            if (toggleAction) {
                toggleAction->setChecked(visible);
            }
        }
    };

    applyDockConfig(m_fileDock, m_showFileDockAction, QStringLiteral("file_dock"), true);
    applyDockConfig(m_flagDock, m_showFlagDockAction, QStringLiteral("flag_dock"), true);
    applyDockConfig(m_labelDock, m_showLabelDockAction, QStringLiteral("label_dock"), true);
    applyDockConfig(m_shapeDock, m_showShapeDockAction, QStringLiteral("shape_dock"), true);

    setActionFromConfig(m_autoSaveAction, QStringLiteral("auto_save"));
    setActionFromConfig(m_keepPreviousAction, QStringLiteral("keep_prev"));
    setActionFromConfig(m_keepPreviousZoomAction, QStringLiteral("keep_prev_scale"));
    setActionFromConfig(m_keepPreviousBrightnessContrastAction,
                        QStringLiteral("keep_prev_brightness_contrast"));
    setActionFromConfig(m_embedImageDataAction, QStringLiteral("with_image_data"));
    setActionFromConfig(m_fillDrawingAction, QStringLiteral("canvas.fill_drawing"));
    if (values.contains(QStringLiteral("sort_labels"))) {
        m_labelMeSortLabels = values.value(QStringLiteral("sort_labels")).toBool();
    }
    if (values.contains(QStringLiteral("show_label_text_field"))) {
        m_labelMeShowLabelTextField = values.value(QStringLiteral("show_label_text_field")).toBool();
    }
    if (values.contains(QStringLiteral("label_completion"))) {
        const QString completion = values.value(QStringLiteral("label_completion")).toString().trimmed().toLower();
        if (completion == QStringLiteral("contains") || completion == QStringLiteral("startswith")) {
            m_labelMeLabelCompletion = completion;
        }
    }
    if (values.contains(QStringLiteral("fit_to_content.column"))) {
        m_labelMeFitToContentColumn = values.value(QStringLiteral("fit_to_content.column")).toBool();
    }
    if (values.contains(QStringLiteral("fit_to_content.row"))) {
        m_labelMeFitToContentRow = values.value(QStringLiteral("fit_to_content.row")).toBool();
    }
    const QHash<QString, QAction *> shortcutActions = {
        {QStringLiteral("close"), m_closeAction},
        {QStringLiteral("quit"), m_quitAction},
        {QStringLiteral("open"), m_openAction},
        {QStringLiteral("open_dir"), m_openDirAction},
        {QStringLiteral("save"), m_saveAction},
        {QStringLiteral("save_as"), m_saveAsAction},
        {QStringLiteral("save_to"), m_changeSaveDirAction},
        {QStringLiteral("delete_file"), m_deleteAnnotationAction},
        {QStringLiteral("open_next"), m_nextAction},
        {QStringLiteral("open_prev"), m_prevAction},
        {QStringLiteral("zoom_in"), m_zoomInAction},
        {QStringLiteral("zoom_out"), m_zoomOutAction},
        {QStringLiteral("zoom_to_original"), m_zoomOriginalAction},
        {QStringLiteral("fit_window"), m_fitWindowAction},
        {QStringLiteral("fit_width"), m_fitWidthAction},
        {QStringLiteral("create_polygon"), m_createPolygonModeAction},
        {QStringLiteral("create_rectangle"), m_createModeAction},
        {QStringLiteral("create_oriented_rectangle"), m_createOrientedRectangleModeAction},
        {QStringLiteral("create_circle"), m_createCircleModeAction},
        {QStringLiteral("create_line"), m_createLineModeAction},
        {QStringLiteral("create_point"), m_createPointModeAction},
        {QStringLiteral("create_linestrip"), m_createLinestripModeAction},
        {QStringLiteral("create_points"), m_createPointsModeAction},
        {QStringLiteral("create_mask"), m_createMaskModeAction},
        {QStringLiteral("create_ai_points"), m_createAiPointsModeAction},
        {QStringLiteral("create_ai_box"), m_createAiBoxModeAction},
        {QStringLiteral("edit_shape"), m_editModeAction},
        {QStringLiteral("delete_shape"), m_deleteAction},
        {QStringLiteral("duplicate_shape"), m_copyAction},
        {QStringLiteral("copy_shape"), m_copyShapesAction},
        {QStringLiteral("paste_shape"), m_pasteShapesAction},
        {QStringLiteral("undo"), m_undoAction},
        {QStringLiteral("undo_last_point"), m_undoLastPointAction},
        {QStringLiteral("edit_label"), m_editLabelAction},
        {QStringLiteral("toggle_keep_prev_mode"), m_keepPreviousAction},
        {QStringLiteral("remove_selected_point"), m_removeSelectedPointAction},
        {QStringLiteral("show_all_shapes"), m_showAllAction},
        {QStringLiteral("hide_all_shapes"), m_hideAllAction},
        {QStringLiteral("toggle_all_shapes"), m_toggleAllAction}};
    for (auto it = shortcutActions.cbegin(); it != shortcutActions.cend(); ++it) {
        const QString key = QStringLiteral("shortcuts.") + it.key();
        if (!values.contains(key) || !it.value()) {
            continue;
        }
        QList<QKeySequence> shortcuts;
        for (const QString &shortcut : LabelMeConfig::stringList(values.value(key))) {
            const QKeySequence sequence(shortcut, QKeySequence::PortableText);
            if (!sequence.isEmpty()) {
                shortcuts.append(sequence);
            }
        }
        it.value()->setShortcuts(shortcuts);
    }
    if (values.contains(QStringLiteral("epsilon"))) {
        m_canvas->setEpsilon(values.value(QStringLiteral("epsilon")).toDouble());
    }
    if (values.contains(QStringLiteral("shape.point_size"))) {
        m_canvas->setPointSize(values.value(QStringLiteral("shape.point_size")).toInt());
    }
    if (values.contains(QStringLiteral("canvas.num_backups"))) {
        m_labelMeNumBackups = qBound(1, values.value(QStringLiteral("canvas.num_backups")).toInt(), 10000);
    }
    if (values.contains(QStringLiteral("shape_color"))) {
        const QString mode = values.value(QStringLiteral("shape_color")).toString().trimmed().toLower();
        m_labelMeShapeColorMode = (mode == QStringLiteral("auto") || mode == QStringLiteral("manual"))
                                      ? mode
                                      : QStringLiteral("default");
    }
    if (values.contains(QStringLiteral("default_shape_color"))) {
        const QColor configured = colorFromConfig(values.value(QStringLiteral("default_shape_color")),
                                                   m_labelMeDefaultShapeColor);
        if (configured.isValid()) {
            m_labelMeDefaultShapeColor = configured;
            m_labelMeDefaultShapeColor.setAlpha(255);
        }
    }
    if (values.contains(QStringLiteral("shift_auto_shape_color"))) {
        m_labelMeShiftAutoShapeColor = values.value(QStringLiteral("shift_auto_shape_color")).toInt();
    }
    m_labelMeLabelColors.clear();
    const QVariantMap inlineLabelColors = values.value(QStringLiteral("label_colors")).toMap();
    for (auto it = inlineLabelColors.cbegin(); it != inlineLabelColors.cend(); ++it) {
        const QColor configured = colorFromConfig(it.value(), QColor());
        if (configured.isValid()) {
            QColor rgb = configured;
            rgb.setAlpha(255);
            m_labelMeLabelColors.insert(it.key(), rgb);
        }
    }
    for (auto it = values.cbegin(); it != values.cend(); ++it) {
        const QString prefix = QStringLiteral("label_colors.");
        if (!it.key().startsWith(prefix)) {
            continue;
        }
        const QColor configured = colorFromConfig(it.value(), QColor());
        if (configured.isValid()) {
            QColor rgb = configured;
            rgb.setAlpha(255);
            m_labelMeLabelColors.insert(it.key().mid(prefix.size()), rgb);
        }
    }
    const QStringList crosshairShapeTypes = {
        QStringLiteral("polygon"),
        QStringLiteral("rectangle"),
        QStringLiteral("oriented_rectangle"),
        QStringLiteral("circle"),
        QStringLiteral("line"),
        QStringLiteral("point"),
        QStringLiteral("linestrip"),
        QStringLiteral("points"),
        QStringLiteral("mask"),
        QStringLiteral("ai_points_to_shape"),
        QStringLiteral("ai_box_to_shape")};
    for (const QString &shapeType : crosshairShapeTypes) {
        const QString key = QStringLiteral("canvas.crosshair.") + shapeType;
        if (values.contains(key)) {
            m_canvas->setCrosshairEnabledForShapeType(shapeType, values.value(key).toBool());
        }
    }
    if (values.contains(QStringLiteral("canvas.double_click"))) {
        const QString doubleClick = values.value(QStringLiteral("canvas.double_click")).toString().trimmed().toLower();
        m_canvas->setDoubleClickClose(doubleClick.isEmpty() || doubleClick == QStringLiteral("close"));
    }
    if (values.contains(QStringLiteral("canvas.snapping"))) {
        m_canvas->setSnapping(values.value(QStringLiteral("canvas.snapping")).toBool());
    }

    if (values.contains(QStringLiteral("display_label_popup"))) {
        m_settings.setValue(QStringLiteral("labelme/displayLabelPopup"),
                            values.value(QStringLiteral("display_label_popup")).toBool());
    }
    if (values.contains(QStringLiteral("file_search")) && m_fileSearchEdit) {
        m_fileSearchEdit->setText(values.value(QStringLiteral("file_search")).toString());
    }
    if (values.contains(QStringLiteral("validate_label"))) {
        m_validateLabelPolicy = values.value(QStringLiteral("validate_label")).toString().trimmed().toLower();
        if (m_validateLabelPolicy != QStringLiteral("exact")) {
            m_validateLabelPolicy.clear();
        }
        m_settings.setValue(QStringLiteral("labelme/validateLabel"), m_validateLabelPolicy);
    }
    if (values.contains(QStringLiteral("language"))) {
        const QString language = values.value(QStringLiteral("language")).toString().trimmed();
        if (!language.isEmpty() && language != m_strings.language()) {
            changeLanguage(language);
        }
    }
    if (values.contains(QStringLiteral("ai.default")) && m_aiModelCombo) {
        const QString requestedModel = LabelMeConfig::migrateAiModelName(
                                           values.value(QStringLiteral("ai.default")).toString().trimmed());
        int modelIndex = m_aiModelCombo->findData(requestedModel);
        if (modelIndex < 0) {
            for (int index = 0; index < m_aiModelCombo->count(); ++index) {
                if (m_aiModelCombo->itemText(index).compare(requestedModel, Qt::CaseInsensitive) == 0) {
                    modelIndex = index;
                    break;
                }
            }
        }
        if (modelIndex >= 0) {
            m_aiModelCombo->setCurrentIndex(modelIndex);
        }
    }

    if (values.contains(QStringLiteral("shape.line_color"))) {
        m_lineColor = colorFromConfig(values.value(QStringLiteral("shape.line_color")), m_lineColor);
        m_canvas->setLineColor(m_lineColor);
    }
    if (values.contains(QStringLiteral("shape.fill_color"))) {
        m_fillColor = colorFromConfig(values.value(QStringLiteral("shape.fill_color")), m_fillColor);
        m_canvas->setFillColor(m_fillColor);
    }
    if (values.contains(QStringLiteral("shape.vertex_fill_color"))) {
        m_canvas->setVertexFillColor(colorFromConfig(values.value(QStringLiteral("shape.vertex_fill_color")),
                                                     m_canvas->vertexFillColor()));
    }
    if (values.contains(QStringLiteral("shape.hvertex_fill_color"))) {
        m_canvas->setHoverVertexFillColor(colorFromConfig(values.value(QStringLiteral("shape.hvertex_fill_color")),
                                                          m_canvas->hoverVertexFillColor()));
    }
    if (values.contains(QStringLiteral("shape.select_line_color"))) {
        m_canvas->setSelectedLineColor(colorFromConfig(values.value(QStringLiteral("shape.select_line_color")),
                                                        m_canvas->selectedLineColor()));
    }
    if (values.contains(QStringLiteral("shape.select_fill_color"))) {
        m_canvas->setSelectedFillColor(colorFromConfig(values.value(QStringLiteral("shape.select_fill_color")),
                                                        m_canvas->selectedFillColor()));
    }

    if (values.contains(QStringLiteral("flags"))) {
        m_labelMeConfiguredFlags.clear();
        for (const QString &flag : LabelMeConfig::stringList(values.value(QStringLiteral("flags")))) {
            m_labelMeConfiguredFlags.insert(flag, false);
        }
        for (auto it = m_labelMeConfiguredFlags.cbegin(); it != m_labelMeConfiguredFlags.cend(); ++it) {
            if (!m_labelMeTopLevelFlags.contains(it.key())) {
                m_labelMeTopLevelFlags.insert(it.key(), it.value());
            }
        }
        refreshTopLevelFlagsList();
    }

    const QString labelFlagSource = labelFlagPresetSourceFromConfig(values);
    if (!labelFlagSource.isEmpty()) {
        m_settings.setValue(QStringLiteral("labelme/labelFlags"), labelFlagSource);
        m_labelFlagPresets = labelFlagPresetsFromSource(labelFlagSource);
        applyLabelFlagDefaults(&m_canvas->shapesRef());
    }

    const QStringList configuredLabels = cliLabels.isEmpty()
                                             ? LabelMeConfig::stringList(values.value(QStringLiteral("labels")))
                                             : cliLabels;
    if (!configuredLabels.isEmpty()) {
        const QString savedDefaultLabel = m_settings.value(QStringLiteral("defaultLabel")).toString();
        m_classList.clear();
        QSignalBlocker blocker(m_defaultLabelCombo);
        m_defaultLabelCombo->clear();
        for (const QString &label : configuredLabels) {
            addClassLabel(label);
        }
        const int savedIndex = m_defaultLabelCombo->findText(savedDefaultLabel);
        if (savedIndex >= 0) {
            m_defaultLabelCombo->setCurrentIndex(savedIndex);
        } else if (m_defaultLabelCombo->count() > 0) {
            m_defaultLabelCombo->setCurrentIndex(0);
        }
        m_settings.setValue(QStringLiteral("labelHistory"), m_classList);
        if (m_useDefaultLabel && m_useDefaultLabel->isChecked()) {
            rememberLastUsedLabel(m_defaultLabelCombo->currentText());
        }
    }

    m_canvas->setFillDrawing(m_fillDrawingAction && m_fillDrawingAction->isChecked());
    refreshLabels();
    refreshActions();
    updateFileDockTitle();
}

bool MainWindow::persistLabelMeConfigValue(const QString &key, const QVariant &value) {
    if (m_configFilePath.isEmpty() || m_configOverrides) {
        return true;
    }
    QString error;
    if (!LabelMeConfig::setFileValue(m_configFilePath, key, value, &error)) {
        statusBar()->showMessage(error, 5000);
        return false;
    }
    return true;
}

void MainWindow::createUi() {
    m_canvas = new Canvas(this);
    m_canvas->setLanguage(m_strings.language());
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setObjectName(QStringLiteral("scrollArea"));
    m_scrollArea->setWidget(m_canvas);
    m_scrollArea->setWidgetResizable(false);
    m_scrollArea->setAlignment(Qt::AlignCenter);
    setCentralWidget(m_scrollArea);

    auto *labelListWidget = new LabelListWidget(this);
    m_labelList = labelListWidget;
    m_labelList->setObjectName("labelList");
    m_labelList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_labelList->setDragEnabled(true);
    m_labelList->setAcceptDrops(true);
    m_labelList->setDropIndicatorShown(true);
    m_labelList->setDragDropMode(QAbstractItemView::InternalMove);
    m_labelList->setDefaultDropAction(Qt::MoveAction);
    labelListWidget->setSelectionRestoreCallback([this](const QVector<int> &rows) {
        m_labelList->clearSelection();
        for (const int row : rows) {
            if (row >= 0 && row < m_labelList->count()) {
                m_labelList->item(row)->setSelected(true);
            }
        }
        QVector<int> selectedRows;
        for (QListWidgetItem *item : m_labelList->selectedItems()) {
            const int row = m_labelList->row(item);
            if (row >= 0) {
                selectedRows.append(row);
            }
        }
        m_canvas->setSelectedIndices(selectedRows);
    });

    m_uniqueLabelList = new QListWidget(this);
    m_uniqueLabelList->setObjectName(QStringLiteral("uniqueLabelList"));
    m_uniqueLabelList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_uniqueLabelList->setMaximumHeight(96);
    m_uniqueLabelList->setUniformItemSizes(true);
    m_uniqueLabelList->setWordWrap(false);
    m_uniqueLabelList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_uniqueLabelList->setToolTip(m_strings.get(QStringLiteral("uniqueLabelTooltip")));
    m_uniqueLabelList->setStyleSheet(QStringLiteral(
        "QListWidget { outline: 0; }"
        "QListWidget::item { padding: 2px 6px; min-height: 20px; }"
        "QListWidget::item:selected { background: #d8ecff; color: #111827; border-left: 3px solid #2f80ed; }"
        "QListWidget::item:hover { background: #edf6ff; }"));
    refreshUniqueLabelList();
    m_filterCombo = new QComboBox(this);
    m_filterCombo->setObjectName("filterCombo");
    m_aiModelCombo = new QComboBox(this);
    m_aiModelCombo->setObjectName(QStringLiteral("aiModelCombo"));
    m_aiModelCombo->addItem(QStringLiteral("EfficientSam (speed)"), QStringLiteral("efficientsam:10m"));
    m_aiModelCombo->addItem(QStringLiteral("EfficientSam (accuracy)"), QStringLiteral("efficientsam:latest"));
    m_aiModelCombo->addItem(QStringLiteral("Sam (speed)"), QStringLiteral("sam:100m"));
    m_aiModelCombo->addItem(QStringLiteral("Sam (balanced)"), QStringLiteral("sam:300m"));
    m_aiModelCombo->addItem(QStringLiteral("Sam (accuracy)"), QStringLiteral("sam:latest"));
    m_aiModelCombo->addItem(QStringLiteral("Sam2 (speed)"), QStringLiteral("sam2:small"));
    m_aiModelCombo->addItem(QStringLiteral("Sam2 (balanced)"), QStringLiteral("sam2:latest"));
    m_aiModelCombo->addItem(QStringLiteral("Sam2 (accuracy)"), QStringLiteral("sam2:large"));
    m_aiModelCombo->addItem(QStringLiteral("Sam3"), QStringLiteral("sam3:latest"));
    m_aiOutputFormatCombo = new QComboBox(this);
    m_aiOutputFormatCombo->setObjectName(QStringLiteral("aiOutputFormatCombo"));
    for (const QString &format : AiAssistBridge::supportedOutputFormats()) {
        m_aiOutputFormatCombo->addItem(format, format);
    }
    const int savedAiModel = m_aiModelCombo->findData(m_settings.value(QStringLiteral("ai/model"), QStringLiteral("sam2:latest")));
    if (savedAiModel >= 0) {
        m_aiModelCombo->setCurrentIndex(savedAiModel);
    }
    const int savedAiFormat = m_aiOutputFormatCombo->findData(m_settings.value(QStringLiteral("ai/outputFormat"), QStringLiteral("polygon")));
    if (savedAiFormat >= 0) {
        m_aiOutputFormatCombo->setCurrentIndex(savedAiFormat);
    }
    m_aiTextPromptEdit = new QLineEdit(this);
    m_aiTextPromptEdit->setObjectName(QStringLiteral("aiTextPromptEdit"));
    m_aiTextPromptEdit->setPlaceholderText(QStringLiteral("person, sofa"));
    m_aiTextPromptEdit->setText(m_settings.value(QStringLiteral("ai/textPrompt")).toString());

    m_aiTextModelCombo = new QComboBox(this);
    m_aiTextModelCombo->setObjectName(QStringLiteral("aiTextModelCombo"));
    m_aiTextModelCombo->addItem(QStringLiteral("SAM3 (smart)"), QStringLiteral("sam3:latest"));
    m_aiTextModelCombo->addItem(QStringLiteral("YOLO-World (fast)"), QStringLiteral("yoloworld:latest"));
    const int savedAiTextModel = m_aiTextModelCombo->findData(
        m_settings.value(QStringLiteral("ai/textModel"), QStringLiteral("yoloworld:latest")));
    if (savedAiTextModel >= 0) {
        m_aiTextModelCombo->setCurrentIndex(savedAiTextModel);
    }

    m_aiTextScoreSpin = new QDoubleSpinBox(this);
    m_aiTextScoreSpin->setObjectName(QStringLiteral("aiTextScoreSpin"));
    m_aiTextScoreSpin->setRange(0.0, 1.0);
    m_aiTextScoreSpin->setSingleStep(0.05);
    m_aiTextScoreSpin->setDecimals(2);
    m_aiTextScoreSpin->setValue(m_settings.value(QStringLiteral("ai/textScore"), 0.1).toDouble());

    m_aiTextIouSpin = new QDoubleSpinBox(this);
    m_aiTextIouSpin->setObjectName(QStringLiteral("aiTextIouSpin"));
    m_aiTextIouSpin->setRange(0.0, 1.0);
    m_aiTextIouSpin->setSingleStep(0.05);
    m_aiTextIouSpin->setDecimals(2);
    m_aiTextIouSpin->setValue(m_settings.value(QStringLiteral("ai/textIou"), 0.5).toDouble());

    m_aiTextRunButton = new QToolButton(this);
    m_aiTextRunButton->setObjectName(QStringLiteral("aiTextRunButton"));
    m_aiTextRunButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_aiProgressBar = new QProgressBar(this);
    m_aiProgressBar->setObjectName(QStringLiteral("aiProgressBar"));
    m_aiProgressBar->setRange(0, 0);
    m_aiProgressBar->setTextVisible(false);
    m_aiProgressBar->setFixedHeight(8);
    m_aiProgressBar->setVisible(false);
    m_aiCancelButton = new QToolButton(this);
    m_aiCancelButton->setObjectName(QStringLiteral("aiCancelButton"));
    m_aiCancelButton->setAutoRaise(true);
    m_aiCancelButton->setText(QStringLiteral("×"));
    m_aiCancelButton->setVisible(false);
    m_defaultLabelCombo = new QComboBox(this);
    m_defaultLabelCombo->setObjectName("defaultLabelCombo");
    m_defaultLabelCombo->addItems(m_classList);
    m_useDefaultLabel = new QCheckBox(this);
    m_useDefaultLabel->setObjectName("useDefaultLabel");
    m_useDefaultLabel->setChecked(m_settings.value("useDefaultLabel", false).toBool());
    m_difficult = new QCheckBox(this);
    m_difficult->setObjectName(QStringLiteral("difficultCheckBox"));
    auto *topLevelFlagsLabel = new QLabel(m_strings.get(QStringLiteral("topLevelFlags")), this);
    topLevelFlagsLabel->setObjectName(QStringLiteral("topLevelFlagsLabel"));
    m_topLevelFlagsEdit = new QPlainTextEdit(this);
    m_topLevelFlagsEdit->setObjectName(QStringLiteral("topLevelFlagsEdit"));
    m_topLevelFlagsEdit->setPlaceholderText(QStringLiteral("key=true\nother=false"));
    m_topLevelFlagsEdit->setFixedHeight(72);
    const QString defaultLabel = m_settings.value("defaultLabel").toString();
    int defaultLabelIndex = m_defaultLabelCombo->findText(defaultLabel);
    if (defaultLabelIndex >= 0) {
        m_defaultLabelCombo->setCurrentIndex(defaultLabelIndex);
    }

    QWidget *labelPanel = new QWidget(this);
    QVBoxLayout *labelLayout = new QVBoxLayout(labelPanel);
    labelLayout->setContentsMargins(0, 0, 0, 0);
    auto *newLabelTitle = new QLabel(m_strings.get(QStringLiteral("newLabel")), labelPanel);
    newLabelTitle->setObjectName(QStringLiteral("newLabelTitle"));
    labelLayout->addWidget(newLabelTitle);
    labelLayout->addWidget(m_uniqueLabelList);
    labelLayout->addWidget(m_useDefaultLabel);
    labelLayout->addWidget(m_defaultLabelCombo);
    labelLayout->addWidget(m_difficult);
    labelLayout->addWidget(m_filterCombo);
    auto *aiModelLabel = new QLabel(m_strings.get(QStringLiteral("aiModel")), labelPanel);
    aiModelLabel->setObjectName(QStringLiteral("aiModelLabel"));
    labelLayout->addWidget(aiModelLabel);
    labelLayout->addWidget(m_aiModelCombo);
    auto *aiOutputLabel = new QLabel(m_strings.get(QStringLiteral("aiOutput")), labelPanel);
    aiOutputLabel->setObjectName(QStringLiteral("aiOutputLabel"));
    labelLayout->addWidget(aiOutputLabel);
    labelLayout->addWidget(m_aiOutputFormatCombo);
    auto *aiTextPromptLabel = new QLabel(m_strings.get(QStringLiteral("aiTextPrompt")), labelPanel);
    aiTextPromptLabel->setObjectName(QStringLiteral("aiTextPromptLabel"));
    labelLayout->addWidget(aiTextPromptLabel);
    auto *aiTextPromptRow = new QHBoxLayout;
    aiTextPromptRow->setContentsMargins(0, 0, 0, 0);
    aiTextPromptRow->addWidget(m_aiTextPromptEdit, 1);
    aiTextPromptRow->addWidget(m_aiTextRunButton);
    labelLayout->addLayout(aiTextPromptRow);
    auto *aiTextSettingsRow = new QHBoxLayout;
    aiTextSettingsRow->setContentsMargins(0, 0, 0, 0);
    aiTextSettingsRow->addWidget(m_aiTextModelCombo, 1);
    auto *aiScoreLabel = new QLabel(m_strings.get(QStringLiteral("score")), labelPanel);
    aiScoreLabel->setObjectName(QStringLiteral("aiScoreLabel"));
    aiTextSettingsRow->addWidget(aiScoreLabel);
    aiTextSettingsRow->addWidget(m_aiTextScoreSpin);
    auto *aiIouLabel = new QLabel(m_strings.get(QStringLiteral("iou")), labelPanel);
    aiIouLabel->setObjectName(QStringLiteral("aiIouLabel"));
    aiTextSettingsRow->addWidget(aiIouLabel);
    aiTextSettingsRow->addWidget(m_aiTextIouSpin);
    labelLayout->addLayout(aiTextSettingsRow);
    auto *aiProgressRow = new QHBoxLayout;
    aiProgressRow->setContentsMargins(0, 0, 0, 0);
    aiProgressRow->addWidget(m_aiProgressBar, 1);
    aiProgressRow->addWidget(m_aiCancelButton);
    labelLayout->addLayout(aiProgressRow);
    labelLayout->addWidget(topLevelFlagsLabel);
    labelLayout->addWidget(m_topLevelFlagsEdit);
    m_fileList = new QListWidget(this);
    m_fileList->setObjectName("fileList");
    QFont fileFont = m_fileList->font();
    fileFont.setPointSize(qMax(fileFont.pointSize(), 10));
    m_fileList->setFont(fileFont);
    m_fileList->setWordWrap(false);
    m_fileList->setTextElideMode(Qt::ElideMiddle);
    m_fileList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_fileList->setStyleSheet(QStringLiteral(
        "QListWidget { outline: 0; }"
        "QListWidget::item { padding: 2px 6px; min-height: 20px; }"
        "QListWidget::item[activeFile=\"true\"] { background: #e7f5ec; color: #0f3d24; }"
        "QListWidget::item:selected { background: #d8ecff; color: #111827; border-left: 3px solid #2f80ed; }"
        "QListWidget::item:selected:active { background: #c7e3ff; }"
        "QListWidget::item:hover { background: #edf6ff; }"));
    m_fileDock = new QDockWidget(this);
    m_fileDock->setObjectName("files");
    m_fileDock->setWidget(m_fileList);
    m_fileLabelFilterMenu = new QMenu(this);
    m_fileLabelFilterMenu->setObjectName(QStringLiteral("fileLabelFilterMenu"));
    auto *fileDockTitleBar = new QWidget(m_fileDock);
    fileDockTitleBar->setObjectName(QStringLiteral("fileDockTitleBar"));
    auto *fileDockTitleLayout = new QHBoxLayout(fileDockTitleBar);
    fileDockTitleLayout->setContentsMargins(6, 0, 2, 0);
    fileDockTitleLayout->setSpacing(4);
    m_fileDockTitleLabel = new QLabel(fileDockTitleBar);
    m_fileDockTitleLabel->setObjectName(QStringLiteral("fileDockTitleLabel"));
    m_fileDockTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_fileDockTitleLabel->setTextInteractionFlags(Qt::NoTextInteraction);
    m_fileSearchEdit = new QLineEdit(fileDockTitleBar);
    m_fileSearchEdit->setObjectName(QStringLiteral("fileSearchEdit"));
    m_fileSearchEdit->setClearButtonEnabled(true);
    m_fileSearchEdit->setFixedWidth(118);
    m_fileSearchEdit->setPlaceholderText(m_strings.get(QStringLiteral("fileSearchPlaceholder")));
    m_fileSearchEdit->setToolTip(m_strings.get(QStringLiteral("fileSearchTooltip")));
    m_fileLabelFilterButton = new QToolButton(fileDockTitleBar);
    m_fileLabelFilterButton->setObjectName(QStringLiteral("fileLabelFilterButton"));
    m_fileLabelFilterButton->setAutoRaise(true);
    m_fileLabelFilterButton->setPopupMode(QToolButton::InstantPopup);
    m_fileLabelFilterButton->setMenu(m_fileLabelFilterMenu);
    m_fileDockCloseButton = new QToolButton(fileDockTitleBar);
    m_fileDockCloseButton->setObjectName(QStringLiteral("fileDockCloseButton"));
    m_fileDockCloseButton->setAutoRaise(true);
    m_fileDockCloseButton->setText(QStringLiteral("x"));
    m_fileDockCloseButton->setToolTip(m_strings.get(QStringLiteral("hideFileList")));
    connect(m_fileDockCloseButton, &QToolButton::clicked, m_fileDock, &QDockWidget::hide);
    fileDockTitleLayout->addWidget(m_fileDockTitleLabel, 1);
    fileDockTitleLayout->addWidget(m_fileSearchEdit);
    fileDockTitleLayout->addWidget(m_fileLabelFilterButton);
    fileDockTitleLayout->addWidget(m_fileDockCloseButton);
    m_fileDock->setTitleBarWidget(fileDockTitleBar);
    addDockWidget(Qt::RightDockWidgetArea, m_fileDock);
    m_showFileDockAction = m_fileDock->toggleViewAction();
    m_showFileDockAction->setObjectName(QStringLiteral("showFileDockAction"));

    m_labelDock = new QDockWidget(this);
    m_labelDock->setObjectName("labels");
    m_labelDock->setWidget(labelPanel);
    addDockWidget(Qt::RightDockWidgetArea, m_labelDock);
    m_showLabelDockAction = m_labelDock->toggleViewAction();
    m_showLabelDockAction->setObjectName(QStringLiteral("showLabelDockAction"));

    m_shapeDock = new QDockWidget(this);
    m_shapeDock->setObjectName(QStringLiteral("shapeLabels"));
    m_shapeDock->setWidget(m_labelList);
    addDockWidget(Qt::RightDockWidgetArea, m_shapeDock);
    m_showShapeDockAction = m_shapeDock->toggleViewAction();
    m_showShapeDockAction->setObjectName(QStringLiteral("showShapeDockAction"));

    m_flagList = new QListWidget(this);
    m_flagList->setObjectName(QStringLiteral("flagList"));
    m_flagList->setSelectionMode(QAbstractItemView::NoSelection);
    m_flagDock = new QDockWidget(this);
    m_flagDock->setObjectName(QStringLiteral("flags"));
    m_flagDock->setWidget(m_flagList);
    addDockWidget(Qt::RightDockWidgetArea, m_flagDock);
    splitDockWidget(m_flagDock, m_labelDock, Qt::Vertical);
    m_showFlagDockAction = m_flagDock->toggleViewAction();
    m_showFlagDockAction->setObjectName(QStringLiteral("showFlagDockAction"));
    splitDockWidget(m_labelDock, m_shapeDock, Qt::Vertical);
    splitDockWidget(m_shapeDock, m_fileDock, Qt::Vertical);

    m_coordinates = new QLabel(this);
    m_performanceLabel = new QLabel(this);
    m_performanceLabel->setObjectName("performanceLabel");
    m_performanceLabel->setStyleSheet(QStringLiteral("QLabel { color: #777; padding: 0 8px; }"));
    statusBar()->addWidget(m_performanceLabel, 0);
    statusBar()->addPermanentWidget(m_coordinates);

    m_miniMapOverlay = new MiniMapOverlay(m_canvas, m_scrollArea, m_scrollArea->viewport());
    m_miniMapOverlay->setObjectName("miniMapOverlay");
    m_performanceMonitor = new PerformanceMonitor(this);
    updatePerformanceLabel();
}

void MainWindow::createActions() {
    m_openAction = new QAction(this);
    m_openAction->setShortcut(QKeySequence::Open);
    m_openDirAction = new QAction(this);
    m_openDirAction->setShortcut(QKeySequence("Ctrl+U"));
    m_openAnnotationAction = new QAction(this);
    m_openAnnotationAction->setShortcut(QKeySequence("Ctrl+Shift+O"));
    m_openWithImageViewerAction = new QAction(this);
    m_openWithImageViewerAction->setObjectName(QStringLiteral("openWithImageViewerAction"));
    m_openFileLocationAction = new QAction(this);
    m_openFileLocationAction->setObjectName(QStringLiteral("openFileLocationAction"));
    m_fileContextOpenAction = new QAction(this);
    m_fileContextOpenAction->setObjectName(QStringLiteral("fileContextOpenAction"));
    m_fileContextRevealAction = new QAction(this);
    m_fileContextRevealAction->setObjectName(QStringLiteral("fileContextRevealAction"));
    m_fileContextCopyPathAction = new QAction(this);
    m_fileContextCopyPathAction->setObjectName(QStringLiteral("fileContextCopyPathAction"));
    m_fileContextMarkAction = new QAction(this);
    m_fileContextMarkAction->setObjectName(QStringLiteral("fileContextMarkAction"));
    m_fileContextDeleteAction = new QAction(this);
    m_fileContextDeleteAction->setObjectName(QStringLiteral("fileContextDeleteAction"));
    m_closeAction = new QAction(this);
    m_closeAction->setShortcut(QKeySequence("Ctrl+W"));
    m_quitAction = new QAction(this);
    m_quitAction->setObjectName(QStringLiteral("quitAction"));
    m_quitAction->setShortcut(QKeySequence("Ctrl+Q"));
    m_quitAction->setMenuRole(QAction::QuitRole);
    m_resetAllAction = new QAction(this);
    m_resetLayoutAction = new QAction(this);
    m_resetLayoutAction->setObjectName(QStringLiteral("resetLayoutAction"));
    m_saveAction = new QAction(this);
    m_saveAction->setObjectName(QStringLiteral("saveAction"));
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAsAction = new QAction(this);
    m_saveAsAction->setObjectName(QStringLiteral("saveAsAction"));
    m_saveAsAction->setShortcut(QKeySequence::SaveAs);
    m_changeSaveDirAction = new QAction(this);
    m_changeSaveDirAction->setObjectName(QStringLiteral("changeSaveDirAction"));
    m_changeSaveDirAction->setShortcut(QKeySequence("Ctrl+Shift+R"));
    m_formatAction = new QAction(this);
    m_formatAction->setObjectName(QStringLiteral("formatAction"));
    m_nextAction = new QAction(this);
    m_nextAction->setShortcut(QKeySequence("D"));
    m_prevAction = new QAction(this);
    m_prevAction->setShortcut(QKeySequence("A"));
    m_nextCopyAction = new QAction(this);
    m_nextCopyAction->setObjectName(QStringLiteral("nextCopyAction"));
    m_nextCopyAction->setShortcut(QKeySequence("Ctrl+Shift+D"));
    m_nextCopyAction->setText(QStringLiteral("Next Image (Copy Labels)"));
    m_prevCopyAction = new QAction(this);
    m_prevCopyAction->setObjectName(QStringLiteral("prevCopyAction"));
    m_prevCopyAction->setShortcut(QKeySequence("Ctrl+Shift+A"));
    m_prevCopyAction->setText(QStringLiteral("Prev Image (Copy Labels)"));
    addAction(m_nextCopyAction);
    addAction(m_prevCopyAction);
    m_verifyAction = new QAction(this);
    m_verifyAction->setObjectName(QStringLiteral("verifyAction"));
    m_verifyAction->setShortcut(QKeySequence("Space"));
    m_verifyAction->setCheckable(true);
    m_editLabelAction = new QAction(this);
    m_editLabelAction->setShortcut(QKeySequence("Ctrl+E"));
    m_undoAction = new QAction(this);
    m_undoAction->setShortcut(QKeySequence::Undo);
    m_undoLastPointAction = new QAction(this);
    m_undoLastPointAction->setObjectName(QStringLiteral("undoLastPointAction"));
    // LabelMe uses the same default shortcut for shape-history undo and
    // undoing the last draft point. Only the action enabled for the current
    // drawing state handles Ctrl+Z.
    m_undoLastPointAction->setShortcut(QKeySequence::Undo);
    m_redoAction = new QAction(this);
    m_redoAction->setShortcut(QKeySequence::Redo);
    m_prevShapeAction = new QAction(this);
    m_prevShapeAction->setObjectName(QStringLiteral("prevShapeAction"));
    m_prevShapeAction->setShortcut(QKeySequence(QStringLiteral("Q")));
    m_nextShapeAction = new QAction(this);
    m_nextShapeAction->setObjectName(QStringLiteral("nextShapeAction"));
    m_nextShapeAction->setShortcut(QKeySequence(QStringLiteral("E")));
    m_deleteAction = new QAction(this);
    m_deleteAction->setShortcuts({QKeySequence(Qt::Key_Delete), QKeySequence(QStringLiteral("X"))});
    m_deleteAllShapesAction = new QAction(this);
    m_deleteAllShapesAction->setObjectName(QStringLiteral("deleteAllShapesAction"));
    m_copyAction = new QAction(this);
    m_copyAction->setShortcut(QKeySequence("Ctrl+D"));
    m_copyShapesAction = new QAction(this);
    m_copyShapesAction->setObjectName(QStringLiteral("copyShapesAction"));
    m_copyShapesAction->setShortcut(QKeySequence::Copy);
    m_pasteShapesAction = new QAction(this);
    m_pasteShapesAction->setObjectName(QStringLiteral("pasteShapesAction"));
    m_pasteShapesAction->setShortcut(QKeySequence::Paste);
    m_copyHereAction = new QAction(this);
    m_copyHereAction->setObjectName(QStringLiteral("copyHereAction"));
    m_moveHereAction = new QAction(this);
    m_moveHereAction->setObjectName(QStringLiteral("moveHereAction"));
    m_insertPolygonPointAction = new QAction(this);
    m_insertPolygonPointAction->setObjectName(QStringLiteral("insertPolygonPointAction"));
    m_addPointToEdgeAction = new QAction(this);
    m_addPointToEdgeAction->setObjectName(QStringLiteral("addPointToEdgeAction"));
    m_removePolygonPointAction = new QAction(this);
    m_removePolygonPointAction->setObjectName(QStringLiteral("removePolygonPointAction"));
    m_removeSelectedPointAction = new QAction(this);
    m_removeSelectedPointAction->setObjectName(QStringLiteral("removeSelectedPointAction"));
    m_removeSelectedPointAction->setShortcuts({QKeySequence(Qt::Key_Backspace), QKeySequence(QStringLiteral("Meta+H"))});
    m_copyPreviousAction = new QAction(this);
    m_copyPreviousAction->setShortcut(QKeySequence("Ctrl+Shift+V"));
    m_deleteImageAction = new QAction(this);
    m_deleteImageAction->setShortcut(QKeySequence("Ctrl+Delete"));
    m_deleteAnnotationAction = new QAction(this);
    m_deleteAnnotationAction->setObjectName(QStringLiteral("deleteAnnotationAction"));
    m_createModeAction = new QAction(this);
    m_createModeAction->setObjectName(QStringLiteral("createModeAction"));
    m_createModeAction->setShortcuts({QKeySequence("W"), QKeySequence("Ctrl+R")});
    m_createModeAction->setCheckable(true);
    m_createModeAction->setChecked(false);
    m_createPolygonModeAction = new QAction(this);
    m_createPolygonModeAction->setObjectName(QStringLiteral("createPolygonModeAction"));
    m_createPolygonModeAction->setShortcuts({QKeySequence(QStringLiteral("P")),
                                             QKeySequence(QStringLiteral("Ctrl+N"))});
    m_createPolygonModeAction->setCheckable(true);
    m_createPolygonModeAction->setChecked(false);
    m_createPointModeAction = new QAction(this);
    m_createPointModeAction->setObjectName(QStringLiteral("createPointModeAction"));
    m_createPointModeAction->setCheckable(true);
    m_createPointModeAction->setChecked(false);
    m_createPointsModeAction = new QAction(this);
    m_createPointsModeAction->setObjectName(QStringLiteral("createPointsModeAction"));
    m_createPointsModeAction->setCheckable(true);
    m_createPointsModeAction->setChecked(false);
    m_createAiPointsModeAction = new QAction(this);
    m_createAiPointsModeAction->setObjectName(QStringLiteral("createAiPointsModeAction"));
    m_createAiPointsModeAction->setCheckable(true);
    m_createAiPointsModeAction->setChecked(false);
    m_createAiBoxModeAction = new QAction(this);
    m_createAiBoxModeAction->setObjectName(QStringLiteral("createAiBoxModeAction"));
    m_createAiBoxModeAction->setCheckable(true);
    m_createAiBoxModeAction->setChecked(false);
    m_createLineModeAction = new QAction(this);
    m_createLineModeAction->setObjectName(QStringLiteral("createLineModeAction"));
    m_createLineModeAction->setCheckable(true);
    m_createLineModeAction->setChecked(false);
    m_createLinestripModeAction = new QAction(this);
    m_createLinestripModeAction->setObjectName(QStringLiteral("createLinestripModeAction"));
    m_createLinestripModeAction->setCheckable(true);
    m_createLinestripModeAction->setChecked(false);
    m_createCircleModeAction = new QAction(this);
    m_createCircleModeAction->setObjectName(QStringLiteral("createCircleModeAction"));
    m_createCircleModeAction->setCheckable(true);
    m_createCircleModeAction->setChecked(false);
    m_createOrientedRectangleModeAction = new QAction(this);
    m_createOrientedRectangleModeAction->setObjectName(QStringLiteral("createOrientedRectangleModeAction"));
    m_createOrientedRectangleModeAction->setCheckable(true);
    m_createOrientedRectangleModeAction->setChecked(false);
    m_createMaskModeAction = new QAction(this);
    m_createMaskModeAction->setObjectName(QStringLiteral("createMaskModeAction"));
    m_createMaskModeAction->setCheckable(true);
    m_createMaskModeAction->setChecked(false);
    m_maskEditAction = new QAction(this);
    m_maskEditAction->setObjectName(QStringLiteral("maskEditAction"));
    m_maskEditAction->setCheckable(true);
    m_maskEditAction->setChecked(false);
    m_editModeAction = new QAction(this);
    m_editModeAction->setObjectName("editModeAction");
    m_editModeAction->setCheckable(true);
    m_editModeAction->setChecked(true);
    m_viewModeAction = new QAction(this);
    m_viewModeAction->setObjectName("viewModeAction");
    m_viewModeAction->setShortcut(QKeySequence("V"));
    m_viewModeAction->setCheckable(true);
    m_viewModeAction->setChecked(false);
    m_editabilityAction = new QAction(this);
    m_editabilityAction->setObjectName(QStringLiteral("editabilityAction"));
    // Editability is a persisted preference, not a mode shortcut. V is
    // reserved for LabelMe's explicit view mode action.
    m_editabilityAction->setShortcut(QKeySequence());
    m_editabilityAction->setCheckable(true);
    m_editabilityAction->setChecked(m_settings.value("view/editingAllowed", true).toBool());
    m_advancedModeAction = new QAction(this);
    m_advancedModeAction->setObjectName(QStringLiteral("advancedModeAction"));
    m_advancedModeAction->setShortcut(QKeySequence("Ctrl+Alt+A"));
    m_advancedModeAction->setCheckable(true);
    m_advancedModeAction->setChecked(m_settings.value("advanced", false).toBool());
    m_hideAllAction = new QAction(this);
    m_hideAllAction->setShortcut(QKeySequence("Ctrl+H"));
    m_showAllAction = new QAction(this);
    // Ctrl+A belongs to LabelMe's canvas select-all action; show-all remains available from View.
    m_showAllAction->setShortcut(QKeySequence());
    m_toggleAllAction = new QAction(this);
    m_toggleAllAction->setObjectName(QStringLiteral("toggleAllAction"));
    m_toggleAllAction->setShortcut(QKeySequence(QStringLiteral("T")));
    m_zoomInAction = new QAction(this);
    m_zoomInAction->setObjectName(QStringLiteral("zoomInAction"));
    m_zoomInAction->setShortcuts({QKeySequence(QStringLiteral("Ctrl++")),
                                  QKeySequence(QStringLiteral("Ctrl+="))});
    m_zoomOutAction = new QAction(this);
    m_zoomOutAction->setObjectName(QStringLiteral("zoomOutAction"));
    m_zoomOutAction->setShortcut(QKeySequence("Ctrl+-"));
    m_zoomOriginalAction = new QAction(this);
    m_zoomOriginalAction->setObjectName(QStringLiteral("zoomOriginalAction"));
    m_zoomOriginalAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+0")));
    m_fitWindowAction = new QAction(this);
    m_fitWindowAction->setObjectName(QStringLiteral("fitWindowAction"));
    m_fitWindowAction->setShortcut(QKeySequence("Ctrl+F"));
    m_fitWindowAction->setCheckable(true);
    m_fitWindowAction->setChecked(true);
    m_fitWidthAction = new QAction(this);
    m_fitWidthAction->setObjectName(QStringLiteral("fitWidthAction"));
    m_fitWidthAction->setShortcut(QKeySequence("Ctrl+Shift+F"));
    m_fitWidthAction->setCheckable(true);
    m_brightenAction = new QAction(this);
    m_brightenAction->setShortcut(QKeySequence("Ctrl+Shift++"));
    m_darkenAction = new QAction(this);
    m_darkenAction->setShortcut(QKeySequence("Ctrl+Shift+-"));
    m_brightnessOriginalAction = new QAction(this);
    m_brightnessOriginalAction->setShortcut(QKeySequence("Ctrl+Shift+="));
    m_brightnessContrastAction = new QAction(this);
    m_brightnessContrastAction->setObjectName(QStringLiteral("brightnessContrastAction"));
    m_keepPreviousBrightnessContrastAction = new QAction(this);
    m_keepPreviousBrightnessContrastAction->setObjectName(QStringLiteral("keepPreviousBrightnessContrastAction"));
    m_keepPreviousBrightnessContrastAction->setCheckable(true);
    m_keepPreviousBrightnessContrastAction->setChecked(m_settings.value(QStringLiteral("view/keepPreviousBrightnessContrast"), false).toBool());
    m_drawSquareAction = new QAction(this);
    m_drawSquareAction->setCheckable(true);
    m_drawSquareAction->setChecked(m_settings.value("draw/square", false).toBool());
    m_fillDrawingAction = new QAction(this);
    m_fillDrawingAction->setObjectName(QStringLiteral("fillDrawingAction"));
    m_fillDrawingAction->setCheckable(true);
    // LabelMe's default_config.yaml enables filled polygon previews.
    m_fillDrawingAction->setChecked(m_settings.value("view/fillDrawing", true).toBool());
    m_boxLineColorAction = new QAction(this);
    m_boxLineColorAction->setShortcut(QKeySequence("Ctrl+L"));
    m_shapeLineColorAction = new QAction(this);
    m_shapeLineColorAction->setObjectName(QStringLiteral("shapeLineColorAction"));
    m_shapeFillColorAction = new QAction(this);
    m_shapeFillColorAction->setObjectName(QStringLiteral("shapeFillColorAction"));
    m_infoAction = new QAction(this);
    m_shortcutsAction = new QAction(this);
    m_tutorialAction = new QAction(this);
    m_tutorialAction->setObjectName(QStringLiteral("tutorialAction"));
    m_settingsAction = new QAction(this);
    m_settingsAction->setObjectName(QStringLiteral("settingsAction"));
    m_settingsAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+,")));
    m_settingsAction->setMenuRole(QAction::PreferencesRole);
    m_autoSaveAction = new QAction(this);
    m_autoSaveAction->setObjectName(QStringLiteral("autoSaveAction"));
    m_autoSaveAction->setCheckable(true);
    // LabelMe enables auto-save by default; an explicit user setting still wins.
    m_autoSaveAction->setChecked(m_settings.value("autosave", true).toBool());
    m_keepPreviousAction = new QAction(this);
    m_keepPreviousAction->setObjectName(QStringLiteral("keepPreviousAction"));
    m_keepPreviousAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+P")));
    m_keepPreviousAction->setCheckable(true);
    m_keepPreviousAction->setChecked(m_settings.value("keepPrevious", false).toBool());
    m_keepPreviousZoomAction = new QAction(this);
    m_keepPreviousZoomAction->setObjectName(QStringLiteral("keepPreviousZoomAction"));
    m_keepPreviousZoomAction->setCheckable(true);
    m_keepPreviousZoomAction->setChecked(m_settings.value("view/keepPreviousZoom", false).toBool());
    m_singleClassAction = new QAction(this);
    m_singleClassAction->setCheckable(true);
    m_singleClassAction->setChecked(m_settings.value("singleclass", false).toBool());
    m_displayLabelsAction = new QAction(this);
    m_displayLabelsAction->setCheckable(true);
    m_displayLabelsAction->setChecked(m_settings.value("paintlabel", false).toBool());
    m_embedImageDataAction = new QAction(this);
    m_embedImageDataAction->setObjectName(QStringLiteral("embedImageDataAction"));
    m_embedImageDataAction->setCheckable(true);
    m_embedImageDataAction->setChecked(m_settings.value("labelme/embedImageData", false).toBool());
    m_editLabelFlagsAction = new QAction(this);
    m_editLabelFlagsAction->setObjectName(QStringLiteral("editLabelFlagsAction"));
    m_miniMapAction = new QAction(this);
    m_miniMapAction->setObjectName("miniMapAction");
    m_miniMapAction->setCheckable(true);
    m_miniMapAction->setChecked(m_settings.value("view/miniMapEnabled", true).toBool());
    m_showPerformanceAction = new QAction(this);
    m_showPerformanceAction->setObjectName("showPerformanceAction");
    m_showPerformanceAction->setCheckable(true);
    m_showPerformanceAction->setChecked(m_settings.value("view/showPerformance", true).toBool());
    m_samplingModeAction = new QAction(this);
    m_samplingModeAction->setObjectName("samplingModeAction");
    m_samplingModeAction->setCheckable(true);
    m_samplingModeAction->setChecked(m_settings.value("view/samplingMode").toString() == "Smooth");
    m_thumbnailModeAction = new QAction(this);
    m_thumbnailModeAction->setObjectName("thumbnailModeAction");
    m_thumbnailModeAction->setCheckable(true);
    m_thumbnailModeAction->setChecked(m_settings.value("view/fileThumbnails", false).toBool());
    m_lineColor = m_settings.value("line/color", m_lineColor).value<QColor>();
    m_fillColor = m_settings.value("fill/color", m_fillColor).value<QColor>();
    m_canvas->setLineColor(m_lineColor);
    m_canvas->setFillColor(m_fillColor);
    m_canvas->setDrawSquare(m_drawSquareAction->isChecked());
    m_canvas->setFillDrawing(m_fillDrawingAction->isChecked());
    m_canvas->setSamplingMode(m_samplingModeAction->isChecked() ? Canvas::SamplingMode::Smooth : Canvas::SamplingMode::FastNearest);
    assignActionIcons();
}

void MainWindow::createMenusAndToolbars() {
    m_fileMenu = menuBar()->addMenu(QString());
    m_fileMenu->setObjectName(QStringLiteral("fileMenu"));
    m_fileMenu->addActions({m_openAction, m_openDirAction, m_changeSaveDirAction, m_openAnnotationAction});
    m_recentFilesMenu = m_fileMenu->addMenu(QString());
    m_recentFilesMenu->setObjectName(QStringLiteral("recentFilesMenu"));
    m_recentDirsMenu = m_fileMenu->addMenu(QString());
    m_recentDirsMenu->setObjectName(QStringLiteral("recentDirsMenu"));
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_copyPreviousAction);
    m_fileMenu->addActions({m_saveAction, m_formatAction, m_saveAsAction, m_autoSaveAction, m_embedImageDataAction,
                            m_closeAction, m_settingsAction, m_resetAllAction});
    m_fileMenu->addAction(m_verifyAction);
    m_fileMenu->addActions({m_deleteImageAction, m_deleteAnnotationAction});
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_prevAction);
    m_fileMenu->addAction(m_nextAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_quitAction);

    m_viewMenu = menuBar()->addMenu(QString());
    m_viewMenu->setObjectName(QStringLiteral("viewMenu"));
    m_viewMenu->addActions({m_undoAction, m_undoLastPointAction, m_redoAction, m_copyShapesAction,
                            m_pasteShapesAction, m_copyAction, m_deleteAction, m_deleteAllShapesAction});
    m_viewMenu->addActions({m_prevShapeAction, m_nextShapeAction});
    m_viewMenu->addAction(m_addPointToEdgeAction);
    m_viewMenu->addAction(m_removeSelectedPointAction);
    m_viewMenu->addSeparator();
    m_viewMenu->addActions({m_showFlagDockAction, m_showLabelDockAction,
                            m_showShapeDockAction, m_showFileDockAction});
    m_viewMenu->addAction(m_resetLayoutAction);
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(m_advancedModeAction);
    m_viewMenu->addAction(m_createModeAction);
    m_viewMenu->addAction(m_createPolygonModeAction);
    m_viewMenu->addAction(m_createPointModeAction);
    m_viewMenu->addAction(m_createPointsModeAction);
    m_viewMenu->addAction(m_createAiPointsModeAction);
    m_viewMenu->addAction(m_createAiBoxModeAction);
    m_viewMenu->addAction(m_createLineModeAction);
    m_viewMenu->addAction(m_createLinestripModeAction);
    m_viewMenu->addAction(m_createCircleModeAction);
    m_viewMenu->addAction(m_createOrientedRectangleModeAction);
    m_viewMenu->addAction(m_createMaskModeAction);
    m_viewMenu->addAction(m_maskEditAction);
    m_viewMenu->addAction(m_editModeAction);
    m_viewMenu->addAction(m_viewModeAction);
    m_viewMenu->addAction(m_editabilityAction);
    m_viewMenu->addAction(m_autoSaveAction);
    m_viewMenu->addAction(m_keepPreviousAction);
    m_viewMenu->addAction(m_keepPreviousZoomAction);
    m_viewMenu->addAction(m_singleClassAction);
    m_viewMenu->addAction(m_displayLabelsAction);
    m_viewMenu->addAction(m_editLabelFlagsAction);
    m_viewMenu->addAction(m_drawSquareAction);
    m_viewMenu->addAction(m_fillDrawingAction);
    m_viewMenu->addAction(m_boxLineColorAction);
    m_viewMenu->addSeparator();
    m_viewMenu->addActions({m_hideAllAction, m_showAllAction, m_toggleAllAction});
    m_viewMenu->addSeparator();
    m_viewMenu->addActions({m_zoomInAction, m_zoomOutAction, m_zoomOriginalAction});
    m_viewMenu->addActions({m_fitWindowAction, m_fitWidthAction});
    m_viewMenu->addActions({m_miniMapAction, m_showPerformanceAction, m_samplingModeAction, m_thumbnailModeAction});
    m_viewMenu->addSeparator();
    m_viewMenu->addActions({m_darkenAction, m_brightenAction, m_brightnessOriginalAction});
    m_viewMenu->addAction(m_brightnessContrastAction);
    m_viewMenu->addAction(m_keepPreviousBrightnessContrastAction);

    m_languageMenu = m_viewMenu->addMenu(QString());
    QHash<QString, QString> languageNames{{"en", "English"}, {"zh-CN", QString::fromUtf8("简体中文")},
                                          {"zh-TW", QString::fromUtf8("繁體中文")}, {"ja-JP", QString::fromUtf8("日本語")}};
    for (const QString &language : StringBundle::supportedLanguages()) {
        QAction *action = m_languageMenu->addAction(languageNames.value(language, language));
        action->setCheckable(true);
        action->setData(language);
        action->setChecked(language == m_strings.language());
        connect(action, &QAction::triggered, this, [this, language](bool checked) {
            if (checked) changeLanguage(language);
        });
    }

    m_helpMenu = menuBar()->addMenu(QString());
    m_helpMenu->setObjectName(QStringLiteral("helpMenu"));
    m_helpMenu->addActions({m_tutorialAction, m_infoAction, m_shortcutsAction});
    m_toolBar = new QToolBar(QStringLiteral("Tools"), this);
    m_toolBar->setObjectName(QStringLiteral("mainToolBar"));
    m_toolBar->setMovable(false);
    m_toolBar->setFloatable(false);
    m_toolBar->setIconSize(QSize(22, 22));
    m_toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_toolBar->setStyleSheet(QStringLiteral(
        "QToolBar { background: transparent; border: 0; spacing: 1px; }"
        "QToolButton { min-width: 34px; max-width: 34px; min-height: 32px; padding: 0; border: 1px solid transparent; border-radius: 5px; background: transparent; }"
        "QToolButton:hover { background: #edf7f1; border-color: #cfe7d8; }"
        "QToolButton:pressed { background: #dff1e6; border-color: #a8d4b9; }"
        "QToolButton:checked { background: #d7eadf; border-color: #7ec79d; }"));
    m_zoomWidget = new QSpinBox(this);
    m_zoomWidget->setObjectName(QStringLiteral("zoomWidget"));
    m_zoomWidget->setRange(1, 1600);
    m_zoomWidget->setValue(100);
    m_zoomWidget->setSuffix(QStringLiteral(" %"));
    m_zoomWidget->setAlignment(Qt::AlignCenter);
    m_zoomWidget->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_zoomWidget->setMinimumWidth(fontMetrics().horizontalAdvance(QStringLiteral("1600 %")) + 10);
    m_zoomWidget->setToolTip(m_strings.get(QStringLiteral("zoomLevel")));
    connect(m_zoomWidget, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int percent) {
        if (!m_canvas || !m_scrollArea) {
            return;
        }
        const double oldScale = m_canvas->scale();
        m_fitMode = FitMode::Manual;
        {
            QSignalBlocker fitWindowBlocker(m_fitWindowAction);
            QSignalBlocker fitWidthBlocker(m_fitWidthAction);
            m_fitWindowAction->setChecked(false);
            m_fitWidthAction->setChecked(false);
        }
        const QPoint anchor = zoomAnchorPosition();
        m_canvas->setScale(percent / 100.0);
        if (!qFuzzyCompare(oldScale, m_canvas->scale())) {
            onCanvasScaleChanged(oldScale, m_canvas->scale(), anchor);
        }
    });
    m_mainModeMenu = new QMenu(this);
    m_mainModeMenu->addActions({m_viewModeAction, m_editModeAction, m_createModeAction, m_createPolygonModeAction,
                                m_createPointModeAction, m_createPointsModeAction, m_createAiPointsModeAction,
                                m_createAiBoxModeAction, m_createLineModeAction, m_createLinestripModeAction, m_createCircleModeAction,
                                m_createOrientedRectangleModeAction, m_createMaskModeAction});
    m_mainModeButton = new QToolButton(this);
    m_mainModeButton->setObjectName(QStringLiteral("mainModeButton"));
    m_mainModeButton->setMenu(m_mainModeMenu);
    m_mainModeButton->setPopupMode(QToolButton::InstantPopup);
    m_mainModeButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_mainModeButton->setAutoRaise(true);
    m_mainModeButton->setFocusPolicy(Qt::NoFocus);
    syncMainModeButton();

    m_openWithMenu = new QMenu(this);
    m_openWithMenu->addAction(m_openWithImageViewerAction);
    m_openWithMenu->addAction(m_openFileLocationAction);
    m_openWithButton = new QToolButton(this);
    m_openWithButton->setObjectName(QStringLiteral("openWithButton"));
    m_openWithButton->setMenu(m_openWithMenu);
    m_openWithButton->setDefaultAction(m_openWithImageViewerAction);
    m_openWithButton->setPopupMode(QToolButton::MenuButtonPopup);
    m_openWithButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_openWithButton->setAutoRaise(true);
    m_openWithButton->setFocusPolicy(Qt::NoFocus);

    m_fileListContextMenu = new QMenu(this);
    m_fileListContextMenu->setObjectName(QStringLiteral("fileListContextMenu"));
    m_fileListContextMenu->addAction(m_fileContextOpenAction);
    m_fileListContextMenu->addAction(m_fileContextRevealAction);
    m_fileListContextMenu->addAction(m_fileContextCopyPathAction);
    m_fileListContextMenu->addSeparator();
    m_fileListContextMenu->addAction(m_fileContextMarkAction);
    m_fileListContextMenu->addSeparator();
    m_fileListContextMenu->addAction(m_fileContextDeleteAction);

    populateToolbarForMode();
    createFooterControls();
    rebuildRecentFilesMenu();
    rebuildRecentDirsMenu();
}

void MainWindow::createFooterControls() {
    if (m_footerModeCombo) {
        return;
    }

    m_footerModeCombo = new QComboBox(this);
    m_footerModeCombo->setObjectName(QStringLiteral("footerModeCombo"));
    m_footerModeCombo->addItem(m_strings.get(QStringLiteral("footerViewMode")), QStringLiteral("view"));
    m_footerModeCombo->addItem(m_strings.get(QStringLiteral("footerEditMode")), QStringLiteral("edit"));
    m_footerModeCombo->addItem(m_strings.get(QStringLiteral("footerCreateMode")), QStringLiteral("create"));
    m_footerModeCombo->setFixedWidth(132);
    connect(m_footerModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        const QString mode = m_footerModeCombo->itemData(index).toString();
        if (mode == QStringLiteral("view")) setViewMode();
        else if (mode == QStringLiteral("edit")) setEditMode();
        else if (mode == QStringLiteral("create")) setCreateMode();
    });

    auto makeShortcutButton = [this](const QString &objectName, const QString &text, const QString &tooltip) {
        auto *button = new QToolButton(this);
        button->setObjectName(objectName);
        button->setText(text);
        button->setToolTip(tooltip);
        button->setAutoRaise(true);
        button->setFocusPolicy(Qt::NoFocus);
        button->setToolButtonStyle(Qt::ToolButtonTextOnly);
        return button;
    };
    m_footerViewShortcut = makeShortcutButton(QStringLiteral("footerViewShortcut"), QStringLiteral("V"),
                                              m_strings.get(QStringLiteral("footerViewTooltip")));
    m_footerEditShortcut = makeShortcutButton(QStringLiteral("footerEditShortcut"),
                                              m_strings.get(QStringLiteral("footerEditMode")),
                                              m_strings.get(QStringLiteral("footerEditTooltip")));
    m_footerCreateShortcut = makeShortcutButton(QStringLiteral("footerCreateShortcut"), QStringLiteral("W"),
                                                m_strings.get(QStringLiteral("footerCreateTooltip")));
    connect(m_footerViewShortcut, &QToolButton::clicked, this, &MainWindow::setViewMode);
    connect(m_footerEditShortcut, &QToolButton::clicked, this, &MainWindow::setEditMode);
    connect(m_footerCreateShortcut, &QToolButton::clicked, this, &MainWindow::setCreateMode);

    auto *modePanel = new QWidget(this);
    modePanel->setObjectName(QStringLiteral("footerModePanel"));
    auto *modeLayout = new QHBoxLayout(modePanel);
    modeLayout->setContentsMargins(0, 0, 0, 0);
    modeLayout->setSpacing(3);
    modeLayout->addWidget(m_footerModeCombo);
    modeLayout->addWidget(m_footerViewShortcut);
    modeLayout->addWidget(m_footerEditShortcut);
    modeLayout->addWidget(m_footerCreateShortcut);

    m_footerFormatCombo = new QComboBox(this);
    m_footerFormatCombo->setObjectName(QStringLiteral("footerFormatCombo"));
    m_footerFormatCombo->addItems({QStringLiteral("PascalVOC"), QStringLiteral("YOLO"), QStringLiteral("CreateML"), QStringLiteral("LabelMe")});
    m_footerFormatCombo->setFixedWidth(108);
    connect(m_footerFormatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        setFormat(static_cast<SaveFormat>(index));
        setDirty(true);
    });

    m_footerMiniMapButton = new QToolButton(this);
    m_footerMiniMapButton->setObjectName(QStringLiteral("footerMiniMapButton"));
    m_footerMiniMapButton->setDefaultAction(m_miniMapAction);
    m_footerMiniMapButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_footerMiniMapButton->setAutoRaise(true);
    m_footerMiniMapButton->setFocusPolicy(Qt::NoFocus);

    statusBar()->removeWidget(m_coordinates);
    statusBar()->addPermanentWidget(modePanel, 0);
    statusBar()->addPermanentWidget(m_footerFormatCombo, 0);
    statusBar()->addPermanentWidget(m_footerMiniMapButton, 0);
    statusBar()->addPermanentWidget(m_coordinates, 0);

    syncModeFooterControls();
    syncFormatFooterControls();
}

void MainWindow::installFramelessChrome() {
    if (m_topChrome) {
        return;
    }

    QMenuBar *existingMenuBar = menuBar();
    existingMenuBar->setNativeMenuBar(false);
    existingMenuBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    existingMenuBar->setStyleSheet(QStringLiteral(
        "QMenuBar { background: transparent; border: 0; padding: 0 2px; }"
        "QMenuBar::item { padding: 6px 8px; border-radius: 4px; }"
        "QMenuBar::item:selected { background: #edf7f1; }"
        "QMenuBar::item:pressed { background: #dff1e6; }"));

    m_topChrome = new QWidget(this);
    m_topChrome->setObjectName(QStringLiteral("framelessTopChrome"));
    auto *layout = new QVBoxLayout(m_topChrome);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_titleBar = new FramelessTitleBar(m_topChrome);
    m_titleToolContainer = new QWidget(m_titleBar);
    m_titleToolContainer->setObjectName(QStringLiteral("titleToolContainer"));
    auto *toolLayout = new QHBoxLayout(m_titleToolContainer);
    toolLayout->setContentsMargins(0, 0, 0, 0);
    toolLayout->setSpacing(6);
    toolLayout->addWidget(existingMenuBar, 0);
    toolLayout->addWidget(m_toolBar, 1);
    m_titleBar->setToolWidget(m_titleToolContainer);
    layout->addWidget(m_titleBar);
    setMenuWidget(m_topChrome);

    connect(m_titleBar, &FramelessTitleBar::dragRequested, this, &MainWindow::startSystemMove);
    connect(m_titleBar, &FramelessTitleBar::minimizeRequested, this, &QWidget::showMinimized);
    connect(m_titleBar, &FramelessTitleBar::maximizeRestoreRequested, this, &MainWindow::toggleMaximizeRestore);
    connect(m_titleBar, &FramelessTitleBar::closeRequested, this, &QWidget::close);

    updateFramelessChrome();
}

void MainWindow::applyNativeWindowChrome() {
#ifdef Q_OS_WIN
    const HWND hwnd = reinterpret_cast<HWND>(winId());
    if (!hwnd) {
        return;
    }

    const LONG_PTR currentStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
    const LONG_PTR nextStyle = static_cast<LONG_PTR>(
        withWindowsWindowChromeStyle(static_cast<quintptr>(currentStyle)));
    if (nextStyle == currentStyle) {
        return;
    }

    SetWindowLongPtrW(hwnd, GWL_STYLE, nextStyle);
    SetWindowPos(hwnd,
                 nullptr,
                 0,
                 0,
                 0,
                 0,
                 SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
#endif
}

void MainWindow::connectSignals() {
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openFile);
    connect(m_openDirAction, &QAction::triggered, this, &MainWindow::openDir);
    connect(m_openAnnotationAction, &QAction::triggered, this, &MainWindow::openAnnotationDialog);
    connect(m_openWithImageViewerAction, &QAction::triggered, this, &MainWindow::openCurrentImageWithViewer);
    connect(m_openFileLocationAction, &QAction::triggered, this, &MainWindow::revealCurrentImageInFolder);
    connect(m_fileContextOpenAction, &QAction::triggered, this, &MainWindow::openContextFile);
    connect(m_fileContextRevealAction, &QAction::triggered, this, &MainWindow::revealContextFile);
    connect(m_fileContextCopyPathAction, &QAction::triggered, this, &MainWindow::copyContextFilePath);
    connect(m_fileContextMarkAction, &QAction::triggered, this, &MainWindow::toggleContextFileMark);
    connect(m_fileContextDeleteAction, &QAction::triggered, this, &MainWindow::deleteContextFile);
    connect(m_closeAction, &QAction::triggered, this, &MainWindow::closeFile);
    connect(m_quitAction, &QAction::triggered, this, &QWidget::close);
    connect(m_resetAllAction, &QAction::triggered, this, &MainWindow::resetAllSettings);
    connect(m_resetLayoutAction, &QAction::triggered, this, &MainWindow::resetLayout);
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    connect(m_saveAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(m_changeSaveDirAction, &QAction::triggered, this, &MainWindow::changeSaveDir);
    connect(m_formatAction, &QAction::triggered, this, &MainWindow::changeFormat);
    connect(m_nextAction, &QAction::triggered, this, &MainWindow::openNextImage);
    connect(m_prevAction, &QAction::triggered, this, &MainWindow::openPrevImage);
    connect(m_nextCopyAction, &QAction::triggered, this, [this]() {
        m_copyPreviousNavigation = true;
        openNextImage();
        m_copyPreviousNavigation = false;
    });
    connect(m_prevCopyAction, &QAction::triggered, this, [this]() {
        m_copyPreviousNavigation = true;
        openPrevImage();
        m_copyPreviousNavigation = false;
    });
    connect(m_verifyAction, &QAction::triggered, this, &MainWindow::verifyImage);
    connect(m_editLabelAction, &QAction::triggered, this, [this]() { editCurrentLabel(); });
    connect(m_undoAction, &QAction::triggered, this, &MainWindow::undoShapeOperation);
    connect(m_undoLastPointAction, &QAction::triggered, this, [this]() {
        if (m_canvas->undoLastDrawingPoint()) {
            refreshActions();
        }
    });
    connect(m_redoAction, &QAction::triggered, this, &MainWindow::redoShapeOperation);
    connect(m_prevShapeAction, &QAction::triggered, this, [this]() { selectAdjacentShape(-1); });
    connect(m_nextShapeAction, &QAction::triggered, this, [this]() { selectAdjacentShape(1); });
    connect(m_deleteAction, &QAction::triggered, this, &MainWindow::deleteCurrentShape);
    connect(m_deleteAllShapesAction, &QAction::triggered, this, &MainWindow::deleteAllShapes);
    connect(m_copyAction, &QAction::triggered, this, &MainWindow::copyCurrentShape);
    connect(m_copyShapesAction, &QAction::triggered, this, &MainWindow::copySelectedShapesToClipboard);
    connect(m_pasteShapesAction, &QAction::triggered, this, &MainWindow::pasteShapesFromClipboard);
    connect(m_copyHereAction, &QAction::triggered, this, &MainWindow::copyShapeHere);
    connect(m_moveHereAction, &QAction::triggered, this, &MainWindow::moveShapeHere);
    connect(m_insertPolygonPointAction, &QAction::triggered, this, &MainWindow::insertPolygonPointHere);
    connect(m_addPointToEdgeAction, &QAction::triggered, this, [this]() {
        if (m_canvas->addPointToEdge()) {
            refreshLabels();
            recordShapeHistory();
            setDirty(true);
        }
    });
    connect(m_removePolygonPointAction, &QAction::triggered, this, &MainWindow::removePolygonPointHere);
    connect(m_removeSelectedPointAction, &QAction::triggered, this, &MainWindow::removeSelectedPoint);
    connect(m_copyPreviousAction, &QAction::triggered, this, &MainWindow::copyPreviousBoundingBoxes);
    connect(m_deleteImageAction, &QAction::triggered, this, &MainWindow::deleteCurrentImage);
    connect(m_deleteAnnotationAction, &QAction::triggered, this, &MainWindow::deleteCurrentAnnotationFile);
    connect(m_createModeAction, &QAction::triggered, this, &MainWindow::setCreateMode);
    connect(m_createPolygonModeAction, &QAction::triggered, this, &MainWindow::setPolygonCreateMode);
    connect(m_createPointModeAction, &QAction::triggered, this, &MainWindow::setPointCreateMode);
    connect(m_createPointsModeAction, &QAction::triggered, this, &MainWindow::setPointsCreateMode);
    connect(m_createAiPointsModeAction, &QAction::triggered, this, &MainWindow::setAiPointsCreateMode);
    connect(m_createAiBoxModeAction, &QAction::triggered, this, &MainWindow::setAiBoxCreateMode);
    connect(m_createLineModeAction, &QAction::triggered, this, &MainWindow::setLineCreateMode);
    connect(m_createLinestripModeAction, &QAction::triggered, this, &MainWindow::setLinestripCreateMode);
    connect(m_createCircleModeAction, &QAction::triggered, this, &MainWindow::setCircleCreateMode);
    connect(m_createOrientedRectangleModeAction, &QAction::triggered, this, &MainWindow::setOrientedRectangleCreateMode);
    connect(m_createMaskModeAction, &QAction::triggered, this, &MainWindow::setMaskCreateMode);
    connect(m_maskEditAction, &QAction::toggled, this, [this](bool checked) {
        m_canvas->setMaskEditing(checked);
    });
    connect(m_editModeAction, &QAction::triggered, this, &MainWindow::setEditMode);
    connect(m_viewModeAction, &QAction::triggered, this, &MainWindow::setViewMode);
    connect(m_editabilityAction, &QAction::toggled, this, &MainWindow::setEditabilityAllowed);
    connect(m_advancedModeAction, &QAction::toggled, this, &MainWindow::setAdvancedMode);
    connect(m_autoSaveAction, &QAction::toggled, this, [this](bool checked) {
        m_settings.setValue(QStringLiteral("autosave"), checked);
        if (checked && m_dirty) {
            setDirty(true);
        }
    });
    connect(m_keepPreviousAction, &QAction::toggled, this, [this](bool checked) {
        m_settings.setValue("keepPrevious", checked);
    });
    connect(m_keepPreviousZoomAction, &QAction::toggled, this, [this](bool checked) {
        m_settings.setValue("view/keepPreviousZoom", checked);
    });
    connect(m_hideAllAction, &QAction::triggered, this, [this]() { toggleAllShapesVisible(false); });
    connect(m_showAllAction, &QAction::triggered, this, [this]() { toggleAllShapesVisible(true); });
    connect(m_toggleAllAction, &QAction::triggered, this, &MainWindow::toggleAllShapes);
    connect(m_zoomInAction, &QAction::triggered, this, &MainWindow::zoomIn);
    connect(m_zoomOutAction, &QAction::triggered, this, &MainWindow::zoomOut);
    connect(m_zoomOriginalAction, &QAction::triggered, this, &MainWindow::resetZoom);
    connect(m_fitWindowAction, &QAction::triggered, this, &MainWindow::fitWindow);
    connect(m_fitWidthAction, &QAction::triggered, this, &MainWindow::fitWidth);
    connect(m_brightenAction, &QAction::triggered, this, &MainWindow::brighten);
    connect(m_darkenAction, &QAction::triggered, this, &MainWindow::darken);
    connect(m_brightnessOriginalAction, &QAction::triggered, this, &MainWindow::resetBrightness);
    connect(m_brightnessContrastAction, &QAction::triggered, this, &MainWindow::openBrightnessContrastDialog);
    connect(m_keepPreviousBrightnessContrastAction, &QAction::toggled, this, [this](bool checked) {
        m_settings.setValue(QStringLiteral("view/keepPreviousBrightnessContrast"), checked);
    });
    connect(m_boxLineColorAction, &QAction::triggered, this, &MainWindow::chooseBoxLineColor);
    connect(m_shapeLineColorAction, &QAction::triggered, this, &MainWindow::chooseShapeLineColor);
    connect(m_shapeFillColorAction, &QAction::triggered, this, &MainWindow::chooseShapeFillColor);
    connect(m_infoAction, &QAction::triggered, this, &MainWindow::showInfoDialog);
    connect(m_shortcutsAction, &QAction::triggered, this, &MainWindow::showShortcutsDialog);
    connect(m_tutorialAction, &QAction::triggered, this, &MainWindow::openTutorial);
    connect(m_settingsAction, &QAction::triggered, this, &MainWindow::showSettingsDialog);
    connect(m_drawSquareAction, &QAction::toggled, m_canvas, &Canvas::setDrawSquare);
    connect(m_fillDrawingAction, &QAction::toggled, this, [this](bool checked) {
        m_canvas->setFillDrawing(checked);
        m_settings.setValue("view/fillDrawing", checked);
    });
    connect(m_displayLabelsAction, &QAction::toggled, m_canvas, &Canvas::setPaintLabels);
    connect(m_embedImageDataAction, &QAction::toggled, this, [this](bool checked) {
        m_settings.setValue("labelme/embedImageData", checked);
    });
    connect(m_editLabelFlagsAction, &QAction::triggered, this, &MainWindow::editLabelFlagPresets);
    connect(m_miniMapAction, &QAction::toggled, this, [this](bool checked) {
        m_miniMapOverlay->setMiniMapEnabled(checked);
        m_settings.setValue("view/miniMapEnabled", checked);
    });
    connect(m_showPerformanceAction, &QAction::toggled, this, [this](bool checked) {
        m_performanceLabel->setVisible(checked);
        m_settings.setValue("view/showPerformance", checked);
    });
    connect(m_samplingModeAction, &QAction::toggled, this, [this](bool checked) {
        m_canvas->setSamplingMode(checked ? Canvas::SamplingMode::Smooth : Canvas::SamplingMode::FastNearest);
        m_settings.setValue("view/samplingMode", checked ? QStringLiteral("Smooth") : QStringLiteral("FastNearest"));
        updatePerformanceLabel();
    });
    connect(m_thumbnailModeAction, &QAction::toggled, this, &MainWindow::setFileThumbnailMode);
    connect(m_useDefaultLabel, &QCheckBox::toggled, this, [this](bool checked) {
        m_settings.setValue("useDefaultLabel", checked);
    });
    connect(m_defaultLabelCombo, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        m_settings.setValue("defaultLabel", text);
        if (m_useDefaultLabel && m_useDefaultLabel->isChecked()) {
            rememberLastUsedLabel(text);
        }
    });
    connect(m_difficult, &QCheckBox::toggled, this, [this](bool checked) {
        const int row = m_canvas ? m_canvas->currentIndex() : -1;
        QVector<Shape> &shapes = m_canvas->shapesRef();
        if (row < 0 || row >= shapes.size()) return;
        shapes[row].difficult = checked;
        shapes[row].flags.insert(QStringLiteral("difficult"), checked);
        recordShapeHistory();
        setDirty(true);
    });
    connect(m_topLevelFlagsEdit, &QPlainTextEdit::textChanged, this, [this]() {
        m_labelMeTopLevelFlags = flagsFromEditorText(m_topLevelFlagsEdit->toPlainText());
        m_labelMeTopLevelFlags.remove(QStringLiteral("verified"));
        refreshTopLevelFlagsList();
        if (m_format == SaveFormat::LabelMe) {
            setDirty(true);
        }
    });
    connect(m_flagList, &QListWidget::itemChanged, this, [this](QListWidgetItem *item) {
        if (!item) {
            return;
        }
        const QString key = item->text().trimmed();
        if (key.isEmpty() || key == QStringLiteral("verified")) {
            return;
        }
        m_labelMeTopLevelFlags.insert(key, item->checkState() == Qt::Checked);
        QSignalBlocker blocker(m_topLevelFlagsEdit);
        m_topLevelFlagsEdit->setPlainText(flagsToEditorText(m_labelMeTopLevelFlags));
        if (m_format == SaveFormat::LabelMe) {
            setDirty(true);
        }
    });
    connect(m_canvas, &Canvas::selectionChanged, this, &MainWindow::onCanvasSelectionChanged);
    connect(m_canvas, &Canvas::shapeEditStarted, this, &MainWindow::onCanvasShapeEditStarted);
    connect(m_canvas, &Canvas::shapeEditFinished, this, &MainWindow::onCanvasShapeEditFinished);
    connect(m_canvas, &Canvas::selectionSetChanged, this, [this](const QVector<int> &indices) {
        QSignalBlocker blocker(m_labelList);
        m_labelList->clearSelection();
        for (int index : indices) {
            if (index >= 0 && index < m_labelList->count()) {
                m_labelList->item(index)->setSelected(true);
            }
        }
        if (m_canvas->currentIndex() >= 0 && m_canvas->currentIndex() < m_labelList->count()) {
            m_labelList->setCurrentRow(m_canvas->currentIndex(), QItemSelectionModel::NoUpdate);
        }
        scrollLabelListToCurrentShape();
        refreshActions();
    });
    connect(m_canvas, &Canvas::shapesChanged, this, &MainWindow::onCanvasShapesChanged);
    connect(m_canvas, &Canvas::contextMenuRequested, this, &MainWindow::showCanvasContextMenu);
    connect(m_canvas, &Canvas::edgeSelectionChanged, this, [this](bool) { refreshActions(); });
    connect(m_canvas, &Canvas::vertexSelectionChanged, this, [this](bool) { refreshActions(); });
    connect(m_canvas, &Canvas::scaleChanged, this, &MainWindow::onCanvasScaleChanged);
    connect(m_canvas, &Canvas::scaleValueChanged, this, [this](double) { syncZoomWidget(); });
    connect(m_canvas, &Canvas::frameRendered, this, &MainWindow::onCanvasFrameRendered);
    connect(m_canvas, &Canvas::editModeRequested, this, &MainWindow::setEditMode);
    connect(m_canvas, &Canvas::shapeCreated, this, &MainWindow::onCanvasShapeCreated);
    connect(m_canvas, &Canvas::previousShapeRequested, this, [this]() { selectAdjacentShape(-1); });
    connect(m_canvas, &Canvas::nextShapeRequested, this, [this]() { selectAdjacentShape(1); });
    connect(m_canvas, &Canvas::deleteShapeRequested, this, [this]() {
        if (m_deleteAction->isEnabled()) {
            deleteCurrentShape();
        }
    });
    connect(m_canvas, &Canvas::removeSelectedPointRequested, this, [this]() {
        if (m_removeSelectedPointAction->isEnabled()) {
            removeSelectedPoint();
        }
    });
    connect(m_canvas, &Canvas::scrollRequested, this, [this](int dx, int dy) {
        m_scrollArea->horizontalScrollBar()->setValue(m_scrollArea->horizontalScrollBar()->value() - dx);
        m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->value() - dy);
    });
    connect(m_canvas, &Canvas::wheelScrollRequested, this, [this](int dx, int dy) {
        auto applyNaturalScroll = [](QScrollBar *bar, int delta) {
            if (delta == 0) {
                return;
            }
            const int step = qMax(1, bar->singleStep());
            const int units = qRound(-static_cast<double>(delta) * 0.1 * step);
            bar->setValue(bar->value() + units);
        };
        applyNaturalScroll(m_scrollArea->horizontalScrollBar(), dx);
        applyNaturalScroll(m_scrollArea->verticalScrollBar(), dy);
    });
    connect(m_scrollArea->horizontalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
        if (!m_filePath.isEmpty()) {
            m_horizontalScrollByFile.insert(m_filePath, value);
        }
    });
    connect(m_scrollArea->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
        if (!m_filePath.isEmpty()) {
            m_verticalScrollByFile.insert(m_filePath, value);
        }
    });
    m_canvas->installEventFilter(this);
    m_labelList->installEventFilter(this);
    m_uniqueLabelList->installEventFilter(this);
    connect(m_performanceMonitor, &PerformanceMonitor::performanceUpdated, this, [this](const QString &text) {
        m_resourcePerformanceText = text;
        updatePerformanceLabel();
    });
    m_miniMapOverlay->setMiniMapEnabled(m_miniMapAction->isChecked());
    m_performanceLabel->setVisible(m_showPerformanceAction->isChecked());
    m_performanceMonitor->start(1000);
    connect(m_canvas, &Canvas::statusTextChanged, m_coordinates, &QLabel::setText);
    connect(m_canvas, &Canvas::drawingStateChanged, this, [this](bool drawing) {
        // Multi-point, oriented-rectangle, and mask creation begin before a
        // shapeEditStarted signal is needed by the basic-shape transaction.
        // Capture the document state at the common drawing boundary so a
        // cancelled label popup can restore the exact pre-creation state.
        if (drawing) {
            m_shapeEditDirtyBefore = m_dirty;
        }
        refreshActions();
    });
    connect(m_labelList, &QListWidget::itemSelectionChanged, this, &MainWindow::onLabelSelectionChanged);
    connect(m_labelList->model(), &QAbstractItemModel::rowsMoved, this,
            [this](const QModelIndex &, int, int, const QModelIndex &, int) {
                syncShapeOrderFromLabelList();
            });
    connect(m_labelList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *) { editCurrentLabel(); });
    connect(m_labelList, &QListWidget::itemChanged, this, &MainWindow::onLabelItemChanged);
    m_labelList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_labelList, &QListWidget::customContextMenuRequested, this, &MainWindow::showLabelListContextMenu);
    m_fileList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_fileList, &QListWidget::customContextMenuRequested, this, &MainWindow::showFileListContextMenu);
    connect(m_fileList, &QListWidget::itemDoubleClicked, this, &MainWindow::onFileDoubleClicked);
    connect(m_fileList, &QListWidget::itemSelectionChanged, this, [this]() {
        m_fileContextPath.clear();
        updateFileContextActions();
    });
    connect(m_fileSearchEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        m_fileNameFilter = text.trimmed();
        populateFileList();
    });
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onFilterChanged);
    connect(m_aiModelCombo, &QComboBox::currentTextChanged, this, [this]() {
        m_settings.setValue(QStringLiteral("ai/model"), m_aiModelCombo->currentData().toString());
    });
    connect(m_aiOutputFormatCombo, &QComboBox::currentTextChanged, this, [this]() {
        m_settings.setValue(QStringLiteral("ai/outputFormat"), m_aiOutputFormatCombo->currentData().toString());
    });
    connect(m_aiTextRunButton, &QToolButton::clicked, this, &MainWindow::startAiTextAssist);
    connect(m_aiCancelButton, &QToolButton::clicked, this, &MainWindow::cancelAiAssist);
    connect(m_aiTextPromptEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        m_settings.setValue(QStringLiteral("ai/textPrompt"), text);
        refreshActions();
    });
    connect(m_aiTextModelCombo, &QComboBox::currentTextChanged, this, [this]() {
        m_settings.setValue(QStringLiteral("ai/textModel"), m_aiTextModelCombo->currentData().toString());
    });
    connect(m_aiTextScoreSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        m_settings.setValue(QStringLiteral("ai/textScore"), value);
    });
    connect(m_aiTextIouSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        m_settings.setValue(QStringLiteral("ai/textIou"), value);
    });
}

void MainWindow::loadSettings() {
    QString savedLanguage = m_settings.value("language").toString();
    if (savedLanguage.isEmpty()) {
        savedLanguage = StringBundle::systemLanguage();
        m_settings.setValue("language", savedLanguage);
    }
    m_strings = StringBundle(savedLanguage);
    m_saveDir = m_settings.value("savedir").toString();
    m_format = static_cast<SaveFormat>(m_settings.value("labelFileFormat", 0).toInt());
    m_recentFiles = m_settings.value("recentFiles").toStringList();
    m_recentDirs = m_settings.value("recentDirs").toStringList();
    const QVariantMap lastFilesByDir = m_settings.value(QStringLiteral("lastFileByDir")).toMap();
    for (auto it = lastFilesByDir.cbegin(); it != lastFilesByDir.cend(); ++it) {
        const QString directory = it.key().trimmed();
        const QString file = it.value().toString().trimmed();
        if (!directory.isEmpty() && !file.isEmpty()) {
            m_lastFileByDir.insert(QFileInfo(directory).absoluteFilePath(), QFileInfo(file).absoluteFilePath());
        }
    }
    m_lastUsedLabel = m_settings.value("lastUsedLabel", m_settings.value("defaultLabel").toString()).toString();
    m_validateLabelPolicy = m_settings.value(QStringLiteral("labelme/validateLabel")).toString().trimmed().toLower();
    if (m_validateLabelPolicy != QStringLiteral("exact")) {
        m_validateLabelPolicy.clear();
    }
    m_labelFlagPresets = labelFlagPresetsFromSource(m_settings.value(QStringLiteral("labelme/labelFlags")).toString());
}

void MainWindow::saveLastFileByDir() {
    QVariantMap lastFilesByDir;
    for (auto it = m_lastFileByDir.cbegin(); it != m_lastFileByDir.cend(); ++it) {
        lastFilesByDir.insert(it.key(), it.value());
    }
    m_settings.setValue(QStringLiteral("lastFileByDir"), lastFilesByDir);
}

void MainWindow::saveSettings() {
    m_settings.setValue("language", m_strings.language());
    m_settings.setValue("savedir", m_saveDir);
    m_settings.setValue("labelFileFormat", static_cast<int>(m_format));
    m_settings.setValue("recentFiles", m_recentFiles);
    m_settings.setValue("recentDirs", m_recentDirs);
    saveLastFileByDir();
    m_settings.setValue("filename", m_dirPath.isEmpty() ? m_filePath : QString());
    m_settings.setValue("autosave", m_autoSaveAction ? m_autoSaveAction->isChecked() : false);
    m_settings.setValue("keepPrevious", m_keepPreviousAction ? m_keepPreviousAction->isChecked() : false);
    m_settings.setValue("view/keepPreviousZoom", m_keepPreviousZoomAction ? m_keepPreviousZoomAction->isChecked() : false);
    m_settings.setValue("view/keepPreviousBrightnessContrast",
                        m_keepPreviousBrightnessContrastAction ? m_keepPreviousBrightnessContrastAction->isChecked() : false);
    m_settings.setValue("singleclass", m_singleClassAction ? m_singleClassAction->isChecked() : false);
    m_settings.setValue("paintlabel", m_displayLabelsAction ? m_displayLabelsAction->isChecked() : false);
    m_settings.setValue("labelme/embedImageData", m_embedImageDataAction ? m_embedImageDataAction->isChecked() : false);
    m_settings.setValue("useDefaultLabel", m_useDefaultLabel ? m_useDefaultLabel->isChecked() : false);
    m_settings.setValue("defaultLabel", m_defaultLabelCombo ? m_defaultLabelCombo->currentText() : QString());
    m_settings.setValue("lastUsedLabel", m_lastUsedLabel);
    m_settings.setValue(QStringLiteral("labelme/validateLabel"), m_validateLabelPolicy);
    m_settings.setValue(QStringLiteral("labelme/displayLabelPopup"),
                        m_settings.value(QStringLiteral("labelme/displayLabelPopup"), true).toBool());
    m_settings.setValue("draw/square", m_drawSquareAction ? m_drawSquareAction->isChecked() : false);
    m_settings.setValue("view/fillDrawing", m_fillDrawingAction ? m_fillDrawingAction->isChecked() : false);
    m_settings.setValue("view/miniMapEnabled", m_miniMapAction ? m_miniMapAction->isChecked() : true);
    m_settings.setValue("view/showPerformance", m_showPerformanceAction ? m_showPerformanceAction->isChecked() : true);
    m_settings.setValue("view/samplingMode", m_samplingModeAction && m_samplingModeAction->isChecked() ? QStringLiteral("Smooth") : QStringLiteral("FastNearest"));
    m_settings.setValue("view/fileThumbnails", m_thumbnailModeAction ? m_thumbnailModeAction->isChecked() : false);
    m_settings.setValue("ai/model", m_aiModelCombo ? m_aiModelCombo->currentData().toString() : QStringLiteral("sam2:latest"));
    m_settings.setValue("ai/outputFormat", m_aiOutputFormatCombo ? m_aiOutputFormatCombo->currentData().toString() : QStringLiteral("polygon"));
    m_settings.setValue("ai/textPrompt", m_aiTextPromptEdit ? m_aiTextPromptEdit->text() : QString());
    m_settings.setValue("ai/textModel", m_aiTextModelCombo ? m_aiTextModelCombo->currentData().toString()
                                                             : QStringLiteral("yoloworld:latest"));
    m_settings.setValue("ai/textScore", m_aiTextScoreSpin ? m_aiTextScoreSpin->value() : 0.1);
    m_settings.setValue("ai/textIou", m_aiTextIouSpin ? m_aiTextIouSpin->value() : 0.5);
    m_settings.setValue("advanced", m_advancedModeAction ? m_advancedModeAction->isChecked() : false);
    m_settings.setValue("line/color", m_lineColor);
    m_settings.setValue("fill/color", m_fillColor);
    m_settings.setValue("labelHistory", m_classList);
    m_settings.setValue("window/size", size());
    m_settings.setValue("window/position", pos());
    m_settings.setValue("window/state", saveState(DockStateVersion));
}

void MainWindow::loadPredefinedClasses() {
    loadPredefinedClassesFromFile(ResourcePaths::filePath(QStringLiteral("data/predefined_classes.txt")));
    mergePersistedLabelHistory();
}

void MainWindow::loadPredefinedClassesFromFile(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    m_classList.clear();
    while (!file.atEnd()) {
        QString line = QString::fromUtf8(file.readLine()).trimmed();
        addClassLabel(line);
    }
}

void MainWindow::mergePersistedLabelHistory() {
    const QStringList persisted = m_settings.value("labelHistory").toStringList();
    for (const QString &label : persisted) {
        addClassLabel(label);
    }
}

void MainWindow::addClassLabel(const QString &label) {
    const QString text = label.trimmed();
    if (text.isEmpty() || m_classList.contains(text)) {
        return;
    }
    m_classList.append(text);
    if (m_defaultLabelCombo && m_defaultLabelCombo->findText(text) < 0) {
        m_defaultLabelCombo->addItem(text);
    }
    refreshUniqueLabelList();
}

void MainWindow::refreshUniqueLabelList() {
    if (!m_uniqueLabelList) {
        return;
    }

    QString selectedLabel;
    if (QListWidgetItem *item = m_uniqueLabelList->currentItem(); item && item->isSelected()) {
        selectedLabel = item->data(Qt::UserRole).toString();
    }

    QSignalBlocker blocker(m_uniqueLabelList);
    m_uniqueLabelList->clear();
    for (const QString &label : m_classList) {
        auto *item = new QListWidgetItem(label, m_uniqueLabelList);
        item->setData(Qt::UserRole, label);
        item->setForeground(colorForLabel(label));
        item->setToolTip(label);
    }
    if (!selectedLabel.isEmpty()) {
        for (int row = 0; row < m_uniqueLabelList->count(); ++row) {
            if (m_uniqueLabelList->item(row)->data(Qt::UserRole).toString() == selectedLabel) {
                m_uniqueLabelList->setCurrentRow(row);
                break;
            }
        }
    }
}

QMap<QString, bool> MainWindow::labelFlagDefaultsForLabel(const QString &label) const {
    QMap<QString, bool> defaults;
    for (auto it = m_labelFlagPresets.cbegin(); it != m_labelFlagPresets.cend(); ++it) {
        const QRegularExpression regex(it.key());
        if (!regex.isValid()) {
            continue;
        }
        const QRegularExpressionMatch match = regex.match(label);
        if (!match.hasMatch() || match.capturedStart() != 0) {
            continue;
        }
        for (const QString &key : it.value()) {
            defaults.insert(key, false);
        }
    }
    return defaults;
}

QMap<QString, bool> MainWindow::mergeLabelFlagDefaults(const QString &label, const QMap<QString, bool> &flags) const {
    QMap<QString, bool> merged = labelFlagDefaultsForLabel(label);
    for (auto it = flags.cbegin(); it != flags.cend(); ++it) {
        merged.insert(it.key(), it.value());
    }
    return merged;
}

void MainWindow::applyLabelFlagDefaults(QVector<Shape> *shapes) const {
    if (!shapes || m_labelFlagPresets.isEmpty()) {
        return;
    }
    for (Shape &shape : *shapes) {
        shape.flags = mergeLabelFlagDefaults(shape.label, shape.flags);
        if (shape.flags.contains(QStringLiteral("difficult"))) {
            shape.difficult = shape.flags.value(QStringLiteral("difficult"));
        }
    }
}

void MainWindow::applyConfiguredLabelColors(QVector<Shape> *shapes) const {
    if (!shapes) {
        return;
    }
    for (Shape &shape : *shapes) {
        applyShapeLabelColors(&shape);
    }
}

QString MainWindow::preferredNewShapeLabel() const {
    if (m_uniqueLabelList) {
        const QList<QListWidgetItem *> selected = m_uniqueLabelList->selectedItems();
        if (!selected.isEmpty()) {
            const QString selectedLabel = selected.first()->data(Qt::UserRole).toString().trimmed();
            if (!selectedLabel.isEmpty() && validateLabel(selectedLabel)) {
                return selectedLabel;
            }
        }
    }
    if (!m_lastUsedLabel.trimmed().isEmpty() && validateLabel(m_lastUsedLabel)) {
        return m_lastUsedLabel.trimmed();
    }
    if (m_useDefaultLabel && m_useDefaultLabel->isChecked() && m_defaultLabelCombo &&
        m_defaultLabelCombo->currentIndex() >= 0 && validateLabel(m_defaultLabelCombo->currentText())) {
        return m_defaultLabelCombo->currentText().trimmed();
    }
    for (const QString &candidate : m_classList) {
        if (validateLabel(candidate)) {
            return candidate.trimmed();
        }
    }
    // LabelMe keeps an unlabeled draft when there is no learned, selected, or
    // configured label. The creation workflow then must show the label dialog
    // even when display_label_popup is disabled; silently inventing "object"
    // prevents the user from assigning a real class at creation time.
    return QString();
}

QColor MainWindow::colorForLabel(const QString &label) const {
    const QString normalized = label.trimmed();
    if (m_labelMeShapeColorMode == QStringLiteral("manual")) {
        const auto manual = m_labelMeLabelColors.constFind(normalized);
        if (manual != m_labelMeLabelColors.cend()) {
            return manual.value();
        }
        return m_labelMeDefaultShapeColor;
    }

    if (m_labelMeShapeColorMode == QStringLiteral("auto")) {
        int labelIndex = -1;
        QStringList seenLabels;
        if (m_canvas) {
            for (const Shape &shape : m_canvas->shapes()) {
                if (!seenLabels.contains(shape.label)) {
                    seenLabels.append(shape.label);
                }
            }
            labelIndex = seenLabels.indexOf(normalized);
        }
        if (labelIndex < 0) {
            labelIndex = m_classList.indexOf(normalized);
        }
        if (labelIndex < 0) {
            labelIndex = seenLabels.size();
        }
        constexpr qint64 kImgvizLabelColormapSize = 256;
        const qint64 rawColorIndex = 1LL + labelIndex + m_labelMeShiftAutoShapeColor;
        qint64 colorIndex = rawColorIndex % kImgvizLabelColormapSize;
        if (colorIndex < 0) {
            colorIndex += kImgvizLabelColormapSize;
        }
        QColor color = imgvizLabelColor(static_cast<int>(colorIndex));
        color.setAlpha(255);
        return color;
    }

    return m_labelMeDefaultShapeColor.isValid() ? m_labelMeDefaultShapeColor
                                                  : fallbackColorForLabel(normalized);
}

QColor MainWindow::fillColorForLabel(const QString &label) const {
    QColor color = colorForLabel(label);
    color.setAlpha(128);
    return color;
}

void MainWindow::applyShapeLabelColors(Shape *shape) const {
    if (!shape) {
        return;
    }
    shape->lineColor = colorForLabel(shape->label);
    shape->fillColor = fillColorForLabel(shape->label);
}

bool MainWindow::validateLabel(const QString &label) const {
    if (m_validateLabelPolicy.isEmpty()) {
        return true;
    }
    if (m_validateLabelPolicy == QStringLiteral("exact")) {
        return m_classList.contains(label.trimmed());
    }
    return false;
}

void MainWindow::rememberLastUsedLabel(const QString &label) {
    const QString text = label.trimmed();
    if (text.isEmpty()) {
        return;
    }
    m_lastUsedLabel = text;
    m_settings.setValue("lastUsedLabel", text);
    addClassLabel(text);
    m_settings.setValue("labelHistory", m_classList);
}

bool MainWindow::loadImage(const QString &path) {
    if (!maybeSave()) return false;
    const bool copyPreviousOnNavigation = m_copyPreviousNavigation;
    const bool keepPrevious = (m_keepPreviousAction && m_keepPreviousAction->isChecked()) ||
                              copyPreviousOnNavigation;
    const QVector<Shape> previousShapes = keepPrevious && m_canvas ? m_canvas->shapes() : QVector<Shape>();
    const bool keepPreviousZoom = m_keepPreviousZoomAction && m_keepPreviousZoomAction->isChecked();
    const bool keepPreviousBrightnessContrast = m_keepPreviousBrightnessContrastAction &&
                                                m_keepPreviousBrightnessContrastAction->isChecked();
    const bool hadPreviousImage = !m_filePath.isEmpty();
    const QString previousImagePath = m_filePath;
    const double previousScale = m_canvas ? m_canvas->scale() : 1.0;
    const int previousBrightness = m_canvas ? m_canvas->brightness() : 50;
    const int previousContrast = m_canvas ? m_canvas->contrast() : 50;
    const FitMode previousFitMode = m_fitMode;
    if (hadPreviousImage) {
        m_zoomByFile.insert(previousImagePath, previousScale);
        m_fitModeByFile.insert(previousImagePath, static_cast<int>(previousFitMode));
        m_brightnessByFile.insert(previousImagePath, previousBrightness);
        m_contrastByFile.insert(previousImagePath, previousContrast);
        rememberScrollPosition(previousImagePath);
    }
    QSize sourceSize;
    QString imageError;
    QImage image = ImageIO::readPreview(path, CanvasPreviewMaxSide, &sourceSize, &imageError);
    if (image.isNull()) {
        QMessageBox::warning(this, QStringLiteral("labelImgCpp"),
                             m_strings.get(QStringLiteral("cannotOpen")).arg(path));
        return false;
    }
    m_filePath = QFileInfo(path).absoluteFilePath();
    m_annotationPathOverride.clear();
    m_hasAnnotationPathOverride = false;
    if (m_fileDock) {
        m_fileDock->setEnabled(true);
    }
    const QString imageDir = QFileInfo(m_filePath).absolutePath();
    m_lastFileByDir.insert(imageDir, m_filePath);
    saveLastFileByDir();
    m_viewedFiles.insert(m_filePath);
    const int imageIndex = m_imageList.indexOf(m_filePath);
    if (imageIndex >= 0) {
        m_currentImageIndex = imageIndex;
    }
    m_settings.setValue("lastOpenDir", QFileInfo(m_filePath).absolutePath());
    setCanvasImage(image, sourceSize);
    m_canvas->setEnabled(true);
    m_labelMeImageData.clear();
    m_labelMeImagePath.clear();
    m_labelMeVersion.clear();
    m_labelMeTopLevelFlags.clear();
    m_labelMeOtherData = QJsonObject();
    QString annotationLoadError;
    loadAnnotationsForCurrentImage(&annotationLoadError);
    m_annotationLoadFailed = !annotationLoadError.isEmpty();
    const bool carriedPrevious = keepPrevious && !previousShapes.isEmpty() && m_canvas->shapes().isEmpty();
    if (carriedPrevious) {
        m_canvas->setShapes(previousShapes);
        statusBar()->showMessage(m_strings.get(QStringLiteral("keepPreviousAnnotationStatus")), 3000);
    }
    if (m_brightnessByFile.contains(m_filePath) && m_contrastByFile.contains(m_filePath)) {
        m_canvas->setBrightness(m_brightnessByFile.value(m_filePath));
        m_canvas->setContrast(m_contrastByFile.value(m_filePath));
    } else if (keepPreviousBrightnessContrast && hadPreviousImage) {
        m_canvas->setBrightness(previousBrightness);
        m_canvas->setContrast(previousContrast);
    } else {
        m_canvas->setBrightness(50);
        m_canvas->setContrast(50);
    }
    const bool hasStoredView = m_zoomByFile.contains(m_filePath) && m_fitModeByFile.contains(m_filePath);
    if (hasStoredView) {
        const int storedMode = qBound(0, m_fitModeByFile.value(m_filePath), 2);
        m_fitMode = static_cast<FitMode>(storedMode);
        m_canvas->setScale(m_zoomByFile.value(m_filePath));
    } else if (keepPreviousZoom && hadPreviousImage) {
        m_fitMode = previousFitMode;
        if (m_fitMode == FitMode::Manual) {
            m_canvas->setScale(previousScale);
        }
    }
    {
        QSignalBlocker fitWindowBlocker(m_fitWindowAction);
        QSignalBlocker fitWidthBlocker(m_fitWidthAction);
        m_fitWindowAction->setChecked(m_fitMode == FitMode::Window);
        m_fitWidthAction->setChecked(m_fitMode == FitMode::Width);
    }
    updateFitScale();
    restoreScrollPosition(m_filePath);
    addRecentFile(m_filePath);
    refreshLabels();
    resetShapeHistory();
    refreshFileListSelection();
    setDirty(carriedPrevious);
    m_verifyAction->setChecked(m_verified);
    if (annotationLoadError.isEmpty()) {
        statusBar()->showMessage(m_strings.get(QStringLiteral("loadedImage"))
                                     .arg(QFileInfo(m_filePath).fileName()), 5000);
    } else {
        statusBar()->showMessage(m_strings.get(QStringLiteral("failedLoad"))
                                     .arg(QFileInfo(m_filePath).fileName(), annotationLoadError),
                                 8000);
    }
    return true;
}

bool MainWindow::loadLabelMeWithRepair(const QString &path, AnnotationDocument *document) {
    if (!document) {
        return false;
    }

    QString loadError;
    if (AnnotationIO::loadLabelMe(path, document, &loadError)) {
        return true;
    }

    AnnotationDocument repaired;
    QString repairError;
    if (!AnnotationIO::loadLabelMe(path, &repaired, &repairError, true) ||
        !repaired.imageDataRepaired) {
        const QString detail = loadError.isEmpty() ? repairError : loadError;
        statusBar()->showMessage(m_strings.get(QStringLiteral("failedLoad")).arg(path, detail), 8000);
        return false;
    }

    QMessageBox repairDialog(QMessageBox::Warning,
                              m_strings.get(QStringLiteral("labelMeRepairTitle")),
                              m_strings.get(QStringLiteral("labelMeRepairText")),
                              QMessageBox::Yes | QMessageBox::No,
                              this);
    repairDialog.setObjectName(QStringLiteral("labelMeRepairDialog"));
    repairDialog.setInformativeText(QStringLiteral("%1\n%2")
                                        .arg(loadError, repaired.imageDataRepairMessage));
    repairDialog.setDefaultButton(QMessageBox::Yes);
    if (repairDialog.exec() != QMessageBox::Yes) {
        statusBar()->showMessage(m_strings.get(QStringLiteral("labelMeRepairCancelled")), 5000);
        return false;
    }

    *document = repaired;
    statusBar()->showMessage(m_strings.get(QStringLiteral("labelMeRepairApplied")), 5000);
    return true;
}

bool MainWindow::loadAnnotation(const QString &path) {
    if (m_filePath.isEmpty() || path.isEmpty()) return false;
    const auto fail = [this, &path](const QString &detail) {
        statusBar()->showMessage(m_strings.get(QStringLiteral("failedLoad")).arg(path, detail), 8000);
        return false;
    };
    AnnotationDocument doc;
    QFileInfo info(path);
    if (info.suffix().compare("xml", Qt::CaseInsensitive) == 0) {
        setFormat(SaveFormat::PascalVoc);
        if (!AnnotationIO::loadPascalVoc(path, &doc)) {
            return fail(m_strings.get(QStringLiteral("invalidPascalVoc")));
        }
    } else if (info.suffix().compare("txt", Qt::CaseInsensitive) == 0) {
        setFormat(SaveFormat::Yolo);
        QImage image = readImageWithAutoTransform(m_filePath);
        if (!AnnotationIO::loadYolo(path, image.size(), &doc)) {
            return fail(m_strings.get(QStringLiteral("invalidYolo")));
        }
    } else if (info.suffix().compare("json", Qt::CaseInsensitive) == 0) {
        QFile jsonFile(path);
        if (!jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return fail(m_strings.get(QStringLiteral("cannotOpenAnnotation")));
        }
        QJsonParseError parseError;
        const QJsonDocument json = QJsonDocument::fromJson(jsonFile.readAll(), &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            return fail(parseError.errorString());
        }
        const QJsonObject object = json.isObject() ? json.object() : QJsonObject();
        const bool isLabelMe = json.isObject() &&
                               object.contains(QStringLiteral("shapes")) &&
                               object.contains(QStringLiteral("imagePath")) &&
                               object.contains(QStringLiteral("imageData"));
        if (isLabelMe) {
            setFormat(SaveFormat::LabelMe);
            if (!loadLabelMeWithRepair(path, &doc)) return false;
        } else if (json.isArray()) {
            setFormat(SaveFormat::CreateMl);
            if (!AnnotationIO::loadCreateMl(path, m_filePath, &doc)) {
                return fail(m_strings.get(QStringLiteral("invalidCreateMl")));
            }
        } else {
            return fail(m_strings.get(QStringLiteral("unsupportedJsonAnnotation")));
        }
    } else {
        return fail(m_strings.get(QStringLiteral("unsupportedAnnotation")));
    }
    applyLabelFlagDefaults(&doc.shapes);
    applyConfiguredLabelColors(&doc.shapes);
    m_canvas->setShapes(doc.shapes);
    m_canvas->setCurrentIndex(-1);
    m_labelMeImageData = (m_format == SaveFormat::LabelMe) ? doc.imageData : QString();
    m_labelMeImagePath = (m_format == SaveFormat::LabelMe) ? doc.imagePath : QString();
    m_labelMeVersion = (m_format == SaveFormat::LabelMe) ? doc.labelMeVersion : QString();
    m_labelMeTopLevelFlags = (m_format == SaveFormat::LabelMe) ? doc.topLevelFlags : QMap<QString, bool>();
    for (auto it = m_labelMeConfiguredFlags.cbegin(); it != m_labelMeConfiguredFlags.cend(); ++it) {
        if (!m_labelMeTopLevelFlags.contains(it.key())) {
            m_labelMeTopLevelFlags.insert(it.key(), it.value());
        }
    }
    m_labelMeOtherData = (m_format == SaveFormat::LabelMe) ? doc.labelMeOtherData : QJsonObject();
    m_verified = doc.verified;
    m_verifyAction->setChecked(m_verified);
    m_annotationPathOverride = info.absoluteFilePath();
    m_annotationOverrideFormat = m_format;
    m_hasAnnotationPathOverride = true;
    m_annotationLoadFailed = false;
    m_canvas->setEnabled(true);
    refreshLabels();
    resetShapeHistory();
    syncTopLevelFlagsEditor();
    setDirty(doc.imageDataRepaired);
    statusBar()->showMessage(m_strings.get(QStringLiteral("loadedAnnotation")).arg(path), 5000);
    return true;
}

bool MainWindow::loadStandaloneLabelMe(const QString &path) {
    if (!maybeSave()) return false;

    const bool hadPreviousImage = !m_filePath.isEmpty();
    const QString previousImagePath = m_filePath;
    const int previousBrightness = m_canvas ? m_canvas->brightness() : 50;
    const int previousContrast = m_canvas ? m_canvas->contrast() : 50;
    const bool keepPreviousBrightnessContrast = m_keepPreviousBrightnessContrastAction &&
                                                m_keepPreviousBrightnessContrastAction->isChecked();
    if (hadPreviousImage) {
        m_brightnessByFile.insert(previousImagePath, previousBrightness);
        m_contrastByFile.insert(previousImagePath, previousContrast);
        rememberScrollPosition(previousImagePath);
    }

    AnnotationDocument doc;
    if (!loadLabelMeWithRepair(path, &doc)) {
        return false;
    }

    QImage image;
    QString resolvedImagePath;
    if (!doc.imageData.isEmpty()) {
        if (!image.loadFromData(QByteArray::fromBase64(doc.imageData.toLatin1()))) {
            return false;
        }
        image = ImageIO::normalizeForDisplay(image);
    } else {
        const QFileInfo annotationInfo(path);
        resolvedImagePath = QDir::isAbsolutePath(doc.imagePath)
                                ? doc.imagePath
                                : annotationInfo.dir().filePath(doc.imagePath);
        QSize sourceSize;
        image = ImageIO::readPreview(resolvedImagePath,
                                     CanvasPreviewMaxSide,
                                     &sourceSize);
        if (image.isNull()) {
            return false;
        }
        if (sourceSize.isValid() && sourceSize != image.size()) {
            doc.imageSize = sourceSize;
        }
    }

    const QFileInfo annotationInfo(path);
    const QString imagePath = resolvedImagePath.isEmpty()
                                  ? (doc.imagePath.isEmpty()
                                         ? annotationInfo.dir().filePath(annotationInfo.completeBaseName() + QStringLiteral(".png"))
                                         : annotationInfo.dir().filePath(doc.imagePath))
                                  : resolvedImagePath;

    setFormat(SaveFormat::LabelMe);
    m_filePath = QFileInfo(imagePath).absoluteFilePath();
    m_annotationPathOverride = annotationInfo.absoluteFilePath();
    m_annotationOverrideFormat = SaveFormat::LabelMe;
    m_hasAnnotationPathOverride = true;
    m_annotationLoadFailed = false;
    m_dirPath.clear();
    m_imageList.clear();
    m_currentImageIndex = -1;
    if (m_fileDock) {
        m_fileDock->setEnabled(false);
    }
    m_labelMeImageData = doc.imageData;
    m_labelMeImagePath = doc.imagePath;
    m_labelMeVersion = doc.labelMeVersion;
    m_labelMeTopLevelFlags = doc.topLevelFlags;
    for (auto it = m_labelMeConfiguredFlags.cbegin(); it != m_labelMeConfiguredFlags.cend(); ++it) {
        if (!m_labelMeTopLevelFlags.contains(it.key())) {
            m_labelMeTopLevelFlags.insert(it.key(), it.value());
        }
    }
    m_labelMeOtherData = doc.labelMeOtherData;
    m_verified = doc.verified;
    applyLabelFlagDefaults(&doc.shapes);
    applyConfiguredLabelColors(&doc.shapes);
    QSize sourceSize = doc.imageSize.isValid() && !doc.imageSize.isEmpty()
                           ? doc.imageSize
                           : image.size();
    setCanvasImage(image, sourceSize);
    m_canvas->setEnabled(true);
    m_canvas->setShapes(doc.shapes);
    m_canvas->setCurrentIndex(-1);
    if (m_brightnessByFile.contains(m_filePath) && m_contrastByFile.contains(m_filePath)) {
        m_canvas->setBrightness(m_brightnessByFile.value(m_filePath));
        m_canvas->setContrast(m_contrastByFile.value(m_filePath));
    } else if (keepPreviousBrightnessContrast && hadPreviousImage) {
        m_canvas->setBrightness(previousBrightness);
        m_canvas->setContrast(previousContrast);
    } else {
        m_canvas->setBrightness(50);
        m_canvas->setContrast(50);
    }
    updateFitScale();
    restoreScrollPosition(m_filePath);
    refreshLabels();
    resetShapeHistory();
    refreshFileListSelection();
    syncTopLevelFlagsEditor();
    setDirty(doc.imageDataRepaired);
    m_verifyAction->setChecked(m_verified);
    statusBar()->showMessage(m_strings.get(QStringLiteral("loadedAnnotation")).arg(path), 5000);
    return true;
}

AnnotationDocument MainWindow::annotationDocumentForImage(const QString &imagePath, const QSize &imageSize,
                                                          SaveFormat *detectedFormat,
                                                          QString *errorMessage) const {
    if (errorMessage) {
        errorMessage->clear();
    }
    AnnotationDocument doc;
    if (detectedFormat) {
        *detectedFormat = m_format;
    }
    doc.imagePath = imagePath;
    doc.imageSize = imageSize;
    if (m_format == SaveFormat::LabelMe && !m_outputFilePath.isEmpty()) {
        const QFileInfo outputInfo(m_outputFilePath);
        if (outputInfo.exists()) {
            QString labelMeError;
            AnnotationDocument fixedDocument;
            if (!AnnotationIO::loadLabelMe(m_outputFilePath, &fixedDocument, &labelMeError)) {
                if (errorMessage) {
                    *errorMessage = QStringLiteral("%1: %2")
                                        .arg(m_outputFilePath,
                                             labelMeError.isEmpty()
                                                 ? QStringLiteral("invalid LabelMe annotation")
                                                 : labelMeError);
                }
                return doc;
            }

            // A fixed output file is only valid for the image it describes;
            // navigating another image must not reuse the first image's JSON.
            bool belongsToImage = fixedDocument.imagePath.isEmpty();
            if (!belongsToImage) {
                const QString resolved = QDir(outputInfo.absolutePath()).absoluteFilePath(fixedDocument.imagePath);
                const QString expected = QFileInfo(imagePath).absoluteFilePath();
                belongsToImage = QFileInfo(resolved).absoluteFilePath().compare(expected, Qt::CaseInsensitive) == 0;
            }
            if (belongsToImage) {
                doc = fixedDocument;
                if (detectedFormat) {
                    *detectedFormat = SaveFormat::LabelMe;
                }
            }
        }
        if (!imageSize.isEmpty()) {
            doc.imageSize = imageSize;
        }
        return doc;
    }

    QStringList dirs;
    if (!m_saveDir.isEmpty()) {
        dirs.append(m_saveDir);
    }
    dirs.append(QFileInfo(imagePath).absolutePath());
    dirs.removeDuplicates();

    for (const QString &dir : dirs) {
        const QString base = QDir(dir).filePath(QFileInfo(imagePath).completeBaseName());
        const QString vocPath = base + ".xml";
        if (QFileInfo::exists(vocPath)) {
            if (AnnotationIO::loadPascalVoc(base + ".xml", &doc) && detectedFormat) {
                *detectedFormat = SaveFormat::PascalVoc;
            } else if (errorMessage) {
                *errorMessage = QStringLiteral("%1: invalid Pascal VOC annotation").arg(vocPath);
            }
            break;
        }
        const QString yoloPath = base + ".txt";
        if (QFileInfo::exists(yoloPath)) {
            if (AnnotationIO::loadYolo(yoloPath, imageSize, &doc) && detectedFormat) {
                *detectedFormat = SaveFormat::Yolo;
            } else if (errorMessage) {
                *errorMessage = QStringLiteral("%1: invalid YOLO annotation").arg(yoloPath);
            }
            break;
        }
        const QString jsonPath = base + ".json";
        if (QFileInfo::exists(jsonPath)) {
            QString labelMeError;
            if (!AnnotationIO::loadLabelMe(jsonPath, &doc, &labelMeError)) {
                if (AnnotationIO::loadCreateMl(jsonPath, imagePath, &doc) && detectedFormat) {
                    *detectedFormat = SaveFormat::CreateMl;
                } else if (errorMessage) {
                    *errorMessage = QStringLiteral("%1: %2")
                                        .arg(jsonPath,
                                             labelMeError.isEmpty()
                                                 ? QStringLiteral("invalid JSON annotation")
                                                 : labelMeError);
                }
            } else if (detectedFormat) {
                *detectedFormat = SaveFormat::LabelMe;
            }
            break;
        }
    }
    if (!imageSize.isEmpty()) {
        doc.imageSize = imageSize;
    }
    return doc;
}

void MainWindow::loadAnnotationsForCurrentImage(QString *errorMessage) {
    QSize imageSize = m_canvas ? m_canvas->pixmapSize() : QSize();
    if (!imageSize.isValid() || imageSize.isEmpty()) {
        imageSize = readImageWithAutoTransform(m_filePath).size();
    }
    SaveFormat detectedFormat = m_format;
    AnnotationDocument doc = annotationDocumentForImage(m_filePath, imageSize, &detectedFormat, errorMessage);
    if (detectedFormat != m_format) {
        setFormat(detectedFormat);
    }
    applyLabelFlagDefaults(&doc.shapes);
    applyConfiguredLabelColors(&doc.shapes);
    m_canvas->setShapes(doc.shapes);
    m_canvas->setCurrentIndex(-1);
    m_labelMeImageData = doc.imageData;
    m_labelMeImagePath = doc.imagePath;
    m_labelMeVersion = doc.labelMeVersion;
    m_labelMeTopLevelFlags = doc.topLevelFlags;
    for (auto it = m_labelMeConfiguredFlags.cbegin(); it != m_labelMeConfiguredFlags.cend(); ++it) {
        if (!m_labelMeTopLevelFlags.contains(it.key())) {
            m_labelMeTopLevelFlags.insert(it.key(), it.value());
        }
    }
    m_labelMeOtherData = doc.labelMeOtherData;
    m_verified = doc.verified;
    syncTopLevelFlagsEditor();
}

void MainWindow::refreshLabels() {
    m_labelList->blockSignals(true);
    m_labelList->clear();
    QStringList uniqueLabels;
    const QVector<Shape> shapes = m_canvas->shapes();
    for (int i = 0; i < shapes.size(); ++i) {
        const Shape &shape = shapes[i];
        addClassLabel(shape.label);
        QListWidgetItem *item = new QListWidgetItem(labelListDisplayText(shape));
        item->setData(Qt::UserRole, shape.label);
        item->setData(Qt::UserRole + 1, i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEditable);
        item->setCheckState(shape.visible ? Qt::Checked : Qt::Unchecked);
        item->setIcon(labelColorDotIcon(colorForLabel(shape.label)));
        m_labelList->addItem(item);
        if (!uniqueLabels.contains(shape.label)) uniqueLabels.append(shape.label);
    }
    m_labelList->blockSignals(false);

    m_filterCombo->blockSignals(true);
    m_filterCombo->clear();
    uniqueLabels.sort();
    m_filterCombo->addItem(QString());
    m_filterCombo->addItems(uniqueLabels);
    m_filterCombo->blockSignals(false);

    const QVector<int> selectedIndices = m_canvas->selectedIndices();
    if (!selectedIndices.isEmpty()) {
        for (int index : selectedIndices) {
            if (index >= 0 && index < m_labelList->count()) {
                m_labelList->item(index)->setSelected(true);
            }
        }
        if (m_canvas->currentIndex() >= 0 && m_canvas->currentIndex() < m_labelList->count()) {
            m_labelList->setCurrentRow(m_canvas->currentIndex(), QItemSelectionModel::NoUpdate);
        }
    } else if (m_canvas->currentIndex() >= 0 && m_canvas->currentIndex() < m_labelList->count()) {
        m_labelList->setCurrentRow(m_canvas->currentIndex(), QItemSelectionModel::ClearAndSelect);
    }
    scrollLabelListToCurrentShape();
    refreshActions();
}

void MainWindow::scrollLabelListToCurrentShape() {
    if (!m_canvas || !m_labelList) {
        return;
    }
    const int row = m_canvas->currentIndex();
    if (row >= 0 && row < m_labelList->count()) {
        m_labelList->scrollToItem(m_labelList->item(row), QAbstractItemView::EnsureVisible);
    }
}

void MainWindow::syncShapeOrderFromLabelList() {
    if (!m_canvas || !m_labelList || m_labelList->count() != m_canvas->shapes().size()) {
        return;
    }

    const QVector<Shape> oldShapes = m_canvas->shapes();
    const QVector<int> oldSelected = m_canvas->selectedIndices();
    const int oldCurrent = m_canvas->currentIndex();
    QVector<Shape> reordered;
    reordered.reserve(oldShapes.size());
    QVector<int> sourceIndices;
    sourceIndices.reserve(oldShapes.size());
    for (int row = 0; row < m_labelList->count(); ++row) {
        bool ok = false;
        const int sourceIndex = m_labelList->item(row)->data(Qt::UserRole + 1).toInt(&ok);
        if (!ok || sourceIndex < 0 || sourceIndex >= oldShapes.size() || sourceIndices.contains(sourceIndex)) {
            return;
        }
        sourceIndices.push_back(sourceIndex);
        reordered.push_back(oldShapes.at(sourceIndex));
    }

    m_canvas->shapesRef() = reordered;
    QVector<int> selected;
    for (int row = 0; row < sourceIndices.size(); ++row) {
        if (oldSelected.contains(sourceIndices.at(row))) {
            selected.push_back(row);
        }
    }
    m_canvas->setSelectedIndices(selected);
    if (selected.isEmpty() && oldCurrent >= 0) {
        const int newCurrent = sourceIndices.indexOf(oldCurrent);
        if (newCurrent >= 0) {
            m_canvas->setCurrentIndex(newCurrent);
        }
    }
    refreshLabels();
    recordShapeHistory();
    setDirty(true);
}

void MainWindow::populateFileList() {
    if (!m_fileList) {
        return;
    }

    rebuildFileLabelFilterMenu();
    const bool thumbnailMode = m_thumbnailModeAction && m_thumbnailModeAction->isChecked();
    QRegularExpression fileSearchPattern;
    bool fileSearchPatternValid = true;
    if (!m_fileNameFilter.isEmpty()) {
        fileSearchPattern = QRegularExpression(m_fileNameFilter,
                                                QRegularExpression::CaseInsensitiveOption);
        fileSearchPatternValid = fileSearchPattern.isValid();
    }
    m_fileList->clear();
    m_fileList->setViewMode(QListView::ListMode);
    m_fileList->setMovement(QListView::Static);
    m_fileList->setResizeMode(QListView::Adjust);
    m_fileList->setIconSize(thumbnailMode ? QSize(72, 54) : QSize(0, 0));
    m_fileList->setSpacing(thumbnailMode ? 4 : 0);
    m_fileList->setUniformItemSizes(!thumbnailMode);

    for (const QString &path : m_imageList) {
        // LabelMe treats this field as a regex search over the full path. Keep
        // an invalid pattern harmless, matching its re.error fallback.
        if (!m_fileNameFilter.isEmpty() && fileSearchPatternValid &&
            !fileSearchPattern.match(path).hasMatch()) {
            continue;
        }
        if (!m_fileLabelFilter.isEmpty() && !labelsForImage(path).contains(m_fileLabelFilter)) {
            continue;
        }
        auto *item = new QListWidgetItem(path);
        item->setToolTip(path);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(hasAnnotationForImage(path) ? Qt::Checked : Qt::Unchecked);
        item->setSizeHint(thumbnailMode ? QSize(0, 64) : QSize(0, 22));
        if (thumbnailMode) {
            item->setIcon(fileThumbnailIcon(path));
        }
        m_fileList->addItem(item);
    }
    refreshFileListSelection();
    updateFileDockTitle();
}

QIcon MainWindow::fileThumbnailIcon(const QString &path) const {
    QSize sourceSize;
    const QImage image = ImageIO::readPreview(path, 512, &sourceSize);
    if (image.isNull()) {
        return {};
    }
    if (!sourceSize.isValid() || sourceSize.isEmpty()) {
        sourceSize = image.size();
    }

    const QSize iconSize(72, 54);
    const QSize scaledSize = image.size().scaled(iconSize, Qt::KeepAspectRatio);
    const QRect targetRect(QPoint((iconSize.width() - scaledSize.width()) / 2,
                                  (iconSize.height() - scaledSize.height()) / 2),
                           scaledSize);
    QPixmap thumbnail(iconSize);
    thumbnail.fill(QColor(250, 250, 250));

    QPainter painter(&thumbnail);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.drawImage(targetRect, image);

    const AnnotationDocument doc = annotationDocumentForImage(path, sourceSize);
    if (!doc.imageSize.isEmpty()) {
        const double xScale = static_cast<double>(targetRect.width()) / doc.imageSize.width();
        const double yScale = static_cast<double>(targetRect.height()) / doc.imageSize.height();
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setPen(QPen(Qt::yellow, 3));
        painter.setBrush(Qt::NoBrush);
        for (const Shape &shape : doc.shapes) {
            const QRectF box = shape.boundingRect();
            painter.drawRect(QRectF(targetRect.x() + box.x() * xScale,
                                    targetRect.y() + box.y() * yScale,
                                    box.width() * xScale,
                                    box.height() * yScale));
        }
    }
    return QIcon(thumbnail);
}

QStringList MainWindow::labelsForImage(const QString &path) const {
    QImageReader reader(path);
    reader.setAutoTransform(true);
    QSize imageSize = reader.size();
    if (!imageSize.isValid() || imageSize.isEmpty()) {
        const QImage image = readImageWithAutoTransform(path);
        imageSize = image.size();
    }

    const AnnotationDocument doc = annotationDocumentForImage(path, imageSize);
    QSet<QString> uniqueLabels;
    for (const Shape &shape : doc.shapes) {
        const QString label = shape.label.trimmed();
        if (!label.isEmpty()) {
            uniqueLabels.insert(label);
        }
    }
    QStringList labels(uniqueLabels.cbegin(), uniqueLabels.cend());
    labels.sort(Qt::CaseInsensitive);
    return labels;
}

void MainWindow::rebuildFileLabelFilterMenu() {
    if (!m_fileLabelFilterMenu) {
        return;
    }

    QHash<QString, int> labelCounts;
    for (const QString &path : m_imageList) {
        const QStringList labels = labelsForImage(path);
        for (const QString &label : labels) {
            labelCounts[label] += 1;
        }
    }
    if (!m_fileLabelFilter.isEmpty() && !labelCounts.contains(m_fileLabelFilter)) {
        m_fileLabelFilter.clear();
    }

    m_fileLabelFilterMenu->clear();
    auto addFilterAction = [this](const QString &label, const QString &text) {
        QAction *action = m_fileLabelFilterMenu->addAction(text);
        action->setCheckable(true);
        action->setChecked(m_fileLabelFilter == label);
        action->setData(label);
        connect(action, &QAction::triggered, this, [this, label]() {
            m_fileLabelFilter = label;
            populateFileList();
        });
    };

    addFilterAction(QString(), m_strings.get(QStringLiteral("allFiles")).arg(m_imageList.size()));
    QStringList labels = labelCounts.keys();
    labels.sort(Qt::CaseInsensitive);
    if (!labels.isEmpty()) {
        m_fileLabelFilterMenu->addSeparator();
    }
    for (const QString &label : labels) {
        addFilterAction(label, QStringLiteral("%1 (%2)").arg(label).arg(labelCounts.value(label)));
    }
}

void MainWindow::updateFileDockTitle() {
    if (!m_fileDockTitleLabel) {
        return;
    }

    const int total = m_imageList.size();
    const int visible = m_fileList ? m_fileList->count() : total;
    QString countText = QString::number(total);
    if (!m_fileLabelFilter.isEmpty() || !m_fileNameFilter.isEmpty()) {
        countText = QStringLiteral("%1/%2").arg(visible).arg(total);
    }

    QString title = m_strings.get(QStringLiteral("fileListTitle")).arg(countText);
    if (!m_fileLabelFilter.isEmpty()) {
        title += QString::fromUtf8(" · %1").arg(m_fileLabelFilter);
    }
    if (!m_fileNameFilter.isEmpty()) {
        title += QStringLiteral(" · ") + m_strings.get(QStringLiteral("searchFilterPrefix")).arg(m_fileNameFilter);
    }
    m_fileDockTitleLabel->setText(title);
    m_fileDockTitleLabel->setToolTip(title);
    if (m_fileDock) {
        m_fileDock->setWindowTitle(title);
    }
}

void MainWindow::refreshFileListSelection() {
    QListWidgetItem *activeItem = nullptr;
    for (int i = 0; i < m_fileList->count(); ++i) {
        QListWidgetItem *item = m_fileList->item(i);
        const QString itemPath = QFileInfo(item->text()).absoluteFilePath();
        const bool active = itemPath == m_filePath;
        const bool visited = !active && m_viewedFiles.contains(itemPath);
        const bool marked = m_markedFiles.contains(itemPath);
        item->setData(ActiveFileRole, active);
        item->setData(VisitedFileRole, visited);
        item->setData(MarkedFileRole, marked);
        QFont font = item->font();
        font.setBold(active);
        item->setFont(font);
        item->setForeground(active ? QBrush(QColor(15, 61, 36)) : (visited ? QBrush(QColor(107, 114, 128)) : QBrush()));
        item->setBackground(active ? QBrush(QColor(231, 245, 236)) : (marked ? QBrush(QColor(255, 247, 196)) : QBrush()));
        item->setData(Qt::UserRole, active ? QStringLiteral("activeFile") : QString());
        if (active) {
            activeItem = item;
        }
    }
    if (activeItem) {
        m_fileList->setCurrentItem(activeItem, QItemSelectionModel::ClearAndSelect);
        m_fileList->scrollToItem(activeItem);
    }
}

void MainWindow::refreshTexts() {
    m_openAction->setText(m_strings.get("openFile"));
    m_openDirAction->setText(m_strings.get("openDir"));
    m_openAnnotationAction->setText(m_strings.get("openAnnotation"));
    m_openWithImageViewerAction->setText(m_strings.get("openWithImageViewer"));
    m_openFileLocationAction->setText(m_strings.get("openFileLocation"));
    m_fileContextOpenAction->setText(m_strings.get("fileContextOpen"));
    m_fileContextRevealAction->setText(m_strings.get("fileContextReveal"));
    m_fileContextCopyPathAction->setText(m_strings.get("fileContextCopyPath"));
    m_fileContextMarkAction->setText(m_strings.get("fileContextMark"));
    m_fileContextDeleteAction->setText(m_strings.get("fileContextDelete"));
    m_closeAction->setText(m_strings.get("closeCur"));
    m_quitAction->setText(m_strings.get("quit"));
    m_resetAllAction->setText(m_strings.get("resetAll"));
    m_resetLayoutAction->setText(m_strings.get("resetLayout"));
    m_saveAction->setText(m_strings.get("save"));
    m_saveAsAction->setText(m_strings.get("saveAs"));
    m_changeSaveDirAction->setText(m_strings.get("changeSaveDir"));
    m_nextAction->setText(m_strings.get("nextImg"));
    m_prevAction->setText(m_strings.get("prevImg"));
    m_nextCopyAction->setText(m_strings.get("nextImgCopy"));
    m_prevCopyAction->setText(m_strings.get("prevImgCopy"));
    m_verifyAction->setText(m_strings.get("verifyImg"));
    m_editLabelAction->setText(m_strings.get("editLabel"));
    m_undoAction->setText(m_strings.get("undo"));
    m_undoLastPointAction->setText(m_strings.get("undoLastPoint"));
    m_redoAction->setText(m_strings.get("redo"));
    m_prevShapeAction->setText(m_strings.get("prevShape"));
    m_nextShapeAction->setText(m_strings.get("nextShape"));
    m_deleteAction->setText(m_strings.get("delBox"));
    m_deleteAllShapesAction->setText(m_strings.get("deleteAllShapes"));
    m_copyAction->setText(m_strings.get("dupBox"));
    m_copyShapesAction->setText(m_strings.get("copyShapes"));
    m_pasteShapesAction->setText(m_strings.get("pasteShapes"));
    m_copyHereAction->setText(m_strings.get("copyHere"));
    m_moveHereAction->setText(m_strings.get("moveHere"));
    m_insertPolygonPointAction->setText(m_strings.get("insertPolygonPoint"));
    m_addPointToEdgeAction->setText(m_strings.get("addPointToEdge"));
    m_removePolygonPointAction->setText(m_strings.get("removePolygonPoint"));
    m_removeSelectedPointAction->setText(m_strings.get("removeSelectedPoint"));
    m_copyPreviousAction->setText(m_strings.get("copyPrevBounding"));
    m_deleteImageAction->setText(m_strings.get("deleteImg"));
    m_deleteAnnotationAction->setText(m_strings.get("deleteAnnotation"));
    m_createModeAction->setText(m_strings.get("crtBox"));
    m_createPolygonModeAction->setText(m_strings.get("createPolygon"));
    m_createPointModeAction->setText(m_strings.get("createPoint"));
    m_createPointsModeAction->setText(m_strings.get("createPoints"));
    m_createAiPointsModeAction->setText(m_strings.get("createAiPoints"));
    m_createAiBoxModeAction->setText(m_strings.get("createAiBox"));
    m_createLineModeAction->setText(m_strings.get("createLine"));
    m_createLinestripModeAction->setText(m_strings.get("createLinestrip"));
    m_createCircleModeAction->setText(m_strings.get("createCircle"));
    m_createOrientedRectangleModeAction->setText(m_strings.get("createOrientedRectangle"));
    m_createMaskModeAction->setText(m_strings.get("createMask"));
    m_maskEditAction->setText(m_strings.get("editMask"));
    m_editModeAction->setText(m_strings.get("editBox"));
    m_viewModeAction->setText(m_strings.get("viewMode"));
    m_editabilityAction->setText(m_strings.get("editability"));
    m_advancedModeAction->setText(m_strings.get("advancedMode"));
    m_hideAllAction->setText(m_strings.get("hideAllBox"));
    m_showAllAction->setText(m_strings.get("showAllBox"));
    m_toggleAllAction->setText(m_strings.get("toggleAll"));
    m_zoomInAction->setText(m_strings.get("zoomin"));
    m_zoomOutAction->setText(m_strings.get("zoomout"));
    m_zoomOriginalAction->setText(m_strings.get("originalsize"));
    if (m_zoomWidget) m_zoomWidget->setToolTip(m_strings.get("zoomLevel"));
    m_fitWindowAction->setText(m_strings.get("fitWin"));
    m_fitWidthAction->setText(m_strings.get("fitWidth"));
    m_brightenAction->setText(m_strings.get("lightbrighten"));
    m_darkenAction->setText(m_strings.get("lightdarken"));
    m_brightnessOriginalAction->setText(m_strings.get("lightreset"));
    m_brightnessContrastAction->setText(m_strings.get("brightnessContrast"));
    m_keepPreviousBrightnessContrastAction->setText(m_strings.get("keepPreviousBrightness"));
    m_drawSquareAction->setText(m_strings.get("drawSquares"));
    m_fillDrawingAction->setText(m_strings.get("fillDrawing"));
    m_boxLineColorAction->setText(m_strings.get("boxLineColor"));
    m_shapeLineColorAction->setText(m_strings.get("shapeLineColor"));
    m_shapeFillColorAction->setText(m_strings.get("shapeFillColor"));
    m_infoAction->setText(m_strings.get("info"));
    m_shortcutsAction->setText(m_strings.get("shortcut"));
    m_tutorialAction->setText(m_strings.get("tutorialDefault"));
    m_settingsAction->setText(m_strings.get("settings"));
    m_autoSaveAction->setText(m_strings.get("autoSaveMode"));
    m_keepPreviousAction->setText(m_strings.get("keepPrevious"));
    m_keepPreviousZoomAction->setText(m_strings.get("keepPreviousZoom"));
    m_singleClassAction->setText(m_strings.get("singleClsMode"));
    m_displayLabelsAction->setText(m_strings.get("displayLabel"));
    m_embedImageDataAction->setText(m_strings.get("saveWithImageData"));
    m_editLabelFlagsAction->setText(m_strings.get("editLabelFlags"));
    if (m_aiTextPromptEdit) {
        m_aiTextPromptEdit->setPlaceholderText(QStringLiteral("person, sofa"));
        m_aiTextPromptEdit->setToolTip(m_strings.get("aiPromptTooltip"));
    }
    if (m_aiTextRunButton) {
        m_aiTextRunButton->setText(m_strings.get("run"));
        m_aiTextRunButton->setToolTip(m_strings.get("aiPromptRunTooltip"));
    }
    if (m_aiCancelButton) {
        m_aiCancelButton->setText(QStringLiteral("×"));
        m_aiCancelButton->setToolTip(m_strings.get("aiCancel"));
    }
    if (m_uniqueLabelList) {
        m_uniqueLabelList->setToolTip(m_strings.get("uniqueLabelTooltip"));
    }
    const auto refreshLabel = [this](const QString &objectName, const QString &key) {
        if (QLabel *label = findChild<QLabel *>(objectName)) {
            label->setText(m_strings.get(key));
        }
    };
    refreshLabel(QStringLiteral("newLabelTitle"), QStringLiteral("newLabel"));
    refreshLabel(QStringLiteral("aiModelLabel"), QStringLiteral("aiModel"));
    refreshLabel(QStringLiteral("aiOutputLabel"), QStringLiteral("aiOutput"));
    refreshLabel(QStringLiteral("aiTextPromptLabel"), QStringLiteral("aiTextPrompt"));
    refreshLabel(QStringLiteral("aiScoreLabel"), QStringLiteral("score"));
    refreshLabel(QStringLiteral("aiIouLabel"), QStringLiteral("iou"));
    refreshLabel(QStringLiteral("topLevelFlagsLabel"), QStringLiteral("topLevelFlags"));
    m_miniMapAction->setText(m_strings.get("miniMap"));
    m_showPerformanceAction->setText(m_strings.get("performance"));
    m_samplingModeAction->setText(m_samplingModeAction->isChecked() ? m_strings.get("smoothSampling") : m_strings.get("fastSampling"));
    m_thumbnailModeAction->setText(m_strings.get("thumbnailMode"));
    if (m_flagDock) {
        m_flagDock->setWindowTitle(m_strings.get("flags"));
    }
    if (m_showFlagDockAction) {
        m_showFlagDockAction->setText(m_strings.get("flags"));
    }
    m_showLabelDockAction->setText(m_strings.get("labelDock"));
    if (m_showShapeDockAction) {
        m_showShapeDockAction->setText(m_strings.get("shapeDock"));
    }
    m_showFileDockAction->setText(m_strings.get("fileDock"));
    if (m_fileLabelFilterButton) {
        m_fileLabelFilterButton->setText(m_strings.get("filter"));
        m_fileLabelFilterButton->setToolTip(m_strings.get("labelFilterTooltip"));
    }
    if (m_fileSearchEdit) {
        m_fileSearchEdit->setPlaceholderText(m_strings.get("fileSearchPlaceholder"));
        m_fileSearchEdit->setToolTip(m_strings.get("fileSearchTooltip"));
    }
    if (m_footerModeCombo) {
        const int viewIndex = m_footerModeCombo->findData(QStringLiteral("view"));
        const int editIndex = m_footerModeCombo->findData(QStringLiteral("edit"));
        const int createIndex = m_footerModeCombo->findData(QStringLiteral("create"));
        if (viewIndex >= 0) {
            m_footerModeCombo->setItemText(viewIndex, m_strings.get(QStringLiteral("footerViewMode")));
        }
        if (editIndex >= 0) {
            m_footerModeCombo->setItemText(editIndex, m_strings.get(QStringLiteral("footerEditMode")));
        }
        if (createIndex >= 0) {
            m_footerModeCombo->setItemText(createIndex, m_strings.get(QStringLiteral("footerCreateMode")));
        }
    }
    if (m_footerViewShortcut) {
        m_footerViewShortcut->setText(QStringLiteral("V"));
        m_footerViewShortcut->setToolTip(m_strings.get(QStringLiteral("footerViewTooltip")));
    }
    if (m_footerEditShortcut) {
        m_footerEditShortcut->setText(m_strings.get(QStringLiteral("footerEditMode")));
        m_footerEditShortcut->setToolTip(m_strings.get(QStringLiteral("footerEditTooltip")));
    }
    if (m_footerCreateShortcut) {
        m_footerCreateShortcut->setText(QStringLiteral("W"));
        m_footerCreateShortcut->setToolTip(m_strings.get(QStringLiteral("footerCreateTooltip")));
    }
    m_fileDockCloseButton->setToolTip(m_strings.get("hideFileList"));
    m_labelDock->setWindowTitle(m_strings.get("labelDock"));
    if (m_shapeDock) {
        m_shapeDock->setWindowTitle(m_strings.get("shapeDock"));
    }
    rebuildFileLabelFilterMenu();
    updateFileDockTitle();
    m_fileMenu->setTitle(m_strings.get("menu_file"));
    m_viewMenu->setTitle(m_strings.get("menu_view"));
    m_helpMenu->setTitle(m_strings.get("menu_help"));
    m_languageMenu->setTitle(m_strings.get("language"));
    m_recentFilesMenu->setTitle(m_strings.get("menu_openRecent"));
    m_recentDirsMenu->setTitle(m_strings.get("recentDirs"));
    m_useDefaultLabel->setText(m_strings.get("useDefaultLabel"));
    m_difficult->setText(m_strings.get("useDifficult"));
    m_formatAction->setText(currentFormatName());
    refreshActionToolTips();
    syncModeFooterControls();
    rebuildRecentFilesMenu();
    rebuildRecentDirsMenu();
    updateFileContextActions();
}

void MainWindow::refreshActionToolTips() {
    const QList<QAction *> actions = {
        m_openAction,
        m_openDirAction,
        m_openAnnotationAction,
        m_closeAction,
        m_quitAction,
        m_resetAllAction,
        m_resetLayoutAction,
        m_saveAction,
        m_saveAsAction,
        m_changeSaveDirAction,
        m_formatAction,
        m_nextAction,
        m_prevAction,
        m_verifyAction,
        m_editLabelAction,
        m_undoAction,
        m_undoLastPointAction,
        m_redoAction,
        m_prevShapeAction,
        m_nextShapeAction,
        m_deleteAction,
        m_deleteAllShapesAction,
        m_copyAction,
        m_copyShapesAction,
        m_pasteShapesAction,
        m_insertPolygonPointAction,
        m_addPointToEdgeAction,
        m_removePolygonPointAction,
        m_removeSelectedPointAction,
        m_copyPreviousAction,
        m_deleteImageAction,
        m_deleteAnnotationAction,
        m_createModeAction,
        m_createPolygonModeAction,
        m_createPointModeAction,
        m_createPointsModeAction,
        m_createAiPointsModeAction,
        m_createAiBoxModeAction,
        m_createLineModeAction,
        m_createLinestripModeAction,
        m_createCircleModeAction,
        m_createOrientedRectangleModeAction,
        m_createMaskModeAction,
        m_maskEditAction,
        m_editModeAction,
        m_viewModeAction,
        m_editabilityAction,
        m_advancedModeAction,
        m_hideAllAction,
        m_showAllAction,
        m_toggleAllAction,
        m_zoomInAction,
        m_zoomOutAction,
        m_zoomOriginalAction,
        m_fitWindowAction,
        m_fitWidthAction,
        m_brightenAction,
        m_darkenAction,
        m_brightnessOriginalAction,
        m_brightnessContrastAction,
        m_keepPreviousBrightnessContrastAction,
        m_drawSquareAction,
        m_fillDrawingAction,
        m_boxLineColorAction,
        m_shapeLineColorAction,
        m_shapeFillColorAction,
        m_infoAction,
        m_shortcutsAction,
        m_tutorialAction,
        m_settingsAction,
        m_autoSaveAction,
        m_keepPreviousAction,
        m_keepPreviousZoomAction,
        m_singleClassAction,
        m_displayLabelsAction,
        m_embedImageDataAction,
        m_editLabelFlagsAction,
        m_miniMapAction,
        m_showPerformanceAction,
        m_samplingModeAction,
        m_thumbnailModeAction,
    };
    for (QAction *action : actions) {
        if (action) {
            action->setToolTip(action->text());
        }
    }
}

void MainWindow::assignActionIcons() {
    setToolbarIcon(m_openAction, QStringLiteral("toolbar/open.svg"));
    setToolbarIcon(m_openDirAction, QStringLiteral("toolbar/open-dir.svg"));
    setToolbarIcon(m_openAnnotationAction, QStringLiteral("toolbar/open-dir.svg"));
    setToolbarIcon(m_openWithImageViewerAction, QStringLiteral("toolbar/open-dir.svg"));
    setToolbarIcon(m_openFileLocationAction, QStringLiteral("toolbar/open-dir.svg"));
    setToolbarIcon(m_closeAction, QStringLiteral("close.png"));
    setToolbarIcon(m_resetAllAction, QStringLiteral("resetall.png"));
    setToolbarIcon(m_resetLayoutAction, QStringLiteral("toolbar/fit-window.svg"));
    setToolbarIcon(m_saveAction, QStringLiteral("toolbar/save.svg"));
    setToolbarIcon(m_saveAsAction, QStringLiteral("toolbar/save.svg"));
    setToolbarIcon(m_changeSaveDirAction, QStringLiteral("toolbar/save-dir.svg"));
    setToolbarIcon(m_nextAction, QStringLiteral("toolbar/next.svg"));
    setToolbarIcon(m_prevAction, QStringLiteral("toolbar/prev.svg"));
    setToolbarIcon(m_verifyAction, QStringLiteral("toolbar/verify.svg"));
    setToolbarIcon(m_editLabelAction, QStringLiteral("toolbar/edit-label.svg"));
    setToolbarIcon(m_undoAction, QStringLiteral("toolbar/prev.svg"));
    setToolbarIcon(m_undoLastPointAction, QStringLiteral("toolbar/prev.svg"));
    setToolbarIcon(m_redoAction, QStringLiteral("toolbar/next.svg"));
    setToolbarIcon(m_prevShapeAction, QStringLiteral("toolbar/prev.svg"));
    setToolbarIcon(m_nextShapeAction, QStringLiteral("toolbar/next.svg"));
    setToolbarIcon(m_deleteAction, QStringLiteral("toolbar/delete.svg"));
    setToolbarIcon(m_deleteAllShapesAction, QStringLiteral("toolbar/delete.svg"));
    setToolbarIcon(m_copyAction, QStringLiteral("toolbar/copy.svg"));
    setToolbarIcon(m_copyShapesAction, QStringLiteral("toolbar/copy.svg"));
    setToolbarIcon(m_pasteShapesAction, QStringLiteral("toolbar/copy.svg"));
    setToolbarIcon(m_copyHereAction, QStringLiteral("toolbar/copy.svg"));
    setToolbarIcon(m_moveHereAction, QStringLiteral("toolbar/edit-box.svg"));
    setToolbarIcon(m_addPointToEdgeAction, QStringLiteral("toolbar/create-box.svg"));
    setToolbarIcon(m_copyPreviousAction, QStringLiteral("toolbar/copy.svg"));
    setToolbarIcon(m_deleteImageAction, QStringLiteral("toolbar/delete.svg"));
    setToolbarIcon(m_deleteAnnotationAction, QStringLiteral("toolbar/delete.svg"));
    setToolbarIcon(m_removeSelectedPointAction, QStringLiteral("toolbar/delete.svg"));
    setToolbarIcon(m_createModeAction, QStringLiteral("toolbar/create-box.svg"));
    setToolbarIcon(m_createPolygonModeAction, QStringLiteral("toolbar/create-box.svg"));
    setToolbarIcon(m_createPointModeAction, QStringLiteral("toolbar/create-box.svg"));
    setToolbarIcon(m_createPointsModeAction, QStringLiteral("toolbar/create-box.svg"));
    setToolbarIcon(m_createAiPointsModeAction, QStringLiteral("toolbar/create-box.svg"));
    setToolbarIcon(m_createAiBoxModeAction, QStringLiteral("toolbar/create-box.svg"));
    setToolbarIcon(m_createLineModeAction, QStringLiteral("toolbar/create-box.svg"));
    setToolbarIcon(m_createLinestripModeAction, QStringLiteral("toolbar/create-box.svg"));
    setToolbarIcon(m_createCircleModeAction, QStringLiteral("toolbar/create-box.svg"));
    setToolbarIcon(m_createOrientedRectangleModeAction, QStringLiteral("toolbar/create-box.svg"));
    setToolbarIcon(m_createMaskModeAction, QStringLiteral("toolbar/create-box.svg"));
    setToolbarIcon(m_maskEditAction, QStringLiteral("toolbar/edit-box.svg"));
    setToolbarIcon(m_editModeAction, QStringLiteral("toolbar/edit-box.svg"));
    setToolbarIcon(m_viewModeAction, QStringLiteral("toolbar/show-all.svg"));
    setToolbarIcon(m_editabilityAction, QStringLiteral("toolbar/edit-box.svg"));
    setToolbarIcon(m_advancedModeAction, QStringLiteral("toolbar/fit-window.svg"));
    setToolbarIcon(m_hideAllAction, QStringLiteral("toolbar/hide-all.svg"));
    setToolbarIcon(m_showAllAction, QStringLiteral("toolbar/show-all.svg"));
    setToolbarIcon(m_toggleAllAction, QStringLiteral("toolbar/show-all.svg"));
    setToolbarIcon(m_zoomInAction, QStringLiteral("toolbar/zoom-in.svg"));
    setToolbarIcon(m_zoomOutAction, QStringLiteral("toolbar/zoom-out.svg"));
    setToolbarIcon(m_zoomOriginalAction, QStringLiteral("zoom.png"));
    setToolbarIcon(m_fitWindowAction, QStringLiteral("toolbar/fit-window.svg"));
    setToolbarIcon(m_fitWidthAction, QStringLiteral("toolbar/fit-width.svg"));
    setToolbarIcon(m_brightenAction, QStringLiteral("toolbar/brighten.svg"));
    setToolbarIcon(m_darkenAction, QStringLiteral("toolbar/darken.svg"));
    setToolbarIcon(m_brightnessOriginalAction, QStringLiteral("toolbar/brightness-reset.svg"));
    setToolbarIcon(m_brightnessContrastAction, QStringLiteral("toolbar/brightness-reset.svg"));
    setToolbarIcon(m_keepPreviousBrightnessContrastAction, QStringLiteral("toolbar/copy.svg"));
    setToolbarIcon(m_drawSquareAction, QStringLiteral("toolbar/create-box.svg"));
    setToolbarIcon(m_fillDrawingAction, QStringLiteral("color.png"));
    setToolbarIcon(m_boxLineColorAction, QStringLiteral("color_line.png"));
    setToolbarIcon(m_shapeLineColorAction, QStringLiteral("color_line.png"));
    setToolbarIcon(m_shapeFillColorAction, QStringLiteral("color.png"));
    setToolbarIcon(m_infoAction, QStringLiteral("help.png"));
    setToolbarIcon(m_shortcutsAction, QStringLiteral("toolbar/edit-label.svg"));
    setToolbarIcon(m_settingsAction, QStringLiteral("toolbar/edit-box.svg"));
    setToolbarIcon(m_autoSaveAction, QStringLiteral("toolbar/save.svg"));
    setToolbarIcon(m_keepPreviousAction, QStringLiteral("toolbar/copy.svg"));
    setToolbarIcon(m_keepPreviousZoomAction, QStringLiteral("toolbar/fit-window.svg"));
    setToolbarIcon(m_singleClassAction, QStringLiteral("toolbar/edit-label.svg"));
    setToolbarIcon(m_displayLabelsAction, QStringLiteral("toolbar/show-all.svg"));
    setToolbarIcon(m_miniMapAction, QStringLiteral("toolbar/fit-window.svg"));
    setToolbarIcon(m_showPerformanceAction, QStringLiteral("zoom.png"));
    setToolbarIcon(m_samplingModeAction, QStringLiteral("toolbar/fit-window.svg"));
    setToolbarIcon(m_thumbnailModeAction, QStringLiteral("toolbar/show-all.svg"));

    switch (m_format) {
    case SaveFormat::PascalVoc:
        setToolbarIcon(m_formatAction, QStringLiteral("toolbar/format-voc.svg"));
        break;
    case SaveFormat::Yolo:
        setToolbarIcon(m_formatAction, QStringLiteral("toolbar/format-yolo.svg"));
        break;
    case SaveFormat::CreateMl:
        setToolbarIcon(m_formatAction, QStringLiteral("toolbar/format-createml.svg"));
        break;
    case SaveFormat::LabelMe:
        setToolbarIcon(m_formatAction, QStringLiteral("toolbar/format-createml.svg"));
        break;
    }
}

void MainWindow::refreshActions() {
    bool hasImage = !m_filePath.isEmpty();
    const bool editingAllowed = !m_editabilityAction || m_editabilityAction->isChecked();
    const bool drawing = m_canvas->isDrawing();
    // Canvas owns the editing selection. The label list can retain a visual
    // current row while it is being synchronized or filtered, so it must not
    // enable destructive/editing actions by itself.
    const bool hasSelection = m_canvas->hasSelection();
    m_saveAction->setEnabled(hasImage && m_dirty && !m_annotationLoadFailed);
    m_saveAsAction->setEnabled(hasImage);
    m_openAnnotationAction->setEnabled(hasImage);
    m_openWithImageViewerAction->setEnabled(hasImage);
    m_openFileLocationAction->setEnabled(hasImage);
    if (m_openWithButton) {
        m_openWithButton->setEnabled(hasImage);
    }
    updateFileContextActions();
    m_closeAction->setEnabled(hasImage);
    m_changeSaveDirAction->setEnabled(true);
    m_verifyAction->setEnabled(hasImage && !drawing);
    m_undoAction->setEnabled(!drawing && !m_undoStack.isEmpty());
    m_undoLastPointAction->setEnabled(drawing && editingAllowed);
    m_redoAction->setEnabled(!m_redoStack.isEmpty());
    m_prevShapeAction->setEnabled(hasImage);
    m_nextShapeAction->setEnabled(hasImage);
    m_nextCopyAction->setEnabled(hasImage && m_currentImageIndex + 1 < m_imageList.size());
    m_prevCopyAction->setEnabled(hasImage && m_currentImageIndex > 0);
    m_deleteImageAction->setEnabled(hasImage);
    const QString activeAnnotationPath = (usesAnnotationPathOverride())
                                             ? m_annotationPathOverride
                                             : (hasImage ? annotationPathForImage(m_filePath) : QString());
    m_deleteAnnotationAction->setEnabled(hasImage && QFileInfo::exists(activeAnnotationPath));
    m_copyPreviousAction->setEnabled(hasImage && m_currentImageIndex > 0);
    const QList<QAction *> createActions = {
        m_createModeAction,
        m_createPolygonModeAction,
        m_createPointModeAction,
        m_createPointsModeAction,
        m_createAiPointsModeAction,
        m_createAiBoxModeAction,
        m_createLineModeAction,
        m_createLinestripModeAction,
        m_createCircleModeAction,
        m_createOrientedRectangleModeAction,
        m_createMaskModeAction};
    QAction *activeCreateAction = nullptr;
    for (QAction *action : createActions) {
        if (action && action->isChecked()) {
            activeCreateAction = action;
            break;
        }
    }
    for (QAction *action : createActions) {
        if (action) {
            action->setEnabled(hasImage && editingAllowed && action != activeCreateAction);
        }
    }
    const bool maskSelected = m_canvas->hasSelection() &&
                              m_canvas->currentIndex() < m_canvas->shapes().size() &&
                              m_canvas->shapes()[m_canvas->currentIndex()].shapeType == QStringLiteral("mask");
    m_maskEditAction->setEnabled(maskSelected && editingAllowed && !drawing);
    if (!maskSelected || !editingAllowed || drawing) {
        QSignalBlocker blocker(m_maskEditAction);
        m_maskEditAction->setChecked(false);
        m_canvas->setMaskEditing(false);
    }
    m_editModeAction->setEnabled(hasImage && editingAllowed && !drawing);
    m_viewModeAction->setEnabled(hasImage);
    m_advancedModeAction->setEnabled(true);
    m_editLabelAction->setEnabled(hasSelection && editingAllowed && !drawing);
    m_deleteAction->setEnabled(hasSelection && editingAllowed && !drawing);
    m_deleteAllShapesAction->setEnabled(hasImage && !m_canvas->shapes().isEmpty() &&
                                        editingAllowed && !drawing);
    m_addPointToEdgeAction->setEnabled(m_canvas->canAddPointToEdge() && editingAllowed && !drawing);
    m_removeSelectedPointAction->setEnabled(m_canvas->canRemoveSelectedPoint() && editingAllowed && !drawing);
    m_copyAction->setEnabled(hasSelection && editingAllowed && !drawing);
    m_copyShapesAction->setEnabled(hasSelection && editingAllowed && !drawing);
    const bool hasClipboardShapes = !m_shapeClipboard.isEmpty() ||
                                    !shapesFromClipboardMime(QApplication::clipboard()->mimeData()).isEmpty();
    m_pasteShapesAction->setEnabled(hasImage && editingAllowed && !drawing && hasClipboardShapes);
    m_copyHereAction->setEnabled(hasSelection && editingAllowed && !drawing);
    m_moveHereAction->setEnabled(hasSelection && editingAllowed && !drawing);
    m_hideAllAction->setEnabled(hasImage);
    m_showAllAction->setEnabled(hasImage);
    m_toggleAllAction->setEnabled(hasImage);
    m_zoomInAction->setEnabled(hasImage);
    m_zoomOutAction->setEnabled(hasImage);
    m_zoomOriginalAction->setEnabled(hasImage);
    if (m_zoomWidget) m_zoomWidget->setEnabled(hasImage);
    m_fitWindowAction->setEnabled(hasImage);
    m_fitWidthAction->setEnabled(hasImage);
    m_brightenAction->setEnabled(hasImage);
    m_darkenAction->setEnabled(hasImage);
    m_brightnessOriginalAction->setEnabled(hasImage);
    m_brightnessContrastAction->setEnabled(hasImage);
    m_keepPreviousBrightnessContrastAction->setEnabled(true);
    m_fillDrawingAction->setEnabled(true);
    m_keepPreviousZoomAction->setEnabled(true);
    m_miniMapAction->setEnabled(true);
    m_showPerformanceAction->setEnabled(true);
    m_samplingModeAction->setEnabled(true);
    m_thumbnailModeAction->setEnabled(true);
    m_boxLineColorAction->setEnabled(true);
    m_shapeLineColorAction->setEnabled(hasSelection && editingAllowed && !drawing);
    m_shapeFillColorAction->setEnabled(hasSelection && editingAllowed && !drawing);
    const bool textPromptMode = hasImage && editingAllowed && isAiTextCreateMode(m_canvas->createShapeType());
    m_aiTextPromptEdit->setEnabled(textPromptMode && !m_aiTextPromptRunning);
    m_aiTextModelCombo->setEnabled(textPromptMode && !m_aiTextPromptRunning);
    m_aiTextScoreSpin->setEnabled(textPromptMode && !m_aiTextPromptRunning);
    m_aiTextIouSpin->setEnabled(textPromptMode && !m_aiTextPromptRunning);
    m_aiTextRunButton->setEnabled(textPromptMode && !m_aiTextPromptRunning &&
                                  !m_aiTextPromptEdit->text().trimmed().isEmpty() && !m_aiRequestRunning);
}

void MainWindow::populateToolbarForMode() {
    if (!m_toolBar) return;
    m_toolBar->clear();

    // Keep the primary actions in the same scan order as LabelMe.  The
    // corresponding menu actions remain available for keyboard users and
    // for commands that do not fit in the compact title-bar toolbar.
    m_toolBar->addActions({m_openAction, m_openDirAction, m_prevAction, m_nextAction,
                           m_saveAction, m_deleteAnnotationAction});
    m_toolBar->addSeparator();
    m_toolBar->addWidget(m_openWithButton);
    m_toolBar->addWidget(m_mainModeButton);
    m_toolBar->addActions({m_editLabelAction, m_copyAction, m_deleteAction,
                           m_undoAction, m_redoAction});
    m_toolBar->addSeparator();
    m_toolBar->addActions({m_brightnessContrastAction, m_fitWindowAction, m_fitWidthAction,
                           m_zoomInAction, m_zoomOutAction});
    m_toolBar->addWidget(m_zoomWidget);
    m_toolBar->addSeparator();
    m_toolBar->addActions({m_verifyAction, m_autoSaveAction});
    m_toolBar->addSeparator();
    if (m_advancedModeAction && m_advancedModeAction->isChecked()) {
        m_toolBar->addActions({m_hideAllAction, m_showAllAction});
    }
    m_toolBar->addActions({m_samplingModeAction, m_thumbnailModeAction});
}

void MainWindow::rebuildRecentFilesMenu() {
    if (!m_recentFilesMenu) return;
    m_recentFilesMenu->clear();
    QStringList existing;
    for (const QString &path : m_recentFiles) {
        if (QFileInfo::exists(path) && !existing.contains(path)) {
            existing.append(path);
        }
    }
    m_recentFiles = existing.mid(0, 7);
    for (int i = 0; i < m_recentFiles.size(); ++i) {
        const QString path = m_recentFiles[i];
        QAction *action = m_recentFilesMenu->addAction(QString("&%1 %2").arg(i + 1).arg(QFileInfo(path).fileName()));
        action->setToolTip(path);
        connect(action, &QAction::triggered, this, [this, path]() { loadRecentFile(path); });
    }
    m_recentFilesMenu->setEnabled(!m_recentFiles.isEmpty());
}

void MainWindow::addRecentFile(const QString &path) {
    QString absolute = QFileInfo(path).absoluteFilePath();
    m_recentFiles.removeAll(absolute);
    m_recentFiles.prepend(absolute);
    while (m_recentFiles.size() > 7) {
        m_recentFiles.removeLast();
    }
    m_settings.setValue("recentFiles", m_recentFiles);
    rebuildRecentFilesMenu();
}

void MainWindow::rebuildRecentDirsMenu() {
    if (!m_recentDirsMenu) return;
    m_recentDirsMenu->clear();
    QStringList existing;
    for (const QString &path : m_recentDirs) {
        if (QFileInfo(path).isDir() && !existing.contains(path)) {
            existing.append(path);
        }
    }
    m_recentDirs = existing.mid(0, 10);
    for (int i = 0; i < m_recentDirs.size(); ++i) {
        const QString path = m_recentDirs[i];
        QAction *action = m_recentDirsMenu->addAction(QString("&%1 %2").arg(i + 1).arg(QDir::toNativeSeparators(path)));
        action->setToolTip(path);
        action->setData(path);
        connect(action, &QAction::triggered, this, [this, path]() { loadRecentDir(path); });
    }
    m_recentDirsMenu->setEnabled(!m_recentDirs.isEmpty());
}

void MainWindow::addRecentDir(const QString &path) {
    QString absolute = QFileInfo(path).absoluteFilePath();
    if (!QFileInfo(absolute).isDir()) {
        return;
    }
    m_recentDirs.removeAll(absolute);
    m_recentDirs.prepend(absolute);
    while (m_recentDirs.size() > 10) {
        m_recentDirs.removeLast();
    }
    m_settings.setValue("recentDirs", m_recentDirs);
    rebuildRecentDirsMenu();
}

void MainWindow::loadRecentFile(const QString &path) {
    if (QFileInfo::exists(path)) {
        openPath(path);
    } else {
        m_recentFiles.removeAll(path);
        rebuildRecentFilesMenu();
    }
}

void MainWindow::loadRecentDir(const QString &path) {
    if (!QFileInfo(path).isDir()) {
        m_recentDirs.removeAll(path);
        rebuildRecentDirsMenu();
        return;
    }
    if (!maybeSave()) return;
    m_settings.setValue("lastOpenDir", path);
    m_dirPath = QFileInfo(path).absoluteFilePath();
    m_imageList = scanImages(m_dirPath);
    m_currentImageIndex = 0;
    populateFileList();
    addRecentDir(m_dirPath);
    const QString preferred = preferredImageForCurrentDir();
    if (!preferred.isEmpty()) {
        loadImage(preferred);
    }
}

QString MainWindow::preferredImageForCurrentDir() const {
    if (m_imageList.isEmpty()) {
        return {};
    }
    const QString dirPath = QFileInfo(m_dirPath).absoluteFilePath();
    const QString remembered = m_lastFileByDir.value(dirPath);
    if (!remembered.isEmpty() && m_imageList.contains(remembered) && QFileInfo::exists(remembered)) {
        return remembered;
    }
    return m_imageList.first();
}

void MainWindow::setFileThumbnailMode(bool enabled) {
    m_settings.setValue("view/fileThumbnails", enabled);
    populateFileList();
}

void MainWindow::setFormat(SaveFormat format) {
    m_format = format;
    m_formatAction->setText(currentFormatName());
    assignActionIcons();
    refreshActionToolTips();
    syncFormatFooterControls();
    syncTopLevelFlagsEditor();
    m_settings.setValue("labelFileFormat", static_cast<int>(m_format));
}

void MainWindow::setCanvasImage(const QImage &image, const QSize &sourceSize) {
    if (!m_canvas) {
        return;
    }
    const QSize effectiveSourceSize = sourceSize.isValid() && !sourceSize.isEmpty()
                                          ? sourceSize
                                          : image.size();
    if (!image.isNull() && effectiveSourceSize.isValid() &&
        effectiveSourceSize != image.size()) {
        m_canvas->setPreviewPixmap(QPixmap::fromImage(image), effectiveSourceSize);
    } else {
        m_canvas->setPixmap(QPixmap::fromImage(image));
    }
}

void MainWindow::upgradePreviewImage() {
    if (m_upgradingPreviewImage || !m_canvas || !m_canvas->isPreviewImage() ||
        m_filePath.isEmpty()) {
        return;
    }

    const QString imagePath = m_filePath;
    m_upgradingPreviewImage = true;
    statusBar()->showMessage(m_strings.get(QStringLiteral("loadingHighResolution")));

    const auto result = std::make_shared<QImage>();
    QThread *thread = QThread::create([imagePath, result]() {
        *result = readImageWithAutoTransform(imagePath);
    });
    thread->setParent(this);
    m_previewUpgradeThread = thread;
    connect(thread, &QThread::finished, this, [this, thread, result, imagePath]() {
        if (m_previewUpgradeThread == thread) {
            m_previewUpgradeThread = nullptr;
        }
        thread->deleteLater();

        if (m_filePath != imagePath || !m_canvas) {
            m_upgradingPreviewImage = false;
            return;
        }

        const QSize expectedSize = m_canvas->pixmapSize();
        if (result->isNull() ||
            (expectedSize.isValid() && !expectedSize.isEmpty() && result->size() != expectedSize)) {
            m_upgradingPreviewImage = false;
            statusBar()->showMessage(m_strings.get(QStringLiteral("highResolutionFailed")), 5000);
            return;
        }

        const QVector<int> selectedIndices = m_canvas->selectedIndices();
        const int brightness = m_canvas->brightness();
        const int contrast = m_canvas->contrast();
        const int horizontal = m_scrollArea ? m_scrollArea->horizontalScrollBar()->value() : 0;
        const int vertical = m_scrollArea ? m_scrollArea->verticalScrollBar()->value() : 0;
        m_canvas->setPixmap(QPixmap::fromImage(*result));
        m_canvas->setSelectedIndices(selectedIndices);
        m_canvas->setBrightness(brightness);
        m_canvas->setContrast(contrast);
        m_upgradingPreviewImage = false;

        QTimer::singleShot(0, this, [this, imagePath, horizontal, vertical]() {
            if (m_filePath != imagePath || !m_scrollArea) {
                return;
            }
            m_scrollArea->horizontalScrollBar()->setValue(horizontal);
            m_scrollArea->verticalScrollBar()->setValue(vertical);
            if (m_miniMapOverlay) {
                m_miniMapOverlay->refreshGeometry();
            }
            statusBar()->showMessage(m_strings.get(QStringLiteral("highResolutionLoaded")), 2500);
            updatePerformanceLabel();
        });
    });
    thread->start();
}

void MainWindow::stopPreviewUpgradeThread() {
    if (!m_previewUpgradeThread) {
        m_upgradingPreviewImage = false;
        return;
    }
    if (m_previewUpgradeThread->isRunning()) {
        m_previewUpgradeThread->requestInterruption();
        m_previewUpgradeThread->quit();
        m_previewUpgradeThread->wait();
    }
    m_previewUpgradeThread = nullptr;
    m_upgradingPreviewImage = false;
}

void MainWindow::updateFitScale() {
    QSize pixmapSize = m_canvas->pixmapSize();
    if (pixmapSize.isEmpty() || m_fitMode == FitMode::Manual) {
        syncZoomWidget();
        return;
    }
    QSize viewport = m_scrollArea->viewport()->size();
    if (m_fitMode == FitMode::Width) {
        m_canvas->setScale(qMax(0.05, (viewport.width() - 2.0) / pixmapSize.width()));
        syncZoomWidget();
        return;
    }
    const double widthScale = (viewport.width() - 2.0) / pixmapSize.width();
    const double heightScale = (viewport.height() - 2.0) / pixmapSize.height();
    m_canvas->setScale(qMax(0.05, qMin(widthScale, heightScale)));
    syncZoomWidget();
}

void MainWindow::syncZoomWidget() {
    if (!m_zoomWidget || !m_canvas) {
        return;
    }
    QSignalBlocker blocker(m_zoomWidget);
    m_zoomWidget->setValue(qBound(m_zoomWidget->minimum(),
                                  qRound(m_canvas->scale() * 100.0),
                                  m_zoomWidget->maximum()));
}

QPoint MainWindow::zoomAnchorPosition() const {
    if (!m_canvas || !m_scrollArea || !m_scrollArea->viewport()) {
        return {};
    }
    return m_canvas->mapFrom(m_scrollArea->viewport(),
                             m_scrollArea->viewport()->rect().center());
}

void MainWindow::rememberScrollPosition(const QString &imagePath) {
    if (imagePath.isEmpty() || !m_scrollArea) {
        return;
    }
    m_horizontalScrollByFile.insert(imagePath, m_scrollArea->horizontalScrollBar()->value());
    m_verticalScrollByFile.insert(imagePath, m_scrollArea->verticalScrollBar()->value());
}

void MainWindow::restoreScrollPosition(const QString &imagePath) {
    if (imagePath.isEmpty() || !m_scrollArea ||
        (!m_horizontalScrollByFile.contains(imagePath) && !m_verticalScrollByFile.contains(imagePath))) {
        return;
    }
    const int horizontal = m_horizontalScrollByFile.value(imagePath, 0);
    const int vertical = m_verticalScrollByFile.value(imagePath, 0);
    QTimer::singleShot(0, this, [this, imagePath, horizontal, vertical]() {
        if (!m_scrollArea || m_filePath != imagePath) {
            return;
        }
        m_scrollArea->horizontalScrollBar()->setValue(horizontal);
        m_scrollArea->verticalScrollBar()->setValue(vertical);
        if (m_miniMapOverlay) {
            m_miniMapOverlay->refreshGeometry();
        }
    });
}

void MainWindow::openFile() {
    const QStringList imageFilters = supportedImageNameFilters();
    const QString imagePattern = imageFilters.join(' ');
    const QString filter = QStringLiteral("Image & Label files (%1 *.json);;Images (%1);;LabelMe (*.json);;All Files (*)")
                               .arg(imagePattern);
    const QString path = QFileDialog::getOpenFileName(this, m_strings.get("openFile"),
                                                      m_settings.value("lastOpenDir").toString(), filter);
    if (path.isEmpty()) {
        return;
    }
    if (!openPath(path)) {
        QMessageBox::warning(this, QStringLiteral("labelImgCpp"),
                             m_strings.get(QStringLiteral("cannotOpen")).arg(path));
    }
}

bool MainWindow::openPath(const QString &path) {
    const QFileInfo info(path);
    if (!info.isFile()) {
        return false;
    }
    if (info.suffix().compare(QStringLiteral("json"), Qt::CaseInsensitive) == 0) {
        return loadStandaloneLabelMe(info.absoluteFilePath());
    }
    const QString imagePath = info.absoluteFilePath();
    const QString imageDir = info.absolutePath();
    if (!loadImage(imagePath)) {
        return false;
    }
    m_dirPath = imageDir;
    m_imageList = scanImages(imageDir);
    if (!m_imageList.contains(imagePath)) {
        m_imageList.append(imagePath);
        std::sort(m_imageList.begin(), m_imageList.end(), naturalPathLess);
    }
    m_currentImageIndex = m_imageList.indexOf(imagePath);
    if (m_fileDock) {
        m_fileDock->setEnabled(true);
    }
    populateFileList();
    return true;
}

void MainWindow::openDir() {
    QString dir = QFileDialog::getExistingDirectory(this, m_strings.get("openDir"), m_settings.value("lastOpenDir").toString());
    if (dir.isEmpty()) return;
    m_settings.setValue("lastOpenDir", dir);
    m_dirPath = dir;
    m_imageList = scanImages(dir);
    m_currentImageIndex = 0;
    populateFileList();
    addRecentDir(m_dirPath);
    const QString preferred = preferredImageForCurrentDir();
    if (!preferred.isEmpty()) loadImage(preferred);
}

void MainWindow::openAnnotationDialog() {
    if (m_filePath.isEmpty()) {
        statusBar()->showMessage(m_strings.get(QStringLiteral("pleaseSelectImage")), 5000);
        return;
    }
    QString filter;
    if (m_format == SaveFormat::PascalVoc) filter = "Pascal VOC (*.xml)";
    else if (m_format == SaveFormat::Yolo) filter = "YOLO (*.txt)";
    else if (m_format == SaveFormat::CreateMl) filter = "CreateML (*.json)";
    else filter = "LabelMe (*.json)";
    QString path = QFileDialog::getOpenFileName(this, m_strings.get("openAnnotation"),
                                                QFileInfo(m_filePath).absolutePath(),
                                                filter + ";;All supported (*.xml *.txt *.json)");
    if (!path.isEmpty() && !openAnnotation(path)) {
        QMessageBox::warning(this, QStringLiteral("labelImgCpp"),
                             m_strings.get(QStringLiteral("cannotOpenAnnotation")).arg(path));
    }
}

bool MainWindow::openAnnotation(const QString &path) {
    return loadAnnotation(path);
}

void MainWindow::openCurrentImageWithViewer() {
    if (m_filePath.isEmpty()) {
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_filePath));
}

void MainWindow::revealCurrentImageInFolder() {
    if (m_filePath.isEmpty()) {
        return;
    }
#ifdef Q_OS_WIN
    QProcess::startDetached(QStringLiteral("explorer.exe"),
                            {QStringLiteral("/select,%1").arg(QDir::toNativeSeparators(m_filePath))});
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(m_filePath).absolutePath()));
#endif
}

void MainWindow::openNextImage() {
    if (m_imageList.isEmpty()) return;
    if (m_currentImageIndex + 1 < m_imageList.size()) ++m_currentImageIndex;
    loadImage(m_imageList[m_currentImageIndex]);
}

void MainWindow::openPrevImage() {
    if (m_imageList.isEmpty()) return;
    if (m_currentImageIndex > 0) --m_currentImageIndex;
    loadImage(m_imageList[m_currentImageIndex]);
}

void MainWindow::closeFile() {
    if (!maybeSave()) return;
    m_filePath.clear();
    m_annotationPathOverride.clear();
    m_hasAnnotationPathOverride = false;
    m_annotationLoadFailed = false;
    m_labelMeImageData.clear();
    m_labelMeImagePath.clear();
    m_labelMeVersion.clear();
    m_labelMeTopLevelFlags.clear();
    m_labelMeOtherData = QJsonObject();
    m_verified = false;
    m_canvas->setPixmap(QPixmap());
    m_canvas->setShapes({});
    m_canvas->setEnabled(false);
    m_labelList->clear();
    m_filterCombo->clear();
    m_coordinates->clear();
    m_fileList->clearSelection();
    syncTopLevelFlagsEditor();
    setDirty(false);
    refreshActions();
}

void MainWindow::resetAllSettings() {
    if (QMessageBox::question(this, "labelImgCpp", m_strings.get("resetAll"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }
    m_settings.clear();
    saveSettings();
    QProcess::startDetached(QCoreApplication::applicationFilePath(), {});
    close();
}

void MainWindow::resetLayout() {
    m_settings.remove(QStringLiteral("window/state"));
    if (!m_defaultDockState.isEmpty()) {
        restoreState(m_defaultDockState, DockStateVersion);
    }
    statusBar()->showMessage(m_strings.get(QStringLiteral("layoutRestored")), 2000);
}

bool MainWindow::saveCurrentFile() {
    if (m_filePath.isEmpty()) return false;
    if (m_annotationLoadFailed) {
        statusBar()->showMessage(m_strings.get(QStringLiteral("saveBlockedAfterLoadFailure"))
                                     .arg(QFileInfo(m_filePath).fileName()),
                                 8000);
        return false;
    }
    QString path = (usesAnnotationPathOverride())
                       ? m_annotationPathOverride
                       : annotationPathForImage(m_filePath);
    AnnotationDocument doc = currentDocument();
    bool ok = false;
    if (m_format == SaveFormat::PascalVoc) ok = AnnotationIO::savePascalVoc(path, doc);
    if (m_format == SaveFormat::Yolo) ok = AnnotationIO::saveYolo(path, doc, m_classList);
    if (m_format == SaveFormat::CreateMl) ok = AnnotationIO::saveCreateMl(path, doc);
    if (m_format == SaveFormat::LabelMe) ok = AnnotationIO::saveLabelMe(path, doc);
    if (ok) {
        setDirty(false);
        if (!m_imageList.isEmpty()) {
            populateFileList();
        }
    } else {
        statusBar()->showMessage(m_strings.get(QStringLiteral("failedSave")).arg(path), 8000);
    }
    return ok;
}

void MainWindow::saveFile() {
    saveCurrentFile();
}

void MainWindow::saveFileAs() {
    QString filter;
    QString defaultSuffix;
    if (m_format == SaveFormat::PascalVoc) {
        filter = QStringLiteral("Pascal VOC (*.xml)");
        defaultSuffix = QStringLiteral("xml");
    } else if (m_format == SaveFormat::Yolo) {
        filter = QStringLiteral("YOLO (*.txt)");
        defaultSuffix = QStringLiteral("txt");
    } else if (m_format == SaveFormat::CreateMl) {
        filter = QStringLiteral("CreateML (*.json)");
        defaultSuffix = QStringLiteral("json");
    } else {
        filter = QStringLiteral("LabelMe (*.json)");
        defaultSuffix = QStringLiteral("json");
    }

    const QString defaultDirectory = m_saveDir.isEmpty()
                                         ? (m_filePath.isEmpty()
                                                ? m_settings.value(QStringLiteral("lastOpenDir")).toString()
                                                : QFileInfo(m_filePath).absolutePath())
                                         : m_saveDir;
    QFileDialog dialog(this, m_strings.get("saveAs"), defaultDirectory, filter);
    dialog.setObjectName(QStringLiteral("saveAsDialog"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDefaultSuffix(defaultSuffix);
    dialog.setOption(QFileDialog::DontConfirmOverwrite, false);
    if (!m_filePath.isEmpty()) {
        dialog.selectFile(QFileInfo(m_filePath).completeBaseName());
    }
    if (dialog.exec() != QDialog::Accepted || dialog.selectedFiles().isEmpty()) {
        return;
    }
    const QString path = dialog.selectedFiles().first();
    AnnotationDocument doc = currentDocument();
    if (m_format == SaveFormat::LabelMe && !m_filePath.isEmpty()) {
        // Rebase imagePath against the new JSON location instead of retaining
        // a path that was relative to the previous annotation file.
        doc.imagePath = m_filePath;
    }
    bool ok = false;
    if (m_format == SaveFormat::PascalVoc) ok = AnnotationIO::savePascalVoc(path, doc);
    if (m_format == SaveFormat::Yolo) ok = AnnotationIO::saveYolo(path, doc, m_classList);
    if (m_format == SaveFormat::CreateMl) ok = AnnotationIO::saveCreateMl(path, doc);
    if (m_format == SaveFormat::LabelMe) ok = AnnotationIO::saveLabelMe(path, doc);
    if (ok) {
        m_annotationPathOverride = path;
        m_annotationOverrideFormat = m_format;
        m_hasAnnotationPathOverride = true;
        m_annotationLoadFailed = false;
        setDirty(false);
        if (!m_imageList.isEmpty()) {
            populateFileList();
        }
    } else {
        statusBar()->showMessage(m_strings.get(QStringLiteral("failedSave")).arg(path), 8000);
    }
}

void MainWindow::changeSaveDir() {
    QString dir = QFileDialog::getExistingDirectory(this, m_strings.get("changeSaveDir"), m_saveDir);
    if (!dir.isEmpty()) {
        m_saveDir = dir;
        m_settings.setValue("savedir", m_saveDir);
    }
}

void MainWindow::changeFormat() {
    if (m_format == SaveFormat::PascalVoc) setFormat(SaveFormat::Yolo);
    else if (m_format == SaveFormat::Yolo) setFormat(SaveFormat::CreateMl);
    else if (m_format == SaveFormat::CreateMl) setFormat(SaveFormat::LabelMe);
    else setFormat(SaveFormat::PascalVoc);
    setDirty(true);
}

void MainWindow::verifyImage() {
    m_verified = !m_verified;
    m_verifyAction->setChecked(m_verified);
    setDirty(true);
}

bool MainWindow::editCurrentLabel() {
    QVector<Shape> &shapes = m_canvas->shapesRef();
    QVector<int> selected = m_canvas->selectedIndices();
    if (selected.isEmpty()) {
        return false;
    }

    const int activeIndex = selected.first();
    const Shape &firstShape = shapes[activeIndex];
    const QString currentLabel = firstShape.label;
    bool editText = true;
    bool editGroupId = true;
    bool editDescription = true;
    bool editFlags = true;
    for (int index : selected) {
        if (index < 0 || index >= shapes.size()) {
            continue;
        }
        const Shape &shape = shapes[index];
        editText = editText && shape.label == firstShape.label;
        editGroupId = editGroupId && shape.groupId == firstShape.groupId;
        editDescription = editDescription && shape.description == firstShape.description;
        editFlags = editFlags && shape.flags == firstShape.flags;
    }

    QStringList labelChoices = m_classList;
    labelChoices.removeDuplicates();
    if (m_labelMeSortLabels) {
        labelChoices.sort(Qt::CaseInsensitive);
    }
    int currentIndex = labelChoices.indexOf(currentLabel);
    if (currentIndex < 0 && !currentLabel.isEmpty()) {
        labelChoices.prepend(currentLabel);
        currentIndex = 0;
    }

    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("labelEditDialog"));
    dialog.setWindowTitle(m_strings.get("editLabel"));
    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout();
    layout->addLayout(form);

    auto *labelCombo = new QComboBox(&dialog);
    labelCombo->setObjectName(QStringLiteral("labelEditCombo"));
    labelCombo->setEditable(true);
    labelCombo->addItems(labelChoices);
    if (currentIndex >= 0) {
        labelCombo->setCurrentIndex(currentIndex);
    } else {
        labelCombo->setCurrentText(currentLabel);
    }
    if (!editText) {
        labelCombo->setEnabled(false);
        labelCombo->setToolTip(m_strings.get("mixedLabelTooltip"));
    }
    if (m_labelMeShowLabelTextField) {
        form->addRow(m_strings.get("labelDialog"), labelCombo);
    } else {
        labelCombo->hide();
    }

    auto *labelEditList = new QListWidget(&dialog);
    labelEditList->setObjectName(QStringLiteral("labelEditList"));
    labelEditList->setSelectionMode(QAbstractItemView::SingleSelection);
    // LabelMe disables both the editable field and its history list when a
    // multi-selection contains different labels. Leaving the history list
    // active would allow a click to mutate a field that is supposed to be
    // read-only for this mixed selection.
    labelEditList->setEnabled(editText);
    labelEditList->setDragDropMode(m_labelMeSortLabels ? QAbstractItemView::NoDragDrop
                                                        : QAbstractItemView::InternalMove);
    labelEditList->setSortingEnabled(false);
    for (int row = 0; row < labelChoices.size(); ++row) {
        labelEditList->insertItem(row, new QListWidgetItem(labelChoices.at(row)));
    }
    if (m_labelMeSortLabels) {
        labelEditList->sortItems(Qt::AscendingOrder);
    }
    if (m_labelMeFitToContentColumn && labelEditList->count() > 0) {
        labelEditList->setMinimumWidth(labelEditList->sizeHintForColumn(0) + 2);
    }
    if (m_labelMeFitToContentRow && labelEditList->count() > 0) {
        labelEditList->setMinimumHeight(labelEditList->sizeHintForRow(0) * labelEditList->count() + 2);
    } else {
        labelEditList->setFixedHeight(120);
    }
    layout->addWidget(labelEditList);
    connect(labelEditList, &QListWidget::currentRowChanged, &dialog,
            [labelCombo, labelEditList](int row) {
                if (row < 0) {
                    return;
                }
                if (QListWidgetItem *item = labelEditList->item(row)) {
                    labelCombo->setCurrentText(item->text());
                }
    });
    int highlightedRow = currentIndex;
    const QList<QListWidgetItem *> matchingItems =
        labelEditList->findItems(currentLabel, Qt::MatchExactly);
    if (!matchingItems.isEmpty()) {
        highlightedRow = labelEditList->row(matchingItems.first());
    }
    if (highlightedRow >= 0) {
        if (m_labelMeSortLabels) {
            labelEditList->setCurrentRow(highlightedRow);
        } else {
            labelEditList->setDragDropMode(QAbstractItemView::NoDragDrop);
            labelEditList->setCurrentRow(highlightedRow);
            labelEditList->setDragDropMode(QAbstractItemView::InternalMove);
        }
    }
    auto *labelListKeyForwarder = new LabelListKeyForwarder(labelEditList, labelCombo, &dialog);
    labelCombo->lineEdit()->installEventFilter(labelListKeyForwarder);
    auto *completionModel = new QStringListModel(labelChoices, labelCombo);
    auto *completer = new QCompleter(completionModel, labelCombo);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    if (m_labelMeLabelCompletion == QStringLiteral("contains")) {
        completer->setCompletionMode(QCompleter::PopupCompletion);
        completer->setFilterMode(Qt::MatchContains);
    } else {
        completer->setCompletionMode(QCompleter::InlineCompletion);
        completer->setFilterMode(Qt::MatchStartsWith);
    }
    labelCombo->setCompleter(completer);

    auto *groupIdSpin = new QSpinBox(&dialog);
    groupIdSpin->setObjectName(QStringLiteral("labelGroupIdSpin"));
    groupIdSpin->setRange(-1, (std::numeric_limits<int>::max)());
    groupIdSpin->setSpecialValueText(m_strings.get(QStringLiteral("none")));
    groupIdSpin->setValue(editGroupId ? firstShape.groupId : -1);
    groupIdSpin->setEnabled(editGroupId);
    if (m_labelMeShowLabelTextField) {
        form->addRow(m_strings.get(QStringLiteral("groupId")), groupIdSpin);
    } else {
        groupIdSpin->hide();
    }

    auto *descriptionEdit = new QPlainTextEdit(&dialog);
    descriptionEdit->setObjectName(QStringLiteral("labelDescriptionEdit"));
    descriptionEdit->setPlainText(editDescription ? firstShape.description : QString());
    descriptionEdit->setEnabled(editDescription);
    descriptionEdit->setPlaceholderText(m_strings.get(QStringLiteral("labelDescriptionPlaceholder")));
    descriptionEdit->setFixedHeight(70);
    form->addRow(m_strings.get(QStringLiteral("labelDescription")), descriptionEdit);

    auto *flagsEdit = new QPlainTextEdit(&dialog);
    flagsEdit->setObjectName(QStringLiteral("labelFlagsEdit"));
    QMap<QString, bool> editableFlags = mergeLabelFlagDefaults(currentLabel, firstShape.flags);
    if (firstShape.difficult && !editableFlags.contains(QStringLiteral("difficult"))) {
        editableFlags.insert(QStringLiteral("difficult"), true);
    }
    flagsEdit->setPlainText(editFlags ? flagsToEditorText(editableFlags) : QString());
    flagsEdit->setEnabled(editFlags);
    flagsEdit->setPlaceholderText(QStringLiteral("key=true\nother=false"));
    flagsEdit->setFixedHeight(90);
    form->addRow(QStringLiteral("flags"), flagsEdit);

    auto *flagsList = new QListWidget(&dialog);
    flagsList->setObjectName(QStringLiteral("labelFlagsChecklist"));
    flagsList->setFixedHeight(90);
    flagsList->setSelectionMode(QAbstractItemView::NoSelection);
    flagsList->setEnabled(editFlags);
    form->addRow(m_strings.get(QStringLiteral("labelFlagsPresetTitle")), flagsList);

    bool syncingFlags = false;
    auto syncFlagsList = [flagsList, &syncingFlags](const QMap<QString, bool> &flags) {
        syncingFlags = true;
        QSignalBlocker blocker(flagsList);
        QSignalBlocker modelBlocker(flagsList->model());
        flagsList->clear();
        for (auto it = flags.cbegin(); it != flags.cend(); ++it) {
            auto *item = new QListWidgetItem(it.key(), flagsList);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(it.value() ? Qt::Checked : Qt::Unchecked);
        }
        syncingFlags = false;
    };
    syncFlagsList(editableFlags);

    connect(labelCombo, &QComboBox::currentTextChanged, &dialog,
            [this, flagsEdit, syncFlagsList](const QString &label) {
        if (m_labelFlagPresets.isEmpty() || !flagsEdit->isEnabled()) {
            return;
        }
        const QMap<QString, bool> currentFlags = flagsFromEditorText(flagsEdit->toPlainText());
        QMap<QString, bool> defaults = labelFlagDefaultsForLabel(label);
        const QStringList keys = defaults.keys();
        for (const QString &key : keys) {
            if (currentFlags.contains(key)) {
                defaults.insert(key, currentFlags.value(key));
            }
        }
        QSignalBlocker blocker(flagsEdit);
        flagsEdit->setPlainText(flagsToEditorText(defaults));
        syncFlagsList(defaults);
    });
    connect(flagsEdit, &QPlainTextEdit::textChanged, &dialog, [flagsEdit, syncFlagsList]() {
        syncFlagsList(flagsFromEditorText(flagsEdit->toPlainText()));
    });
    connect(flagsList, &QListWidget::itemChanged, &dialog, [&syncingFlags, flagsEdit](QListWidgetItem *item) {
        if (syncingFlags || !item || !flagsEdit->isEnabled()) {
            return;
        }
        QMap<QString, bool> flags = flagsFromEditorText(flagsEdit->toPlainText());
        flags.insert(item->text(), item->checkState() == Qt::Checked);
        flagsEdit->setPlainText(flagsToEditorText(flags));
    });

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog,
            [this, &dialog, labelCombo, editText]() {
        const QString text = labelCombo->currentText().trimmed();
        if (editText && text.isEmpty()) {
            labelCombo->setFocus();
            return;
        }
        if (editText && !validateLabel(text)) {
            QMessageBox::warning(this, m_strings.get("invalidLabel"),
                                 m_strings.get("invalidLabelDetail"));
            labelCombo->setFocus();
            return;
        }
        dialog.accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    // Match LabelMe's popup behavior: the active label is ready to be
    // replaced immediately when the editor opens.
    labelCombo->setFocus(Qt::PopupFocusReason);
    if (labelCombo->lineEdit()) {
        labelCombo->lineEdit()->selectAll();
    }

    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }

    QString text = labelCombo->currentText().trimmed();
    if (editText && text.isEmpty()) {
        return false;
    }
    if (editText && !validateLabel(text)) {
        QMessageBox::warning(this, m_strings.get("invalidLabel"), m_strings.get("invalidLabelDetail"));
        return false;
    }
    const int groupId = groupIdSpin->value();
    const QString description = descriptionEdit->toPlainText();
    QMap<QString, bool> flags = flagsFromEditorText(flagsEdit->toPlainText());
    for (int row = 0; row < flagsList->count(); ++row) {
        const QListWidgetItem *item = flagsList->item(row);
        flags.insert(item->text(), item->checkState() == Qt::Checked);
    }
    const bool difficult = flags.value(QStringLiteral("difficult"), false);

    if (editText && m_singleClassAction->isChecked()) {
        for (Shape &shape : shapes) {
            shape.label = text;
            applyShapeLabelColors(&shape);
        }
    }
    for (int index : selected) {
        if (index < 0 || index >= shapes.size()) {
            continue;
        }
        Shape &shape = shapes[index];
        if (editText && !m_singleClassAction->isChecked()) {
            shape.label = text;
            applyShapeLabelColors(&shape);
        }
        if (editGroupId) {
            shape.groupId = groupId;
        }
        if (editDescription) {
            shape.description = description;
            shape.descriptionPresent = true;
            shape.descriptionIsNull = false;
        }
        if (editFlags) {
            shape.flags = flags;
            shape.difficult = difficult;
        }
    }
    QSignalBlocker difficultBlocker(m_difficult);
    if (editFlags) {
        m_difficult->setChecked(difficult);
    }
    if (editText) {
        rememberLastUsedLabel(text);
    }
    refreshLabels();
    if (!m_shapeHistoryPending) {
        recordShapeHistory();
    } else {
        refreshActions();
    }
    setDirty(true);
    m_canvas->update();
    return true;
}

void MainWindow::deleteCurrentShape() {
    const QVector<int> selected = m_canvas->selectedIndices();
    if (selected.size() > 1) {
        m_canvas->deleteSelected();
    } else if (selected.size() == 1) {
        m_canvas->deleteCurrent();
    } else {
        return;
    }
    refreshLabels();
    recordShapeHistory();
    setDirty(true);
}

void MainWindow::deleteAllShapes() {
    if (!m_canvas || m_canvas->isDrawing() || m_canvas->shapes().isEmpty()) {
        return;
    }
    m_canvas->setShapes({});
    refreshLabels();
    recordShapeHistory();
    setDirty(true);
}

void MainWindow::copyCurrentShape() {
    QVector<int> selected = m_canvas->selectedIndices();
    if (selected.isEmpty()) {
        int row = m_canvas->currentIndex();
        if (row < 0 || row >= m_canvas->shapes().size()) {
            row = m_labelList->currentRow();
        }
        if (row >= 0 && row < m_canvas->shapes().size()) {
            m_canvas->setCurrentIndex(row);
            selected = m_canvas->selectedIndices();
        }
    }
    if (!selected.isEmpty() && m_canvas->duplicateSelected(QPointF())) {
        refreshLabels();
        recordShapeHistory();
        setDirty(true);
        return;
    }
}

void MainWindow::copySelectedShapesToClipboard() {
    m_shapeClipboard.clear();
    const QVector<Shape> shapes = m_canvas->shapes();
    const QVector<int> selected = m_canvas->selectedIndices();
    for (int index : selected) {
        if (index >= 0 && index < shapes.size()) {
            Shape copy = shapes[index].copy();
            copy.selected = false;
            m_shapeClipboard.push_back(copy);
        }
    }
    if (m_shapeClipboard.isEmpty() && m_canvas->hasSelection()) {
        Shape copy = shapes[m_canvas->currentIndex()].copy();
        copy.selected = false;
        m_shapeClipboard.push_back(copy);
    }
    QJsonArray jsonShapes;
    for (const Shape &shape : m_shapeClipboard) {
        jsonShapes.append(shapeToClipboardJson(shape));
    }
    if (!jsonShapes.isEmpty()) {
        const QByteArray payload = QJsonDocument(jsonShapes).toJson(QJsonDocument::Compact);
        auto *mimeData = new QMimeData();
        mimeData->setText(QString::fromUtf8(payload));
        mimeData->setData(QStringLiteral("application/x-labelme-shapes"), payload);
        QApplication::clipboard()->setMimeData(mimeData);
    }
    refreshActions();
    statusBar()->showMessage(m_strings.get(QStringLiteral("copiedShapes"))
                                 .arg(m_shapeClipboard.size()),
                             2500);
}

void MainWindow::pasteShapesFromClipboard() {
    if (m_filePath.isEmpty()) {
        return;
    }

    // LabelMe's clipboard is an in-process deep-copy buffer. Prefer it when
    // available so C++ runtime-only fields (palette, visibility, point
    // prompts) survive a paste in the same window; the JSON payload is the
    // interoperability path for another window or process.
    QVector<Shape> clipboardShapes = m_shapeClipboard;
    if (clipboardShapes.isEmpty()) {
        clipboardShapes = shapesFromClipboardMime(QApplication::clipboard()->mimeData());
    }
    if (clipboardShapes.isEmpty()) {
        return;
    }

    QVector<Shape> &shapes = m_canvas->shapesRef();
    QVector<int> pastedIndices;
    pastedIndices.reserve(clipboardShapes.size());
    for (const Shape &stored : clipboardShapes) {
        Shape copy = stored.copy();
        copy.selected = false;
        shapes.push_back(copy);
        pastedIndices.push_back(shapes.size() - 1);
    }
    m_canvas->update();
    refreshLabels();
    m_canvas->setSelectedIndices(pastedIndices);
    recordShapeHistory();
    setDirty(true);
    statusBar()->showMessage(m_strings.get(QStringLiteral("pastedShapes"))
                                 .arg(pastedIndices.size()),
                             2500);
}

void MainWindow::copyPreviousBoundingBoxes() {
    if (m_currentImageIndex <= 0 || m_currentImageIndex >= m_imageList.size()) return;
    AnnotationDocument previous;
    QString previousPath = m_imageList[m_currentImageIndex - 1];
    QString base = QFileInfo(previousPath).absolutePath() + "/" + QFileInfo(previousPath).completeBaseName();
    if (QFileInfo::exists(base + ".xml")) {
        AnnotationIO::loadPascalVoc(base + ".xml", &previous);
    } else if (QFileInfo::exists(base + ".txt")) {
        QImage image = readImageWithAutoTransform(previousPath);
        AnnotationIO::loadYolo(base + ".txt", image.size(), &previous);
    } else if (QFileInfo::exists(base + ".json")) {
        if (!AnnotationIO::loadLabelMe(base + ".json", &previous)) {
            AnnotationIO::loadCreateMl(base + ".json", previousPath, &previous);
        }
    }
    if (previous.shapes.isEmpty()) return;
    QVector<Shape> shapes = m_canvas->shapes();
    shapes += previous.shapes;
    m_canvas->setShapes(shapes);
    refreshLabels();
    recordShapeHistory();
    setDirty(true);
}

void MainWindow::deleteCurrentImage() {
    if (m_filePath.isEmpty()) return;
    if (QMessageBox::question(this, QStringLiteral("labelImgCpp"),
                              m_strings.get(QStringLiteral("deleteImageQuestion")).arg(m_filePath),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }
    QString deleted = m_filePath;
    QFile::moveToTrash(deleted);
    int removedIndex = m_imageList.indexOf(deleted);
    if (removedIndex >= 0) {
        m_imageList.removeAt(removedIndex);
    }
    populateFileList();
    if (m_imageList.isEmpty()) {
        m_filePath.clear();
        m_annotationPathOverride.clear();
        m_hasAnnotationPathOverride = false;
        m_labelMeImageData.clear();
        m_labelMeImagePath.clear();
        m_labelMeVersion.clear();
        m_labelMeTopLevelFlags.clear();
        m_labelMeOtherData = QJsonObject();
        m_canvas->setPixmap(QPixmap());
        m_canvas->setShapes({});
        refreshLabels();
        syncTopLevelFlagsEditor();
        setDirty(false);
        return;
    }
    m_currentImageIndex = qMin(removedIndex, m_imageList.size() - 1);
    loadImage(m_imageList[m_currentImageIndex]);
}

void MainWindow::deleteCurrentAnnotationFile() {
    if (m_filePath.isEmpty()) {
        return;
    }
    const QString path = (usesAnnotationPathOverride())
                             ? m_annotationPathOverride
                             : annotationPathForImage(m_filePath);
    if (!QFileInfo::exists(path)) {
        refreshActions();
        return;
    }
    if (QMessageBox::question(this, m_strings.get("deleteAnnotation"),
                              m_strings.get("deleteAnnotationConfirm"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }
    if (!QFile::remove(path)) {
        QMessageBox::warning(this, m_strings.get("deleteAnnotation"),
                             m_strings.get(QStringLiteral("cannotDelete")).arg(path));
        return;
    }

    m_annotationPathOverride.clear();
    m_hasAnnotationPathOverride = false;
    m_labelMeImageData.clear();
    m_labelMeImagePath.clear();
    m_labelMeVersion.clear();
    m_labelMeTopLevelFlags.clear();
    m_labelMeOtherData = QJsonObject();
    m_verified = false;
    m_verifyAction->setChecked(false);
    m_canvas->setShapes({});
    refreshLabels();
    resetShapeHistory();
    setDirty(false);
    if (!m_imageList.isEmpty()) {
        populateFileList();
    }
    statusBar()->showMessage(m_strings.get(QStringLiteral("deletedAnnotation")).arg(path), 4000);
    refreshActions();
}

void MainWindow::toggleAllShapesVisible(bool visible) {
    m_canvas->setAllShapesVisible(visible);
    refreshLabels();
    recordShapeHistory();
    setDirty(true);
}

void MainWindow::toggleAllShapes() {
    const QVector<Shape> shapes = m_canvas->shapes();
    bool allVisible = !shapes.isEmpty();
    for (const Shape &shape : shapes) {
        if (!shape.visible) {
            allVisible = false;
            break;
        }
    }
    toggleAllShapesVisible(!allVisible);
}

void MainWindow::zoomIn() {
    m_fitMode = FitMode::Manual;
    m_fitWindowAction->setChecked(false);
    m_fitWidthAction->setChecked(false);
    const double oldScale = m_canvas->scale();
    const QPoint anchor = zoomAnchorPosition();
    const int currentPercent = m_zoomWidget ? m_zoomWidget->value() : qRound(oldScale * 100.0);
    const int targetPercent = qBound(1, static_cast<int>(qCeil(currentPercent * 1.1)), 1600);
    m_canvas->setScale(targetPercent / 100.0);
    if (!qFuzzyCompare(oldScale, m_canvas->scale())) {
        onCanvasScaleChanged(oldScale, m_canvas->scale(), anchor);
    }
    syncZoomWidget();
}

void MainWindow::zoomOut() {
    m_fitMode = FitMode::Manual;
    m_fitWindowAction->setChecked(false);
    m_fitWidthAction->setChecked(false);
    const double oldScale = m_canvas->scale();
    const QPoint anchor = zoomAnchorPosition();
    const int currentPercent = m_zoomWidget ? m_zoomWidget->value() : qRound(oldScale * 100.0);
    const int targetPercent = qBound(1, static_cast<int>(qFloor(currentPercent * 0.9)), 1600);
    m_canvas->setScale(targetPercent / 100.0);
    if (!qFuzzyCompare(oldScale, m_canvas->scale())) {
        onCanvasScaleChanged(oldScale, m_canvas->scale(), anchor);
    }
    syncZoomWidget();
}

void MainWindow::resetZoom() {
    m_fitMode = FitMode::Manual;
    m_fitWindowAction->setChecked(false);
    m_fitWidthAction->setChecked(false);
    const double oldScale = m_canvas->scale();
    const QPoint anchor = zoomAnchorPosition();
    m_canvas->setScale(1.0);
    if (!qFuzzyCompare(oldScale, m_canvas->scale())) {
        onCanvasScaleChanged(oldScale, m_canvas->scale(), anchor);
    }
    syncZoomWidget();
}

void MainWindow::fitWindow() {
    m_fitMode = m_fitWindowAction->isChecked() ? FitMode::Window : FitMode::Manual;
    if (m_fitMode == FitMode::Window) m_fitWidthAction->setChecked(false);
    updateFitScale();
}

void MainWindow::fitWidth() {
    m_fitMode = m_fitWidthAction->isChecked() ? FitMode::Width : FitMode::Manual;
    if (m_fitMode == FitMode::Width) m_fitWindowAction->setChecked(false);
    updateFitScale();
}

void MainWindow::brighten() {
    m_canvas->addBrightness(5);
}

void MainWindow::darken() {
    m_canvas->addBrightness(-5);
}

void MainWindow::resetBrightness() {
    m_canvas->setBrightness(50);
    m_canvas->setContrast(50);
    if (!m_filePath.isEmpty()) {
        m_brightnessByFile.insert(m_filePath, 50);
        m_contrastByFile.insert(m_filePath, 50);
    }
}

void MainWindow::openBrightnessContrastDialog() {
    if (m_filePath.isEmpty()) {
        return;
    }

    const int originalBrightness = m_canvas->brightness();
    const int originalContrast = m_canvas->contrast();
    QDialog dialog(this);
    dialog.setWindowTitle(m_strings.get(QStringLiteral("brightnessContrastTitle")));
    dialog.setModal(true);

    auto *layout = new QFormLayout(&dialog);
    auto *brightnessSlider = new QSlider(Qt::Horizontal, &dialog);
    auto *contrastSlider = new QSlider(Qt::Horizontal, &dialog);
    brightnessSlider->setRange(0, 150);
    contrastSlider->setRange(0, 150);
    brightnessSlider->setValue(originalBrightness);
    contrastSlider->setValue(originalContrast);
    brightnessSlider->setTickPosition(QSlider::TicksBelow);
    contrastSlider->setTickPosition(QSlider::TicksBelow);

    auto *brightnessValue = new QLabel(&dialog);
    auto *contrastValue = new QLabel(&dialog);
    auto makeRow = [](QSlider *slider, QLabel *value, QWidget *parent) {
        auto *row = new QWidget(parent);
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->addWidget(slider, 1);
        value->setMinimumWidth(48);
        value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        rowLayout->addWidget(value);
        return row;
    };
    layout->addRow(m_strings.get(QStringLiteral("brightnessLabel")), makeRow(brightnessSlider, brightnessValue, &dialog));
    layout->addRow(m_strings.get(QStringLiteral("contrastLabel")), makeRow(contrastSlider, contrastValue, &dialog));

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    const auto updatePreview = [this, brightnessSlider, contrastSlider, brightnessValue, contrastValue]() {
        const int brightness = brightnessSlider->value();
        const int contrast = contrastSlider->value();
        m_canvas->setBrightness(brightness);
        m_canvas->setContrast(contrast);
        brightnessValue->setText(QString::number(brightness / 50.0, 'f', 2));
        contrastValue->setText(QString::number(contrast / 50.0, 'f', 2));
    };
    connect(brightnessSlider, &QSlider::valueChanged, &dialog, [updatePreview](int) { updatePreview(); });
    connect(contrastSlider, &QSlider::valueChanged, &dialog, [updatePreview](int) { updatePreview(); });
    updatePreview();

    if (dialog.exec() == QDialog::Accepted) {
        m_brightnessByFile.insert(m_filePath, brightnessSlider->value());
        m_contrastByFile.insert(m_filePath, contrastSlider->value());
        statusBar()->showMessage(m_strings.get(QStringLiteral("brightnessContrastApplied")), 2000);
    } else {
        m_canvas->setBrightness(originalBrightness);
        m_canvas->setContrast(originalContrast);
    }
}

void MainWindow::showCanvasContextMenu(const QPoint &globalPosition, const QPointF &imagePosition) {
    m_lastCanvasContextImagePos = imagePosition;
    const bool pendingRightDrag = m_canvas->hasPendingRightDrag();
    QMenu menu(this);
    menu.addAction(m_createPolygonModeAction);
    menu.addAction(m_createModeAction);
    menu.addAction(m_createOrientedRectangleModeAction);
    menu.addAction(m_createCircleModeAction);
    menu.addAction(m_createPointModeAction);
    menu.addAction(m_createPointsModeAction);
    menu.addAction(m_createLineModeAction);
    menu.addAction(m_createLinestripModeAction);
    menu.addAction(m_createAiPointsModeAction);
    menu.addAction(m_createAiBoxModeAction);
    menu.addAction(m_createMaskModeAction);
    menu.addSeparator();
    menu.addAction(m_editModeAction);
    menu.addAction(m_editLabelAction);
    menu.addAction(m_copyAction);
    menu.addAction(m_copyShapesAction);
    menu.addAction(m_pasteShapesAction);
    if (m_canvas->hasSelection() &&
        m_canvas->currentIndex() < m_canvas->shapes().size() &&
        m_canvas->shapes()[m_canvas->currentIndex()].shapeType == QStringLiteral("mask")) {
        menu.addAction(m_maskEditAction);
    }
    menu.addAction(m_deleteAction);
    menu.addAction(m_deleteAllShapesAction);
    menu.addAction(m_undoAction);
    menu.addAction(m_undoLastPointAction);
    menu.addAction(m_shapeLineColorAction);
    menu.addAction(m_shapeFillColorAction);
    if (m_canvas->hasSelection()) {
        menu.addSeparator();
        menu.addAction(m_addPointToEdgeAction);
        menu.addAction(m_insertPolygonPointAction);
        menu.addAction(m_removePolygonPointAction);
        menu.addAction(m_removeSelectedPointAction);
        menu.addSeparator();
        menu.addAction(m_copyHereAction);
        menu.addAction(m_moveHereAction);
    }
    menu.addSeparator();
    menu.addAction(m_hideAllAction);
    menu.addAction(m_showAllAction);
    menu.exec(globalPosition);
    if (pendingRightDrag && m_canvas->hasPendingRightDrag()) {
        m_canvas->cancelRightDrag();
    }
}

void MainWindow::copyShapeHere() {
    if (m_canvas->hasPendingRightDrag()) {
        if (m_canvas->finishRightDrag(true)) {
            refreshLabels();
            recordShapeHistory();
            setDirty(true);
        }
        return;
    }
    if (m_canvas->copyCurrentTo(m_lastCanvasContextImagePos)) {
        refreshLabels();
        recordShapeHistory();
        setDirty(true);
    }
}

void MainWindow::moveShapeHere() {
    if (m_canvas->hasPendingRightDrag()) {
        if (m_canvas->finishRightDrag(false)) {
            refreshLabels();
            recordShapeHistory();
            setDirty(true);
        }
        return;
    }
    if (m_canvas->moveCurrentTo(m_lastCanvasContextImagePos)) {
        refreshLabels();
        recordShapeHistory();
        setDirty(true);
    }
}

void MainWindow::insertPolygonPointHere() {
    if (m_canvas->insertPointAt(m_lastCanvasContextImagePos)) {
        refreshLabels();
        recordShapeHistory();
        setDirty(true);
    }
}

void MainWindow::removePolygonPointHere() {
    if (m_canvas->removePointAt(m_lastCanvasContextImagePos)) {
        refreshLabels();
        recordShapeHistory();
        setDirty(true);
    }
}

void MainWindow::removeSelectedPoint() {
    if (m_canvas->removeSelectedPoint()) {
        refreshLabels();
        recordShapeHistory();
        setDirty(true);
    }
}

void MainWindow::chooseBoxLineColor() {
    QColor color = QColorDialog::getColor(m_lineColor, this, m_strings.get("boxLineColor"));
    if (!color.isValid()) return;
    m_lineColor = color;
    m_canvas->setLineColor(color);
    m_settings.setValue("line/color", color);
}

void MainWindow::chooseShapeLineColor() {
    const int row = m_canvas ? m_canvas->currentIndex() : -1;
    QVector<Shape> &shapes = m_canvas->shapesRef();
    if (row < 0 || row >= shapes.size()) return;
    QColor color = QColorDialog::getColor(shapes[row].lineColor, this, m_strings.get("shapeLineColor"));
    if (!color.isValid()) return;
    shapes[row].lineColor = color;
    m_canvas->update();
    recordShapeHistory();
    setDirty(true);
}

void MainWindow::chooseShapeFillColor() {
    const int row = m_canvas ? m_canvas->currentIndex() : -1;
    QVector<Shape> &shapes = m_canvas->shapesRef();
    if (row < 0 || row >= shapes.size()) return;
    QColor color = QColorDialog::getColor(shapes[row].fillColor, this, m_strings.get("shapeFillColor"));
    if (!color.isValid()) return;
    shapes[row].fillColor = color;
    m_canvas->update();
    recordShapeHistory();
    setDirty(true);
}

void MainWindow::editLabelFlagPresets() {
    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("labelFlagsConfigDialog"));
    dialog.setWindowTitle(m_strings.get(QStringLiteral("editLabelFlags")));
    auto *layout = new QVBoxLayout(&dialog);
    const QString source = m_settings.value(QStringLiteral("labelme/labelFlags")).toString();
    const bool sourceIsFile = labelFlagPresetSourceIsFile(source);

    auto *edit = new QPlainTextEdit(&dialog);
    edit->setObjectName(QStringLiteral("labelFlagsConfigEdit"));
    edit->setPlaceholderText(QStringLiteral("dog=occluded,truncated\n{person.*: [male, tall]}"));
    edit->setPlainText(sourceIsFile ? labelFlagPresetSourceText(source) : source);
    edit->setMinimumSize(420, 220);
    layout->addWidget(edit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const QString text = edit->toPlainText().trimmed();
    QString presetSource = text;
    if (sourceIsFile) {
        QFile file(source.trimmed());
        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            file.write(text.toUtf8());
            presetSource = source;
        }
    }
    m_settings.setValue(QStringLiteral("labelme/labelFlags"), presetSource);
    m_labelFlagPresets = labelFlagPresetsFromSource(presetSource);
    QVector<Shape> &shapes = m_canvas->shapesRef();
    applyLabelFlagDefaults(&shapes);
    refreshLabels();
    recordShapeHistory();
    setDirty(true);
}

void MainWindow::showInfoDialog() {
    QMessageBox::information(this, m_strings.get("info"), "labelImgCpp\nQt 6 Widgets C++ port");
}

void MainWindow::showShortcutsDialog() {
    QMessageBox::information(this, m_strings.get("shortcut"),
                               "A/D: previous/next image\nCtrl+Shift+A/D: previous/next image and copy shapes\nW: create box\nP: create polygon\nMode menu: create point/line/linestrip/circle/oriented rectangle/mask\nV: view mode\nEditability: Settings\nQ/E: previous/next box\nX/S/Delete: delete label\nCtrl+Z/Y: undo/redo\nCtrl+C/V: copy/paste selected shapes\nCtrl+D: duplicate selected shapes\nCtrl+Shift+V: copy previous image boxes\nZ/C: previous/next label\nAlt+Click edge: insert polygon/linestrip point\nAlt+Shift+Click point: remove polygon/linestrip point\nBackspace: remove hovered polygon/linestrip point\nSpace: verified\nCtrl+Wheel: zoom\nCtrl+Shift+Wheel: brightness");
}

void MainWindow::openTutorial() {
    QDesktopServices::openUrl(QUrl(QStringLiteral("https://github.com/labelmeai/labelme/tree/main/examples/tutorial")));
}

void MainWindow::showSettingsDialog() {
    if (m_configOverrides) {
        statusBar()->showMessage(m_strings.get(QStringLiteral("settingsControlledByCli")), 4000);
        return;
    }
    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("settingsDialog"));
    dialog.setWindowTitle(m_strings.get("settings"));
    dialog.resize(560, 520);

    auto *tabs = new QTabWidget(&dialog);
    auto *generalPage = new QWidget(tabs);
    auto *generalForm = new QFormLayout(generalPage);
    auto *viewPage = new QWidget(tabs);
    auto *viewForm = new QFormLayout(viewPage);
    tabs->addTab(generalPage, m_strings.get(QStringLiteral("settingsGeneral")));
    tabs->addTab(viewPage, m_strings.get(QStringLiteral("settingsViewAnnotation")));

    auto *languageCombo = new QComboBox(generalPage);
    languageCombo->setObjectName(QStringLiteral("settingsLanguageCombo"));
    const QHash<QString, QString> languageNames{{"en", "English"},
                                                 {"zh-CN", QString::fromUtf8("简体中文")},
                                                 {"zh-TW", QString::fromUtf8("繁體中文")},
                                                 {"ja-JP", QString::fromUtf8("日本語")}};
    for (const QString &language : StringBundle::supportedLanguages()) {
        languageCombo->addItem(languageNames.value(language, language), language);
    }
    const int languageIndex = languageCombo->findData(m_strings.language());
    if (languageIndex >= 0) {
        languageCombo->setCurrentIndex(languageIndex);
    }
    generalForm->addRow(m_strings.get(QStringLiteral("language")), languageCombo);

    auto *settingsLabels = new QPlainTextEdit(generalPage);
    settingsLabels->setObjectName(QStringLiteral("settingsLabels"));
    settingsLabels->setPlainText(m_classList.join(QLatin1Char('\n')));
    settingsLabels->setFixedHeight(90);
    generalForm->addRow(m_strings.get(QStringLiteral("labels")), settingsLabels);

    auto *formatCombo = new QComboBox(generalPage);
    formatCombo->setObjectName(QStringLiteral("settingsFormatCombo"));
    formatCombo->addItem(QStringLiteral("Pascal VOC"), static_cast<int>(SaveFormat::PascalVoc));
    formatCombo->addItem(QStringLiteral("YOLO"), static_cast<int>(SaveFormat::Yolo));
    formatCombo->addItem(QStringLiteral("CreateML"), static_cast<int>(SaveFormat::CreateMl));
    formatCombo->addItem(QStringLiteral("LabelMe"), static_cast<int>(SaveFormat::LabelMe));
    formatCombo->setCurrentIndex(formatCombo->findData(static_cast<int>(m_format)));
    generalForm->addRow(m_strings.get(QStringLiteral("labelFormat")), formatCombo);

    auto *validateLabelCombo = new QComboBox(generalPage);
    validateLabelCombo->setObjectName(QStringLiteral("settingsValidateLabelCombo"));
    validateLabelCombo->addItem(m_strings.get("validateLabelNone"), QString());
    validateLabelCombo->addItem(m_strings.get("validateLabelExact"), QStringLiteral("exact"));
    const int validateLabelIndex = validateLabelCombo->findData(m_validateLabelPolicy);
    validateLabelCombo->setCurrentIndex(validateLabelIndex >= 0 ? validateLabelIndex : 0);
    generalForm->addRow(m_strings.get("validateLabel"), validateLabelCombo);

    auto addCheck = [](QFormLayout *form, QWidget *parent, const QString &objectName,
                       const QString &label, bool checked) {
        auto *check = new QCheckBox(label, parent);
        check->setObjectName(objectName);
        check->setChecked(checked);
        form->addRow(check);
        return check;
    };

    QCheckBox *autoSave = addCheck(generalForm, generalPage, QStringLiteral("settingsAutoSave"),
                                   m_autoSaveAction->text(), m_autoSaveAction->isChecked());
    QCheckBox *keepPrevious = addCheck(generalForm, generalPage, QStringLiteral("settingsKeepPrevious"),
                                       m_keepPreviousAction->text(), m_keepPreviousAction->isChecked());
    QCheckBox *embedImageData = addCheck(generalForm, generalPage, QStringLiteral("settingsEmbedImageData"),
                                         m_embedImageDataAction->text(), m_embedImageDataAction->isChecked());
    QCheckBox *advanced = addCheck(generalForm, generalPage, QStringLiteral("settingsAdvanced"),
                                   m_advancedModeAction->text(), m_advancedModeAction->isChecked());
    QCheckBox *displayLabelPopup = addCheck(
        generalForm, generalPage, QStringLiteral("settingsDisplayLabelPopup"),
        m_strings.get(QStringLiteral("displayLabelPopup")),
        m_settings.value(QStringLiteral("labelme/displayLabelPopup"), true).toBool());

    bool openConfigAsText = false;
    auto *openConfigButton = new QPushButton(m_strings.get(QStringLiteral("openConfigFile")), generalPage);
    openConfigButton->setObjectName(QStringLiteral("settingsOpenConfigButton"));
    openConfigButton->setEnabled(!m_configFilePath.isEmpty() && QFileInfo::exists(m_configFilePath));
    generalForm->addRow(openConfigButton);
    connect(openConfigButton, &QPushButton::clicked, &dialog, [&dialog, &openConfigAsText]() {
        openConfigAsText = true;
        dialog.reject();
    });

    QCheckBox *editingAllowed = addCheck(viewForm, viewPage, QStringLiteral("settingsEditingAllowed"),
                                         m_editabilityAction->text(), m_editabilityAction->isChecked());
    QCheckBox *keepZoom = addCheck(viewForm, viewPage, QStringLiteral("settingsKeepPreviousZoom"),
                                   m_keepPreviousZoomAction->text(), m_keepPreviousZoomAction->isChecked());
    QCheckBox *keepBrightness = addCheck(viewForm, viewPage, QStringLiteral("settingsKeepPreviousBrightnessContrast"),
                                         m_keepPreviousBrightnessContrastAction->text(),
                                         m_keepPreviousBrightnessContrastAction->isChecked());
    QCheckBox *drawSquare = addCheck(viewForm, viewPage, QStringLiteral("settingsDrawSquare"),
                                     m_drawSquareAction->text(), m_drawSquareAction->isChecked());
    QCheckBox *fillDrawing = addCheck(viewForm, viewPage, QStringLiteral("settingsFillDrawing"),
                                      m_fillDrawingAction->text(), m_fillDrawingAction->isChecked());
    QCheckBox *singleClass = addCheck(viewForm, viewPage, QStringLiteral("settingsSingleClass"),
                                      m_singleClassAction->text(), m_singleClassAction->isChecked());
    QCheckBox *displayLabels = addCheck(viewForm, viewPage, QStringLiteral("settingsDisplayLabels"),
                                        m_displayLabelsAction->text(), m_displayLabelsAction->isChecked());
    QCheckBox *miniMap = addCheck(viewForm, viewPage, QStringLiteral("settingsMiniMap"),
                                  m_miniMapAction->text(), m_miniMapAction->isChecked());
    QCheckBox *performance = addCheck(viewForm, viewPage, QStringLiteral("settingsPerformance"),
                                      m_showPerformanceAction->text(), m_showPerformanceAction->isChecked());
    QCheckBox *smoothSampling = addCheck(viewForm, viewPage, QStringLiteral("settingsSmoothSampling"),
                                         m_samplingModeAction->text(), m_samplingModeAction->isChecked());
    QCheckBox *thumbnails = addCheck(viewForm, viewPage, QStringLiteral("settingsThumbnails"),
                                     m_thumbnailModeAction->text(), m_thumbnailModeAction->isChecked());

    auto *epsilon = new QDoubleSpinBox(viewPage);
    epsilon->setObjectName(QStringLiteral("settingsEpsilon"));
    epsilon->setRange(0.0, 1000.0);
    epsilon->setDecimals(2);
    epsilon->setSingleStep(0.5);
    epsilon->setValue(m_canvas->epsilon());
    viewForm->addRow(m_strings.get(QStringLiteral("epsilon")), epsilon);

    auto *pointSize = new QSpinBox(viewPage);
    pointSize->setObjectName(QStringLiteral("settingsPointSize"));
    pointSize->setRange(1, 64);
    pointSize->setValue(m_canvas->pointSize());
    viewForm->addRow(m_strings.get(QStringLiteral("pointSize")), pointSize);

    QCheckBox *doubleClickClose = addCheck(
        viewForm, viewPage, QStringLiteral("settingsDoubleClickClose"),
        m_strings.get(QStringLiteral("doubleClickClose")), m_canvas->doubleClickClose());
    QCheckBox *snapping = addCheck(viewForm, viewPage, QStringLiteral("settingsSnapping"),
                                   m_strings.get(QStringLiteral("snapping")), m_canvas->snapping());
    QCheckBox *crosshair = addCheck(
        viewForm, viewPage, QStringLiteral("settingsCrosshair"), m_strings.get(QStringLiteral("crosshair")),
        m_canvas->crosshairEnabledForShapeType(QStringLiteral("rectangle")));

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    auto *layout = new QVBoxLayout(&dialog);
    layout->addWidget(tabs);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) {
        if (openConfigAsText && !m_configFilePath.isEmpty()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(m_configFilePath));
        }
        return;
    }

    const QString language = languageCombo->currentData().toString();
    if (!language.isEmpty() && language != m_strings.language()) {
        changeLanguage(language);
    }
    QStringList configuredLabels;
    for (const QString &line : settingsLabels->toPlainText().split(QRegularExpression(QStringLiteral("[\\r\\n]+")),
                                                                    Qt::SkipEmptyParts)) {
        const QString label = line.trimmed();
        if (!label.isEmpty() && !configuredLabels.contains(label)) {
            configuredLabels.append(label);
        }
    }
    const QString requestedValidateLabel = validateLabelCombo->currentData().toString();
    if (requestedValidateLabel == QStringLiteral("exact") && configuredLabels.isEmpty()) {
        QMessageBox::warning(this, m_strings.get("invalidLabel"),
                             m_strings.get(QStringLiteral("exactLabelsRequired")));
        return;
    }

    // Persist the editable LabelMe configuration before mutating live widgets.
    // A failed write must not leave the application showing values that will
    // disappear on the next restart.
    if (!persistLabelMeConfigValue(QStringLiteral("language"), language) ||
        !persistLabelMeConfigValue(QStringLiteral("auto_save"), autoSave->isChecked()) ||
        !persistLabelMeConfigValue(QStringLiteral("keep_prev"), keepPrevious->isChecked()) ||
        !persistLabelMeConfigValue(QStringLiteral("with_image_data"), embedImageData->isChecked()) ||
        !persistLabelMeConfigValue(QStringLiteral("keep_prev_scale"), keepZoom->isChecked()) ||
        !persistLabelMeConfigValue(QStringLiteral("keep_prev_brightness_contrast"), keepBrightness->isChecked()) ||
        !persistLabelMeConfigValue(QStringLiteral("validate_label"), requestedValidateLabel.isEmpty()
                                                                    ? QVariant()
                                                                    : QVariant(requestedValidateLabel)) ||
        !persistLabelMeConfigValue(QStringLiteral("display_label_popup"), displayLabelPopup->isChecked()) ||
        !persistLabelMeConfigValue(QStringLiteral("canvas.fill_drawing"), fillDrawing->isChecked()) ||
        !persistLabelMeConfigValue(QStringLiteral("epsilon"), epsilon->value()) ||
        !persistLabelMeConfigValue(QStringLiteral("shape.point_size"), pointSize->value()) ||
        !persistLabelMeConfigValue(QStringLiteral("canvas.double_click"),
                                   doubleClickClose->isChecked() ? QVariant(QStringLiteral("close"))
                                                                 : QVariant(QStringLiteral("none"))) ||
        !persistLabelMeConfigValue(QStringLiteral("canvas.snapping"), snapping->isChecked()) ||
        !persistLabelMeConfigValue(QStringLiteral("canvas.crosshair.rectangle"), crosshair->isChecked()) ||
        !persistLabelMeConfigValue(QStringLiteral("labels"), QVariant(configuredLabels))) {
        return;
    }

    const QStringList previousLabelHistory = m_classList;
    m_classList.clear();
    {
        QSignalBlocker blocker(m_defaultLabelCombo);
        m_defaultLabelCombo->clear();
        for (const QString &label : configuredLabels) {
            addClassLabel(label);
        }
        for (const QString &label : previousLabelHistory) {
            addClassLabel(label);
        }
        if (m_defaultLabelCombo->count() > 0 && m_defaultLabelCombo->currentIndex() < 0) {
            m_defaultLabelCombo->setCurrentIndex(0);
        }
    }
    m_settings.setValue(QStringLiteral("labelHistory"), m_classList);
    setFormat(static_cast<SaveFormat>(formatCombo->currentData().toInt()));
    m_validateLabelPolicy = validateLabelCombo->currentData().toString();
    m_settings.setValue(QStringLiteral("labelme/validateLabel"), m_validateLabelPolicy);
    m_autoSaveAction->setChecked(autoSave->isChecked());
    m_keepPreviousAction->setChecked(keepPrevious->isChecked());
    m_embedImageDataAction->setChecked(embedImageData->isChecked());
    m_advancedModeAction->setChecked(advanced->isChecked());
    m_settings.setValue(QStringLiteral("labelme/displayLabelPopup"), displayLabelPopup->isChecked());
    m_editabilityAction->setChecked(editingAllowed->isChecked());
    m_keepPreviousZoomAction->setChecked(keepZoom->isChecked());
    m_keepPreviousBrightnessContrastAction->setChecked(keepBrightness->isChecked());
    m_drawSquareAction->setChecked(drawSquare->isChecked());
    m_fillDrawingAction->setChecked(fillDrawing->isChecked());
    m_singleClassAction->setChecked(singleClass->isChecked());
    m_displayLabelsAction->setChecked(displayLabels->isChecked());
    m_miniMapAction->setChecked(miniMap->isChecked());
    m_showPerformanceAction->setChecked(performance->isChecked());
    m_samplingModeAction->setChecked(smoothSampling->isChecked());
    m_thumbnailModeAction->setChecked(thumbnails->isChecked());
    m_canvas->setEpsilon(epsilon->value());
    m_canvas->setPointSize(pointSize->value());
    m_canvas->setDoubleClickClose(doubleClickClose->isChecked());
    m_canvas->setSnapping(snapping->isChecked());
    m_canvas->setCrosshairEnabledForShapeType(QStringLiteral("rectangle"), crosshair->isChecked());
    saveSettings();
    refreshActions();
}

void MainWindow::setAdvancedMode(bool enabled) {
    if (m_advancedModeAction->isChecked() != enabled) {
        QSignalBlocker blocker(m_advancedModeAction);
        m_advancedModeAction->setChecked(enabled);
    }
    populateToolbarForMode();
    if (enabled) {
        setEditMode();
        if (m_labelDock) m_labelDock->setFeatures(m_labelDock->features() | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    } else {
        setEditMode();
        if (m_labelDock) m_labelDock->setFeatures(QDockWidget::DockWidgetClosable);
    }
    m_settings.setValue("advanced", enabled);
}

void MainWindow::setCreateShapeMode(const QString &shapeType, QAction *activeAction) {
    if (m_editabilityAction && !m_editabilityAction->isChecked()) {
        setViewMode();
        return;
    }
    {
        QSignalBlocker createBlocker(m_createModeAction);
        QSignalBlocker polygonBlocker(m_createPolygonModeAction);
        QSignalBlocker pointBlocker(m_createPointModeAction);
        QSignalBlocker pointsBlocker(m_createPointsModeAction);
        QSignalBlocker aiPointsBlocker(m_createAiPointsModeAction);
        QSignalBlocker aiBoxBlocker(m_createAiBoxModeAction);
        QSignalBlocker lineBlocker(m_createLineModeAction);
        QSignalBlocker linestripBlocker(m_createLinestripModeAction);
        QSignalBlocker circleBlocker(m_createCircleModeAction);
        QSignalBlocker orientedBlocker(m_createOrientedRectangleModeAction);
        QSignalBlocker maskBlocker(m_createMaskModeAction);
        QSignalBlocker editBlocker(m_editModeAction);
        QSignalBlocker viewBlocker(m_viewModeAction);
        m_createModeAction->setChecked(activeAction == m_createModeAction);
        m_createPolygonModeAction->setChecked(activeAction == m_createPolygonModeAction);
        m_createPointModeAction->setChecked(activeAction == m_createPointModeAction);
        m_createPointsModeAction->setChecked(activeAction == m_createPointsModeAction);
        m_createAiPointsModeAction->setChecked(activeAction == m_createAiPointsModeAction);
        m_createAiBoxModeAction->setChecked(activeAction == m_createAiBoxModeAction);
        m_createLineModeAction->setChecked(activeAction == m_createLineModeAction);
        m_createLinestripModeAction->setChecked(activeAction == m_createLinestripModeAction);
        m_createCircleModeAction->setChecked(activeAction == m_createCircleModeAction);
        m_createOrientedRectangleModeAction->setChecked(activeAction == m_createOrientedRectangleModeAction);
        m_createMaskModeAction->setChecked(activeAction == m_createMaskModeAction);
        m_editModeAction->setChecked(false);
        m_viewModeAction->setChecked(false);
    }
    {
        QSignalBlocker blocker(m_maskEditAction);
        m_maskEditAction->setChecked(false);
    }
    m_canvas->setMaskEditing(false);
    m_canvas->setCreateShapeType(shapeType);
    m_canvas->setCreateMode(true);
    const bool aiMode = activeAction == m_createAiPointsModeAction || activeAction == m_createAiBoxModeAction;
    updateAiModelAvailability(shapeType == QStringLiteral("ai_points_to_shape"));
    m_aiModelCombo->setEnabled(aiMode);
    m_aiOutputFormatCombo->setEnabled(aiMode);
    syncModeFooterControls();
    refreshActions();
}

void MainWindow::updateAiModelAvailability(bool pointPrompt) {
    if (!m_aiModelCombo) {
        return;
    }

    auto *model = qobject_cast<QStandardItemModel *>(m_aiModelCombo->model());
    if (!model) {
        return;
    }

    const QStringList unsupportedForPoints = {QStringLiteral("sam3:latest")};
    for (int index = 0; index < m_aiModelCombo->count(); ++index) {
        QStandardItem *item = model->item(index);
        if (!item) {
            continue;
        }
        const bool unsupported = unsupportedForPoints.contains(m_aiModelCombo->itemData(index).toString());
        if (pointPrompt && unsupported) {
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        } else {
            item->setFlags(item->flags() | Qt::ItemIsEnabled);
        }
    }

    if (pointPrompt && unsupportedForPoints.contains(m_aiModelCombo->currentData().toString())) {
        const int fallbackIndex = m_aiModelCombo->findData(QStringLiteral("sam2:latest"));
        if (fallbackIndex >= 0) {
            QSignalBlocker blocker(m_aiModelCombo);
            m_aiModelCombo->setCurrentIndex(fallbackIndex);
            m_settings.setValue(QStringLiteral("ai/model"), m_aiModelCombo->currentData().toString());
        }
    }
}

void MainWindow::setCreateMode() {
    setCreateShapeMode(QStringLiteral("rectangle"), m_createModeAction);
}

void MainWindow::setPolygonCreateMode() {
    setCreateShapeMode(QStringLiteral("polygon"), m_createPolygonModeAction);
}

void MainWindow::setPointCreateMode() {
    setCreateShapeMode(QStringLiteral("point"), m_createPointModeAction);
}

void MainWindow::setPointsCreateMode() {
    setCreateShapeMode(QStringLiteral("points"), m_createPointsModeAction);
}

void MainWindow::setAiPointsCreateMode() {
    setCreateShapeMode(QStringLiteral("ai_points_to_shape"), m_createAiPointsModeAction);
}

void MainWindow::setAiBoxCreateMode() {
    setCreateShapeMode(QStringLiteral("ai_box_to_shape"), m_createAiBoxModeAction);
}

void MainWindow::setLineCreateMode() {
    setCreateShapeMode(QStringLiteral("line"), m_createLineModeAction);
}

void MainWindow::setLinestripCreateMode() {
    setCreateShapeMode(QStringLiteral("linestrip"), m_createLinestripModeAction);
}

void MainWindow::setCircleCreateMode() {
    setCreateShapeMode(QStringLiteral("circle"), m_createCircleModeAction);
}

void MainWindow::setOrientedRectangleCreateMode() {
    setCreateShapeMode(QStringLiteral("oriented_rectangle"), m_createOrientedRectangleModeAction);
}

void MainWindow::setMaskCreateMode() {
    setCreateShapeMode(QStringLiteral("mask"), m_createMaskModeAction);
}

void MainWindow::setEditMode() {
    if (m_editabilityAction && !m_editabilityAction->isChecked()) {
        setViewMode();
        return;
    }
    {
        QSignalBlocker createBlocker(m_createModeAction);
        QSignalBlocker polygonBlocker(m_createPolygonModeAction);
        QSignalBlocker pointBlocker(m_createPointModeAction);
        QSignalBlocker pointsBlocker(m_createPointsModeAction);
        QSignalBlocker aiPointsBlocker(m_createAiPointsModeAction);
        QSignalBlocker aiBoxBlocker(m_createAiBoxModeAction);
        QSignalBlocker lineBlocker(m_createLineModeAction);
        QSignalBlocker linestripBlocker(m_createLinestripModeAction);
        QSignalBlocker circleBlocker(m_createCircleModeAction);
        QSignalBlocker orientedBlocker(m_createOrientedRectangleModeAction);
        QSignalBlocker maskBlocker(m_createMaskModeAction);
        QSignalBlocker editBlocker(m_editModeAction);
        QSignalBlocker viewBlocker(m_viewModeAction);
        m_createModeAction->setChecked(false);
        m_createPolygonModeAction->setChecked(false);
        m_createPointModeAction->setChecked(false);
        m_createPointsModeAction->setChecked(false);
        m_createAiPointsModeAction->setChecked(false);
        m_createAiBoxModeAction->setChecked(false);
        m_createLineModeAction->setChecked(false);
        m_createLinestripModeAction->setChecked(false);
        m_createCircleModeAction->setChecked(false);
        m_createOrientedRectangleModeAction->setChecked(false);
        m_createMaskModeAction->setChecked(false);
        m_editModeAction->setChecked(true);
        m_viewModeAction->setChecked(false);
    }
    m_canvas->setEditMode();
    updateAiModelAvailability(false);
    m_aiModelCombo->setEnabled(false);
    m_aiOutputFormatCombo->setEnabled(false);
    syncModeFooterControls();
    refreshActions();
}

void MainWindow::setViewMode() {
    {
        QSignalBlocker createBlocker(m_createModeAction);
        QSignalBlocker polygonBlocker(m_createPolygonModeAction);
        QSignalBlocker pointBlocker(m_createPointModeAction);
        QSignalBlocker pointsBlocker(m_createPointsModeAction);
        QSignalBlocker aiPointsBlocker(m_createAiPointsModeAction);
        QSignalBlocker aiBoxBlocker(m_createAiBoxModeAction);
        QSignalBlocker lineBlocker(m_createLineModeAction);
        QSignalBlocker linestripBlocker(m_createLinestripModeAction);
        QSignalBlocker circleBlocker(m_createCircleModeAction);
        QSignalBlocker orientedBlocker(m_createOrientedRectangleModeAction);
        QSignalBlocker maskBlocker(m_createMaskModeAction);
        QSignalBlocker editBlocker(m_editModeAction);
        QSignalBlocker viewBlocker(m_viewModeAction);
        m_createModeAction->setChecked(false);
        m_createPolygonModeAction->setChecked(false);
        m_createPointModeAction->setChecked(false);
        m_createPointsModeAction->setChecked(false);
        m_createAiPointsModeAction->setChecked(false);
        m_createAiBoxModeAction->setChecked(false);
        m_createLineModeAction->setChecked(false);
        m_createLinestripModeAction->setChecked(false);
        m_createCircleModeAction->setChecked(false);
        m_createOrientedRectangleModeAction->setChecked(false);
        m_createMaskModeAction->setChecked(false);
        m_editModeAction->setChecked(false);
        m_viewModeAction->setChecked(true);
    }
    {
        QSignalBlocker blocker(m_maskEditAction);
        m_maskEditAction->setChecked(false);
    }
    m_canvas->setMaskEditing(false);
    m_canvas->setViewMode();
    updateAiModelAvailability(false);
    m_aiModelCombo->setEnabled(false);
    m_aiOutputFormatCombo->setEnabled(false);
    syncModeFooterControls();
    refreshActions();
}

void MainWindow::setEditabilityAllowed(bool allowed) {
    m_settings.setValue("view/editingAllowed", allowed);
    if (!allowed) {
        setViewMode();
        return;
    }
    refreshActions();
}

void MainWindow::showLabelListContextMenu(const QPoint &position) {
    QMenu menu(this);
    menu.addAction(m_editLabelAction);
    menu.addAction(m_deleteAction);
    menu.addAction(m_deleteAllShapesAction);
    menu.exec(m_labelList->mapToGlobal(position));
}

void MainWindow::showFileListContextMenu(const QPoint &position) {
    if (!m_fileList || !m_fileListContextMenu) {
        return;
    }
    QListWidgetItem *item = m_fileList->itemAt(position);
    if (!item) {
        return;
    }
    m_fileList->setCurrentItem(item, QItemSelectionModel::ClearAndSelect);
    m_fileContextPath = QFileInfo(item->text()).absoluteFilePath();
    updateFileContextActions();
    m_fileListContextMenu->exec(m_fileList->viewport()->mapToGlobal(position));
}

QString MainWindow::contextFilePath() const {
    if (!m_fileContextPath.isEmpty() && m_imageList.contains(m_fileContextPath)) {
        return m_fileContextPath;
    }
    if (m_fileList && m_fileList->currentItem()) {
        return QFileInfo(m_fileList->currentItem()->text()).absoluteFilePath();
    }
    return {};
}

void MainWindow::updateFileContextActions() {
    const QString path = contextFilePath();
    const bool hasTarget = !path.isEmpty() && QFileInfo::exists(path);
    const bool marked = hasTarget && m_markedFiles.contains(path);
    const QList<QAction *> actions = {
        m_fileContextOpenAction,
        m_fileContextRevealAction,
        m_fileContextCopyPathAction,
        m_fileContextMarkAction,
        m_fileContextDeleteAction,
    };
    for (QAction *action : actions) {
        if (action) {
            action->setEnabled(hasTarget);
        }
    }
    if (m_fileContextMarkAction) {
        m_fileContextMarkAction->setText(marked ? m_strings.get("fileContextUnmark") : m_strings.get("fileContextMark"));
    }
}

void MainWindow::openContextFile() {
    const QString path = contextFilePath();
    if (path.isEmpty()) {
        return;
    }
    const int index = m_imageList.indexOf(path);
    if (index >= 0) {
        m_currentImageIndex = index;
    }
    loadImage(path);
    m_fileContextPath = path;
    updateFileContextActions();
}

void MainWindow::revealContextFile() {
    const QString path = contextFilePath();
    if (path.isEmpty()) {
        return;
    }
#ifdef Q_OS_WIN
    QProcess::startDetached(QStringLiteral("explorer.exe"),
                            {QStringLiteral("/select,%1").arg(QDir::toNativeSeparators(path))});
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
#endif
}

void MainWindow::copyContextFilePath() {
    const QString path = contextFilePath();
    if (path.isEmpty()) {
        return;
    }
    QApplication::clipboard()->setText(path);
    statusBar()->showMessage(m_strings.get(QStringLiteral("pathCopied")), 2000);
}

void MainWindow::toggleContextFileMark() {
    const QString path = contextFilePath();
    if (path.isEmpty()) {
        return;
    }
    const QString selectedPath = m_fileList && m_fileList->currentItem()
                                     ? QFileInfo(m_fileList->currentItem()->text()).absoluteFilePath()
                                     : QString();
    if (m_markedFiles.contains(path)) {
        m_markedFiles.remove(path);
    } else {
        m_markedFiles.insert(path);
    }
    refreshFileListSelection();
    if (!selectedPath.isEmpty() && m_fileList) {
        for (int i = 0; i < m_fileList->count(); ++i) {
            QListWidgetItem *item = m_fileList->item(i);
            if (QFileInfo(item->text()).absoluteFilePath() == selectedPath) {
                m_fileList->setCurrentItem(item, QItemSelectionModel::ClearAndSelect);
                break;
            }
        }
    }
    updateFileContextActions();
}

void MainWindow::deleteContextFile() {
    const QString path = contextFilePath();
    if (path.isEmpty()) {
        return;
    }
    if (QMessageBox::question(this, QStringLiteral("labelImgCpp"),
                              m_strings.get(QStringLiteral("deleteImageQuestion")).arg(path),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    const bool deletingCurrent = QFileInfo(path).absoluteFilePath() == m_filePath;
    const int removedIndex = m_imageList.indexOf(path);
    QFile::moveToTrash(path);
    if (removedIndex >= 0) {
        m_imageList.removeAt(removedIndex);
    }
    m_markedFiles.remove(path);
    m_viewedFiles.remove(path);
    for (auto it = m_lastFileByDir.begin(); it != m_lastFileByDir.end();) {
        if (it.value() == path) {
            it = m_lastFileByDir.erase(it);
        } else {
            ++it;
        }
    }
    m_fileContextPath.clear();

    if (deletingCurrent) {
        if (m_imageList.isEmpty()) {
            m_filePath.clear();
            m_annotationPathOverride.clear();
            m_hasAnnotationPathOverride = false;
            m_labelMeImageData.clear();
            m_labelMeImagePath.clear();
            m_labelMeVersion.clear();
            m_labelMeTopLevelFlags.clear();
            m_labelMeOtherData = QJsonObject();
            m_canvas->setPixmap(QPixmap());
            m_canvas->setShapes({});
            refreshLabels();
            populateFileList();
            syncTopLevelFlagsEditor();
            setDirty(false);
            return;
        }
        m_currentImageIndex = qMin(qMax(removedIndex, 0), m_imageList.size() - 1);
        loadImage(m_imageList[m_currentImageIndex]);
    } else {
        populateFileList();
    }
    updateFileContextActions();
}

void MainWindow::onCanvasScaleChanged(double oldScale, double newScale, const QPoint &widgetPosition) {
    if (oldScale <= 0.0 || qFuzzyCompare(oldScale, newScale)) {
        return;
    }
    m_fitMode = FitMode::Manual;
    {
        QSignalBlocker fitWindowBlocker(m_fitWindowAction);
        QSignalBlocker fitWidthBlocker(m_fitWidthAction);
        m_fitWindowAction->setChecked(false);
        m_fitWidthAction->setChecked(false);
    }
    QScrollBar *hBar = m_scrollArea->horizontalScrollBar();
    QScrollBar *vBar = m_scrollArea->verticalScrollBar();
    const QPointF imagePoint = QPointF(widgetPosition.x() / oldScale,
                                       widgetPosition.y() / oldScale) -
                               m_canvas->imageOriginOffsetForScale(oldScale);
    const QPointF newWidgetPoint = (imagePoint + m_canvas->imageOriginOffset()) * newScale;
    hBar->setValue(hBar->value() + qRound(newWidgetPoint.x() - widgetPosition.x()));
    vBar->setValue(vBar->value() + qRound(newWidgetPoint.y() - widgetPosition.y()));
    if (m_miniMapOverlay) {
        m_miniMapOverlay->refreshGeometry();
    }
    if (!m_upgradingPreviewImage && m_canvas->isPreviewImage() &&
        newScale >= CanvasPreviewUpgradeScale) {
        QTimer::singleShot(0, this, [this]() {
            if (m_canvas && m_canvas->scale() >= CanvasPreviewUpgradeScale) {
                upgradePreviewImage();
            }
        });
    }
    syncZoomWidget();
    updatePerformanceLabel();
}

void MainWindow::onCanvasFrameRendered(double frameMs) {
    Q_UNUSED(frameMs);
    if (!m_fpsTimer.isValid()) {
        m_fpsTimer.start();
        m_fpsFrameCount = 1;
        return;
    }

    ++m_fpsFrameCount;
    const qint64 elapsedMs = m_fpsTimer.elapsed();
    if (elapsedMs < 500) {
        return;
    }

    m_displayFps = qBound(0.0, m_fpsFrameCount * 1000.0 / elapsedMs, 240.0);
    m_fpsFrameCount = 0;
    m_fpsTimer.restart();
    updatePerformanceLabel();
}

void MainWindow::editCreatedShapeLabel(int index) {
    if (index < 0 || index >= m_canvas->shapes().size()) {
        return;
    }
    QVector<Shape> &shapes = m_canvas->shapesRef();
    if (shapes[index].label.trimmed().isEmpty()) {
        const QString label = preferredNewShapeLabel();
        shapes[index].label = label;
        applyShapeLabelColors(&shapes[index]);
        shapes[index].flags = mergeLabelFlagDefaults(label, shapes[index].flags);
        shapes[index].difficult = m_difficult ? m_difficult->isChecked() : false;
        refreshLabels();
    }
    m_canvas->setCurrentIndex(index);
    if (m_labelList && index < m_labelList->count()) {
        m_labelList->setCurrentRow(index, QItemSelectionModel::ClearAndSelect);
    }
    const bool showPopup = m_settings.value(QStringLiteral("labelme/displayLabelPopup"), true).toBool();
    if (showPopup || shapes[index].label.trimmed().isEmpty()) {
        if (!editCurrentLabel()) {
            // LabelMe treats a cancelled label popup as cancelling the new
            // annotation itself. Do not leave an unlabeled draft behind.
            m_canvas->discardShapeAt(index);
            refreshLabels();
            setDirty(m_shapeEditDirtyBefore);
        }
    }
}

void MainWindow::onCanvasShapeCreated(int index) {
    const bool aiMode = (m_createAiPointsModeAction && m_createAiPointsModeAction->isChecked()) ||
                        (m_createAiBoxModeAction && m_createAiBoxModeAction->isChecked());
    if (aiMode) {
        startAiAssist(index);
        return;
    }
    editCreatedShapeLabel(index);
}

QString MainWindow::aiBridgeScriptPath() const {
    const QString configuredPath = m_settings.value(QStringLiteral("ai/bridgePath")).toString().trimmed();
    if (!configuredPath.isEmpty() && QFileInfo::exists(configuredPath)) {
        return QFileInfo(configuredPath).absoluteFilePath();
    }
    const QString sharedPath = ResourcePaths::filePath(QStringLiteral("cpp/tools/labelme_ai_bridge.py"));
    if (QFileInfo::exists(sharedPath)) {
        return sharedPath;
    }
    const QString besideExecutable = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("labelme_ai_bridge.py"));
    return QFileInfo::exists(besideExecutable) ? besideExecutable : QString();
}

void MainWindow::onAiSessionProgress(const QString &modelName,
                                     int fileIndex,
                                     int fileCount,
                                     const QString &fileName,
                                     qint64 bytesDone,
                                     qint64 bytesTotal) {
    if (!m_aiRequestRunning || !m_aiProgressBar) {
        return;
    }

    m_aiProgressBar->setVisible(true);
    if (bytesTotal > 0 && bytesTotal <= (std::numeric_limits<int>::max)()) {
        m_aiProgressBar->setRange(0, static_cast<int>(bytesTotal));
        m_aiProgressBar->setValue(static_cast<int>(qBound<qint64>(0, bytesDone, bytesTotal)));
    } else {
        m_aiProgressBar->setRange(0, 0);
    }
    if (m_aiCancelButton) {
        m_aiCancelButton->setVisible(true);
    }

    const int safeFileCount = qMax(1, fileCount);
    const int safeFileIndex = qBound(0, fileIndex, safeFileCount - 1);
    QString message = m_strings.get(QStringLiteral("aiDownloadProgress"))
                          .arg(modelName)
                          .arg(safeFileIndex + 1)
                          .arg(safeFileCount)
                          .arg(fileName);
    if (bytesDone > 0) {
        message += QStringLiteral(" (%1 B)").arg(bytesDone);
    }
    statusBar()->showMessage(message, 0);
}

void MainWindow::cancelAiAssist() {
    if (!m_aiRequestRunning) {
        return;
    }
    const AiRequestKind kind = m_aiRequestKind;
    const int promptIndex = m_aiPromptIndex;
    if (m_aiSession) {
        m_aiSession->stop();
    }
    m_aiRequestRunning = false;
    m_aiRequestKind = AiRequestKind::None;
    m_aiTextPromptRunning = false;
    if (m_aiProgressBar) {
        m_aiProgressBar->setVisible(false);
    }
    if (m_aiCancelButton) {
        m_aiCancelButton->setVisible(false);
    }
    refreshActions();
    if (kind == AiRequestKind::Point) {
        finishAiAssist(promptIndex, {});
    } else {
        statusBar()->showMessage(m_strings.get(QStringLiteral("aiCancelled")), 3000);
    }
}

void MainWindow::startAiAssist(int index) {
    if (index < 0 || index >= m_canvas->shapes().size()) {
        return;
    }
    if (m_aiRequestRunning) {
        statusBar()->showMessage(m_strings.get(QStringLiteral("aiInferenceRunning")), 2500);
        return;
    }
    const QString scriptPath = aiBridgeScriptPath();
    if (scriptPath.isEmpty()) {
        statusBar()->showMessage(m_strings.get(QStringLiteral("aiBridgeMissing")), 5000);
        editCreatedShapeLabel(index);
        return;
    }

    const Shape promptShape = m_canvas->shapes().at(index);
    AiPrompt prompt;
    prompt.imagePath = m_filePath;
    prompt.modelName = m_aiModelCombo ? m_aiModelCombo->currentData().toString() : QStringLiteral("sam2:latest");
    prompt.outputFormat = m_aiOutputFormatCombo ? m_aiOutputFormatCombo->currentData().toString() : QStringLiteral("polygon");
    if (m_createAiBoxModeAction && m_createAiBoxModeAction->isChecked()) {
        const QRectF box = promptShape.boundingRect();
        prompt.points = {box.topLeft(), box.bottomRight()};
        prompt.pointLabels = {2, 3};
    } else {
        prompt.points = promptShape.points;
        prompt.pointLabels = promptShape.pointLabels;
        if (prompt.pointLabels.size() != prompt.points.size()) {
            prompt.pointLabels = m_canvas->promptPointLabels();
        }
        if (prompt.pointLabels.size() != prompt.points.size()) {
            prompt.pointLabels.fill(1, prompt.points.size());
        }
    }
    m_aiPromptIndex = index;
    m_aiPromptHistoryCaptured = !m_undoStack.isEmpty();
    m_aiPromptBaseShapes = m_aiPromptHistoryCaptured ? m_undoStack.last() : QVector<Shape>();

    m_aiRequestKind = AiRequestKind::Point;
    m_aiRequestRunning = true;
    if (m_aiProgressBar) {
        m_aiProgressBar->setRange(0, 0);
        m_aiProgressBar->setVisible(true);
    }
    if (m_aiCancelButton) {
        m_aiCancelButton->setVisible(true);
    }
    statusBar()->showMessage(m_strings.get(QStringLiteral("aiInferenceProgress")), 0);
    if (!m_aiSession || !m_aiSession->request(scriptPath, AiAssistBridge::requestJson(prompt))) {
        m_aiRequestRunning = false;
        m_aiRequestKind = AiRequestKind::None;
        if (m_aiProgressBar) {
            m_aiProgressBar->setVisible(false);
        }
        if (m_aiCancelButton) {
            m_aiCancelButton->setVisible(false);
        }
        statusBar()->showMessage(m_strings.get(QStringLiteral("aiInferenceFailed"))
                                     .arg(m_strings.get(QStringLiteral("aiPythonStartFailed"))), 6000);
        editCreatedShapeLabel(index);
    }
}

void MainWindow::onAiSessionResponse(const QByteArray &payload) {
    if (!m_aiRequestRunning) {
        return;
    }
    const AiRequestKind kind = m_aiRequestKind;
    const int promptIndex = m_aiPromptIndex;
    const double iouThreshold = m_aiTextIouThreshold;
    QVector<Shape> inferred;
    QString error;
    if (!AiAssistBridge::parseResponse(payload, &inferred, &error)) {
        onAiSessionFailed(error.isEmpty() ? m_strings.get(QStringLiteral("aiBridgeReturnedError")) : error);
        return;
    }
    m_aiRequestRunning = false;
    m_aiRequestKind = AiRequestKind::None;
    if (m_aiProgressBar) {
        m_aiProgressBar->setVisible(false);
    }
    if (m_aiCancelButton) {
        m_aiCancelButton->setVisible(false);
    }
    if (kind == AiRequestKind::Text) {
        m_aiTextPromptRunning = false;
    }
    refreshActions();
    if (kind == AiRequestKind::Point) {
        finishAiAssist(promptIndex, inferred);
    } else if (kind == AiRequestKind::Text) {
        finishAiTextAssist(inferred, iouThreshold);
    }
}

void MainWindow::onAiSessionFailed(const QString &message) {
    if (!m_aiRequestRunning) {
        return;
    }
    const AiRequestKind kind = m_aiRequestKind;
    const int promptIndex = m_aiPromptIndex;
    m_aiRequestRunning = false;
    m_aiRequestKind = AiRequestKind::None;
    m_aiTextPromptRunning = false;
    if (m_aiProgressBar) {
        m_aiProgressBar->setVisible(false);
    }
    if (m_aiCancelButton) {
        m_aiCancelButton->setVisible(false);
    }
    refreshActions();
    if (kind == AiRequestKind::Point) {
        statusBar()->showMessage(m_strings.get(QStringLiteral("aiInferenceFailed")).arg(message), 6000);
        editCreatedShapeLabel(promptIndex);
    } else {
        statusBar()->showMessage(m_strings.get(QStringLiteral("aiTextFailed")).arg(message), 6000);
    }
}

void MainWindow::finishAiAssist(int promptIndex, const QVector<Shape> &inferredShapes) {
    if (inferredShapes.isEmpty()) {
        if (promptIndex >= 0 && promptIndex < m_canvas->shapes().size()) {
            QVector<Shape> updated = m_canvas->shapes();
            updated.removeAt(promptIndex);
            m_restoringHistory = true;
            m_canvas->setShapes(updated);
            m_restoringHistory = false;
            if (m_aiPromptHistoryCaptured && !m_undoStack.isEmpty() &&
                sameShapesForHistory(m_undoStack.last(), m_aiPromptBaseShapes)) {
                m_undoStack.removeLast();
            }
            m_lastShapeSnapshot = updated;
            refreshLabels();
            setDirty(false);
        }
        m_aiPromptBaseShapes.clear();
        m_aiPromptHistoryCaptured = false;
        statusBar()->showMessage(m_strings.get(QStringLiteral("aiNoAnnotationCancelled")), 4000);
        return;
    }
    if (promptIndex < 0 || promptIndex >= m_canvas->shapes().size()) {
        m_aiPromptBaseShapes.clear();
        m_aiPromptHistoryCaptured = false;
        statusBar()->showMessage(m_strings.get(QStringLiteral("aiNoAnnotationCancelled")), 4000);
        return;
    }

    const Shape promptShape = m_canvas->shapes().at(promptIndex);
    QVector<Shape> candidates;
    candidates.reserve(inferredShapes.size());
    for (Shape shape : inferredShapes) {
        shape.label = promptShape.label;
        shape.flags = promptShape.flags;
        shape.difficult = promptShape.difficult;
        shape.paintLabel = promptShape.paintLabel;
        candidates.push_back(shape);
    }
    QVector<Shape> existingShapes = m_canvas->shapes();
    existingShapes.removeAt(promptIndex);
    const QVector<Shape> accepted = AiAssistBridge::suppressOverlappingShapes(
        candidates, existingShapes, 0.5);
    if (accepted.isEmpty()) {
        finishAiAssist(promptIndex, {});
        return;
    }

    QVector<Shape> updated = m_canvas->shapes();
    updated.removeAt(promptIndex);
    QVector<int> selected;
    selected.reserve(accepted.size());
    for (int i = 0; i < accepted.size(); ++i) {
        Shape shape = accepted.at(i);
        applyShapeLabelColors(&shape);
        updated.insert(promptIndex + i, shape);
        selected.push_back(promptIndex + i);
    }
    m_canvas->setShapes(updated);
    m_canvas->setSelectedIndices(selected);
    refreshLabels();
    recordShapeHistory();
    setDirty(true);
    statusBar()->showMessage(m_strings.get(QStringLiteral("aiGeneratedAnnotations")).arg(accepted.size()), 3000);
    m_aiPromptBaseShapes.clear();
    m_aiPromptHistoryCaptured = false;
    editCreatedShapeLabel(promptIndex);
}

void MainWindow::startAiTextAssist() {
    if (m_filePath.isEmpty() || !m_canvas || !isAiTextCreateMode(m_canvas->createShapeType())) {
        statusBar()->showMessage(m_strings.get(QStringLiteral("aiTextModeRequired")), 3000);
        return;
    }
    if (m_aiRequestRunning) {
        statusBar()->showMessage(m_strings.get(QStringLiteral("aiInferenceRunning")), 2500);
        return;
    }

    QStringList texts;
    for (const QString &part : m_aiTextPromptEdit->text().split(QLatin1Char(','), Qt::SkipEmptyParts)) {
        const QString text = part.trimmed();
        if (!text.isEmpty() && !texts.contains(text)) {
            texts.append(text);
        }
    }
    if (texts.isEmpty()) {
        statusBar()->showMessage(m_strings.get(QStringLiteral("aiTextPromptRequired")), 3000);
        return;
    }
    const QString scriptPath = aiBridgeScriptPath();
    if (scriptPath.isEmpty()) {
        statusBar()->showMessage(m_strings.get(QStringLiteral("aiBridgeMissing")), 5000);
        return;
    }

    AiPrompt prompt;
    prompt.imagePath = m_filePath;
    prompt.modelName = m_aiTextModelCombo ? m_aiTextModelCombo->currentData().toString()
                                          : QStringLiteral("yoloworld:latest");
    prompt.textPrompt = true;
    prompt.texts = texts;
    prompt.scoreThreshold = m_aiTextScoreSpin ? m_aiTextScoreSpin->value() : 0.1;
    prompt.iouThreshold = m_aiTextIouSpin ? m_aiTextIouSpin->value() : 0.5;
    if (m_canvas->createShapeType() == QStringLiteral("ai_points_to_shape") ||
        m_canvas->createShapeType() == QStringLiteral("ai_box_to_shape")) {
        prompt.outputFormat = m_aiOutputFormatCombo ? m_aiOutputFormatCombo->currentData().toString()
                                                     : QStringLiteral("polygon");
    } else {
        prompt.outputFormat = m_canvas->createShapeType();
    }

    m_aiTextPromptRunning = true;
    m_aiRequestKind = AiRequestKind::Text;
    m_aiTextIouThreshold = prompt.iouThreshold;
    m_aiRequestRunning = true;
    if (m_aiProgressBar) {
        m_aiProgressBar->setRange(0, 0);
        m_aiProgressBar->setVisible(true);
    }
    if (m_aiCancelButton) {
        m_aiCancelButton->setVisible(true);
    }
    statusBar()->showMessage(m_strings.get(QStringLiteral("aiTextProgress")), 0);
    refreshActions();
    if (!m_aiSession || !m_aiSession->request(scriptPath, AiAssistBridge::requestJson(prompt))) {
        m_aiRequestRunning = false;
        m_aiRequestKind = AiRequestKind::None;
        m_aiTextPromptRunning = false;
        if (m_aiProgressBar) {
            m_aiProgressBar->setVisible(false);
        }
        if (m_aiCancelButton) {
            m_aiCancelButton->setVisible(false);
        }
        refreshActions();
        statusBar()->showMessage(m_strings.get(QStringLiteral("aiTextFailed"))
                                     .arg(m_strings.get(QStringLiteral("aiPythonStartFailed"))), 6000);
    }
}

void MainWindow::finishAiTextAssist(const QVector<Shape> &inferredShapes, double iouThreshold) {
    if (inferredShapes.isEmpty()) {
        statusBar()->showMessage(m_strings.get(QStringLiteral("aiTextNoAnnotations")), 4000);
        return;
    }

    QVector<Shape> candidates;
    candidates.reserve(inferredShapes.size());
    for (const Shape &shape : inferredShapes) {
        if (!shape.label.trimmed().isEmpty()) {
            candidates.append(shape);
        }
    }
    const QVector<Shape> allExistingShapes = m_canvas->shapes();
    QVector<Shape> existingShapes;
    QSet<QString> detectedLabels;
    const QString outputShapeType = candidates.isEmpty()
                                        ? QString()
                                        : candidates.first().shapeType;
    for (const Shape &candidate : candidates) {
        detectedLabels.insert(candidate.label);
    }
    for (const Shape &existing : allExistingShapes) {
        // LabelMe's text prompt NMS only compares existing annotations that
        // could have been returned by this prompt. A different class or
        // geometry type must not suppress a valid new detection.
        if (existing.shapeType == outputShapeType &&
            detectedLabels.contains(existing.label)) {
            existingShapes.append(existing);
        }
    }
    QVector<Shape> updated = allExistingShapes;
    const QVector<Shape> accepted = AiAssistBridge::suppressOverlappingShapes(
        candidates, existingShapes, iouThreshold);
    QVector<int> selected;
    for (Shape shape : accepted) {
        applyShapeLabelColors(&shape);
        shape.paintLabel = m_displayLabelsAction && m_displayLabelsAction->isChecked();
        shape.flags = mergeLabelFlagDefaults(shape.label, shape.flags);
        shape.difficult = shape.flags.value(QStringLiteral("difficult"), shape.difficult);
        shape.visible = true;
        addClassLabel(shape.label);
        selected.append(updated.size());
        updated.append(shape);
    }

    if (selected.isEmpty()) {
        statusBar()->showMessage(m_strings.get(QStringLiteral("aiTextNoNewAnnotations")), 4000);
        return;
    }
    m_canvas->setShapes(updated);
    m_canvas->setSelectedIndices(selected);
    refreshLabels();
    recordShapeHistory();
    setDirty(true);
    statusBar()->showMessage(m_strings.get(QStringLiteral("aiTextGeneratedAnnotations")).arg(selected.size()), 3000);
}

void MainWindow::startSystemMove() {
#ifdef Q_OS_WIN
    HWND hwnd = reinterpret_cast<HWND>(winId());
    if (hwnd) {
        ReleaseCapture();
        SendMessageW(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        return;
    }
#endif
    if (QWindow *handle = windowHandle()) {
        handle->startSystemMove();
    }
}

void MainWindow::toggleMaximizeRestore() {
    if (isMaximized()) {
        showNormal();
    } else {
        showMaximized();
    }
    updateFramelessChrome();
}

void MainWindow::updateFramelessChrome() {
    if (!m_titleBar) {
        return;
    }
    m_titleBar->setMaximized(isMaximized() || isFullScreen());
    const QString title = m_filePath.isEmpty() ? QStringLiteral("labelImgCpp") : QFileInfo(m_filePath).fileName();
    m_titleBar->setTitle(title);
}

void MainWindow::onCanvasSelectionChanged(int index) {
    if (m_noSelectionSlot) {
        m_noSelectionSlot = false;
        return;
    }
    if (index >= 0 && index < m_labelList->count() && m_canvas->selectedIndices().size() <= 1) {
        m_noSelectionSlot = true;
        m_labelList->setCurrentRow(index, QItemSelectionModel::ClearAndSelect);
        scrollLabelListToCurrentShape();
        const QVector<Shape> shapes = m_canvas->shapes();
        if (index < shapes.size()) {
            QSignalBlocker blocker(m_difficult);
            m_difficult->setChecked(shapes[index].difficult);
        }
    }
    refreshActions();
}

void MainWindow::onCanvasShapeEditStarted() {
    if (m_restoringHistory || !m_canvas) {
        return;
    }
    m_shapeEditDirtyBefore = m_dirty;
    m_shapeHistoryPending = true;
}

void MainWindow::onCanvasShapeEditFinished(bool changed) {
    if (!m_shapeHistoryPending) {
        return;
    }
    m_shapeHistoryPending = false;
    if (changed && m_canvas && !sameShapesForHistory(m_canvas->shapes(), m_lastShapeSnapshot)) {
        m_undoStack.push_back(m_lastShapeSnapshot);
        if (m_undoStack.size() > m_labelMeNumBackups) {
            m_undoStack.removeFirst();
        }
        m_redoStack.clear();
        m_lastShapeSnapshot = m_canvas->shapes();
    }
    refreshActions();
}

void MainWindow::onCanvasShapesChanged() {
    QVector<Shape> &shapes = m_canvas->shapesRef();
    bool changed = false;
    for (Shape &shape : shapes) {
        if (!shape.label.isEmpty()) {
            continue;
        }
        const QString label = preferredNewShapeLabel();
        shape.label = label;
        applyShapeLabelColors(&shape);
        shape.flags = mergeLabelFlagDefaults(label, shape.flags);
        shape.difficult = m_difficult->isChecked();
        changed = true;
    }
    if (changed) {
        refreshLabels();
    }
    if (!m_shapeHistoryPending) {
        recordShapeHistory();
    } else {
        refreshActions();
    }
    setDirty(true);
}

void MainWindow::onLabelSelectionChanged() {
    if (m_noSelectionSlot) {
        m_noSelectionSlot = false;
        return;
    }
    QVector<int> selected;
    for (QListWidgetItem *item : m_labelList->selectedItems()) {
        const int row = m_labelList->row(item);
        if (row >= 0) {
            selected.push_back(row);
        }
    }
    if (!selected.isEmpty()) {
        m_canvas->setSelectedIndices(selected);
        const int row = m_canvas->currentIndex();
        const QVector<Shape> shapes = m_canvas->shapes();
        if (row < shapes.size()) {
            QSignalBlocker blocker(m_difficult);
            m_difficult->setChecked(shapes[row].difficult);
        }
    } else {
        m_canvas->setSelectedIndices({});
    }
    refreshActions();
}

void MainWindow::onLabelItemChanged(QListWidgetItem *item) {
    int row = m_labelList->row(item);
    QVector<Shape> &shapes = m_canvas->shapesRef();
    if (row < 0 || row >= shapes.size()) return;
    const bool visible = item->checkState() == Qt::Checked;
    const bool visibilityChanged = shapes[row].visible != visible;
    const QList<QListWidgetItem *> selectedItems = m_labelList->selectedItems();
    if (visibilityChanged && selectedItems.size() > 1 && selectedItems.contains(item)) {
        QSignalBlocker blocker(m_labelList->model());
        for (QListWidgetItem *selectedItem : selectedItems) {
            const int selectedRow = m_labelList->row(selectedItem);
            if (selectedRow < 0 || selectedRow >= shapes.size()) {
                continue;
            }
            shapes[selectedRow].visible = visible;
            selectedItem->setCheckState(visible ? Qt::Checked : Qt::Unchecked);
        }
        recordShapeHistory();
        setDirty(true);
        m_canvas->update();
        return;
    }
    const Shape currentShape = shapes[row];
    const QString displayText = labelListDisplayText(currentShape);
    const QString text = item->text().trimmed() == displayText
                             ? currentShape.label
                             : item->text().trimmed();
    const bool labelChanged = shapes[row].label != text;
    if (labelChanged && !validateLabel(text)) {
        QMessageBox::warning(this, m_strings.get("invalidLabel"), m_strings.get("invalidLabelDetail"));
        refreshLabels();
        return;
    }
    shapes[row].visible = visible;
    shapes[row].label = text;
    applyShapeLabelColors(&shapes[row]);
    if (labelChanged) {
        rememberLastUsedLabel(text);
    }
    if (labelChanged) {
        refreshLabels();
    }
    recordShapeHistory();
    setDirty(true);
    m_canvas->update();
}

void MainWindow::onFileDoubleClicked(QListWidgetItem *item) {
    int index = m_imageList.indexOf(item->text());
    if (index >= 0) m_currentImageIndex = index;
    loadImage(item->text());
}

void MainWindow::onFilterChanged(int index) {
    QString text = m_filterCombo->itemText(index);
    for (int i = 0; i < m_labelList->count(); ++i) {
        QListWidgetItem *item = m_labelList->item(i);
        const QString label = item->data(Qt::UserRole).toString();
        item->setCheckState(text.isEmpty() || label == text ? Qt::Checked : Qt::Unchecked);
    }
}

void MainWindow::changeLanguage(const QString &language) {
    m_strings = StringBundle(language);
    m_settings.setValue("language", m_strings.language());
    if (m_canvas) {
        m_canvas->setLanguage(m_strings.language());
    }
    for (QAction *action : m_languageMenu->actions()) {
        action->setChecked(action->data().toString() == m_strings.language());
    }
    refreshTexts();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    updateFitScale();
    if (m_canvas && m_fitMode == FitMode::Manual) {
        m_canvas->updateGeometry();
        m_canvas->resize(m_canvas->sizeHint());
    }
    updateFramelessChrome();
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Control) {
        m_canvas->setDrawSquare(true);
    } else if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_W) {
        setCreateMode();
    } else if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_V) {
        setViewMode();
    } else if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Z) {
        selectAdjacentLabel(-1);
    } else if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_C) {
        selectAdjacentLabel(1);
    } else if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_S) {
        deleteCurrentShape();
    } else if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_X) {
        if (m_deleteAction->isEnabled()) {
            deleteCurrentShape();
        }
    } else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Z) {
        if (!m_canvas->undoLastDrawingPoint()) {
            undoShapeOperation();
        }
    } else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Y) {
        redoShapeOperation();
    } else if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Q) {
        selectAdjacentShape(-1);
    } else if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_E) {
        selectAdjacentShape(1);
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Control) {
        m_canvas->setDrawSquare(m_drawSquareAction->isChecked());
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (!maybeSave()) {
        event->ignore();
        return;
    }
    stopPreviewUpgradeThread();
    saveSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (!event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }

    for (const QUrl &url : event->mimeData()->urls()) {
        if (isSupportedImagePath(url.toLocalFile())) {
            event->acceptProposedAction();
            return;
        }
    }
    event->ignore();
}

void MainWindow::dropEvent(QDropEvent *event) {
    if (!event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }

    QStringList paths;
    for (const QUrl &url : event->mimeData()->urls()) {
        const QString path = QFileInfo(url.toLocalFile()).absoluteFilePath();
        if (isSupportedImagePath(path) && !paths.contains(path)) {
            paths.append(path);
        }
    }

    if (importDroppedImageFiles(paths)) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

bool MainWindow::importDroppedImageFiles(const QStringList &paths) {
    QStringList newPaths;
    for (const QString &path : paths) {
        const QString absolutePath = QFileInfo(path).absoluteFilePath();
        if (!isSupportedImagePath(absolutePath) || m_imageList.contains(absolutePath) || newPaths.contains(absolutePath)) {
            continue;
        }
        newPaths.append(absolutePath);
    }
    if (newPaths.isEmpty() || !maybeSave()) {
        return false;
    }

    m_imageList.append(newPaths);
    if (m_dirPath.isEmpty()) {
        m_dirPath = QFileInfo(newPaths.first()).absolutePath();
    }
    m_settings.setValue(QStringLiteral("lastOpenDir"), QFileInfo(newPaths.first()).absolutePath());
    addRecentDir(QFileInfo(newPaths.first()).absolutePath());
    populateFileList();
    m_currentImageIndex = m_imageList.indexOf(newPaths.first());
    return loadImage(newPaths.first());
}

void MainWindow::changeEvent(QEvent *event) {
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange) {
        updateFramelessChrome();
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_uniqueLabelList) {
        if (event->type() == QEvent::KeyPress) {
            auto *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Escape) {
                m_uniqueLabelList->clearSelection();
                return true;
            }
        } else if (event->type() == QEvent::MouseButtonPress) {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            if (!m_uniqueLabelList->itemAt(mouseEvent->position().toPoint())) {
                m_uniqueLabelList->clearSelection();
            }
        }
    }
    if ((watched == m_canvas || watched == m_labelList) && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->modifiers() == Qt::NoModifier) {
            if (keyEvent->key() == Qt::Key_Q) {
                selectAdjacentShape(-1);
                return true;
            }
            if (keyEvent->key() == Qt::Key_E) {
                selectAdjacentShape(1);
                return true;
            }
            if (keyEvent->key() == Qt::Key_X) {
                if (m_deleteAction->isEnabled()) {
                    deleteCurrentShape();
                }
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result) {
#ifdef Q_OS_WIN
    if (eventType != "windows_generic_MSG" && eventType != "windows_dispatcher_MSG") {
        return QMainWindow::nativeEvent(eventType, message, result);
    }

    MSG *msg = static_cast<MSG *>(message);
    if (msg->message == WM_NCCALCSIZE && msg->wParam == TRUE) {
        *result = 0;
        return true;
    }

    if (msg->message != WM_NCHITTEST || isMaximized() || isFullScreen() || !m_titleBar) {
        return QMainWindow::nativeEvent(eventType, message, result);
    }

    const HWND hwnd = reinterpret_cast<HWND>(winId());
    POINT nativeClientPosition{GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam)};
    if (!hwnd || !ScreenToClient(hwnd, &nativeClientPosition)) {
        return QMainWindow::nativeEvent(eventType, message, result);
    }

    const qreal nativeScale = qMax<qreal>(1.0, static_cast<qreal>(GetDpiForWindow(hwnd)) / 96.0);
    const QPoint localPosition(qRound(nativeClientPosition.x / nativeScale),
                               qRound(nativeClientPosition.y / nativeScale));
    const QRect titleFrame(m_titleBar->mapTo(this, QPoint(0, 0)), m_titleBar->size());
    const WindowHitRegion region = windowHitRegion(rect(), titleFrame, localPosition, 8);

    switch (region) {
    case WindowHitRegion::TopLeft:
        *result = HTTOPLEFT;
        return true;
    case WindowHitRegion::TopRight:
        *result = HTTOPRIGHT;
        return true;
    case WindowHitRegion::BottomLeft:
        *result = HTBOTTOMLEFT;
        return true;
    case WindowHitRegion::BottomRight:
        *result = HTBOTTOMRIGHT;
        return true;
    case WindowHitRegion::Left:
        *result = HTLEFT;
        return true;
    case WindowHitRegion::Right:
        *result = HTRIGHT;
        return true;
    case WindowHitRegion::Top:
        *result = HTTOP;
        return true;
    case WindowHitRegion::Bottom:
        *result = HTBOTTOM;
        return true;
    case WindowHitRegion::Caption: {
        QWidget *child = childAt(localPosition);
        for (QWidget *widget = child; widget; widget = widget->parentWidget()) {
            if (qobject_cast<QToolButton *>(widget) || qobject_cast<QMenuBar *>(widget) ||
                qobject_cast<QToolBar *>(widget) || widget == m_titleToolContainer) {
                return QMainWindow::nativeEvent(eventType, message, result);
            }
            if (widget == m_titleBar) {
                break;
            }
        }
        *result = HTCAPTION;
        return true;
    }
    case WindowHitRegion::Client:
        break;
    }
#endif
    return QMainWindow::nativeEvent(eventType, message, result);
}

void MainWindow::selectLabelRow(int row) {
    if (m_labelList->count() == 0) return;
    row = qBound(0, row, m_labelList->count() - 1);
    m_labelList->setCurrentRow(row, QItemSelectionModel::ClearAndSelect);
    m_canvas->setCurrentIndex(row);
}

void MainWindow::selectAdjacentLabel(int step) {
    if (m_labelList->count() == 0) return;
    int row = m_labelList->currentRow();
    if (row < 0) {
        selectLabelRow(0);
        return;
    }
    int next = (row + step) % m_labelList->count();
    if (next < 0) next += m_labelList->count();
    selectLabelRow(next);
}

void MainWindow::selectAdjacentShape(int step) {
    const int count = m_canvas ? m_canvas->shapes().size() : 0;
    if (count == 0) {
        return;
    }
    int row = m_canvas->currentIndex();
    if (count == 1) {
        if (row == 0) {
            m_canvas->setCurrentIndex(-1);
            QSignalBlocker blocker(m_labelList);
            m_labelList->setCurrentItem(nullptr, QItemSelectionModel::Clear);
            m_labelList->clearSelection();
        } else {
            selectLabelRow(0);
        }
        refreshActions();
        return;
    }
    if (row < 0) {
        selectLabelRow(0);
        return;
    }
    int next = (row + step) % count;
    if (next < 0) next += count;
    selectLabelRow(next);
}

QString MainWindow::annotationPathForImage(const QString &imagePath) const {
    if (m_format == SaveFormat::LabelMe && !m_outputFilePath.isEmpty()) {
        return m_outputFilePath;
    }
    QString dir = m_saveDir.isEmpty() ? QFileInfo(imagePath).absolutePath() : m_saveDir;
    QString base = QFileInfo(imagePath).completeBaseName();
    if (m_format == SaveFormat::PascalVoc) return QDir(dir).filePath(base + ".xml");
    if (m_format == SaveFormat::Yolo) return QDir(dir).filePath(base + ".txt");
    return QDir(dir).filePath(base + ".json");
}

bool MainWindow::usesAnnotationPathOverride() const {
    return m_hasAnnotationPathOverride &&
           !m_annotationPathOverride.isEmpty() &&
           m_annotationOverrideFormat == m_format;
}

bool MainWindow::hasAnnotationForImage(const QString &imagePath) const {
    if (imagePath.isEmpty()) {
        return false;
    }
    const QFileInfo imageInfo(imagePath);
    QStringList directories;
    if (!m_saveDir.isEmpty()) {
        directories.append(QFileInfo(m_saveDir).absoluteFilePath());
    }
    directories.append(imageInfo.absolutePath());
    directories.removeDuplicates();
    const QString base = imageInfo.completeBaseName();
    for (const QString &directory : directories) {
        const QString prefix = QDir(directory).filePath(base);
        if (QFileInfo::exists(prefix + QStringLiteral(".xml")) ||
            QFileInfo::exists(prefix + QStringLiteral(".txt")) ||
            QFileInfo::exists(prefix + QStringLiteral(".json"))) {
            return true;
        }
    }
    return false;
}

void MainWindow::syncTopLevelFlagsEditor() {
    if (!m_topLevelFlagsEdit) {
        return;
    }
    QSignalBlocker blocker(m_topLevelFlagsEdit);
    m_topLevelFlagsEdit->setEnabled(m_format == SaveFormat::LabelMe);
    m_topLevelFlagsEdit->setPlainText(flagsToEditorText(m_labelMeTopLevelFlags));
    refreshTopLevelFlagsList();
}

void MainWindow::refreshTopLevelFlagsList() {
    if (!m_flagList) {
        return;
    }
    QSignalBlocker blocker(m_flagList);
    m_flagList->clear();
    QStringList keys = m_labelMeTopLevelFlags.keys();
    keys.sort(Qt::CaseInsensitive);
    for (const QString &key : keys) {
        auto *item = new QListWidgetItem(key, m_flagList);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(m_labelMeTopLevelFlags.value(key) ? Qt::Checked : Qt::Unchecked);
    }
    m_flagList->setEnabled(m_format == SaveFormat::LabelMe);
}

AnnotationDocument MainWindow::currentDocument() const {
    AnnotationDocument doc;
    doc.imagePath = m_filePath;
    const QSize canvasSize = m_canvas ? m_canvas->pixmapSize() : QSize();
    QImage image;
    doc.imageSize = canvasSize.isValid() && !canvasSize.isEmpty()
                        ? canvasSize
                        : (image = readImageWithAutoTransform(m_filePath)).size();
    doc.depth = (!image.isNull() && image.isGrayscale()) ? 1 : 3;
    doc.verified = m_verified;
    doc.shapes = m_canvas->shapes();
    if (m_format == SaveFormat::LabelMe) {
        if (!m_labelMeImagePath.isEmpty()) {
            doc.imagePath = m_labelMeImagePath;
        }
        doc.labelMeVersion = m_labelMeVersion;
        if (m_embedImageDataAction && m_embedImageDataAction->isChecked()) {
            doc.imageData = m_labelMeImageData;
            if (doc.imageData.isEmpty()) {
                QByteArray imageBytes;
                if (AnnotationIO::loadImageData(m_filePath, &imageBytes)) {
                    doc.imageData = QString::fromLatin1(imageBytes.toBase64());
                } else {
                    QFile imageFile(m_filePath);
                    if (imageFile.open(QIODevice::ReadOnly)) {
                        doc.imageData = QString::fromLatin1(imageFile.readAll().toBase64());
                    }
                }
            }
        } else {
            doc.imageData.clear();
        }
        doc.topLevelFlags = m_labelMeTopLevelFlags;
        doc.labelMeOtherData = m_labelMeOtherData;
    }
    return doc;
}

void MainWindow::syncModeFooterControls() {
    if (!m_footerModeCombo) {
        return;
    }
    QString mode = QStringLiteral("edit");
    if (m_viewModeAction && m_viewModeAction->isChecked()) {
        mode = QStringLiteral("view");
    } else if ((m_createModeAction && m_createModeAction->isChecked()) ||
               (m_createPolygonModeAction && m_createPolygonModeAction->isChecked()) ||
               (m_createPointModeAction && m_createPointModeAction->isChecked()) ||
               (m_createPointsModeAction && m_createPointsModeAction->isChecked()) ||
               (m_createAiPointsModeAction && m_createAiPointsModeAction->isChecked()) ||
               (m_createAiBoxModeAction && m_createAiBoxModeAction->isChecked()) ||
               (m_createLineModeAction && m_createLineModeAction->isChecked()) ||
               (m_createLinestripModeAction && m_createLinestripModeAction->isChecked()) ||
               (m_createCircleModeAction && m_createCircleModeAction->isChecked()) ||
               (m_createOrientedRectangleModeAction && m_createOrientedRectangleModeAction->isChecked()) ||
               (m_createMaskModeAction && m_createMaskModeAction->isChecked())) {
        mode = QStringLiteral("create");
    }
    const int index = m_footerModeCombo->findData(mode);
    if (index >= 0 && m_footerModeCombo->currentIndex() != index) {
        QSignalBlocker blocker(m_footerModeCombo);
        m_footerModeCombo->setCurrentIndex(index);
    }
    syncMainModeButton();
}

void MainWindow::syncFormatFooterControls() {
    if (!m_footerFormatCombo) {
        return;
    }
    const int index = static_cast<int>(m_format);
    if (m_footerFormatCombo->currentIndex() != index) {
        QSignalBlocker blocker(m_footerFormatCombo);
        m_footerFormatCombo->setCurrentIndex(index);
    }
}

void MainWindow::syncMainModeButton() {
    if (!m_mainModeButton) {
        return;
    }
    QAction *currentMode = m_editModeAction;
    if (m_viewModeAction && m_viewModeAction->isChecked()) {
        currentMode = m_viewModeAction;
    } else if (m_createPolygonModeAction && m_createPolygonModeAction->isChecked()) {
        currentMode = m_createPolygonModeAction;
    } else if (m_createPointModeAction && m_createPointModeAction->isChecked()) {
        currentMode = m_createPointModeAction;
    } else if (m_createPointsModeAction && m_createPointsModeAction->isChecked()) {
        currentMode = m_createPointsModeAction;
    } else if (m_createAiPointsModeAction && m_createAiPointsModeAction->isChecked()) {
        currentMode = m_createAiPointsModeAction;
    } else if (m_createAiBoxModeAction && m_createAiBoxModeAction->isChecked()) {
        currentMode = m_createAiBoxModeAction;
    } else if (m_createLineModeAction && m_createLineModeAction->isChecked()) {
        currentMode = m_createLineModeAction;
    } else if (m_createLinestripModeAction && m_createLinestripModeAction->isChecked()) {
        currentMode = m_createLinestripModeAction;
    } else if (m_createCircleModeAction && m_createCircleModeAction->isChecked()) {
        currentMode = m_createCircleModeAction;
    } else if (m_createOrientedRectangleModeAction && m_createOrientedRectangleModeAction->isChecked()) {
        currentMode = m_createOrientedRectangleModeAction;
    } else if (m_createMaskModeAction && m_createMaskModeAction->isChecked()) {
        currentMode = m_createMaskModeAction;
    } else if (m_createModeAction && m_createModeAction->isChecked()) {
        currentMode = m_createModeAction;
    }
    m_mainModeButton->setDefaultAction(currentMode);
    m_mainModeButton->setMenu(m_mainModeMenu);
    m_mainModeButton->setPopupMode(QToolButton::InstantPopup);
    m_mainModeButton->setToolTip(m_strings.get(QStringLiteral("modeButtonTooltip"))
                                    .arg(currentMode ? currentMode->text() : QString()));
}

void MainWindow::resetShapeHistory() {
    m_shapeHistoryPending = false;
    m_undoStack.clear();
    m_redoStack.clear();
    m_lastShapeSnapshot = m_canvas ? m_canvas->shapes() : QVector<Shape>();
    refreshActions();
}

void MainWindow::recordShapeHistory() {
    if (m_restoringHistory || !m_canvas) {
        return;
    }
    const QVector<Shape> current = m_canvas->shapes();
    if (sameShapesForHistory(current, m_lastShapeSnapshot)) {
        refreshActions();
        return;
    }
    m_undoStack.push_back(m_lastShapeSnapshot);
    if (m_undoStack.size() > m_labelMeNumBackups) {
        m_undoStack.removeFirst();
    }
    m_redoStack.clear();
    m_lastShapeSnapshot = current;
    refreshActions();
}

void MainWindow::restoreShapeHistorySnapshot(const QVector<Shape> &shapes) {
    if (!m_canvas) {
        return;
    }
    m_restoringHistory = true;
    m_canvas->setShapes(shapes);
    // LabelMe restores the shape snapshot without carrying the previous
    // canvas/label-list selection into the restored document.
    m_canvas->setSelectedIndices({});
    m_restoringHistory = false;
    m_lastShapeSnapshot = shapes;
    refreshLabels();
    setDirty(true);
    refreshActions();
}

void MainWindow::undoShapeOperation() {
    if (m_undoStack.isEmpty()) {
        return;
    }
    m_redoStack.push_back(m_canvas->shapes());
    const QVector<Shape> previous = m_undoStack.takeLast();
    restoreShapeHistorySnapshot(previous);
}

void MainWindow::redoShapeOperation() {
    if (m_redoStack.isEmpty()) {
        return;
    }
    m_undoStack.push_back(m_canvas->shapes());
    const QVector<Shape> next = m_redoStack.takeLast();
    restoreShapeHistorySnapshot(next);
}

void MainWindow::refreshWindowTitle() {
    QString title = QStringLiteral("labelImgCpp");
    if (!m_filePath.isEmpty()) {
        QString displayedPath = m_filePath;
        if (m_dirPath.isEmpty() && m_hasAnnotationPathOverride && !m_annotationPathOverride.isEmpty()) {
            displayedPath = m_annotationPathOverride;
        }
        title += QLatin1Char(' ') + displayedPath;
    }
    if (m_dirty) {
        title += QLatin1Char('*');
    }
    setWindowTitle(title);
    updateFramelessChrome();
}

void MainWindow::setDirty(bool dirty) {
    m_dirty = dirty;
    refreshWindowTitle();
    if (dirty && m_autoSaveAction && m_autoSaveAction->isChecked() &&
        !m_filePath.isEmpty() && !m_autoSavePending) {
        m_autoSavePending = true;
        QTimer::singleShot(150, this, [this]() {
            m_autoSavePending = false;
            if (m_dirty && m_autoSaveAction && m_autoSaveAction->isChecked() && !m_filePath.isEmpty()) {
                saveFile();
            }
        });
    }
    refreshActions();
}

bool MainWindow::maybeSave() {
    if (!m_dirty) return true;
    if (m_autoSaveAction && m_autoSaveAction->isChecked()) {
        return saveCurrentFile();
    }
    QMessageBox box(QMessageBox::Question, QStringLiteral("labelImgCpp"),
                    m_strings.get(QStringLiteral("saveChanges")),
                    QMessageBox::NoButton, this);
    QPushButton *yesButton = box.addButton(QMessageBox::Yes);
    QPushButton *noButton = box.addButton(QMessageBox::No);
    QPushButton *cancelButton = box.addButton(QMessageBox::Cancel);
    QPushButton *autoSaveButton = box.addButton(m_strings.get(QStringLiteral("autoSavePrompt")), QMessageBox::AcceptRole);
    autoSaveButton->setObjectName(QStringLiteral("autoSavePromptButton"));
    box.setDefaultButton(yesButton);
    box.exec();

    QAbstractButton *clicked = box.clickedButton();
    if (clicked == cancelButton) return false;
    if (clicked == yesButton) return saveCurrentFile();
    if (clicked == autoSaveButton) {
        if (m_autoSaveAction) {
            m_autoSaveAction->setChecked(true);
            m_settings.setValue("autosave", true);
        }
        return saveCurrentFile();
    }
    Q_UNUSED(noButton);
    return true;
}

QStringList MainWindow::scanImages(const QString &dirPath) const {
    QStringList result;
    QDirIterator it(dirPath, supportedImageNameFilters(), QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) result.append(QFileInfo(it.next()).absoluteFilePath());
    std::sort(result.begin(), result.end(), naturalPathLess);
    return result;
}

QString MainWindow::currentFormatName() const {
    if (m_format == SaveFormat::PascalVoc) return "PascalVOC";
    if (m_format == SaveFormat::Yolo) return "YOLO";
    if (m_format == SaveFormat::CreateMl) return "CreateML";
    return "LabelMe";
}

void MainWindow::updatePerformanceLabel() {
    if (!m_performanceLabel || !m_canvas) {
        return;
    }
    const QString sampling = m_canvas->samplingMode() == Canvas::SamplingMode::Smooth
                                 ? m_strings.get("smoothSampling")
                                 : m_strings.get("fastSampling");
    m_performanceLabel->setText(QStringLiteral("%1 | FPS %2 | Scale %3% | %4")
                                    .arg(m_resourcePerformanceText)
                                    .arg(m_displayFps, 0, 'f', 1)
                                    .arg(m_canvas->scale() * 100.0, 0, 'f', 1)
                                    .arg(sampling));
    if (m_samplingModeAction) {
        m_samplingModeAction->setText(m_samplingModeAction->isChecked()
                                          ? m_strings.get("smoothSampling")
                                          : m_strings.get("fastSampling"));
    }
}
