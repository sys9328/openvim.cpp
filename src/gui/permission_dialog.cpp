#include "permission_dialog.hpp"

#include <QApplication>
#include <QStyle>

PermissionDialog::PermissionDialog(const permission::PermissionRequest& request, QWidget* parent)
    : QDialog(parent)
    , request_(request)
    , selected_action_(permission::PermissionAction::Deny)
{
    setupUI();
    setWindowTitle("Permission Required");

    // Set modal and center on parent
    setModal(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Set dark theme
    setStyleSheet(R"(
        PermissionDialog {
            background-color: #2d2d30;
            color: #d4d4d4;
        }
        QLabel {
            color: #d4d4d4;
        }
        QTextEdit {
            background-color: #1e1e1e;
            color: #d4d4d4;
            border: 1px solid #3e3e42;
            padding: 5px;
        }
        QPushButton {
            background-color: #3c3c3c;
            color: #d4d4d4;
            border: 1px solid #555555;
            padding: 8px 16px;
            border-radius: 4px;
            min-width: 80px;
        }
        QPushButton:hover {
            background-color: #505050;
        }
        QPushButton#allowButton {
            background-color: #007acc;
            color: white;
        }
        QPushButton#allowButton:hover {
            background-color: #005a9e;
        }
        QPushButton#denyButton {
            background-color: #d13438;
            color: white;
        }
        QPushButton#denyButton:hover {
            background-color: #b91d1f;
        }
        QCheckBox {
            color: #d4d4d4;
        }
        QCheckBox::indicator {
            background-color: #3c3c3c;
            border: 1px solid #555555;
        }
        QCheckBox::indicator:checked {
            background-color: #007acc;
        }
    )");
}

PermissionDialog::~PermissionDialog() = default;

void PermissionDialog::setupUI() {
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(10);
    main_layout->setContentsMargins(20, 20, 20, 20);

    // Icon and title
    QHBoxLayout* title_layout = new QHBoxLayout;

    QLabel* icon_label = new QLabel;
    QPixmap warning_pixmap = style()->standardPixmap(QStyle::SP_MessageBoxWarning);
    icon_label->setPixmap(warning_pixmap.scaled(32, 32, Qt::KeepAspectRatio));
    title_layout->addWidget(icon_label);

    title_label_ = new QLabel(QString("Permission Required: %1").arg(QString::fromStdString(request_.tool_name)));
    title_label_->setStyleSheet("font-weight: bold; font-size: 14px;");
    title_layout->addWidget(title_label_, 1);

    main_layout->addLayout(title_layout);

    // Description
    QLabel* desc_label = new QLabel("The following action requires your permission:");
    main_layout->addWidget(desc_label);

    description_text_ = new QTextEdit;
    description_text_->setPlainText(QString::fromStdString(request_.description));
    description_text_->setReadOnly(true);
    description_text_->setMaximumHeight(100);
    main_layout->addWidget(description_text_);

    // Details
    QString details = QString(
        "Tool: %1\n"
        "Action: %2\n"
        "Path: %3"
    ).arg(QString::fromStdString(request_.tool_name),
          QString::fromStdString(request_.action),
          QString::fromStdString(request_.path));

    QLabel* details_label = new QLabel(details);
    details_label->setStyleSheet("font-family: Monospace; background-color: #1e1e1e; padding: 10px; border-radius: 4px;");
    main_layout->addWidget(details_label);

    // Remember choice checkbox
    remember_checkbox_ = new QCheckBox("Remember this choice for this session");
    main_layout->addWidget(remember_checkbox_);

    // Buttons
    QHBoxLayout* button_layout = new QHBoxLayout;
    button_layout->addStretch();

    deny_button_ = new QPushButton("Deny");
    deny_button_->setObjectName("denyButton");
    connect(deny_button_, &QPushButton::clicked, this, &PermissionDialog::onDenyClicked);
    button_layout->addWidget(deny_button_);

    allow_session_button_ = new QPushButton("Allow for Session");
    connect(allow_session_button_, &QPushButton::clicked, this, &PermissionDialog::onAllowForSessionClicked);
    button_layout->addWidget(allow_session_button_);

    allow_button_ = new QPushButton("Allow");
    allow_button_->setObjectName("allowButton");
    allow_button_->setDefault(true);
    connect(allow_button_, &QPushButton::clicked, this, &PermissionDialog::onAllowClicked);
    button_layout->addWidget(allow_button_);

    main_layout->addLayout(button_layout);

    // Set focus to deny button for safety
    deny_button_->setFocus();
}

void PermissionDialog::onAllowClicked() {
    selected_action_ = permission::PermissionAction::Allow;
    accept();
}

void PermissionDialog::onAllowForSessionClicked() {
    selected_action_ = permission::PermissionAction::AllowForSession;
    accept();
}

void PermissionDialog::onDenyClicked() {
    selected_action_ = permission::PermissionAction::Deny;
    reject();
}