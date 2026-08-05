#pragma once

#include <QVariant>
#include <QVariantMap>
#include <QStringList>

namespace LabelMeConfig {

// Reads the scalar/list YAML options used by LabelMe's startup configuration.
// Unknown nested options are retained as dotted keys so callers can opt into
// more settings without coupling the parser to the entire LabelMe schema.
bool loadFile(const QString &path, QVariantMap *values, QString *error = nullptr);

// Parses the same lightweight YAML subset from an inline --config value.
bool loadText(const QString &text, QVariantMap *values, QString *error = nullptr);

// Updates one dotted YAML key and atomically rewrites the lightweight config.
bool setFileValue(const QString &path, const QString &key, const QVariant &value,
                  QString *error = nullptr);

QStringList stringList(const QVariant &value);

// Keeps the config migration used by LabelMe for pre-Sam model names.
QString migrateAiModelName(const QString &modelName);

}
