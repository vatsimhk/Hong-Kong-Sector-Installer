#include "mainwindow.h"
#include "libgit2_callbacks.h"

extern MainWindow* g_mainWindow;

int fetch_progress_cb(const git_indexer_progress *stats, void *payload) {
    /* Do something with network transfer progress */
    int fetch_percent =
        (100 * stats->received_objects) /
        stats->total_objects;
    int index_percent =
        (100 * stats->indexed_objects) /
        stats->total_objects;
    int kbytes = stats->received_bytes / 1024;

    g_mainWindow->setProgressBarMax(stats->total_objects);
    g_mainWindow->setProgressBarMin(0);

    std::stringstream progressMessage;
    if(fetch_percent == 100) {
        progressMessage << "Indexing " << index_percent << "% (" << stats->indexed_objects << "/" << stats->total_objects << " objects)";
        g_mainWindow->setProgressBarValue(stats->indexed_objects);
    } else {
        progressMessage << "Downloading " << fetch_percent << "% (" << kbytes << " kb, " << stats->received_objects << "/" << stats->total_objects << " objects)" << std::endl << std::endl;
        g_mainWindow->setProgressBarValue(stats->received_objects);
    }

    g_mainWindow->setProgressBarText(progressMessage.str());
    g_mainWindow->repaint();
    return 0;
}
