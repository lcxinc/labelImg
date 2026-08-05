#include "core/LabelMeConfig.h"

#include <QFile>
#include <QHash>
#include <QMetaType>
#include <QSaveFile>
#include <QRegularExpression>
#include <QVector>

namespace {
struct PrefixFrame {
    int indent = 0;
    QString prefix;
};

QString withoutComment(const QString &value) {
    bool quoted = false;
    QChar quote;
    for (int i = 0; i < value.size(); ++i) {
        const QChar character = value.at(i);
        if ((character == QLatin1Char('\'') || character == QLatin1Char('"'))) {
            if (!quoted) {
                quoted = true;
                quote = character;
            } else if (quote == character) {
                quoted = false;
            }
        } else if (character == QLatin1Char('#') && !quoted &&
                   (i == 0 || value.at(i - 1).isSpace())) {
            return value.left(i).trimmed();
        }
    }
    return value.trimmed();
}

QStringList splitFlowValues(const QString &contents, QChar delimiter) {
    QStringList result;
    QString token;
    int nesting = 0;
    bool quoted = false;
    QChar quote;
    for (const QChar character : contents) {
        if ((character == QLatin1Char('\'') || character == QLatin1Char('"'))) {
            if (!quoted) {
                quoted = true;
                quote = character;
            } else if (quote == character) {
                quoted = false;
            }
        } else if (!quoted) {
            if (character == QLatin1Char('[') || character == QLatin1Char('{')) {
                ++nesting;
            } else if (character == QLatin1Char(']') || character == QLatin1Char('}')) {
                nesting = qMax(0, nesting - 1);
            }
        }
        if (character == delimiter && !quoted && nesting == 0) {
            result.append(token.trimmed());
            token.clear();
        } else {
            token += character;
        }
    }
    if (!token.trimmed().isEmpty()) {
        result.append(token.trimmed());
    }
    return result;
}

QVariant parseScalar(const QString &raw) {
    const QString value = withoutComment(raw);
    if (value.size() >= 2 &&
        ((value.startsWith(QLatin1Char('\'')) && value.endsWith(QLatin1Char('\''))) ||
         (value.startsWith(QLatin1Char('"')) && value.endsWith(QLatin1Char('"'))))) {
        return value.mid(1, value.size() - 2);
    }
    if (value.compare(QStringLiteral("true"), Qt::CaseInsensitive) == 0) {
        return true;
    }
    if (value.compare(QStringLiteral("false"), Qt::CaseInsensitive) == 0) {
        return false;
    }
    if (value.compare(QStringLiteral("null"), Qt::CaseInsensitive) == 0 ||
        value.compare(QStringLiteral("~")) == 0) {
        return QVariant();
    }
    if (value.startsWith(QLatin1Char('{')) && value.endsWith(QLatin1Char('}'))) {
        QVariantMap map;
        for (const QString &entry : splitFlowValues(value.mid(1, value.size() - 2), QLatin1Char(','))) {
            const QStringList pair = splitFlowValues(entry, QLatin1Char(':'));
            if (pair.size() < 2) {
                return value;
            }
            const QString key = parseScalar(pair.first()).toString().trimmed();
            if (key.isEmpty()) {
                return value;
            }
            const int separator = entry.indexOf(QLatin1Char(':'));
            map.insert(key, parseScalar(entry.mid(separator + 1)));
        }
        return map;
    }
    if (value.startsWith(QLatin1Char('[')) && value.endsWith(QLatin1Char(']'))) {
        QVariantList list;
        const QString contents = value.mid(1, value.size() - 2);
        for (const QString &token : splitFlowValues(contents, QLatin1Char(','))) {
            if (!token.isEmpty()) {
                list.append(parseScalar(token));
            }
        }
        return list;
    }

    bool integerOk = false;
    const qlonglong integer = value.toLongLong(&integerOk);
    if (integerOk) {
        return integer;
    }
    bool doubleOk = false;
    const double floatingPoint = value.toDouble(&doubleOk);
    if (doubleOk) {
        return floatingPoint;
    }
    return value;
}

void insertDottedValue(QVariantMap *root, const QStringList &parts, const QVariant &value, int index = 0) {
    if (!root || index >= parts.size()) {
        return;
    }
    const QString part = parts.at(index);
    if (index == parts.size() - 1) {
        root->insert(part, value);
        return;
    }
    QVariantMap child = root->value(part).toMap();
    insertDottedValue(&child, parts, value, index + 1);
    root->insert(part, child);
}

QString yamlString(const QString &value) {
    const QString trimmed = value.trimmed();
    const QString lower = value.toLower();
    const bool needsQuotes = value.isEmpty() || value != trimmed ||
                             value.contains(QRegularExpression(QStringLiteral("[:#\\[\\]{}&,]"))) ||
                             value.startsWith(QLatin1Char('-')) || value.startsWith(QLatin1Char('!')) ||
                             lower == QStringLiteral("true") || lower == QStringLiteral("false") ||
                             lower == QStringLiteral("null") || lower == QStringLiteral("~") ||
                             lower == QStringLiteral("yes") || lower == QStringLiteral("no") ||
                             lower == QStringLiteral("on") || lower == QStringLiteral("off") ||
                             lower == QStringLiteral("y") || lower == QStringLiteral("n") ||
                             value.contains(QLatin1Char('\n'));
    if (!needsQuotes) {
        return value;
    }
    QString escaped = value;
    escaped.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
    escaped.replace(QLatin1Char('"'), QStringLiteral("\\\""));
    escaped.replace(QLatin1Char('\n'), QStringLiteral("\\n"));
    return QLatin1Char('"') + escaped + QLatin1Char('"');
}

QString yamlValue(const QVariant &value) {
    if (!value.isValid() || value.isNull()) {
        return QStringLiteral("null");
    }
    if (value.typeId() == QMetaType::Bool) {
        return value.toBool() ? QStringLiteral("true") : QStringLiteral("false");
    }
    if (value.typeId() == QMetaType::QStringList || value.typeId() == QMetaType::QVariantList) {
        QStringList parts;
        const QVariantList list = value.typeId() == QMetaType::QStringList
                                      ? [&]() {
                                            QVariantList result;
                                            for (const QString &item : value.toStringList()) result.append(item);
                                            return result;
                                        }()
                                      : value.toList();
        for (const QVariant &item : list) {
            parts.append(yamlValue(item));
        }
        return QLatin1Char('[') + parts.join(QStringLiteral(", ")) + QLatin1Char(']');
    }
    if (value.typeId() == QMetaType::QString) {
        return yamlString(value.toString());
    }
    return value.toString();
}

QVariant intList(std::initializer_list<int> values) {
    QVariantList result;
    for (const int value : values) {
        result.append(value);
    }
    return result;
}

QVariant stringListValue(std::initializer_list<QString> values) {
    QVariantList result;
    for (const QString &value : values) {
        result.append(value);
    }
    return result;
}

const QHash<QString, QVariant> &defaultValues() {
    static const QHash<QString, QVariant> values = [] {
        QHash<QString, QVariant> result;
        const auto add = [&result](const QString &key, const QVariant &value) {
            result.insert(key, value);
        };
        const auto addNull = [&result](const QString &key) {
            result.insert(key, QVariant());
        };

        add(QStringLiteral("auto_save"), true);
        add(QStringLiteral("display_label_popup"), true);
        add(QStringLiteral("with_image_data"), false);
        add(QStringLiteral("keep_prev"), false);
        add(QStringLiteral("keep_prev_scale"), false);
        add(QStringLiteral("keep_prev_brightness_contrast"), false);
        add(QStringLiteral("logger_level"), QStringLiteral("info"));
        addNull(QStringLiteral("language"));
        addNull(QStringLiteral("flags"));
        addNull(QStringLiteral("label_flags"));
        addNull(QStringLiteral("labels"));
        addNull(QStringLiteral("file_search"));
        add(QStringLiteral("sort_labels"), true);
        addNull(QStringLiteral("validate_label"));
        add(QStringLiteral("default_shape_color"), intList({0, 255, 0}));
        add(QStringLiteral("shape_color"), QStringLiteral("auto"));
        add(QStringLiteral("shift_auto_shape_color"), 0);
        addNull(QStringLiteral("label_colors"));

        add(QStringLiteral("shape.line_color"), intList({0, 255, 0, 128}));
        add(QStringLiteral("shape.fill_color"), intList({0, 0, 0, 64}));
        add(QStringLiteral("shape.vertex_fill_color"), intList({0, 255, 0, 255}));
        add(QStringLiteral("shape.select_line_color"), intList({255, 255, 255, 255}));
        add(QStringLiteral("shape.select_fill_color"), intList({0, 255, 0, 64}));
        add(QStringLiteral("shape.hvertex_fill_color"), intList({255, 255, 255, 255}));
        add(QStringLiteral("shape.point_size"), 8);
        add(QStringLiteral("ai.default"), QStringLiteral("Sam2 (balanced)"));

        for (const QString &dock : {QStringLiteral("flag_dock"), QStringLiteral("label_dock"),
                                     QStringLiteral("shape_dock"), QStringLiteral("file_dock")}) {
            add(dock + QStringLiteral(".show"), true);
            add(dock + QStringLiteral(".closable"), true);
            add(dock + QStringLiteral(".movable"), true);
            add(dock + QStringLiteral(".floatable"), true);
        }
        add(QStringLiteral("show_label_text_field"), true);
        add(QStringLiteral("label_completion"), QStringLiteral("startswith"));
        add(QStringLiteral("fit_to_content.column"), true);
        add(QStringLiteral("fit_to_content.row"), false);
        add(QStringLiteral("epsilon"), 10.0);

        add(QStringLiteral("canvas.fill_drawing"), true);
        add(QStringLiteral("canvas.double_click"), QStringLiteral("close"));
        add(QStringLiteral("canvas.snapping"), true);
        add(QStringLiteral("canvas.num_backups"), 10);
        add(QStringLiteral("canvas.crosshair.polygon"), false);
        add(QStringLiteral("canvas.crosshair.rectangle"), true);
        add(QStringLiteral("canvas.crosshair.oriented_rectangle"), false);
        add(QStringLiteral("canvas.crosshair.circle"), false);
        add(QStringLiteral("canvas.crosshair.line"), false);
        add(QStringLiteral("canvas.crosshair.point"), false);
        add(QStringLiteral("canvas.crosshair.linestrip"), false);
        add(QStringLiteral("canvas.crosshair.points"), false);
        add(QStringLiteral("canvas.crosshair.mask"), false);
        add(QStringLiteral("canvas.crosshair.ai_points_to_shape"), false);
        add(QStringLiteral("canvas.crosshair.ai_box_to_shape"), true);

        add(QStringLiteral("shortcuts.close"), QStringLiteral("Ctrl+W"));
        add(QStringLiteral("shortcuts.open"), QStringLiteral("Ctrl+O"));
        add(QStringLiteral("shortcuts.open_dir"), QStringLiteral("Ctrl+U"));
        add(QStringLiteral("shortcuts.quit"), QStringLiteral("Ctrl+Q"));
        add(QStringLiteral("shortcuts.save"), QStringLiteral("Ctrl+S"));
        add(QStringLiteral("shortcuts.save_as"), QStringLiteral("Ctrl+Shift+S"));
        addNull(QStringLiteral("shortcuts.save_to"));
        add(QStringLiteral("shortcuts.delete_file"), QStringLiteral("Ctrl+Delete"));
        add(QStringLiteral("shortcuts.open_next"), stringListValue({QStringLiteral("D"), QStringLiteral("Ctrl+Shift+D")}));
        add(QStringLiteral("shortcuts.open_prev"), stringListValue({QStringLiteral("A"), QStringLiteral("Ctrl+Shift+A")}));
        add(QStringLiteral("shortcuts.zoom_in"), stringListValue({QStringLiteral("Ctrl++"), QStringLiteral("Ctrl+=")}));
        add(QStringLiteral("shortcuts.zoom_out"), QStringLiteral("Ctrl+-"));
        add(QStringLiteral("shortcuts.zoom_to_original"), QStringLiteral("Ctrl+0"));
        add(QStringLiteral("shortcuts.fit_window"), QStringLiteral("Ctrl+F"));
        add(QStringLiteral("shortcuts.fit_width"), QStringLiteral("Ctrl+Shift+F"));
        add(QStringLiteral("shortcuts.create_polygon"), QStringLiteral("Ctrl+N"));
        add(QStringLiteral("shortcuts.create_rectangle"), QStringLiteral("Ctrl+R"));
        for (const QString &key : {QStringLiteral("create_oriented_rectangle"), QStringLiteral("create_circle"),
                                   QStringLiteral("create_line"), QStringLiteral("create_point"),
                                   QStringLiteral("create_linestrip"), QStringLiteral("create_points"),
                                   QStringLiteral("create_mask"), QStringLiteral("create_ai_points"),
                                   QStringLiteral("create_ai_box"), QStringLiteral("show_all_shapes"),
                                   QStringLiteral("hide_all_shapes")}) {
            addNull(QStringLiteral("shortcuts.") + key);
        }
        add(QStringLiteral("shortcuts.edit_shape"), QStringLiteral("Ctrl+J"));
        add(QStringLiteral("shortcuts.delete_shape"), QStringLiteral("Delete"));
        add(QStringLiteral("shortcuts.duplicate_shape"), QStringLiteral("Ctrl+D"));
        add(QStringLiteral("shortcuts.copy_shape"), QStringLiteral("Ctrl+C"));
        add(QStringLiteral("shortcuts.paste_shape"), QStringLiteral("Ctrl+V"));
        add(QStringLiteral("shortcuts.undo"), QStringLiteral("Ctrl+Z"));
        add(QStringLiteral("shortcuts.undo_last_point"), QStringLiteral("Ctrl+Z"));
        add(QStringLiteral("shortcuts.edit_label"), QStringLiteral("Ctrl+E"));
        add(QStringLiteral("shortcuts.toggle_keep_prev_mode"), QStringLiteral("Ctrl+P"));
        add(QStringLiteral("shortcuts.remove_selected_point"), stringListValue({QStringLiteral("Meta+H"), QStringLiteral("Backspace")}));
        add(QStringLiteral("shortcuts.toggle_all_shapes"), QStringLiteral("T"));
        return result;
    }();
    return values;
}

bool assignDottedValue(QVariantMap *root, const QStringList &parts, const QVariant &value,
                       QString *error, int index = 0) {
    if (!root || index < 0 || index >= parts.size()) {
        if (error) *error = QStringLiteral("Invalid config key path");
        return false;
    }
    const QString part = parts.at(index);
    if (index == parts.size() - 1) {
        root->insert(part, value);
        return true;
    }
    if (root->contains(part)) {
        const QVariant existing = root->value(part);
        if (existing.isValid() && !existing.isNull() &&
            existing.typeId() != QMetaType::QVariantMap) {
            if (error) {
                *error = QStringLiteral("Config key %1 conflicts with a non-mapping value at %2")
                             .arg(parts.join(QLatin1Char('.')), part);
            }
            return false;
        }
    }
    QVariantMap child = root->value(part).toMap();
    if (!assignDottedValue(&child, parts, value, error, index + 1)) {
        return false;
    }
    root->insert(part, child);
    return true;
}

void pruneDottedValue(QVariantMap *root, const QStringList &parts, int index = 0) {
    if (!root || index < 0 || index >= parts.size() || !root->contains(parts.at(index))) {
        return;
    }
    const QString part = parts.at(index);
    if (index == parts.size() - 1) {
        root->remove(part);
        return;
    }
    QVariantMap child = root->value(part).toMap();
    pruneDottedValue(&child, parts, index + 1);
    if (child.isEmpty()) {
        root->remove(part);
    } else {
        root->insert(part, child);
    }
}

void appendYamlMap(QStringList *lines, const QVariantMap &map, int indent) {
    if (!lines) {
        return;
    }
    const QString padding(indent, QLatin1Char(' '));
    for (auto it = map.cbegin(); it != map.cend(); ++it) {
        const QVariantMap child = it.value().toMap();
        if (it.value().typeId() == QMetaType::QVariantMap && !child.isEmpty()) {
            lines->append(padding + it.key() + QLatin1Char(':'));
            appendYamlMap(lines, child, indent + 2);
        } else {
            lines->append(padding + it.key() + QStringLiteral(": ") + yamlValue(it.value()));
        }
    }
}

bool rootIsNonMapping(const QByteArray &data) {
    QStringList lines = QString::fromUtf8(data).split(QLatin1Char('\n'));
    for (int lineNumber = 0; lineNumber < lines.size(); ++lineNumber) {
        QString raw = lines.at(lineNumber);
        if (lineNumber == 0 && raw.startsWith(QChar::ByteOrderMark)) {
            raw.remove(0, 1);
        }
        int indent = 0;
        while (indent < raw.size() && raw.at(indent) == QLatin1Char(' ')) {
            ++indent;
        }
        const QString line = raw.mid(indent).trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')) ||
            line == QStringLiteral("---")) {
            continue;
        }
        if (indent != 0) {
            return false;
        }
        if (line == QStringLiteral("-") || line.startsWith(QStringLiteral("- ")) ||
            line.startsWith(QStringLiteral("-["))) {
            return true;
        }
        return line.indexOf(QLatin1Char(':')) <= 0;
    }
    return false;
}
}

