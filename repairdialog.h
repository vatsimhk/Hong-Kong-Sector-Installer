#ifndef REPAIRDIALOG_H
#define REPAIRDIALOG_H

#include <QDialog>

namespace Ui {
class repairDialog;
}

class repairDialog : public QDialog
{
    Q_OBJECT

public:
    explicit repairDialog(QWidget *parent = nullptr);
    ~repairDialog();

    void update_file_list(std::vector<std::string> modified_files_list);

    std::vector<std::string> get_selected_files();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::repairDialog *ui;
    std::vector<std::string> selected_files;
};

#endif // REPAIRDIALOG_H
