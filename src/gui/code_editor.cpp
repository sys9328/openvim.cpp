#include "code_editor.hpp"

#include <QKeyEvent>
#include <QTextBlock>
#include <QPainter>
#include <QFont>

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    // Keywords
    keyword_format_.setForeground(Qt::blue);
    keyword_format_.setFontWeight(QFont::Bold);
    const QString keyword_patterns[] = {
        "\\bclass\\b", "\\bconst\\b", "\\bif\\b", "\\belse\\b",
        "\\bfor\\b", "\\bwhile\\b", "\\breturn\\b", "\\bvoid\\b",
        "\\bint\\b", "\\bfloat\\b", "\\bdouble\\b", "\\bchar\\b",
        "\\bstring\\b", "\\bbool\\b", "\\btrue\\b", "\\bfalse\\b",
        "\\binclude\\b", "\\bnamespace\\b", "\\bpublic\\b", "\\bprivate\\b",
        "\\bprotected\\b", "\\bvirtual\\b", "\\boverride\\b"
    };

    for (const QString& pattern : keyword_patterns) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(pattern);
        rule.format = keyword_format_;
        highlighting_rules_.append(rule);
    }

    // Classes
    class_format_.setForeground(Qt::darkMagenta);
    class_format_.setFontWeight(QFont::Bold);
    HighlightingRule class_rule;
    class_rule.pattern = QRegularExpression("\\bQ[A-Za-z]+\\b");
    class_rule.format = class_format_;
    highlighting_rules_.append(class_rule);

    // Single line comments
    single_line_comment_format_.setForeground(Qt::darkGreen);
    HighlightingRule single_line_comment_rule;
    single_line_comment_rule.pattern = QRegularExpression("//[^\n]*");
    single_line_comment_rule.format = single_line_comment_format_;
    highlighting_rules_.append(single_line_comment_rule);

    // Quotation
    quotation_format_.setForeground(Qt::darkRed);
    HighlightingRule quotation_rule;
    quotation_rule.pattern = QRegularExpression("\".*\"");
    quotation_rule.format = quotation_format_;
    highlighting_rules_.append(quotation_rule);

    // Functions
    function_format_.setForeground(Qt::blue);
    HighlightingRule function_rule;
    function_rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    function_rule.format = function_format_;
    highlighting_rules_.append(function_rule);
}

void SyntaxHighlighter::highlightBlock(const QString& text) {
    for (const HighlightingRule& rule : qAsConst(highlighting_rules_)) {
        QRegularExpressionMatchIterator match_iterator = rule.pattern.globalMatch(text);
        while (match_iterator.hasNext()) {
            QRegularExpressionMatch match = match_iterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

CodeEditor::CodeEditor(QWidget* parent)
    : QPlainTextEdit(parent)
{
    // Set up syntax highlighter
    highlighter_ = new SyntaxHighlighter(document());

    // Set font
    QFont font;
    font.setFamily("Monospace");
    font.setFixedPitch(true);
    font.setPointSize(10);
    setFont(font);

    // Set colors (dark theme like Neovim)
    QPalette palette = this->palette();
    palette.setColor(QPalette::Base, QColor(30, 30, 30));        // Background
    palette.setColor(QPalette::Text, QColor(212, 212, 212));     // Text
    palette.setColor(QPalette::Highlight, QColor(0, 122, 204));  // Selection
    palette.setColor(QPalette::HighlightedText, Qt::white);
    setPalette(palette);

    // Enable line numbers and other features
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setTabStopDistance(40); // 4 spaces

    // Set placeholder text
    setPlaceholderText("// Welcome to openvim - agentic development platform\n"
                      "// Start typing your code here...\n\n"
                      "#include <iostream>\n\n"
                      "int main() {\n"
                      "    std::cout << \"Hello, World!\" << std::endl;\n"
                      "    return 0;\n"
                      "}\n");
}

void CodeEditor::keyPressEvent(QKeyEvent* event) {
    // Handle special key combinations
    if (event->modifiers() & Qt::ControlModifier) {
        switch (event->key()) {
            case Qt::Key_S:
                // TODO: Save file
                event->accept();
                return;
            case Qt::Key_O:
                // TODO: Open file
                event->accept();
                return;
        }
    }

    // Call parent implementation
    QPlainTextEdit::keyPressEvent(event);
}