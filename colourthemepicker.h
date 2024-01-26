#ifndef COLOURTHEMEPICKER_H
#define COLOURTHEMEPICKER_H

#include <QDialog>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace Ui {
class colourThemePicker;
}

class colourThemePicker : public QDialog
{
    Q_OBJECT

public:
    explicit colourThemePicker(QWidget *parent = nullptr);
    ~colourThemePicker();

    /* Returns a vector of string pairs corresponding to all the replacements
   to install the colour theme.

   First - file name
   Second - text to replace

   Arguments:
   Filename - text file containing colour theme data
   selectedTheme - name of selected colour theme
    */
    std::vector<std::pair<std::string, std::string>> readReplacementsFromFile(
        const std::string& filename, const std::string& selectedTheme);

    void replaceBetweenMarkers(const std::string& filename,
                               const std::string& replacement,
                               const std::string& begin, const std::string& end);

    void set_repo_path(std::string path);

    std::string get_selected_theme();
    std::string get_error_message();

private slots:
    void on_button_default_clicked();

    void on_button_black_clicked();

    void on_button_grey_clicked();

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::colourThemePicker *ui;
    std::string repo_path;
    std::string selected_theme;
    std::string error_message;
};

#endif // COLOURTHEMEPICKER_H
