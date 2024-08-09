#include "mergeconflictdialog.h"
#include "ui_mergeconflictdialog.h"

mergeConflictDialog::mergeConflictDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mergeConflictDialog)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Resolve Manually");
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(resolveMergeConflicts()));
}

mergeConflictDialog::~mergeConflictDialog()
{
    delete ui;
}

void mergeConflictDialog::set_repo_path(std::string path) {
    repo_path = path;
}

void mergeConflictDialog::add_file(std::string filename) {
    file_list.push_back(filename);
    update_file_list_display();
}

void mergeConflictDialog::update_file_list_display() {
    std::string complete_file_list;

    for (std::vector<std::string>::iterator t=file_list.begin(); t!=file_list.end(); ++t)
    {
        complete_file_list += *t;
        complete_file_list += "\n";
    }

    ui->file_list_label->setText(QString::fromStdString(complete_file_list));
}

void mergeConflictDialog::load_force_keep_lines() {
    std::ifstream file(repo_path + "/Data/MergeConflictStrategy.txt");

    if (!file.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        return; // Assume nothing and return
    }

    std::string line;
    while (std::getline(file, line)) {
        if(line.find("ForceKeep: ") == std::string::npos) {
            continue;
        }

        line.erase(line.find("ForceKeep: "), 11);
        force_keep_lines.push_back(line); // Add to vector
        std::cout << line << std::endl;
    }

    force_keep_lines.push_back("LastSession");
}

bool mergeConflictDialog::check_for_force_keep(const std::string& line) {
    for(auto & element : force_keep_lines) {
        if(line.find(element) != std::string::npos) {
            std::cout << "found " << element << " in " << line << std::endl;
            return true;
        }
    }
    return false;
}

std::string mergeConflictDialog::findKeepStatus(const std::string& filename, const std::string& repo_path) {
    std::ifstream file(repo_path + "/Data/MergeConflictStrategy.txt");

    if (!file.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        return ""; // Return an empty string to indicate an error
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string currentFilename;
        std::string keepStatus;

        if(line.find("Filename: ") == std::string::npos) {
            continue;
        }
        line.erase(line.find("Filename: "), 10);
        currentFilename = line;
        if(filename.find(currentFilename) == std::string::npos) {
            continue;
        }
        std::getline(file, line);
        line.erase(line.find("Keep: "), 6);
        return line;
    }

    // Return an empty string to indicate that the filename was not found
    return "";
}

void mergeConflictDialog::resolveMergeConflicts() {

    load_force_keep_lines();

    for (std::vector<std::string>::iterator t=file_list.begin(); t!=file_list.end(); ++t)
    {   
        std::string filename = repo_path + "/" + *t;
        std::ifstream inputFile(filename);
        if (!inputFile.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return;
        }

        //bool keep_upstream = filename.find(".asr") == std::string::npos && filename.find("Settings") == std::string::npos && filename.find(".prf") == std::string::npos;
        bool keep_stashed, keep_upstream;
        if(findKeepStatus(filename, repo_path) == "STASHED") {
            keep_stashed = true;
            keep_upstream = false;
        } else if (findKeepStatus(filename, repo_path) == "BOTH") {
            std::cout << "both" << std::endl;
            keep_stashed = true;
            keep_upstream = true;
        } else {
            std::cout << "upstream" << std::endl;
            keep_stashed = false;
            keep_upstream = true;
        }

        std::stringstream buffer;
        std::string line;

        while (std::getline(inputFile, line)) {
            size_t upstreamPos = line.find("<<<<<<< Updated upstream");

            if (upstreamPos != std::string::npos) {
                // Conditions for keeping lines between "<<<<<<< Updated upstream" and "======="
                while (std::getline(inputFile, line) &&
                       line.find("=======") == std::string::npos) {
                    if(keep_upstream && !check_for_force_keep(line)) buffer << line << '\n';
                }
                // Conditions for keeping lines between "=======" and ">>>>>>> Stashed changes"
                while (std::getline(inputFile, line) &&
                       line.find(">>>>>>> Stashed changes") == std::string::npos) {
                    if(keep_stashed || check_for_force_keep(line)) buffer << line << '\n';
                }
            } else {
                // Keep other lines unchanged
                buffer << line << '\n';
            }
        }

        inputFile.close();

        std::ofstream outputFile(filename, std::ios::trunc);
        if (!outputFile.is_open()) {
            std::cerr << "Error opening file for writing: " << filename << std::endl;
            return;
        }

        outputFile << buffer.str();
        outputFile.close();
    }

    this->close();
}

