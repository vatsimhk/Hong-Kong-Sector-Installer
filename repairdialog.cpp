#include "repairdialog.h"
#include "ui_repairdialog.h"

repairDialog::repairDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::repairDialog)
{
    ui->setupUi(this);
}

repairDialog::~repairDialog()
{
    delete ui;
}

void repairDialog::update_file_list(std::vector<std::string> modified_files_list) {
    ui->listWidget->clear();

    for(auto it = modified_files_list.begin(); it != modified_files_list.end(); it++) {
        ui->listWidget->addItem(QString::fromStdString(*it));
    }
}

std::vector<std::string> repairDialog::get_selected_files() {
    return selected_files;
}

void repairDialog::on_buttonBox_accepted()
{
    QList<QListWidgetItem *> widget_list = ui->listWidget->selectedItems();
    foreach(QListWidgetItem * widget_item, widget_list) {
        selected_files.push_back(widget_item->text().toStdString());
    }
}

