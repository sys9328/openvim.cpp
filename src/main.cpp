#include "config.hpp"
#include "db/db.hpp"
#include "llm/llm.hpp"
#include "logging/logger.hpp"
#include "message/message.hpp"
#include "permission/permission.hpp"
#include "session/session.hpp"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QGuiApplication>
#include <QDir>
#include <QStandardPaths>
#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

int main(int argc, char** argv) {
  QGuiApplication app(argc, argv);

  // Set application properties
  app.setApplicationName("openvim");
  app.setApplicationVersion("1.0");
  app.setOrganizationName("openvim");

  std::cout << "Starting openvim - agentic development platform..." << std::endl;

  try {
    std::cout << "Parsing command line arguments..." << std::endl;
    auto cfg = config::parse_args_or_exit(argc, argv);

    std::cout << "Creating logger..." << std::endl;
    logging::Logger log;
    log.info("Starting OpenCode...");
    if (cfg.debug) log.debug("Debug enabled");
    std::cout << "Logger created" << std::endl;

    std::cout << "Connecting to database..." << std::endl;
    db::Db db;
    try {
      db = db::connect(cfg.data_dir);
      log.info("DB ready at data dir: " + cfg.data_dir);
      std::cout << "DB connected successfully" << std::endl;
    } catch (const std::exception& e) {
      log.error(std::string("DB init failed: ") + e.what());
      std::cerr << "Database initialization failed: " << e.what() << std::endl;
      return 1;
    }

    std::cout << "Creating services..." << std::endl;
    session::Service sessions(db);
    message::Service messages(db);
    llm::Service llm(log, messages, cfg);
    std::cout << "Services created successfully" << std::endl;

    // Create an initial session if none exist
    std::cout << "Checking for existing sessions..." << std::endl;
    std::string active_session_id;
    try {
      auto existing = sessions.list();
      if (existing.empty()) {
        std::cout << "No existing sessions, creating welcome session..." << std::endl;
        auto s = sessions.create("Welcome session");
        active_session_id = s.id;
        std::cout << "Welcome session created with ID: " << active_session_id << std::endl;
      } else {
        active_session_id = existing.front().id;
        std::cout << "Using existing session with ID: " << active_session_id << std::endl;
      }
    } catch (const std::exception& e) {
      log.warn(std::string("Session bootstrap failed: ") + e.what());
      std::cerr << "Session bootstrap failed: " << e.what() << std::endl;
    }

    // If DB has no session for some reason, create one
    if (active_session_id.empty()) {
      std::cout << "Creating fallback session..." << std::endl;
      auto s = sessions.create("Welcome session");
      active_session_id = s.id;
    }

    std::cout << "Creating QML application engine..." << std::endl;
    QQmlApplicationEngine engine;
    std::cout << "QML engine created successfully" << std::endl;

    // TODO: Expose services to QML context
    // engine.rootContext()->setContextProperty("logger", &log);
    // engine.rootContext()->setContextProperty("sessions", &sessions);
    // engine.rootContext()->setContextProperty("messages", &messages);
    // engine.rootContext()->setContextProperty("llm", &llm);
    // engine.rootContext()->setContextProperty("activeSessionId", QString::fromStdString(active_session_id));

    if (cfg.test_mode) {
      std::cout << "Test mode: QML engine created successfully!" << std::endl;
      std::cout << "Loading QML file..." << std::endl;

      const QUrl url(QStringLiteral("qrc:/main.qml"));
      engine.load(url);

      if (engine.rootObjects().isEmpty()) {
        std::cerr << "Failed to load QML file" << std::endl;
        return 1;
      }

      std::cout << "QML file loaded successfully!" << std::endl;
      return 0;
    }

    std::cout << "Loading QML interface..." << std::endl;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
      std::cerr << "Failed to load QML interface" << std::endl;
      return 1;
    }

    std::cout << "QML interface loaded, starting event loop..." << std::endl;

    int result = app.exec();
    std::cout << "Application exited with code: " << result << std::endl;
    return result;

  } catch (const std::exception& e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return 1;
  }
}
