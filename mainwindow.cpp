#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <thread>

const char* SECTOR_PACKAGE = "https://github.com/vatsimhk/Hong-Kong-Sector-Package.git";
const char* REMOTE_REF = "origin";
const char* REMOTE_BRANCH = "refs/remotes/origin/main";
const char* LOCAL_BRANCH = "refs/heads/main";

std::string selectRepositoryPath() {
    // Use Qt file dialog for path selection
    QString selectedPath = QFileDialog::getExistingDirectory(nullptr, "Select Install Location", "", QFileDialog::ShowDirsOnly);

    return selectedPath.toStdString();
}

std::string getFileContents(std::string dirEntry) {
    std::ifstream fileStream(dirEntry);
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    std::cout<<buffer.str()<<std::endl;
    return buffer.str();
}

void deleteLegacyFiles(std::string repoPath) {
    for (const auto& entry : std::filesystem::directory_iterator(repoPath)) {
        if (entry.is_regular_file() && entry.path().filename().string().find("Legacy") == 0) {
            // Check if the file name starts with "legacy"
            try {
                std::filesystem::remove(entry);
                std::cout << "Deleted file: " << entry.path() << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error deleting file: " << e.what() << std::endl;
            }
        }
    }
    std::filesystem::remove_all(repoPath + "/Data/Settings/Legacy APP");
    std::filesystem::remove_all(repoPath + "/Data/Settings/Legacy CTR");
    std::filesystem::remove_all(repoPath + "/Data/Settings/Legacy TWR-GND-DEL");
}

