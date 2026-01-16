#ifndef CYBERFORCE_H
#define CYBERFORCE_H

#include <stdbool.h>
#include <time.h>

#define MAX_THREADS 100
#define MAX_WORD_LEN 256
#define MAX_URL_LEN 1024
#define MAX_DATA_LEN 4096
#define MAX_PROXY_LIST 1000

typedef enum {
    MODULE_HTTP,
    MODULE_FTP,
    MODULE_SSH,
    MODULE_MYSQL
} ModuleType;

typedef enum {
    OUTPUT_PLAIN,
    OUTPUT_JSON,
    OUTPUT_CSV,
    OUTPUT_XML
} OutputFormat;

typedef struct {
    char *target;
    int port;
    char *protocol;
    char *userlist_file;
    char *passlist_file;
    char *username;
    char *password;
    char *output_file;
    char *success_pattern;
    char *failure_pattern;
    char *user_agent;
    char *proxy;
    char **proxy_list;
    int proxy_count;
    char *method;
    char *post_data;
    char *cookie;
    char *referer;
    int threads;
    int delay;
    int jitter;
    int timeout;
    int max_retries;
    int rate_limit;
    int verbose;
    bool stop_on_success;
    bool randomize;
    bool rotate_ua;
    bool use_tor;
    bool ssl_verify;
    ModuleType module_type;
    OutputFormat output_format;
    time_t start_time;
    time_t end_time;
} Config;

typedef struct {
    char *username;
    char *password;
    char *target;
    int port;
    int status;  // 0=failure, 1=success, 2=timeout, 3=error
    long http_code;
    size_t response_size;
    double response_time;
    time_t timestamp;
} AttackResult;

// Function prototypes
void print_banner();
int parse_arguments(int argc, char *argv[], Config *config);
int start_attack(Config *config);
void attack_stop();
void cleanup_config(Config *config);
ModuleType get_module_type(const char *module_str);
const char *get_module_str(ModuleType type);

// Thread management
typedef struct thread_pool ThreadPool;
ThreadPool *thread_pool_create(int num_threads);
void thread_pool_destroy(ThreadPool *pool);
void thread_pool_add_task(ThreadPool *pool, void (*task)(void *), void *arg);

// Rate limiting
typedef struct rate_limiter RateLimiter;
RateLimiter *rate_limiter_create(int requests_per_minute);
void rate_limiter_destroy(RateLimiter *limiter);
void rate_limiter_wait(RateLimiter *limiter);

// Wordlist operations
char **load_wordlist(const char *filename, int *count);
void free_wordlist(char **wordlist, int count);
void shuffle_wordlist(char **wordlist, int count);
void generate_wordlist(const char *base_word, const char *output_file, int rules);

// Pattern matching
bool pattern_match(const char *text, const char *pattern, bool use_regex);
bool extract_pattern(const char *text, const char *pattern, char *result, size_t size);

// Proxy management
int load_proxy_list(const char *filename, char ***proxy_list);
const char *get_next_proxy(Config *config);
void rotate_proxy(Config *config);

// Logging
void log_message(const char *message, Config *config);
void log_success(Config *config, const char *username, const char *password, 
                 long http_code, const char *response);
void log_failure(Config *config, const char *username, const char *password, 
                 int error_code, const char *error_msg);
void save_results(Config *config, AttackResult *results, int count);

// Crypto utilities
char *md5_hash(const char *input);
char *sha256_hash(const char *input);
char *base64_encode(const char *input);
char *base64_decode(const char *input);
char *generate_random_string(int length);

// HTTP module
int http_attack(Config *config, const char *username, const char *password, 
                AttackResult *result);

// FTP module
int ftp_attack(Config *config, const char *username, const char *password, 
               AttackResult *result);

// SSH module
int ssh_attack(Config *config, const char *username, const char *password, 
               AttackResult *result);

// MySQL module
int mysql_attack(Config *config, const char *username, const char *password, 
                 AttackResult *result);

#endif