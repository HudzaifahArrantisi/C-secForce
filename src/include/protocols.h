#ifndef PROTOCOLS_H
#define PROTOCOLS_H

#include "cyberforce.h"

// HTTP specific
typedef struct {
    char *url;
    char *method;
    char *post_data;
    char *headers;
    char *cookie;
    char *referer;
    bool follow_redirects;
    bool use_ssl;
    bool verify_ssl;
} HttpConfig;

// FTP specific
typedef struct {
    char *host;
    int port;
    bool anonymous;
    bool passive_mode;
    int timeout;
} FtpConfig;

// SSH specific
typedef struct {
    char *host;
    int port;
    int timeout;
    char *private_key;
    bool use_key;
} SshConfig;

// MySQL specific
typedef struct {
    char *host;
    int port;
    char *database;
    int timeout;
} MysqlConfig;

// Protocol detection
typedef enum {
    PROTO_UNKNOWN,
    PROTO_HTTP,
    PROTO_HTTPS,
    PROTO_FTP,
    PROTO_FTPS,
    PROTO_SSH,
    PROTO_MYSQL,
    PROTO_POSTGRES,
    PROTO_MSSQL
} ProtocolType;

ProtocolType detect_protocol(const char *url);
const char *protocol_to_string(ProtocolType proto);

// Response parsing
typedef struct {
    long http_code;
    char *headers;
    char *body;
    size_t body_size;
    double total_time;
    double connect_time;
    double namelookup_time;
} HttpResponse;

HttpResponse *http_response_create();
void http_response_destroy(HttpResponse *resp);

// Authentication schemes
typedef enum {
    AUTH_NONE,
    AUTH_BASIC,
    AUTH_DIGEST,
    AUTH_NTLM,
    AUTH_BEARER,
    AUTH_OAUTH2
} AuthScheme;

typedef struct {
    AuthScheme scheme;
    char *realm;
    char *nonce;
    char *qop;
    char *username;
    char *password;
} AuthContext;

#endif