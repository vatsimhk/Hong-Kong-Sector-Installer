#include "migratedialog.h"
#include "ui_migratedialog.h"

migrateDialog::migrateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::migrateDialog)
{
    ui->setupUi(this);
}

migrateDialog::~migrateDialog()
{
    delete ui;
}

void migrateDialog::set_repo_paths(std::string repo_path, std::string backup_path) {
    std::string repo_paths_text = "New Install Location: ";
    repo_paths_text += repo_path + "\n\n";

    repo_paths_text += "Backup location: ";
    repo_paths_text += backup_path;

    ui->label_dirs->setText(QString::fromStdString(repo_paths_text));
    repaint();
}