std::string getSctVersion(std::string repoPath) {
    //read sct to find what version we updated to
    std::ifstream file(repoPath + "/Data/Sector/Hong-Kong-Sector-File.sct");
    if (!file.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return "";
    }
    std::string line;
    bool found = false;

    // Read the file line by line
    while (std::getline(file, line)) {
        if (line.find("Hong Kong Sector ") != std::string::npos) {
            found = true;
            break; // Stop searching once the line is found
        }
    }

    // Close the file
    file.close();

    if (found) {
        line.erase(line.begin(), line.begin()+strlen("Hong Kong Sector "));
    }
    return line;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Create a layout to manage widgets

    this->setFixedSize(500,450);

    textLabel = ui->messageBox;
    errorLabel = ui->errorBox;
    installButton = ui->installButton;
    migrateOldButton = ui->migrateButton;
    updateButton = ui->updateButton;
    QPixmap logo(":/images/hkvacc-blue.png");
    QPixmap logoScaled = logo.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->logoLabel->setAlignment(Qt::AlignCenter);
    ui->logoLabel->setPixmap(logoScaled);

    connect(installButton, &QPushButton::released, this, &MainWindow::handleInstallButton);
    connect(updateButton, &QPushButton::released, this, &MainWindow::handleUpdateButton);
    connect(migrateOldButton, &QPushButton::released, this, &MainWindow::handleMigrateButton);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showMessage(const std::string& message)
{
    QString q = QString::fromStdString(message);
    textLabel -> setText(q);
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setWordWrap(true);
}

void MainWindow::showMessage(const std::string& message, const std::string& errorMessage)
{
    QString q = QString::fromStdString(message);
    textLabel -> setText(q);
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setWordWrap(true);

    QString e = QString::fromStdString(errorMessage);
    errorLabel -> setText(e);
    textLabel->setAlignment(Qt::AlignCenter);
    errorLabel->setWordWrap(true);
}

void MainWindow::installPackage() {
    // Initialize the library
    git_libgit2_init();

    // Select the repository path
    std::string repoPath = selectRepositoryPath();
    if (repoPath == "") return;

    // Open the repository or clone if it doesn't exist
    git_repository *repo = nullptr;

    int error = git_repository_open(&repo, repoPath.c_str());
    if (error == GIT_ENOTFOUND) {
        git_clone_options clone_options = GIT_CLONE_OPTIONS_INIT;
        repoPath += "/Hong-Kong-Sector-Package";
        showMessage("Cloning Repository...");
        repaint();
        error = git_clone(&repo, SECTOR_PACKAGE, repoPath.c_str(), &clone_options);
        if (error != 0) {
            showMessage("Error cloning repository: ", std::string(giterr_last()->message));
            git_libgit2_shutdown();
            return;
        }

        git_repository_free(repo);

        deleteLegacyFiles(repoPath);
        showMessage("Hong Kong Sector Package Successfully Installed! Version " + getSctVersion(repoPath));
    } else {
        showMessage("Folder already contains Hong Kong Sector Package");
    }

    git_libgit2_shutdown();
}

void MainWindow::updatePackage() {

    // Select the repository path
    std::string repoPath = selectRepositoryPath();
    if (repoPath == "") return;

    // Initialize the library
    git_libgit2_init();

    // Open the repository or clone if it doesn't exist
    git_repository *repo = nullptr;

    int error = git_repository_open(&repo, repoPath.c_str());
    if (error != 0) {
        showMessage("Error updating Package: ", std::string(giterr_last()->message));
        git_libgit2_shutdown();
        return;
    }

    git_reference* head = nullptr;
    error = git_repository_head(&head, repo);
    if (error != 0) {
        showMessage("Failed to get HEAD reference");
        return;
    }

    git_remote *remote = nullptr;
    git_strarray *remote_list = new git_strarray;

    error = git_remote_list(remote_list, repo);
    if(error == 0) {
        for(size_t i = 0; i < remote_list->count; i++) {
            std::cout << remote_list->strings[i] << std::endl;
        }
        git_strarray_dispose(remote_list);
    }

    error = git_remote_lookup(&remote, repo, REMOTE_REF);
    if (error != 0) {
        showMessage("Error looking up remote: ", std::string(giterr_last()->message));
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }


    //stash settings
    git_stash_save_options save_options = GIT_STASH_SAVE_OPTIONS_INIT;
    git_stash_apply_options apply_options = GIT_STASH_APPLY_OPTIONS_INIT;

    git_oid saved_stash;
    git_signature* default_signature = nullptr;
    git_signature_default(&default_signature, repo);

    error = git_stash_save(&saved_stash, repo, default_signature, "", 0);
    if(error != 0 && error != GIT_ENOTFOUND) {
        showMessage("Error stashing changes: ", std::string(giterr_last()->message));
        git_remote_free(remote);
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }

    // Check for differences
    git_status_options status_options = GIT_STATUS_OPTIONS_INIT;
    git_status_list *status_list = nullptr;
    error = git_status_list_new(&status_list, repo, &status_options);
    if (error != 0) {
        showMessage("Error getting status: ", std::string(giterr_last()->message));
        git_remote_free(remote);
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }


    showMessage("Checking for Updates...");
    repaint();
    git_fetch_options fetch_options = GIT_FETCH_OPTIONS_INIT;
    error = git_remote_fetch(remote, nullptr, &fetch_options, "pull");
    if (error != 0) {
        showMessage("Error pulling changes: ", std::string(giterr_last()->message));
        git_stash_pop(repo, 0, &apply_options);
        git_status_list_free(status_list);
        git_remote_free(remote);
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }

    git_oid head_id;
    git_reference_name_to_id(&head_id, repo, LOCAL_BRANCH);
    git_oid fetch_head_id;
    git_reference_name_to_id(&fetch_head_id, repo, "FETCH_HEAD");

    //checkout main branch
    git_object* main_obj;
    git_object_lookup(&main_obj, repo, &head_id, GIT_OBJECT_ANY);
    git_checkout_options checkout_options = GIT_CHECKOUT_OPTIONS_INIT;
    checkout_options.checkout_strategy = GIT_CHECKOUT_FORCE;
    error = git_checkout_tree(repo, main_obj, &checkout_options);
    if(error != 0) {
        showMessage("error checking out main branch: ", std::string(giterr_last()->message));
        git_object_free(main_obj);
        git_stash_pop(repo, 0, &apply_options);
        git_status_list_free(status_list);
        git_remote_free(remote);
        git_repository_free(repo);
        git_libgit2_shutdown();
    }
    git_object_free(main_obj);

    if(!git_oid_equal(&fetch_head_id, &head_id)) {
        // Differences found, perform fast forward
        git_reference * head_ref = nullptr;
        git_reference_lookup(&head_ref, repo, LOCAL_BRANCH);

        git_reference * new_head_ref = nullptr;
        git_reference_set_target(&new_head_ref, head_ref, &fetch_head_id, "pull: fast-forward");

        git_commit* latest = nullptr;

        git_commit_lookup(&latest, repo, &fetch_head_id);
        git_reset(repo, (git_object*)latest, GIT_RESET_HARD, NULL);

        git_reference_free(new_head_ref);
        git_reference_free(head_ref);

        std::string new_sector_version = getSctVersion(repoPath);

        //re apply custom settings
        git_stash_pop(repo, 0, &apply_options);

        //if the naughty user changed the .sct name, change it back

        std::ifstream inputFile(repoPath + "/Data/Sector/Hong-Kong-Sector-File.sct");
        std::string line;
        std::stringstream updatedContent;
        bool foundInfo = false;

        while (std::getline(inputFile, line)) {
            if (foundInfo) {
                updatedContent << "Hong Kong Sector Version " << new_sector_version << std::endl;
                foundInfo = false;
            } else if (line.find("[INFO]") != std::string::npos) {
                foundInfo = true;
                updatedContent << line << std::endl;
            } else {
                updatedContent << line << std::endl;
            }
        }
        inputFile.close();

        std::ofstream outputFile(repoPath + "/Data/Sector/Hong-Kong-Sector-File.sct");
        outputFile << updatedContent.str();
        outputFile.close();

        showMessage("Sector package updated to version " + new_sector_version);
    } else {
        git_stash_pop(repo, 0, &apply_options);
        showMessage("No sector package updates found.");
    }

    // Clean up
    git_status_list_free(status_list);
    git_remote_free(remote);
    git_repository_free(repo);
    git_libgit2_shutdown();
}

void MainWindow::migrateOldInstall() {

    // Select the repository path
    std::string repoPath = selectRepositoryPath();
    if (repoPath == "") return;

    // Initialize the library
    git_libgit2_init();

    // Open the repository or clone if it doesn't exist
    git_repository *repo = nullptr;

    int error = git_repository_open(&repo, repoPath.c_str());
    if(error == 0) {
        showMessage("New install detected. Use the Update button instead");

        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }

    std::ifstream checkFile(repoPath + "/Data/Sector/Hong-Kong-Sector-File.sct");
    if(!checkFile.is_open()) {
        showMessage("Unable to locate Hong Kong Sector File in this folder");
        git_libgit2_shutdown();
        return;
    }
    checkFile.close();

    git_clone_options clone_options = GIT_CLONE_OPTIONS_INIT;
    showMessage("Cloning Repository...");
    repaint();
    std::string temp_dir = QCoreApplication::applicationDirPath().toStdString() + "/temp";
    error = git_clone(&repo, SECTOR_PACKAGE, temp_dir.c_str(), &clone_options);
    if (error != 0) {
        showMessage("Error cloning repository: ", std::string(giterr_last()->message));
        git_libgit2_shutdown();
        return;
    }

    git_repository_free(repo);
    git_libgit2_shutdown();

    showMessage("Copying your existing settings...");
    repaint();

    //copy common files with settings into the cloned repo
    std::filesystem::remove(temp_dir + "/Data/Plugins/TopSky/TopSkyCPDLChoppieCode.txt");
    std::filesystem::remove(temp_dir + "/Data/Plugins/TopSky/TopSkySettings.txt");
    std::filesystem::remove(temp_dir + "/Data/Settings/GeneralSettings.txt");
    std::filesystem::remove(temp_dir + "/Data/Settings/Lists.txt");
    std::filesystem::remove(temp_dir + "/Data/Settings/LoginProfiles.txt");
    std::filesystem::remove(temp_dir + "/Data/Settings/Plugins.txt");
    std::filesystem::remove(temp_dir + "/Data/Settings/ScreenLayout.txt");
    std::filesystem::remove(temp_dir + "/Data/Settings/Symbology.txt");
    std::filesystem::remove(temp_dir + "/Data/Settings/Symbology_black.txt");
    std::filesystem::remove(temp_dir + "/Data/Settings/Symbology_blue.txt");
    std::filesystem::remove(temp_dir + "/Hong Kong TOPSKY.prf");
    std::filesystem::copy(repoPath + "/Data/Plugins/TopSky/TopSkyCPDLChoppieCode.txt", temp_dir + "/Data/Plugins/TopSky/TopSkyCPDLChoppieCode.txt", std::filesystem::copy_options::overwrite_existing);
    std::filesystem::copy(repoPath + "/Data/Plugins/TopSky/TopSkySettings.txt", temp_dir + "/Data/Plugins/TopSky/TopSkySettings.txt", std::filesystem::copy_options::overwrite_existing);
    std::filesystem::copy(repoPath + "/Data/Settings/GeneralSettings.txt", temp_dir + "/Data/Settings/GeneralSettings.txt", std::filesystem::copy_options::overwrite_existing);
    std::filesystem::copy(repoPath + "/Data/Settings/Lists.txt", temp_dir + "/Data/Settings/Lists.txt", std::filesystem::copy_options::overwrite_existing);
    std::filesystem::copy(repoPath + "/Data/Settings/LoginProfiles.txt", temp_dir + "/Data/Settings/LoginProfiles.txt", std::filesystem::copy_options::overwrite_existing);
    std::filesystem::copy(repoPath + "/Data/Settings/Plugins.txt", temp_dir + "/Data/Settings/Plugins.txt", std::filesystem::copy_options::overwrite_existing);
    std::filesystem::copy(repoPath + "/Data/Settings/ScreenLayout.txt", temp_dir + "/Data/Settings/ScreenLayout.txt", std::filesystem::copy_options::overwrite_existing);
    std::filesystem::copy(repoPath + "/Data/Settings/Symbology.txt", temp_dir + "/Data/Settings/Symbology.txt", std::filesystem::copy_options::overwrite_existing);
    std::filesystem::copy(repoPath + "/Data/Settings/Symbology_black.txt", temp_dir + "/Data/Settings/Symbology_black.txt", std::filesystem::copy_options::overwrite_existing);
    std::filesystem::copy(repoPath + "/Data/Settings/Symbology_blue.txt", temp_dir + "/Data/Settings/Symbology_blue.txt", std::filesystem::copy_options::overwrite_existing);
    std::filesystem::copy(repoPath + "/Hong Kong TOPSKY.prf", temp_dir + "/Hong Kong TOPSKY.prf", std::filesystem::copy_options::overwrite_existing);

    // overwrite everything else
    std::filesystem::remove_all(repoPath);
    repoPath = repoPath.substr(0, repoPath.std::string::find_last_of("\\/"));
    repoPath += "/Hong-Kong-Sector-Package";
    std::filesystem::copy(temp_dir, repoPath, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);

    showMessage("Cleaning up...");
    repaint();
    deleteLegacyFiles(repoPath);
    int count = 0;
    while (count < 5)   //give up after 5, there should be just 2 in a git repo
    {
        try
        {
            std::filesystem::remove_all(temp_dir);
            break;     // all done
        }
        catch (std::filesystem::filesystem_error &e)
        {
            std::this_thread::sleep_for (std::chrono::milliseconds(250));
            std::string f ( e.what() );
            int p = f.find("[");
            p = f.find("[",p+1);
            f = f.substr(p+1);
            f = f.substr(0,f.length()-1);
            std::filesystem::permissions(
                f, std::filesystem::perms::owner_write );
            count++;
        }
    }

    /* git merge method...maybe fix in the future
    git_merge_file_input ancestor_file = GIT_MERGE_FILE_INPUT_INIT;
    ancestor_file.path = NULL;
    ancestor_file.ptr = getFileContents("../ancestor.txt").c_str();
    ancestor_file.size = strlen(ancestor_file.ptr);

    git_merge_file_input ours_file = GIT_MERGE_FILE_INPUT_INIT;
    ours_file.path = NULL;
    ours_file.ptr = getFileContents("../ours.txt").c_str(); // Content from our branch
    ours_file.size = strlen(ours_file.ptr);

    git_merge_file_input theirs_file = GIT_MERGE_FILE_INPUT_INIT;
    theirs_file.path = NULL;
    theirs_file.ptr = getFileContents("../theirs.txt").c_str(); // Content from their branch
    theirs_file.size = strlen(theirs_file.ptr);

    git_merge_file_result result;
    git_merge_file_options merge_options = GIT_MERGE_FILE_OPTIONS_INIT;
    merge_options.favor = GIT_MERGE_FILE_FAVOR_THEIRS;
    git_merge_file(&result, &ancestor_file, &ours_file, &theirs_file, &merge_options);

    std::ofstream result_file;
    result_file.open("./result.txt");
    result_file.write((char*)result.ptr, sizeof(result.ptr));
    result_file.close();
    */

    showMessage("Sector File Updated to " + getSctVersion(repoPath));
}


void MainWindow::handleInstallButton() {
    installButton->setEnabled(false);
    updateButton->setEnabled(false);
    migrateOldButton->setEnabled(false);

    installPackage();

    installButton->setEnabled(true);
    updateButton->setEnabled(true);
    migrateOldButton->setEnabled(true);
}

void MainWindow::handleUpdateButton() {
    installButton->setEnabled(false);
    updateButton->setEnabled(false);
    migrateOldButton->setEnabled(false);

    updatePackage();

    installButton->setEnabled(true);
    updateButton->setEnabled(true);
    migrateOldButton->setEnabled(true);
}

void MainWindow::handleMigrateButton() {
    installButton->setEnabled(false);
    updateButton->setEnabled(false);
    migrateOldButton->setEnabled(false);

    migrateOldInstall();

    installButton->setEnabled(true);
    updateButton->setEnabled(true);
    migrateOldButton->setEnabled(true);
}
