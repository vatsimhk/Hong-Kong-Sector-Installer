#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "libgit2_callbacks.h"
#include <thread>

const char* SECTOR_PACKAGE = "https://github.com/vatsimhk/Hong-Kong-Sector-Package.git";
const char* REMOTE_REF = "origin";
const char* REMOTE_BRANCH = "refs/remotes/origin/main";
const char* LOCAL_BRANCH = "refs/heads/main";

MainWindow *g_mainWindow;

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
    std::filesystem::remove_all(repoPath + "/Data/ASR/Legacy ASR/");
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

char *convert_cstring(const std::string & s)
{
    char *pc = new char[s.size()+1];
    std::strcpy(pc, s.c_str());
    return pc;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Create a layout to manage widgets
    textLabel = ui->messageBox;
    errorLabel = ui->errorBox;
    installButton = ui->installButton;
    updateButton = ui->updateButton;
    colourThemeButton = ui->colourThemeButton;
    repairButton = ui->repairButton;
    optionsButton = ui->optionsButton;

    advanced_options_dialog = new optionsDialog;

    QPixmap logo(":/images/hkvacc-blue.png");
    QPixmap logoScaled = logo.scaled(380, 380, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->logoLabel->setPixmap(logoScaled);
    ui->logoLabel->setAlignment(Qt::AlignCenter);

    ui->progressBar->setVisible(false);
    ui->progressBar->setAlignment(Qt::AlignCenter);

    ui->messageBox->setAlignment(Qt::AlignCenter);
    ui->errorBox->setAlignment(Qt::AlignCenter);

    g_mainWindow = this;
}

MainWindow::~MainWindow()
{
    delete advanced_options_dialog;
    delete ui;
}

void MainWindow::showMessage(const std::string& message)
{
    QString q = QString::fromStdString(message);
    textLabel -> setText(q);
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setWordWrap(true);

    QString e = QString::fromStdString("");
    errorLabel -> setText(e);
}

void MainWindow::showMessage(const std::string& message, const std::string& errorMessage)
{
    QString q = QString::fromStdString(message);
    textLabel -> setText(q);
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setWordWrap(true);

    QString e = QString::fromStdString(errorMessage);
    errorLabel -> setText(e);
    errorLabel->setAlignment(Qt::AlignCenter);
    errorLabel->setWordWrap(true);
}

void MainWindow::set_proxy_settings(git_clone_options& clone_opts) {
    std::string proxy_URL = advanced_options_dialog->get_proxy_URL();
    clone_opts.fetch_opts.proxy_opts = GIT_PROXY_OPTIONS_INIT;
    if(proxy_URL == "") {
       clone_opts.fetch_opts.proxy_opts.type = GIT_PROXY_AUTO;
       return;
    }

    clone_opts.fetch_opts.proxy_opts.type = GIT_PROXY_SPECIFIED;
    clone_opts.fetch_opts.proxy_opts.url = proxy_URL.c_str();

    std::string proxy_username = advanced_options_dialog->get_proxy_username();
    std::string proxy_password = advanced_options_dialog->get_proxy_password();

    if(proxy_username == "" || proxy_password == "" ) {
       return;
    }
}

void MainWindow::installPackage() {
    // Initialize the library
    git_libgit2_init();

    // Select the repository path
    std::string repoPath = selectRepositoryPath();
    if (repoPath == "") return;

    // Open the repository or clone if it doesn't exist
    git_repository *repo = nullptr;

    std::ifstream checkFile(repoPath + "/Data/Sector/Hong-Kong-Sector-File.sct");
    if(checkFile.is_open()) {
       checkFile.close();
       git_libgit2_shutdown();
       showMessage("Folder already contains Hong Kong Sector Package");
       return;
    }

    int error = git_repository_open(&repo, repoPath.c_str());
    if (error == GIT_ENOTFOUND) {
        git_clone_options clone_options = GIT_CLONE_OPTIONS_INIT;
        clone_options.fetch_opts.callbacks.transfer_progress = fetch_progress_cb;
        set_proxy_settings(clone_options);

        repoPath += "/Hong-Kong-Sector-Package";
        showMessage("Cloning Repository...");
        ui->progressBar->reset();
        ui->progressBar->setVisible(true);
        repaint();
        error = git_clone(&repo, SECTOR_PACKAGE, repoPath.c_str(), &clone_options);
        if (error != 0) {
            showMessage("Error cloning repository: ", std::string(giterr_last()->message));
            git_libgit2_shutdown();
            ui->progressBar->setVisible(false);
            return;
        }
        ui->progressBar->setVisible(false);

        git_repository_free(repo);

        deleteLegacyFiles(repoPath);
        showMessage("Hong Kong Sector Package Successfully Installed (" + getSctVersion(repoPath) + ")");
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
        std::ifstream checkFile(repoPath + "/Data/Sector/Hong-Kong-Sector-File.sct");
        if(checkFile.is_open()) {
            checkFile.close();
            git_libgit2_shutdown();
            migrateOldInstall(repoPath);
            return;
        }
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

    git_index* index = nullptr;
    git_commit* latest = nullptr;
    git_oid head_id;
    git_reference_name_to_id(&head_id, repo, LOCAL_BRANCH);
    git_commit_lookup(&latest, repo, &head_id);
    // Reset to remove unstage any changes
    git_reset(repo, (git_object*)latest, GIT_RESET_MIXED, NULL);

    //stash settings
    git_stash_save_options save_options = GIT_STASH_SAVE_OPTIONS_INIT;
    git_stash_apply_options apply_options = GIT_STASH_APPLY_OPTIONS_INIT;
    git_oid saved_stash;
    git_signature* default_signature = nullptr;
    error = git_signature_default(&default_signature, repo);
    if(error == GIT_ENOTFOUND) {
        git_signature_now(&default_signature, "HKvACC", "info@vathk.com");
    }

    showMessage("Stashing your changes...");
    repaint();
    error = git_stash_save(&saved_stash, repo, default_signature, "", 0);
    if(error != 0 && error != GIT_ENOTFOUND) {
        showMessage("Error stashing changes: ", std::string(giterr_last()->message));
        git_remote_free(remote);
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }
    git_signature_free(default_signature);

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
    fetch_options.callbacks.transfer_progress = fetch_progress_cb;
    ui->progressBar->reset();
    ui->progressBar->setVisible(true);
    repaint();

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

    git_oid fetch_head_id;
    git_reference_name_to_id(&fetch_head_id, repo, "FETCH_HEAD");

    //checkout main branch
    git_object* main_obj;
    git_object_lookup(&main_obj, repo, &head_id, GIT_OBJECT_ANY);
    git_checkout_options checkout_options = GIT_CHECKOUT_OPTIONS_INIT;
    checkout_options.checkout_strategy = GIT_CHECKOUT_FORCE;
    error = git_checkout_tree(repo, main_obj, &checkout_options);
    //MOVE HEAD TO DESIRED BRANCH
    error = git_repository_set_head(repo, LOCAL_BRANCH);
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

    ui->progressBar->setVisible(false);
    repaint();

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

        //resolve merge conflicts
        index = nullptr;
        git_repository_index(&index, repo);
        if (git_index_has_conflicts(index))
        {
            mergeConflictDialog* conflict_dialog = new mergeConflictDialog;
            conflict_dialog->setModal(true);
            conflict_dialog->set_repo_path(repoPath);

            git_index_conflict_iterator* conflicts = nullptr;
            git_index_entry *entries[3];
            git_index_conflict_iterator_new(&conflicts, index);
            while (git_index_conflict_next((const git_index_entry**)&entries[0], (const git_index_entry**)&entries[1], (const git_index_entry**)&entries[2], conflicts)
                   != GIT_ITEROVER)
            {
                conflict_dialog->add_file(entries[2]->path);
            }
            git_index_conflict_iterator_free(conflicts);
            git_index_conflict_cleanup(index);

            conflict_dialog->exec();
        }

        git_reset(repo, (git_object*)latest, GIT_RESET_MIXED, NULL);

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

void MainWindow::migrateOldInstall(std::string repoPath) {

    // Initialize the library
    git_libgit2_init();
    git_repository *repo = nullptr;
    int error;

    /*int error = git_repository_open(&repo, repoPath.c_str());
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
    checkFile.close();*/

    std::string newRepoPath = repoPath.substr(0, repoPath.std::string::find_last_of("\\/"));
    std::filesystem::rename(repoPath, newRepoPath + "/Hong-Kong-Sector (Old Backup)");
    repoPath = newRepoPath + "/Hong-Kong-Sector (Old Backup)";
    newRepoPath += "/Hong-Kong-Sector-Package";

    migrateDialog* m_dialog = new migrateDialog;
    m_dialog->set_repo_paths(newRepoPath, repoPath);
    m_dialog->exec();

    git_clone_options clone_options = GIT_CLONE_OPTIONS_INIT;
    clone_options.fetch_opts.callbacks.transfer_progress = fetch_progress_cb;
    ui->progressBar->reset();
    ui->progressBar->setVisible(true);
    repaint();
    showMessage("Cloning Repository...");
    repaint();
    error = git_clone(&repo, SECTOR_PACKAGE, newRepoPath.c_str(), &clone_options);
    if (error != 0) {
        showMessage("Error cloning repository: ", std::string(giterr_last()->message));
        git_libgit2_shutdown();
        ui->progressBar->setVisible(false);
        return;
    }

    ui->progressBar->setVisible(false);
    repaint();

    error = git_repository_open(&repo, newRepoPath.c_str());
    if (error != 0) {
        showMessage("Error opening repository after cloning: ", std::string(giterr_last()->message));
        git_libgit2_shutdown();
        return;
    }

    git_remote *remote = nullptr;
    git_remote_lookup(&remote, repo, REMOTE_REF);

    std::string user_version = getSctVersion(repoPath);
    std::string tag = "refs/tags/" + user_version.substr(0, 4);

    git_reference *ref;
    git_reference_lookup(&ref, repo, LOCAL_BRANCH);

    git_reference *new_ref;
    error = git_reference_lookup(&new_ref, repo, tag.c_str());
    if(error != 0) {
        showMessage("Unable to detect sector file version\n\nThis usually occurs if you changed the version name in the .sct.", std::string(giterr_last()->message));
        git_reference_free(ref);
        git_remote_free(remote);
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }
    git_revwalk *walker;
    git_revwalk_new(&walker, repo);
    git_revwalk_push_ref(walker, tag.c_str());

    git_oid id;
    git_revwalk_next(&id, walker);

    git_reference_set_target(&new_ref, ref, &id,NULL);

    if (0 != git_repository_set_head_detached(repo, &id)) std::cerr << "problem occured while detaching head" << std::endl;


    git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
    opts.checkout_strategy = GIT_CHECKOUT_FORCE;
    if (0 != git_checkout_head(repo, &opts)) std::cout << "problem checkout head" << std::endl;

    git_revwalk_free(walker);

    git_commit* latest = nullptr;
    git_oid fetch_head_id;
    git_reference_name_to_id(&fetch_head_id, repo, "FETCH_HEAD");
    git_commit_lookup(&latest, repo, &fetch_head_id);
    git_reset(repo, (git_object*)latest, GIT_RESET_HARD, NULL);

    showMessage("Copying your existing settings...");
    repaint();

    //copy files into repo
    std::filesystem::remove_all(newRepoPath + "/Data");
    std::filesystem::copy(repoPath + "/Data", newRepoPath + "/Data", std::filesystem::copy_options::recursive);
    std::filesystem::remove(newRepoPath + "/Hong Kong TOPSKY.prf");
    std::filesystem::copy(repoPath + "/Hong Kong TOPSKY.prf", newRepoPath + "/Hong Kong TOPSKY.prf");

    //stash changes
    git_stash_save_options save_options = GIT_STASH_SAVE_OPTIONS_INIT;
    git_stash_apply_options apply_options = GIT_STASH_APPLY_OPTIONS_INIT;

    git_oid saved_stash;
    git_signature* default_signature = nullptr;
    error = git_signature_default(&default_signature, repo);
    if(error == GIT_ENOTFOUND) {
        git_signature_now(&default_signature, "HKvACC", "info@vathk.com");
    }

    showMessage("Stashing your changes...");
    repaint();
    error = git_stash_save(&saved_stash, repo, default_signature, "", 0);
    if(error != 0 && error != GIT_ENOTFOUND) {
        showMessage("Error stashing changes: ", std::string(giterr_last()->message));
        git_remote_free(remote);
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }
    git_signature_free(default_signature);

    //now checkout main branch
    git_object *treeish = NULL;
    git_revparse_single(&treeish, repo, "main");
    git_checkout_tree(repo, treeish, &opts);
    git_repository_set_head(repo, "refs/heads/main");
    git_object_free(treeish);

    //perform fastforward to update
    git_fetch_options fetch_options = GIT_FETCH_OPTIONS_INIT;
    fetch_options.callbacks.transfer_progress = fetch_progress_cb;
    ui->progressBar->reset();
    ui->progressBar->setVisible(true);
    repaint();

    git_remote_fetch(remote, nullptr, &fetch_options, "pull");
    git_reference * head_ref = nullptr;
    git_reference_lookup(&head_ref, repo, LOCAL_BRANCH);

    git_reference * new_head_ref = nullptr;
    git_reference_set_target(&new_head_ref, head_ref, &fetch_head_id, "pull: fast-forward");
    git_reference_name_to_id(&fetch_head_id, repo, "FETCH_HEAD");
    git_commit_lookup(&latest, repo, &fetch_head_id);
    git_reset(repo, (git_object*)latest, GIT_RESET_HARD, NULL);

    ui->progressBar->setVisible(false);
    repaint();

    //re apply custom settings
    git_stash_pop(repo, 0, &apply_options);

    //resolve merge conflicts
    git_index* index = nullptr;
    git_repository_index(&index, repo);
    if (git_index_has_conflicts(index))
    {
        mergeConflictDialog* conflict_dialog = new mergeConflictDialog;
        conflict_dialog->setModal(true);
        conflict_dialog->set_repo_path(newRepoPath);

        git_index_conflict_iterator* conflicts = nullptr;
        git_index_entry *entries[3];
        git_index_conflict_iterator_new(&conflicts, index);
        while (git_index_conflict_next((const git_index_entry**)&entries[0], (const git_index_entry**)&entries[1], (const git_index_entry**)&entries[2], conflicts)
               != GIT_ITEROVER)
        {
            conflict_dialog->add_file(entries[2]->path);
        }
        git_index_conflict_iterator_free(conflicts);
        git_index_conflict_cleanup(index);

        conflict_dialog->exec();
    }

    git_reset(repo, (git_object*)latest, GIT_RESET_MIXED, NULL);
    git_repository_free(repo);
    git_remote_free(remote);
    git_reference_free(ref);
    git_reference_free(new_ref);
    git_libgit2_shutdown();

    deleteLegacyFiles(newRepoPath);
    showMessage("Sector File Migrated and Updated to " + getSctVersion(newRepoPath));
}

void MainWindow::repairPackage() {
    std::string repoPath = selectRepositoryPath();
    if (repoPath == "") return;

    showMessage("Detecting Changes...");
    repaint();

    // Initialize libgit2
    git_libgit2_init();

    // Open the repository
    git_repository* repo = nullptr;
    int error = git_repository_open(&repo, repoPath.c_str());
    if (error != 0) {
        showMessage("Unable to repair Sector Package", "Hong Kong Sector Package not found in this folder.");
        git_libgit2_shutdown();
        return;
    }

    // Get the repository's status
    git_status_options status_options = GIT_STATUS_OPTIONS_INIT;
    git_status_list* status_list = nullptr;
    error = git_status_list_new(&status_list, repo, &status_options);
    if (error != 0) {
        showMessage("Unable to repair Sector Package", giterr_last()->message);
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }

    // Iterate through the status list
    size_t entryCount = git_status_list_entrycount(status_list);
    std::vector<std::string> modified_files_list;
    for (size_t i = 0; i < entryCount; ++i) {
        const git_status_entry* entry = git_status_byindex(status_list, i);

        // Check if the entry represents an uncommitted change
        if (entry->status != GIT_STATUS_CURRENT) {
            const char* path = entry->head_to_index != nullptr ? entry->head_to_index->new_file.path : entry->index_to_workdir->new_file.path;
            modified_files_list.push_back(path);
        }
    }

    repairDialog* repair_dialog = new repairDialog;
    repair_dialog->update_file_list(modified_files_list);
    repair_dialog->exec();

    std::vector<std::string> selected_files_list = repair_dialog->get_selected_files();
    delete repair_dialog;
    if(selected_files_list.empty()) {
        //nothing to repair
        showMessage("Sector Package repair cancelled");
        git_status_list_free(status_list);
        git_repository_free(repo);
        git_libgit2_shutdown();
        return;
    }

    //magic that convers string vector into C style array for libgit2
    std::vector<char*>  selected_files_list_ptr;
    std::transform(selected_files_list.begin(), selected_files_list.end(), std::back_inserter(selected_files_list_ptr), convert_cstring);
    git_checkout_options checkout_options = GIT_CHECKOUT_OPTIONS_INIT;
    checkout_options.checkout_strategy = GIT_CHECKOUT_FORCE; // Want to HARD reset
    checkout_options.paths.strings = &selected_files_list_ptr[0]; // restrict to selected paths
    checkout_options.paths.count = selected_files_list.size();
    git_index* index = nullptr;
    git_repository_index(&index, repo);

    //Perform the reset
    error = git_checkout_index(repo, index, &checkout_options);
    if (error != 0)
        showMessage("Unable to repair Sector Package", giterr_last()->message);

    showMessage("Sector Package repaired successfully");

    // Clean up
    git_status_list_free(status_list);
    git_repository_free(repo);
    git_libgit2_shutdown();
}

void MainWindow::changeColourTheme() {
    std::string repoPath = selectRepositoryPath();
    if (repoPath == "") return;

    git_libgit2_init();
    git_repository *repo = nullptr;

    int error = git_repository_open(&repo, repoPath.c_str());
    if (error != 0) {
        showMessage("Sector Package not found in this folder");
    } else {
        colourThemePicker* colour_theme_picker = new colourThemePicker;
        colour_theme_picker->set_repo_path(repoPath);
        colour_theme_picker->exec();

        if(colour_theme_picker->get_selected_theme() != "") {
            showMessage("Colour theme updated to " + colour_theme_picker->get_selected_theme());
        }
        if(colour_theme_picker->get_error_message() != "") {
            showMessage("Error changing colour theme:", colour_theme_picker->get_error_message());
        }
        delete colour_theme_picker;
    }

    git_libgit2_shutdown();
}

void MainWindow::setProgressBarMax(int value) {
    ui->progressBar->setMaximum(value);
}

void MainWindow::setProgressBarMin(int value) {
    ui->progressBar->setMinimum(value);
}

void MainWindow::setProgressBarValue(int value) {
    ui->progressBar->setValue(value);
}

void MainWindow::setProgressBarText(std::string message) {
    QString progress = QString::fromStdString(message);
    ui->progressBar->setFormat(progress);
}

void MainWindow::enableAllButtons() {
    installButton->setEnabled(true);
    updateButton->setEnabled(true);
    colourThemeButton->setEnabled(true);
    repairButton->setEnabled(true);
    optionsButton->setEnabled(true);
}

void MainWindow::disableAllButtons() {
    installButton->setEnabled(false);
    updateButton->setEnabled(false);
    colourThemeButton->setEnabled(false);
    repairButton->setEnabled(false);
    optionsButton->setEnabled(false);
}

void MainWindow::on_installButton_released()
{
    disableAllButtons();
    installPackage();
    enableAllButtons();
}


void MainWindow::on_updateButton_released()
{
    disableAllButtons();
    updatePackage();
    enableAllButtons();
}


void MainWindow::on_repairButton_released()
{
    disableAllButtons();
    repairPackage();
    enableAllButtons();
}


void MainWindow::on_colourThemeButton_released()
{
    disableAllButtons();
    changeColourTheme();
    enableAllButtons();
}


void MainWindow::on_optionsButton_released()
{
    disableAllButtons();
    advanced_options_dialog->exec();
    enableAllButtons();
}

