#pragma once
#include <QUrl>
#include <QString>
#include <QSettings>

// Application configuration loaded from QSettings (persisted across restarts).
// Also provides hardcoded defaults.
struct AppConfig {
    QUrl    apiBaseUrl{QStringLiteral("http://localhost:8001")};
    int     frameCacheCapacity{60};
    int     prefetchLookahead{30};
    bool    darkTheme{true};
    bool    showAngleAnnotations{true};
    double  minKeypointVisibility{0.5};

    void load(QSettings& s) {
        apiBaseUrl             = QUrl(s.value("api/baseUrl",
                                              apiBaseUrl.toString()).toString());
        frameCacheCapacity     = s.value("decode/cacheCapacity",
                                         frameCacheCapacity).toInt();
        prefetchLookahead      = s.value("decode/prefetchLookahead",
                                          prefetchLookahead).toInt();
        darkTheme              = s.value("ui/darkTheme",    darkTheme).toBool();
        showAngleAnnotations   = s.value("ui/showAngles",   showAngleAnnotations).toBool();
        minKeypointVisibility  = s.value("ui/minVisibility",minKeypointVisibility).toDouble();
    }

    void save(QSettings& s) const {
        s.setValue("api/baseUrl",            apiBaseUrl.toString());
        s.setValue("decode/cacheCapacity",   frameCacheCapacity);
        s.setValue("decode/prefetchLookahead",prefetchLookahead);
        s.setValue("ui/darkTheme",           darkTheme);
        s.setValue("ui/showAngles",          showAngleAnnotations);
        s.setValue("ui/minVisibility",       minKeypointVisibility);
    }
};
