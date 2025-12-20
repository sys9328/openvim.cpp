#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QTextEdit>
#include <QTreeView>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QMenuBar>
#include <QAction>
#include <QLabel>
#include <QTimer>

#include "logging/logger.hpp"
#include "message/message.hpp"
#include "session/session.hpp"
#include "permission/permission.hpp"
#include "llm/llm.hpp"

QT_BEGIN_NAMESPACE
class CodeEditor;
class ChatWidget;
class FileBrowser;
class ToolOutput;
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(logging::Logger& logger,
               session::Service& session_service,
               message::Service& message_service,
               llm::Service& llm_service,
               const std::string& initial_session_id,
               QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onNewSession();
    void onOpenSession();
    void onSaveSession();
    void onQuit();
    void onShowHelp();
    void onShowLogs();

private:
    void setupUI();
    void setupMenus();
    void setupStatusBar();
    void loadSessions();
    void switchToSession(const std::string& session_id);
    void updateWindowTitle();
    void showPermissionDialog(const permission::PermissionRequest& request);

    logging::Logger& logger_;
    session::Service& session_service_;
    message::Service& message_service_;
    llm::Service& llm_service_;
    std::string current_session_id_;

    // UI Components
    CodeEditor* code_editor_;
    ChatWidget* chat_widget_;
    FileBrowser* file_browser_;
    ToolOutput* tool_output_;

    // Layout
    QSplitter* main_splitter_;
    QSplitter* right_splitter_;

    // Status bar
    QLabel* status_label_;
    QLabel* session_label_;
};