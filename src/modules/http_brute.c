#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>

#ifdef _WIN32
    #define strcasecmp _stricmp
#endif

#include "include/cyberforce.h"
#include "include/defines.h"
#include "include/protocols.h"

struct ResponseData {
    char *data;
    size_t size;
    long http_code;
    double total_time;
    double connect_time;
    double namelookup_time;
};

static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    struct ResponseData *resp = (struct ResponseData *)userdata;
    size_t total_size = size * nmemb;
    
    char *new_data = realloc(resp->data, resp->size + total_size + 1);
    if (!new_data) return 0;
    
    resp->data = new_data;
    memcpy(resp->data + resp->size, ptr, total_size);
    resp->size += total_size;
    resp->data[resp->size] = '\0';
    
    return total_size;
}

static int debug_callback(CURL *handle, curl_infotype type, char *data, size_t size, void *userptr) {
    Config *config = (Config *)userptr;
    
    if (config->verbose < 4) return 0;
    
    switch (type) {
        case CURLINFO_TEXT:
            printf("[CURL] %.*s", (int)size, data);
            break;
        case CURLINFO_HEADER_IN:
            printf("[CURL] Received header: %.*s", (int)size, data);
            break;
        case CURLINFO_HEADER_OUT:
            printf("[CURL] Sent header: %.*s", (int)size, data);
            break;
        case CURLINFO_DATA_IN:
        case CURLINFO_DATA_OUT:
        case CURLINFO_SSL_DATA_IN:
        case CURLINFO_SSL_DATA_OUT:
            // Skip binary data
            break;
        default:
            break;
    }
    
    return 0;
}

char *replace_placeholders(const char *template, const char *username, const char *password) {
    if (!template) return NULL;
    
    size_t template_len = strlen(template);
    size_t user_len = username ? strlen(username) : 0;
    size_t pass_len = password ? strlen(password) : 0;
    
    // Calculate maximum possible size
    size_t max_size = template_len + user_len + pass_len + 1;
    char *result = malloc(max_size);
    if (!result) return NULL;
    
    const char *src = template;
    char *dst = result;
    
    while (*src) {
        if (strncmp(src, "{USER}", 6) == 0) {
            if (username) {
                strcpy(dst, username);
                dst += user_len;
            }
            src += 6;
        } else if (strncmp(src, "{PASS}", 6) == 0) {
            if (password) {
                strcpy(dst, password);
                dst += pass_len;
            }
            src += 6;
        } else if (strncmp(src, "{USER_URL}", 10) == 0) {
            if (username) {
                char *encoded = curl_easy_escape(NULL, username, 0);
                if (encoded) {
                    strcpy(dst, encoded);
                    dst += strlen(encoded);
                    curl_free(encoded);
                }
            }
            src += 10;
        } else if (strncmp(src, "{PASS_URL}", 10) == 0) {
            if (password) {
                char *encoded = curl_easy_escape(NULL, password, 0);
                if (encoded) {
                    strcpy(dst, encoded);
                    dst += strlen(encoded);
                    curl_free(encoded);
                }
            }
            src += 10;
        } else if (strncmp(src, "{USER_B64}", 10) == 0) {
            if (username) {
                char *encoded = base64_encode(username);
                if (encoded) {
                    strcpy(dst, encoded);
                    dst += strlen(encoded);
                    free(encoded);
                }
            }
            src += 10;
        } else if (strncmp(src, "{PASS_B64}", 10) == 0) {
            if (password) {
                char *encoded = base64_encode(password);
                if (encoded) {
                    strcpy(dst, encoded);
                    dst += strlen(encoded);
                    free(encoded);
                }
            }
            src += 10;
        } else {
            *dst++ = *src++;
        }
    }
    
    *dst = '\0';
    return result;
}

char *get_random_user_agent() {
    const char *user_agents[] = {
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:89.0) Gecko/20100101 Firefox/89.0",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.1.1 Safari/605.1.15",
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.114 Safari/537.36",
        "Mozilla/5.0 (iPhone; CPU iPhone OS 14_6 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.0 Mobile/15E148 Safari/604.1",
        "Mozilla/5.0 (iPad; CPU OS 14_6 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.0 Mobile/15E148 Safari/604.1",
        "Mozilla/5.0 (Linux; Android 10; SM-G973F) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.114 Mobile Safari/537.36",
        "CyberForce/2.0 Security Scanner",
        "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0)",
        "Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko"
    };
    
    int count = sizeof(user_agents) / sizeof(user_agents[0]);
    int index = rand() % count;
    
    return strdup(user_agents[index]);
}

