#include "core/ShortcutRegistry.h"

#include <QHash>

#include <algorithm>

namespace {
QString settingsKey(const QString &commandId) {
    return QStringLiteral("shortcuts/") + commandId;
}

QString portableText(const QKeySequence &sequence) {
    return sequence.toString(QKeySequence::PortableText);
}
}

bool ShortcutRegistry::addCommand(const ShortcutCommand &command) {
    if (command.id.trimmed().isEmpty() || contains(command.id)) {
        return false;
    }

    bool valid = false;
    const QList<QKeySequence> defaults = normalized(command.defaults, &valid);
    if (!valid) {
        return false;
    }

    ShortcutCommand stored = command;
    stored.id = stored.id.trimmed();
    stored.category = stored.category.trimmed();
    stored.defaults = defaults;
    stored.shortcuts = defaults;
    m_commands.append(stored);
    return true;
}

bool ShortcutRegistry::contains(const QString &commandId) const {
    return indexOf(commandId) >= 0;
}

ShortcutCommand ShortcutRegistry::command(const QString &commandId) const {
    const int index = indexOf(commandId);
    return index >= 0 ? m_commands.at(index) : ShortcutCommand{};
}

QVector<ShortcutCommand> ShortcutRegistry::commands() const {
    return m_commands;
}

bool ShortcutRegistry::setShortcuts(const QString &commandId, const QList<QKeySequence> &shortcuts) {
    const int index = indexOf(commandId);
    if (index < 0) {
        return false;
    }

    bool valid = false;
    const QList<QKeySequence> values = normalized(shortcuts, &valid);
    if (!valid) {
        return false;
    }
    m_commands[index].shortcuts = values;
    return true;
}

bool ShortcutRegistry::setDefaults(const QString &commandId, const QList<QKeySequence> &defaults,
                                   bool resetCurrent) {
    const int index = indexOf(commandId);
    if (index < 0) {
        return false;
    }

    bool valid = false;
    const QList<QKeySequence> values = normalized(defaults, &valid);
    if (!valid) {
        return false;
    }
    m_commands[index].defaults = values;
    if (resetCurrent) {
        m_commands[index].shortcuts = values;
    }
    return true;
}

void ShortcutRegistry::reset(const QString &commandId) {
    const int index = indexOf(commandId);
    if (index >= 0) {
        m_commands[index].shortcuts = m_commands[index].defaults;
    }
}

void ShortcutRegistry::resetAll() {
    for (ShortcutCommand &command : m_commands) {
        command.shortcuts = command.defaults;
    }
}

QVector<ShortcutConflict> ShortcutRegistry::conflicts() const {
    QHash<QString, ShortcutConflict> bySequence;
    for (const ShortcutCommand &command : m_commands) {
        for (int slot = 0; slot < command.shortcuts.size(); ++slot) {
            const QKeySequence &sequence = command.shortcuts.at(slot);
            if (sequence.isEmpty()) {
                continue;
            }
            const QString key = portableText(sequence);
            ShortcutConflict &conflict = bySequence[key];
            conflict.sequence = sequence;
            conflict.locations.append({command.id, slot});
        }
    }

    QVector<ShortcutConflict> result;
    for (auto it = bySequence.cbegin(); it != bySequence.cend(); ++it) {
        if (it.value().locations.size() > 1) {
            result.append(it.value());
        }
    }
    std::sort(result.begin(), result.end(), [](const ShortcutConflict &left, const ShortcutConflict &right) {
        return portableText(left.sequence) < portableText(right.sequence);
    });
    return result;
}

bool ShortcutRegistry::hasConflicts() const {
    return !conflicts().isEmpty();
}

void ShortcutRegistry::loadOverrides(QSettings &settings) {
    for (ShortcutCommand &command : m_commands) {
        const QString key = settingsKey(command.id);
        if (!settings.contains(key)) {
            command.shortcuts = command.defaults;
            continue;
        }

        QList<QKeySequence> values;
        const QStringList stored = settings.value(key).toStringList();
        for (const QString &text : stored) {
            values.append(QKeySequence(text, QKeySequence::PortableText));
        }
        bool valid = false;
        const QList<QKeySequence> normalizedValues = normalized(values, &valid);
        command.shortcuts = valid ? normalizedValues : command.defaults;
    }
}

void ShortcutRegistry::saveOverrides(QSettings &settings) const {
    for (const ShortcutCommand &command : m_commands) {
        const QString key = settingsKey(command.id);
        if (command.shortcuts == command.defaults) {
            settings.remove(key);
            continue;
        }

        QStringList stored;
        for (const QKeySequence &sequence : command.shortcuts) {
            stored.append(portableText(sequence));
        }
        settings.setValue(key, stored);
    }
    settings.sync();
}

QList<QKeySequence> ShortcutRegistry::normalized(const QList<QKeySequence> &shortcuts, bool *ok) {
    bool valid = shortcuts.size() <= 2;
    QList<QKeySequence> result;
    if (valid) {
        for (const QKeySequence &sequence : shortcuts) {
            if (sequence.isEmpty()) {
                continue;
            }
            if (sequence.count() != 1) {
                valid = false;
                result.clear();
                break;
            }
            const QKeySequence normalizedSequence(portableText(sequence), QKeySequence::PortableText);
            if (normalizedSequence.isEmpty() || normalizedSequence.count() != 1) {
                valid = false;
                result.clear();
                break;
            }
            result.append(normalizedSequence);
        }
    }
    if (ok) {
        *ok = valid;
    }
    return valid ? result : QList<QKeySequence>{};
}

int ShortcutRegistry::indexOf(const QString &commandId) const {
    for (int index = 0; index < m_commands.size(); ++index) {
        if (m_commands.at(index).id == commandId) {
            return index;
        }
    }
    return -1;
}
