#ifndef MERGECONFLICTDIALOG_H
#define MERGECONFLICTDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

namespace Ui {
class mergeConflictDialog;
}

class mergeConflictDialog : public QDialog
{
    Q_OBJECT

public:
    explicit mergeConflictDialog(QWidget *parent = nullptr);
    ~mergeConflictDialog();

    void set_repo_path(std::string path);
    void add_file(std::string filename);
    void update_file_list_display();
    void load_force_keep_lines();
    bool check_for_force_keep(const std::string& line);
    std::string findKeepStatus(const std::string& filename, const std::string& repo_path);

private:
    Ui::mergeConflictDialog *ui;
    std::string repo_path;
    std::vector<std::string> file_list;
    std::vector<std::string> force_keep_lines;

private slots:
    void resolveMergeConflicts();
};

#endif // MERGECONFLICTDIALOG_H
