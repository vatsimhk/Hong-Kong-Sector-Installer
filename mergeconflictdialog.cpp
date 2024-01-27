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

void mergeConflictDialog::resolveMergeConflicts() {

    for (std::vector<std::string>::iterator t=file_list.begin(); t!=file_list.end(); ++t)
    {
        std::string filename = repo_path + "/" + *t;
        std::ifstream inputFile(filename);
        if (!inputFile.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return;
        }

        //bool keep_upstream = filename.find(".asr") == std::string::npos && filename.find("Settings") == std::string::npos && filename.find(".prf") == std::string::npos;
        bool keep_upstream = true;

        std::stringstream buffer;
        std::string line;

        while (std::getline(inputFile, line)) {
            size_t upstreamPos = line.find("<<<<<<< Updated upstream");

            if (upstreamPos != std::string::npos) {
                // Keep lines between "<<<<<<< Updated upstream" and "======="
                while (std::getline(inputFile, line) &&
                       line.find("=======") == std::string::npos) {
                    if(keep_upstream) buffer << line << '\n';
                }
                // Skip lines between "=======" and ">>>>>>> Stashed changes"
                while (std::getline(inputFile, line) &&
                       line.find(">>>>>>> Stashed changes") == std::string::npos) {
                    if(!keep_upstream) buffer << line << '\n';
                    // Do nothing, just skip these lines
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