int http_attack(Config *config, const char *username, const char *password, AttackResult *result) {
    CURL *curl = NULL;
    CURLcode res;
    struct curl_slist *headers = NULL;
    struct ResponseData response;
    char *post_data = NULL;
    char *user_agent = NULL;
    char url[BUFFER_LARGE];
    
    memset(&response, 0, sizeof(response));
    
    // Build URL
    if (config->port == 80 || config->port == 443) {
        snprintf(url, sizeof(url), "%s", config->target);
    } else {
        snprintf(url, sizeof(url), "%s:%d", config->target, config->port);
    }
    
    // Initialize CURL
    curl = curl_easy_init();
    if (!curl) {
        if (config->verbose >= 1) {
            fprintf(stderr, "[-] Failed to initialize CURL\n");
        }
        return 0;
    }
    
    // Set User-Agent
    if (config->user_agent) {
        if (strcmp(config->user_agent, "random") == 0 || config->rotate_ua) {
            user_agent = get_random_user_agent();
            curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent);
        } else {
            curl_easy_setopt(curl, CURLOPT_USERAGENT, config->user_agent);
        }
    } else {
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "CyberForce/2.0");
    }
    
    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url);
    
    // Set timeout
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, config->timeout);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, config->timeout);
    
    // Set SSL options
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, config->ssl_verify ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, config->ssl_verify ? 2L : 0L);
    
    // Set proxy
    if (config->proxy) {
        curl_easy_setopt(curl, CURLOPT_PROXY, config->proxy);
        
        if (config->use_tor) {
            // Special handling for Tor
            curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5_HOSTNAME);
        }
    }
    
    // Set write callback
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    // Set debug callback for verbose output
    if (config->verbose >= 4) {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debug_callback);
        curl_easy_setopt(curl, CURLOPT_DEBUGDATA, config);
    }
    
    // Set headers
    headers = curl_slist_append(headers, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    headers = curl_slist_append(headers, "Accept-Language: en-US,en;q=0.5");
    headers = curl_slist_append(headers, "Accept-Encoding: gzip, deflate");
    headers = curl_slist_append(headers, "DNT: 1");
    headers = curl_slist_append(headers, "Connection: keep-alive");
    headers = curl_slist_append(headers, "Upgrade-Insecure-Requests: 1");
    
    if (config->cookie) {
        curl_easy_setopt(curl, CURLOPT_COOKIE, config->cookie);
    }
    
    if (config->referer) {
        curl_easy_setopt(curl, CURLOPT_REFERER, config->referer);
    }
    
    // Set HTTP method and data
    if (config->method) {
        if (strcasecmp(config->method, "POST") == 0) {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            
            if (config->post_data) {
                post_data = replace_placeholders(config->post_data, username, password);
                if (post_data) {
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
                    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
                }
            }
        } else if (strcasecmp(config->method, "GET") == 0) {
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        } else if (strcasecmp(config->method, "PUT") == 0) {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            
            if (config->post_data) {
                post_data = replace_placeholders(config->post_data, username, password);
                if (post_data) {
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
                    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
                }
            }
        } else {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, config->method);
        }
    } else {
        // Default to POST if data is provided, otherwise GET
        if (config->post_data) {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            post_data = replace_placeholders(config->post_data, username, password);
            if (post_data) {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
                headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            }
        } else {
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        }
    }
    
    // Apply headers
    if (headers) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }
    
    // Perform request
    res = curl_easy_perform(curl);
    
    // Get response info
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.http_code);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &response.total_time);
        curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &response.connect_time);
        curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME, &response.namelookup_time);
        
        if (result) {
            result->http_code = response.http_code;
            result->response_size = response.size;
            result->response_time = response.total_time;
        }
    }
    
    // Determine success
    int success = 0;
    
    if (res == CURLE_OK) {
        // Check HTTP status code patterns
        if (response.http_code >= 200 && response.http_code < 300) {
            success = 1;
        } else if (response.http_code == 401 || response.http_code == 403) {
            success = 0;
        } else {
            success = 0;
        }
        
        // Check custom success/failure patterns
        if (config->success_pattern && response.data) {
            if (pattern_match(response.data, config->success_pattern, false)) {
                success = 1;
            }
        }
        
        if (config->failure_pattern && response.data) {
            if (pattern_match(response.data, config->failure_pattern, false)) {
                success = 0;
            }
        }
        
        // Debug output
        if (config->verbose >= 3) {
            printf("[DEBUG] HTTP %ld | Size: %zu | Time: %.3fs | User: %s\n",
                   response.http_code, response.size, response.total_time, username);
        }
    } else {
        if (config->verbose >= 2) {
            fprintf(stderr, "[-] CURL error: %s\n", curl_easy_strerror(res));
        }
    }
    
    // Cleanup
    if (post_data) free(post_data);
    if (user_agent) free(user_agent);
    if (response.data) free(response.data);
    if (headers) curl_slist_free_all(headers);
    if (curl) curl_easy_cleanup(curl);
    
    return success ? 1 : 0;
}
