#include "tool_output.hpp"

#include <QFont>

ToolOutput::ToolOutput(QWidget* parent)
    : QWidget(parent)
{
    setupUI();

    // Set dark theme
    setStyleSheet(R"(
        ToolOutput {
            background-color: #1e1e1e;
            border: 1px solid #3e3e42;
        }
        QTabWidget::pane {
            border: 1px solid #3e3e42;
            background-color: #1e1e1e;
        }
        QTabBar::tab {
            background-color: #2d2d30;
            color: #d4d4d4;
            padding: 8px 12px;
            border: 1px solid #3e3e42;
            margin-right: 2px;
        }
        QTabBar::tab:selected {
            background-color: #007acc;
            color: white;
        }
        QTabBar::tab:hover {
            background-color: #3e3e42;
        }
        QTextEdit {
            background-color: #1e1e1e;
            color: #d4d4d4;
            border: none;
            font-family: Monospace;
            font-size: 9pt;
        }
    )");
}

ToolOutput::~ToolOutput() = default;

void ToolOutput::setupUI() {
    main_layout_ = new QVBoxLayout(this);
    main_layout_->setContentsMargins(5, 5, 5, 5);
    main_layout_->setSpacing(5);

    // Tab widget for different tool outputs
    tab_widget_ = new QTabWidget;
    tab_widget_->setTabsClosable(true);
    connect(tab_widget_, &QTabWidget::tabCloseRequested, [this](int index) {
        if (index > 0) { // Don't close the general tab
            tab_widget_->removeTab(index);
        }
    });

    // General output tab
    general_output_ = new QTextEdit;
    general_output_->setReadOnly(true);
    general_output_->setPlainText("Tool output will appear here...\n");
    tab_widget_->addTab(general_output_, "General");

    main_layout_->addWidget(tab_widget_);
}

void ToolOutput::addToolOutput(const QString& tool_name, const QString& output) {
    // Check if we already have a tab for this tool
    for (int i = 0; i < tab_widget_->count(); ++i) {
        if (tab_widget_->tabText(i) == tool_name) {
            // Append to existing tab
            QTextEdit* text_edit = qobject_cast<QTextEdit*>(tab_widget_->widget(i));
            if (text_edit) {
                text_edit->append(output);
                tab_widget_->setCurrentIndex(i);
            }
            return;
        }
    }

    // Create new tab for this tool
    QTextEdit* tool_output = new QTextEdit;
    tool_output->setReadOnly(true);
    tool_output->setPlainText(output);
    tab_widget_->addTab(tool_output, tool_name);
    tab_widget_->setCurrentWidget(tool_output);
}

void ToolOutput::clearOutput() {
    general_output_->clear();
    general_output_->setPlainText("Tool output cleared.\n");

    // Clear all tool tabs except general
    while (tab_widget_->count() > 1) {
        tab_widget_->removeTab(1);
    }
}