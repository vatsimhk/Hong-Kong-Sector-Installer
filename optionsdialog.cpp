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
    if(std::getline(inputFile, line)) {
        ui->proxyURL->setText(QString::fromStdString(line));
    }

    if(std::getline(inputFile, line)) {
        ui->proxyUsername->setText(QString::fromStdString(line));
    }

    if(std::getline(inputFile, line)) {
        ui->proxyPassword->setText(QString::fromStdString(line));
    }
}

optionsDialog::~optionsDialog()
{
    std::ofstream outputFile(proxy_settings_file, std::ofstream::out);

    if(!outputFile.is_open()) {
        delete ui;
        return;
    }

    outputFile << ui->proxyURL->text().toStdString() << std::endl;
    outputFile << ui->proxyUsername->text().toStdString() << std::endl;
    outputFile << ui->proxyPassword->text().toStdString() << std::endl;

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

