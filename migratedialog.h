#ifndef MIGRATEDIALOG_H
#define MIGRATEDIALOG_H

#include <QDialog>
#include <iostream>

namespace Ui {
class migrateDialog;
}

class migrateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit migrateDialog(QWidget *parent = nullptr);
    ~migrateDialog();

    void set_repo_paths(std::string repo_path, std::string backup_path);

private:
    Ui::migrateDialog *ui;
};

#endif // MIGRATEDIALOG_H
