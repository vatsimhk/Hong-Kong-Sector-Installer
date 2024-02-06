#include "optionsdialog.h"
#include "ui_optionsdialog.h"

optionsDialog::optionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::optionsdialog)
{
    ui->setupUi(this);
}

optionsDialog::~optionsDialog()
{
    delete ui;
}
