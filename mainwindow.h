#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <git2.h>
#include <QFileDialog>
#include <iostream>
#include <QMessageBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <string>
#include <fstream>
#include <sstream>
#include "mergeconflictdialog.h"
#include "migratedialog.h"
#include "colourthemepicker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void showMessage(const std::string& message);
    void showMessage(const std::string& message, const std::string& errorMessage);
    void handleUpdateButton();
    void handleInstallButton();
    void handleColourThemeButton();
    void updatePackage();
    void installPackage();
    void changeColourTheme();
    void migrateOldInstall(std::string repoPath);

    void setProgressBarText(std::string message);
    void setProgressBarValue(int value);
    void setProgressBarMax(int value);
    void setProgressBarMin(int value);


private:
    Ui::MainWindow *ui;
    QLabel *textLabel;
    QLabel *errorLabel;
    QPushButton *installButton;
    QPushButton *updateButton;
    QPushButton *colourThemeButton;
};
#endif // MAINWINDOW_H
