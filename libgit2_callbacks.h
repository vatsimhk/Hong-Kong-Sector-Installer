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

// Callback for the user's custom certificate checks
int proxy_transport_certificate_check_cb(git_cert *cert, int valid, const char *host, void *payload);

// Credential acquisition callback. For now, only supports plaintext username and password
int proxy_credential_acquire_cb(git_credential **out, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload);

#endif // LIBGIT2_CALLBACKS_H
