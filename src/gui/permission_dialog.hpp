#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QCheckBox>

#include "permission/permission.hpp"

class PermissionDialog : public QDialog {
    Q_OBJECT

public:
    PermissionDialog(const permission::PermissionRequest& request, QWidget* parent = nullptr);
    ~PermissionDialog();

    permission::PermissionAction getSelectedAction() const { return selected_action_; }

private slots:
    void onAllowClicked();
    void onAllowForSessionClicked();
    void onDenyClicked();

private:
    void setupUI();

    permission::PermissionRequest request_;
    permission::PermissionAction selected_action_;

    QLabel* title_label_;
    QTextEdit* description_text_;
    QCheckBox* remember_checkbox_;
    QPushButton* allow_button_;
    QPushButton* allow_session_button_;
    QPushButton* deny_button_;
};