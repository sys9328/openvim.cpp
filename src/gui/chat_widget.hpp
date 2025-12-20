#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QLabel>
#include <QTimer>

#include "llm/llm.hpp"
#include "message/message.hpp"
#include "session/session.hpp"

class ChatWidget : public QWidget {
    Q_OBJECT

public:
    ChatWidget(message::Service& message_service,
               session::Service& session_service,
               llm::Service& llm_service,
               QWidget* parent = nullptr);
    ~ChatWidget();

    void addMessage(const message::Message& message);
    void loadMessages(const std::string& session_id);
    void setCurrentSession(const QString& session_id);

private slots:
    void onSendMessage();
    void onMessageTextChanged();

private:
    void setupUI();
    void appendMessageToChat(const QString& sender, const QString& content, bool is_user = false);
    void scrollToBottom();

    message::Service& message_service_;
    session::Service& session_service_;
    llm::Service& llm_service_;
    QString current_session_id_;

    // UI Components
    QVBoxLayout* main_layout_;
    QScrollArea* scroll_area_;
    QWidget* chat_container_;
    QVBoxLayout* chat_layout_;
    QLineEdit* input_field_;
    QPushButton* send_button_;

    // Message history
    QString last_message_content_;
};