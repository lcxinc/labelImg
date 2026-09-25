#include "core/ResourcePaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QtResource>

static void initializeBundledResources() {
    Q_INIT_RESOURCE(labelimg_assets);
}

namespace {
bool hasLabelImgResources(const QString &candidate) {
    QDir dir(candidate);
    return QFileInfo::exists(dir.filePath(QStringLiteral("resources/strings/strings.properties"))) &&
           QFileInfo::exists(dir.filePath(QStringLiteral("data/predefined_classes.txt")));
}
}

QString ResourcePaths::root() {
    QStringList candidates;
    const QString appDir = QCoreApplication::applicationDirPath();
    if (!appDir.isEmpty()) {
        candidates << appDir << QDir(appDir).filePath(QStringLiteral(".."));
    }
#ifdef LABELIMG_REPO_ROOT
    candidates << QString::fromUtf8(LABELIMG_REPO_ROOT);
#endif
    candidates << QDir::currentPath();

    for (const QString &candidate : candidates) {
        const QString cleanPath = QDir(candidate).cleanPath(candidate);
        if (hasLabelImgResources(cleanPath)) {
            return QFileInfo(cleanPath).absoluteFilePath();
        }
    }

    return QFileInfo(appDir.isEmpty() ? QDir::currentPath() : appDir).absoluteFilePath();
}

QString ResourcePaths::filePath(const QString &relativePath) {
    static const bool initialized = [] { initializeBundledResources(); return true; }();
    Q_UNUSED(initialized);
    const QString bundled = QStringLiteral(":/labelimg/") + relativePath;
    // UI assets travel with the executable; stale/missing external copies must
    // not change the UI. Keep editable class files and Python scripts on disk.
    if ((relativePath.startsWith(QStringLiteral("resources/icons")) ||
         relativePath.startsWith(QStringLiteral("resources/strings"))) && QFileInfo::exists(bundled)) {
        return bundled;
    }
    const QString external = QDir(root()).filePath(relativePath);
    return QFileInfo::exists(external) || !QFileInfo::exists(bundled) ? external : bundled;
}
