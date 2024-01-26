#include "colourthemepicker.h"
#include "ui_colourthemepicker.h"

colourThemePicker::colourThemePicker(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::colourThemePicker)
{
    ui->setupUi(this);
}

colourThemePicker::~colourThemePicker()
{
    delete ui;
}

std::vector<std::pair<std::string, std::string>> colourThemePicker::readReplacementsFromFile(
    const std::string& filename, const std::string& selectedTheme) {
    std::ifstream inputFile(filename);

    std::vector<std::pair<std::string, std::string>> replacements;

    if (!inputFile.is_open()) {
        std::cerr << "Error opening replacement file: " << filename << std::endl;
        return replacements;
    }

    std::stringstream replacementBuffer;
    std::stringstream replacementFileName;
    std::string line;
    bool insideReplacementSection = false;
    bool insideCorrectTheme = false;

    while (std::getline(inputFile, line)) {
        if (!insideCorrectTheme) {
            // Skip lines if still not within correct theme
            if (line.find(selectedTheme + " THEME BEGIN") != std::string::npos) {
                insideCorrectTheme = true;
            }
            continue;
        }

        if (line.find(selectedTheme + " THEME END") != std::string::npos) {
            // End of data for this colour theme, escape loop
            break;
        }

        // locate BEGIN and END lines
        size_t beginReplacementPos = line.find("BEGIN");
        size_t endReplacementPos = line.find("END");

        if (beginReplacementPos != std::string::npos) {
            insideReplacementSection = true;
            // BEGIN line contains file name, copy to string buffer
            replacementFileName << repo_path + "/" + line.substr(0, line.find(" BEGIN"));
        } else if (endReplacementPos != std::string::npos) {
            insideReplacementSection = false;

            // Create string pair of file path and replacement text and push to vector
            replacements.push_back(
                std::make_pair(replacementFileName.str(), replacementBuffer.str()));

            // Clear stringstremams
            replacementBuffer.str(std::string());
            replacementFileName.str(std::string());
        } else if (insideReplacementSection) {
            replacementBuffer << line << '\n';
        }
    }

    inputFile.close();
    return replacements;
}

void colourThemePicker::replaceBetweenMarkers(const std::string& filename,
                           const std::string& replacement,
                           const std::string& begin, const std::string& end) {

    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::stringstream buffer;
    std::string line;

    bool insideMarkerSection = false;

    while (std::getline(inputFile, line)) {
        size_t beginMarkerPos = line.find(begin);
        size_t endMarkerPos = line.find(end);

        if (beginMarkerPos != std::string::npos) {
            insideMarkerSection = true;
            buffer << line << '\n';
        } else if (endMarkerPos != std::string::npos) {
            insideMarkerSection = false;
            buffer << replacement << line << '\n';
        } else if (!insideMarkerSection) {
            buffer << line << '\n';
        }
    }
    inputFile.close();

    if (insideMarkerSection) {
        std::cerr << "Missing END line!" << std::endl;
        return;
    }

    std::ofstream outputFile(filename, std::ios::trunc);
    if (!outputFile.is_open()) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }

    outputFile << buffer.str();
    outputFile.close();

    std::cout << "Lines between markers replaced successfully in file: "
              << filename << std::endl;
}

void colourThemePicker::set_repo_path(std::string path) {
    repo_path = path;
}

std::string colourThemePicker::get_selected_theme() {
    return selected_theme;
}

void colourThemePicker::on_button_default_clicked()
{
    selected_theme = "DEFAULT";
}

void colourThemePicker::on_button_black_clicked()
{
    selected_theme = "BLACK";
}

void colourThemePicker::on_button_grey_clicked()
{
    selected_theme = "GREY";
}

void colourThemePicker::on_buttonBox_accepted()
{
    if (selected_theme == "") {
        this->close();
        return;
    }

    std::string colourThemeFile = repo_path + "/Data/ColourThemes.txt";
    std::vector<std::pair<std::string, std::string>> replacements =
        readReplacementsFromFile(colourThemeFile, selected_theme);

    if(replacements.empty()) {
        this->close();
        return;
    }

    for (auto it = begin(replacements); it != end(replacements); it++) {
        std::cout << it->first << std::endl;
        std::cout << it->second << std::endl << std::endl;
        replaceBetweenMarkers(it->first, it->second, "COLOUR THEME BEGIN",
                              "COLOUR THEME END");
    }

    this->close();
}

