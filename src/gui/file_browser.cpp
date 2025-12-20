#include "file_browser.hpp"

#include <QDir>
#include <QHeaderView>

FileBrowser::FileBrowser(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupFileSystemModel();

    // Set dark theme
    setStyleSheet(R"(
        FileBrowser {
            background-color: #1e1e1e;
            border: 1px solid #3e3e42;
        }
        QTreeView {
            background-color: #1e1e1e;
            color: #d4d4d4;
            border: 1px solid #3e3e42;
            selection-background-color: #007acc;
        }
        QTreeView::item:hover {
            background-color: #2d2d30;
        }
        QLabel {
            color: #d4d4d4;
            font-weight: bold;
        }
        QPushButton {
            background-color: #3c3c3c;
            color: #d4d4d4;
            border: 1px solid #555555;
            padding: 4px 8px;
            border-radius: 3px;
        }
        QPushButton:hover {
            background-color: #505050;
        }
    )");
}

FileBrowser::~FileBrowser() = default;

void FileBrowser::setupUI() {
    main_layout_ = new QVBoxLayout(this);
    main_layout_->setContentsMargins(5, 5, 5, 5);
    main_layout_->setSpacing(5);

    // Header with refresh button
    QHBoxLayout* header_layout = new QHBoxLayout;

    path_label_ = new QLabel("Project Files");
    header_layout->addWidget(path_label_);

    header_layout->addStretch();

    refresh_button_ = new QPushButton("↻");
    refresh_button_->setMaximumWidth(30);
    connect(refresh_button_, &QPushButton::clicked, this, &FileBrowser::onRefreshClicked);
    header_layout->addWidget(refresh_button_);

    main_layout_->addLayout(header_layout);

    // File tree view
    tree_view_ = new QTreeView;
    tree_view_->setHeaderHidden(false);
    tree_view_->header()->setStretchLastSection(true);
    tree_view_->header()->setSortIndicator(0, Qt::AscendingOrder);
    tree_view_->setSortingEnabled(true);
    tree_view_->setAlternatingRowColors(false);

    connect(tree_view_, &QTreeView::doubleClicked, this, &FileBrowser::onFileDoubleClicked);

    main_layout_->addWidget(tree_view_);
}

void FileBrowser::setupFileSystemModel() {
    file_model_ = new QFileSystemModel(this);
    file_model_->setRootPath(QDir::currentPath());
    file_model_->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    file_model_->setNameFilterDisables(false);

    // Hide unwanted columns and set headers
    tree_view_->setModel(file_model_);
    tree_view_->setRootIndex(file_model_->index(QDir::currentPath()));

    // Hide size, type, date columns for cleaner look
    tree_view_->hideColumn(1); // Size
    tree_view_->hideColumn(2); // Type
    tree_view_->hideColumn(3); // Date modified

    // Set column width
    tree_view_->setColumnWidth(0, 200);
}

void FileBrowser::onFileDoubleClicked(const QModelIndex& index) {
    QString file_path = file_model_->filePath(index);

    if (QFileInfo(file_path).isFile()) {
        emit fileOpened(file_path);
    } else {
        emit fileSelected(file_path);
    }
}

void FileBrowser::onRefreshClicked() {
    // Force refresh by resetting the root path
    QString current_root = file_model_->rootPath();
    file_model_->setRootPath("");
    file_model_->setRootPath(current_root);
}