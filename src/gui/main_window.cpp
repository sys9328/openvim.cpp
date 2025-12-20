#include "main_window.hpp"

#include <QApplication>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QDir>
#include <QStandardItemModel>

#include "code_editor.hpp"
#include "chat_widget.hpp"
#include "file_browser.hpp"
#include "tool_output.hpp"
#include "permission_dialog.hpp"

MainWindow::MainWindow(logging::Logger& logger,
                       session::Service& session_service,
                       message::Service& message_service,
                       llm::Service& llm_service,
                       const std::string& initial_session_id,
                       QWidget* parent)
    : QMainWindow(parent)
    , logger_(logger)
    , session_service_(session_service)
    , message_service_(message_service)
    , llm_service_(llm_service)
    , current_session_id_(initial_session_id)
{
    setupUI();
    setupMenus();
    setupStatusBar();

    loadSessions();
    switchToSession(initial_session_id);

    // Set window properties
    setWindowTitle("openvim - agentic development platform");
    setMinimumSize(1200, 800);
    resize(1400, 900);

    // Apply dark theme (Neovim-style)
    setStyleSheet(R"(
        QMainWindow {
            background-color: #1e1e1e;
            color: #d4d4d4;
        }
        QMenuBar {
            background-color: #2d2d30;
            color: #d4d4d4;
            border-bottom: 1px solid #3e3e42;
        }
        QMenuBar::item {
            background-color: transparent;
            padding: 4px 8px;
        }
        QMenuBar::item:selected {
            background-color: #007acc;
        }
        QSplitter::handle {
            background-color: #3e3e42;
        }
        QStatusBar {
            background-color: #007acc;
            color: white;
        }
    )");
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    // Create central widget
    QWidget* central_widget = new QWidget;
    setCentralWidget(central_widget);

    // Create main splitter (editor | right panel)
    main_splitter_ = new QSplitter(Qt::Horizontal);

    // Left side: Code editor
    code_editor_ = new CodeEditor(this);
    main_splitter_->addWidget(code_editor_);

    // Right side splitter (chat | tools)
    right_splitter_ = new QSplitter(Qt::Vertical);

    // Top right: Chat widget
    chat_widget_ = new ChatWidget(message_service_, session_service_, llm_service_, this);
    right_splitter_->addWidget(chat_widget_);

    // Bottom right: Tool output
    tool_output_ = new ToolOutput(this);
    right_splitter_->addWidget(tool_output_);

    // Set splitter proportions
    right_splitter_->setSizes({400, 200});

    main_splitter_->addWidget(right_splitter_);
    main_splitter_->setSizes({700, 500});

    // Set central layout
    QHBoxLayout* layout = new QHBoxLayout(central_widget);
    layout->addWidget(main_splitter_);
    layout->setContentsMargins(0, 0, 0, 0);
}

void MainWindow::setupMenus() {
    // File menu
    QMenu* file_menu = menuBar()->addMenu("&File");

    QAction* new_session_act = new QAction("&New Session", this);
    new_session_act->setShortcut(QKeySequence::New);
    connect(new_session_act, &QAction::triggered, this, &MainWindow::onNewSession);
    file_menu->addAction(new_session_act);

    QAction* open_session_act = new QAction("&Open Session", this);
    open_session_act->setShortcut(QKeySequence::Open);
    connect(open_session_act, &QAction::triggered, this, &MainWindow::onOpenSession);
    file_menu->addAction(open_session_act);

    file_menu->addSeparator();

    QAction* quit_act = new QAction("&Quit", this);
    quit_act->setShortcut(QKeySequence::Quit);
    connect(quit_act, &QAction::triggered, this, &MainWindow::onQuit);
    file_menu->addAction(quit_act);

    // View menu
    QMenu* view_menu = menuBar()->addMenu("&View");

    QAction* logs_act = new QAction("&Logs", this);
    logs_act->setShortcut(QKeySequence("L"));
    connect(logs_act, &QAction::triggered, this, &MainWindow::onShowLogs);
    view_menu->addAction(logs_act);

    // Help menu
    QMenu* help_menu = menuBar()->addMenu("&Help");

    QAction* help_act = new QAction("&Help", this);
    help_act->setShortcut(QKeySequence("?"));
    connect(help_act, &QAction::triggered, this, &MainWindow::onShowHelp);
    help_menu->addAction(help_act);
}

