#include "Application.h"
#include <QSettings>
#include <QFontDatabase>
#include <QDebug>

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv) {
    setApplicationName(QStringLiteral("InteractivePitchViewer"));
    setOrganizationName(QStringLiteral("PitchViewer"));
    setApplicationVersion(QStringLiteral("1.0.0"));

    // Load persisted configuration
    QSettings settings;
    config_.load(settings);

    qDebug() << "Application: API base URL:" << config_.apiBaseUrl.toString();
}

Application::~Application() {
    // Persist configuration on exit
    QSettings settings;
    config_.save(settings);
}

int Application::run() {
    mainWindow_ = new MainWindow(config_);
    mainWindow_->show();
    return exec();
}
