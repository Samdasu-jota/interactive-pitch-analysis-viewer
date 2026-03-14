#pragma once
#include <QApplication>
#include "AppConfig.h"
#include "MainWindow.h"

// Application is a thin QApplication subclass that loads config and
// constructs the MainWindow. Keeping this separate from main() makes
// the startup flow testable.
class Application : public QApplication {
    Q_OBJECT

public:
    Application(int& argc, char** argv);
    ~Application() override;

    int run();

private:
    AppConfig    config_;
    MainWindow*  mainWindow_{nullptr};
};
