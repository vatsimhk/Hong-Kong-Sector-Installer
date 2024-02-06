#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>

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
};

#endif // OPTIONSDIALOG_H
