#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <string>
#include <fstream>
#include <iostream>

namespace Ui {
class optionsdialog;
}

class optionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit optionsDialog(QWidget *parent = nullptr);
    ~optionsDialog();

private:
    Ui::optionsdialog *ui;

    const std::string proxy_settings_file = "ProxySettings.txt";
};

#endif // OPTIONSDIALOG_H
