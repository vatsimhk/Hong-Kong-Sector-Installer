#include "optionsdialog.h"
#include "ui_optionsdialog.h"

optionsDialog::optionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::optionsdialog)
{
    ui->setupUi(this);

    std::ifstream inputFile(proxy_settings_file);
    if(!inputFile.is_open()) {
        return;
    }

    std::string line;
    std::vector<QLineEdit *> optionFields = {
        ui->proxyURL,
        ui->proxyUsername,
        ui->proxyPassword,
        ui->branchName
    };
    while(std::getline(inputFile, line)) {
        for(auto &option : optionFields) {
            std::string optionName = option->objectName().toStdString();
            auto it = line.find(optionName);
            if(it != std::string::npos) {
                line.erase(it, optionName.length() + 1);
                option->setText(QString::fromStdString(line));
            }
        }
    }
}

optionsDialog::~optionsDialog()
{
    std::ofstream outputFile(proxy_settings_file, std::ofstream::out);

    if(!outputFile.is_open()) {
        delete ui;
        return;
    }

    outputFile << "proxyURL:" << ui->proxyURL->text().toStdString() << std::endl;
    outputFile << "proxyUsername:" << ui->proxyUsername->text().toStdString() << std::endl;
    outputFile << "proxyPassword:" << ui->proxyPassword->text().toStdString() << std::endl;
    outputFile << "branchName:" << ui->branchName->text().toStdString() << std::endl;

    outputFile.close();

    delete ui;
}

std::string optionsDialog::get_proxy_URL() {
    return ui->proxyURL->text().toStdString();
}

std::string optionsDialog::get_proxy_username() {
    return ui->proxyUsername->text().toStdString();
}

std::string optionsDialog::get_proxy_password() {
    return ui->proxyPassword->text().toStdString();
}

std::string optionsDialog::get_branch_name() {
    return ui->branchName->text().toStdString();
}


