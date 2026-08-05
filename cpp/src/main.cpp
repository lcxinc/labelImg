#include "ui/MainWindow.h"

#include "core/ResourcePaths.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QTextStream>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("labelImgCpp");
    QApplication::setOrganizationName("labelImg");
    QApplication::setWindowIcon(QIcon(ResourcePaths::filePath(QStringLiteral("resources/icons/app-cpp.png"))));

    if (QCoreApplication::arguments().contains(QStringLiteral("--version")) ||
        QCoreApplication::arguments().contains(QStringLiteral("-V"))) {
        QTextStream(stdout) << "labelImgCpp 0.1.0\n";
        return 0;
    }

    const QString userConfigPath = QDir::home().filePath(QStringLiteral(".labelmerc"));
    QString defaultConfigPath = userConfigPath;
    if (!QFileInfo::exists(defaultConfigPath)) {
        QFile configFile(defaultConfigPath);
        if (configFile.open(QIODevice::WriteOnly)) {
            configFile.close();
        } else {
            defaultConfigPath.clear();
        }
    }
    MainWindow window(nullptr, defaultConfigPath);
    window.loadStartupArgs(QCoreApplication::arguments());
    if (QCoreApplication::arguments().contains(QStringLiteral("--reset-config"))) {
        return 0;
    }
    window.show();
    return app.exec();
}
