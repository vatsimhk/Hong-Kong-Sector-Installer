#ifndef LIBGIT2_CALLBACKS_H
#define LIBGIT2_CALLBACKS_H

/*
 * This file contains callbacks used for libgit2 functions.
 *
 * Since libgit2 is written in pure C it does not support classes and objects
 * so all functions here are classless and act as utility functions only
*/

#include <git2.h>

// Callback during git fetch / clone to display progress in progress bar
int fetch_progress_cb(const git_indexer_progress *stats, void *payload);

#endif // LIBGIT2_CALLBACKS_H