namespace LabelMeConfig {

namespace {
void flattenConfigMap(const QVariantMap &source,
                      const QString &prefix,
                      QVariantMap *target) {
    if (!target) {
        return;
    }
    for (auto it = source.cbegin(); it != source.cend(); ++it) {
        const QString key = prefix.isEmpty()
                                ? it.key()
                                : prefix + QLatin1Char('.') + it.key();
        if (it.value().typeId() == QMetaType::QVariantMap &&
            !it.value().toMap().isEmpty()) {
            flattenConfigMap(it.value().toMap(), key, target);
        } else {
            target->insert(key, it.value());
        }
    }
}

bool parseConfigText(const QString &text, QVariantMap *values, QString *error) {
    if (!values) {
        if (error) *error = QStringLiteral("Output config map is null");
        return false;
    }

    values->clear();
    const QString trimmedText = text.trimmed();
    if (trimmedText.startsWith(QLatin1Char('{')) &&
        trimmedText.endsWith(QLatin1Char('}'))) {
        const QVariant parsed = parseScalar(trimmedText);
        if (parsed.typeId() != QMetaType::QVariantMap) {
            if (error) *error = QStringLiteral("Inline config must be a YAML mapping");
            return false;
        }
        flattenConfigMap(parsed.toMap(), QString(), values);
        return true;
    }

    QVector<PrefixFrame> stack;
    const QStringList lines = text.split(QLatin1Char('\n'));
    for (int lineNumber = 0; lineNumber < lines.size(); ++lineNumber) {
        QString raw = lines.at(lineNumber);
        if (lineNumber == 0 && raw.startsWith(QChar::ByteOrderMark)) {
            raw.remove(0, 1);
        }
        int indent = 0;
        while (indent < raw.size() && raw.at(indent) == QLatin1Char(' ')) {
            ++indent;
        }
        const QString line = raw.mid(indent).trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')) || line == QStringLiteral("---")) {
            continue;
        }
        while (!stack.isEmpty() && indent <= stack.last().indent) {
            stack.removeLast();
        }

        if (line.startsWith(QStringLiteral("- "))) {
            if (stack.isEmpty()) {
                if (error) *error = QStringLiteral("List item without a key at line %1").arg(lineNumber + 1);
                return false;
            }
            const QString key = stack.last().prefix;
            QVariantList list = values->value(key).toList();
            list.append(parseScalar(line.mid(2)));
            values->insert(key, list);
            continue;
        }

        const int separator = line.indexOf(QLatin1Char(':'));
        if (separator <= 0) {
            if (error) *error = QStringLiteral("Invalid config line %1").arg(lineNumber + 1);
            return false;
        }
        const QString key = line.left(separator).trimmed();
        if (key.isEmpty()) {
            if (error) *error = QStringLiteral("Empty config key at line %1").arg(lineNumber + 1);
            return false;
        }
        const QString prefix = stack.isEmpty() ? key : stack.last().prefix + QLatin1Char('.') + key;
        const QString value = withoutComment(line.mid(separator + 1));
        if (value.isEmpty()) {
            stack.push_back({indent, prefix});
        } else {
            values->insert(prefix, parseScalar(value));
        }
    }
    return true;
}
}

