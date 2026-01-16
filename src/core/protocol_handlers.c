#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/cyberforce.h"
#include "include/defines.h"

int handle_http_protocol(Config *config, const char *username, const char *password, AttackResult *result) {
    return http_attack(config, username, password, result);
}

int handle_ftp_protocol(Config *config, const char *username, const char *password, AttackResult *result) {
    return ftp_attack(config, username, password, result);
}

int handle_ssh_protocol(Config *config, const char *username, const char *password, AttackResult *result) {
    return ssh_attack(config, username, password, result);
}

int handle_mysql_protocol(Config *config, const char *username, const char *password, AttackResult *result) {
    return mysql_attack(config, username, password, result);
}
