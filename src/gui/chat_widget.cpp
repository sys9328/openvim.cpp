#include "chat_widget.hpp"

#include <QScrollBar>
#include <QDateTime>
#include <QApplication>

ChatWidget::ChatWidget(message::Service& message_service,
                       session::Service& session_service,
                       llm::Service& llm_service,
                       QWidget* parent)
    : QWidget(parent)
    , message_service_(message_service)
    , session_service_(session_service)
    , llm_service_(llm_service)
{
    setupUI();

    // Connect to LLM service events
    // TODO: Connect to message updates

    // Set dark theme
    setStyleSheet(R"(
        ChatWidget {
            background-color: #1e1e1e;
            border: 1px solid #3e3e42;
        }
        QScrollArea {
            border: none;
            background-color: #1e1e1e;
        }
        QLineEdit {
            background-color: #3c3c3c;
            color: #d4d4d4;
            border: 1px solid #555555;
            padding: 8px;
            border-radius: 4px;
        }
        QPushButton {
            background-color: #007acc;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #005a9e;
        }
        QPushButton:pressed {
            background-color: #004275;
        }
    )");
}

ChatWidget::~ChatWidget() = default;

void ChatWidget::setupUI() {
    main_layout_ = new QVBoxLayout(this);
    main_layout_->setContentsMargins(5, 5, 5, 5);
    main_layout_->setSpacing(5);

    // Chat area
    scroll_area_ = new QScrollArea;
    scroll_area_->setWidgetResizable(true);
    scroll_area_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll_area_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    chat_container_ = new QWidget;
    chat_layout_ = new QVBoxLayout(chat_container_);
    chat_layout_->setAlignment(Qt::AlignTop);
    chat_layout_->addStretch(); // Push messages to top

    scroll_area_->setWidget(chat_container_);
    main_layout_->addWidget(scroll_area_);

    // Input area
    QHBoxLayout* input_layout = new QHBoxLayout;
    input_layout->setSpacing(5);

    input_field_ = new QLineEdit;
    input_field_->setPlaceholderText("Type your message here... (press Enter to send)");
    connect(input_field_, &QLineEdit::returnPressed, this, &ChatWidget::onSendMessage);
    connect(input_field_, &QLineEdit::textChanged, this, &ChatWidget::onMessageTextChanged);
    input_layout->addWidget(input_field_);

    send_button_ = new QPushButton("Send");
    send_button_->setEnabled(false);
    connect(send_button_, &QPushButton::clicked, this, &ChatWidget::onSendMessage);
    input_layout->addWidget(send_button_);

    main_layout_->addLayout(input_layout);

    // Add welcome message
    appendMessageToChat("System", "Welcome to openvim - agentic development platform! I'm your AI coding assistant. How can I help you today?", false);
}

void ChatWidget::addMessage(const message::Message& message) {
    QString sender = (message.role == message::Role::User) ? "You" : "Assistant";
    appendMessageToChat(sender, QString::fromStdString(message.content),
                       message.role == message::Role::User);
}

void ChatWidget::loadMessages(const std::string& session_id) {
    current_session_id_ = QString::fromStdString(session_id);

    // Clear existing messages
    QLayoutItem* item;
    while ((item = chat_layout_->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    try {
        auto messages = message_service_.list(session_id);
        for (const auto& message : messages) {
            addMessage(message);
        }
    } catch (const std::exception& e) {
        appendMessageToChat("System", QString("Error loading messages: %1").arg(e.what()));
    }
}

void ChatWidget::onSendMessage() {
    QString message = input_field_->text().trimmed();
    if (message.isEmpty() || current_session_id_.isEmpty()) {
        return;
    }

    // Clear input
    input_field_->clear();
    send_button_->setEnabled(false);

    try {
        // Create user message
        message_service_.create_user(current_session_id_.toStdString(), message.toStdString());

        // Generate title if this is the first message in the session
        auto msg_list = message_service_.list(current_session_id_.toStdString());
        if (msg_list.size() == 1) {  // Just created the user message
            std::string title = llm_service_.generate_title(message.toStdString());
            session_service_.update_title(current_session_id_.toStdString(), title);
        }

        // Send request to LLM
        llm_service_.send_request(current_session_id_.toStdString(), message.toStdString());

    } catch (const std::exception& e) {
        appendMessageToChat("System", QString("Error sending message: %1").arg(e.what()));
        send_button_->setEnabled(true);
    }
}

void ChatWidget::onMessageTextChanged() {
    send_button_->setEnabled(!input_field_->text().trimmed().isEmpty());
}

void ChatWidget::appendMessageToChat(const QString& sender, const QString& content, bool is_user) {
    // Create message widget
    QWidget* message_widget = new QWidget;
    QHBoxLayout* message_layout = new QHBoxLayout(message_widget);
    message_layout->setContentsMargins(5, 5, 5, 5);

    // Message bubble
    QLabel* message_label = new QLabel;
    QString time_str = QDateTime::currentDateTime().toString("hh:mm:ss");

    QString html_content = QString(
        "<div style='background-color: %1; color: %2; padding: 8px; border-radius: 8px; margin: 2px; max-width: 400px; word-wrap: break-word;'>"
        "<b>%3</b> <small style='color: #888;'>(%4)</small><br/>"
        "%5"
        "</div>"
    ).arg(is_user ? "#007acc" : "#2d2d30",
          is_user ? "white" : "#d4d4d4",
          sender,
          time_str,
          content.toHtmlEscaped().replace("\n", "<br/>"));

    message_label->setText(html_content);
    message_label->setTextFormat(Qt::RichText);
    message_label->setWordWrap(true);

    if (is_user) {
        message_layout->addStretch();
        message_layout->addWidget(message_label);
    } else {
        message_layout->addWidget(message_label);
        message_layout->addStretch();
    }

    chat_layout_->insertWidget(chat_layout_->count() - 1, message_widget); // Insert before stretch

    // Scroll to bottom
    QTimer::singleShot(0, this, &ChatWidget::scrollToBottom);
}

void ChatWidget::scrollToBottom() {
    QScrollBar* scrollbar = scroll_area_->verticalScrollBar();
    scrollbar->setValue(scrollbar->maximum());
}