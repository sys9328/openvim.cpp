#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QTreeView>
#include <QFileSystemModel>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

class FileBrowser : public QWidget {
    Q_OBJECT

public:
    FileBrowser(QWidget* parent = nullptr);
    ~FileBrowser();

signals:
    void fileSelected(const QString& file_path);
    void fileOpened(const QString& file_path);

private slots:
    void onFileDoubleClicked(const QModelIndex& index);
    void onRefreshClicked();

private:
    void setupUI();
    void setupFileSystemModel();

    QVBoxLayout* main_layout_;
    QTreeView* tree_view_;
    QFileSystemModel* file_model_;
    QLabel* path_label_;
    QPushButton* refresh_button_;
};