void MainWindow::setupStatusBar() {
    status_label_ = new QLabel("Ready");
    statusBar()->addWidget(status_label_);

    statusBar()->addPermanentWidget(new QLabel("|"));

    session_label_ = new QLabel("No session");
    statusBar()->addPermanentWidget(session_label_);
}

void MainWindow::loadSessions() {
    // TODO: Load sessions from database
    status_label_->setText("Sessions loaded");
}

void MainWindow::updateWindowTitle() {
    try {
        auto session = session_service_.get(current_session_id_);
        setWindowTitle(QString("openvim - %1").arg(QString::fromStdString(session.title)));
    } catch (const std::exception&) {
        setWindowTitle("openvim - agentic development platform");
    }
}

void MainWindow::onNewSession() {
    bool ok;
    QString session_name = QInputDialog::getText(this, "New Session",
                                                "Session name:", QLineEdit::Normal,
                                                "New Session", &ok);
    if (ok && !session_name.isEmpty()) {
        try {
            auto session = session_service_.create(session_name.toStdString());
            switchToSession(session.id);
            status_label_->setText("New session created");
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", QString("Failed to create session: %1").arg(e.what()));
        }
    }
}

void MainWindow::onOpenSession() {
    try {
        auto sessions = session_service_.list();
        if (sessions.empty()) {
            QMessageBox::information(this, "No Sessions", "No sessions available.");
            return;
        }

        QStringList session_names;
        for (const auto& session : sessions) {
            session_names << QString::fromStdString(session.title);
        }

        bool ok;
        QString selected_name = QInputDialog::getItem(this, "Open Session",
                                                     "Select session:", session_names, 0, false, &ok);
        if (ok && !selected_name.isEmpty()) {
            // Find the session by title
            for (const auto& session : sessions) {
                if (QString::fromStdString(session.title) == selected_name) {
                    switchToSession(session.id);
                    break;
                }
            }
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Failed to load sessions: %1").arg(e.what()));
    }
}

void MainWindow::onSaveSession() {
    // Sessions are auto-saved, just show confirmation
    status_label_->setText("Session is auto-saved");
}

void MainWindow::onQuit() {
    QApplication::quit();
}

void MainWindow::onShowHelp() {
    QString help_text = R"(
openvim - AI Code Assistant

Keyboard Shortcuts:
  Ctrl+N    New Session
  Ctrl+O    Open Session
  L         Show Logs
  ?         Show Help
  Ctrl+Q    Quit

Navigation:
  Use mouse or keyboard to navigate between panels
  Resize panels by dragging splitters

Features:
  • AI-powered code assistance
  • Multiple tool integrations
  • Session management
  • Real-time collaboration
)";

    QMessageBox::information(this, "Help", help_text);
}

void MainWindow::onShowLogs() {
    // TODO: Show logs dialog or panel
    QMessageBox::information(this, "Logs", "Logs panel not implemented yet");
}

void MainWindow::switchToSession(const std::string& session_id) {
    current_session_id_ = session_id;

    try {
        auto session = session_service_.get(session_id);
        session_label_->setText("Session: " + QString::fromStdString(session.title));
        updateWindowTitle();

        // Load messages for this session
        chat_widget_->loadMessages(session_id);

        status_label_->setText("Switched to session: " + QString::fromStdString(session.title));
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Failed to switch session: %1").arg(e.what()));
    }
}

void MainWindow::showPermissionDialog(const permission::PermissionRequest& request) {
    PermissionDialog dialog(request, this);
    int result = dialog.exec();

    if (result == QDialog::Accepted) {
        permission::PermissionResponse response{request, dialog.getSelectedAction()};
        permission::default_service->respond(response);
    }
}