bool loadFile(const QString &path, QVariantMap *values, QString *error) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error) *error = QStringLiteral("Cannot open config file: %1").arg(path);
        return false;
    }
    return parseConfigText(QString::fromUtf8(file.readAll()), values, error);
}

bool loadText(const QString &text, QVariantMap *values, QString *error) {
    return parseConfigText(text, values, error);
}

bool setFileValue(const QString &path, const QString &key, const QVariant &value, QString *error) {
    const QString normalizedKey = key.trimmed();
    if (normalizedKey.isEmpty() || normalizedKey.startsWith(QLatin1Char('.')) ||
        normalizedKey.endsWith(QLatin1Char('.'))) {
        if (error) *error = QStringLiteral("Invalid config key: %1").arg(key);
        return false;
    }

    const QStringList parts = normalizedKey.split(QLatin1Char('.'), Qt::SkipEmptyParts);
    const auto defaults = defaultValues();
    if (!defaults.contains(normalizedKey)) {
        if (error) *error = QStringLiteral("Unknown config key: %1").arg(normalizedKey);
        return false;
    }

    QByteArray sourceData;
    const bool replaceNonMappingRoot = QFile::exists(path) && [&]() {
        QFile source(path);
        if (!source.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return false;
        }
        sourceData = source.readAll();
        return rootIsNonMapping(sourceData);
    }();

    QStringList preservedComments;
    if (QFile::exists(path)) {
        QFile source(path);
        if (source.open(QIODevice::ReadOnly | QIODevice::Text)) {
            if (!replaceNonMappingRoot) {
                if (sourceData.isEmpty()) {
                    sourceData = source.readAll();
                }
                const QStringList sourceLines = QString::fromUtf8(sourceData).split(QLatin1Char('\n'));
                for (const QString &sourceLine : sourceLines) {
                    if (sourceLine.trimmed().startsWith(QLatin1Char('#'))) {
                        preservedComments.append(sourceLine);
                    }
                }
            }
        }
    }

    QVariantMap values;
    if (QFile::exists(path) && !replaceNonMappingRoot) {
        QString loadError;
        if (!loadFile(path, &values, &loadError)) {
            if (error) *error = loadError;
            return false;
        }
    }
    QVariantMap nestedValues;
    for (auto it = values.cbegin(); it != values.cend(); ++it) {
        if (!assignDottedValue(&nestedValues,
                               it.key().split(QLatin1Char('.'), Qt::SkipEmptyParts),
                               it.value(), error)) {
            return false;
        }
    }

    const QVariant defaultValue = defaults.value(normalizedKey);
    const bool restoreDefault = defaultValue.isValid() == value.isValid() &&
                                (!defaultValue.isValid() || defaultValue == value);
    if (restoreDefault) {
        pruneDottedValue(&nestedValues, parts);
    } else if (!assignDottedValue(&nestedValues, parts, value, error)) {
        return false;
    }
    values = nestedValues;

    QStringList lines;
    appendYamlMap(&lines, values, 0);
    if (!lines.isEmpty() && !preservedComments.isEmpty()) {
        preservedComments.append(lines);
        lines = preservedComments;
    }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error) *error = QStringLiteral("Cannot write config file: %1").arg(path);
        return false;
    }
    const QByteArray content = lines.isEmpty()
                                   ? QByteArray()
                                   : (lines.join(QLatin1Char('\n')) + QLatin1Char('\n')).toUtf8();
    if (file.write(content) != content.size() || !file.commit()) {
        if (error) *error = QStringLiteral("Cannot commit config file: %1").arg(path);
        return false;
    }
    return true;
}

QStringList stringList(const QVariant &value) {
    if (value.typeId() == QMetaType::QStringList) {
        return value.toStringList();
    }
    if (value.typeId() == QMetaType::QVariantList) {
        QStringList result;
        for (const QVariant &item : value.toList()) {
            const QString text = item.toString().trimmed();
            if (!text.isEmpty()) result.append(text);
        }
        return result;
    }
    const QString text = value.toString().trimmed();
    if (text.isEmpty()) return {};
    return text.split(QRegularExpression(QStringLiteral("[,\\n]")), Qt::SkipEmptyParts);
}

QString migrateAiModelName(const QString &modelName) {
    static const QRegularExpression legacyPattern(QStringLiteral("^SegmentAnything \\((.*)\\)$"));
    const QRegularExpressionMatch match = legacyPattern.match(modelName);
    if (!match.hasMatch()) {
        return modelName;
    }
    return QStringLiteral("Sam (") + match.captured(1) + QLatin1Char(')');
}

}
