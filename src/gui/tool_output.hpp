#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QTabWidget>
#include <QLabel>

class ToolOutput : public QWidget {
    Q_OBJECT

public:
    ToolOutput(QWidget* parent = nullptr);
    ~ToolOutput();

    void addToolOutput(const QString& tool_name, const QString& output);
    void clearOutput();

private:
    void setupUI();

    QVBoxLayout* main_layout_;
    QTabWidget* tab_widget_;
    QTextEdit* general_output_;
};