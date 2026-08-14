#pragma once

#include <QKeySequence>
#include <QList>
#include <QSettings>
#include <QString>
#include <QVector>

struct ShortcutCommand {
    QString id;
    QString category;
    QList<QKeySequence> defaults;
    QList<QKeySequence> shortcuts;
};

struct ShortcutLocation {
    QString commandId;
    int slot = -1;
};

struct ShortcutConflict {
    QKeySequence sequence;
    QVector<ShortcutLocation> locations;
};

class ShortcutRegistry {
public:
    bool addCommand(const ShortcutCommand &command);
    bool contains(const QString &commandId) const;
    ShortcutCommand command(const QString &commandId) const;
    QVector<ShortcutCommand> commands() const;

    bool setShortcuts(const QString &commandId, const QList<QKeySequence> &shortcuts);
    bool setDefaults(const QString &commandId, const QList<QKeySequence> &defaults,
                     bool resetCurrent = true);
    void reset(const QString &commandId);
    void resetAll();

    QVector<ShortcutConflict> conflicts() const;
    bool hasConflicts() const;

    void loadOverrides(QSettings &settings);
    void saveOverrides(QSettings &settings) const;

    static QList<QKeySequence> normalized(const QList<QKeySequence> &shortcuts, bool *ok = nullptr);

private:
    int indexOf(const QString &commandId) const;

    QVector<ShortcutCommand> m_commands;
};
