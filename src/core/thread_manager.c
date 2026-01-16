#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "include/cyberforce.h"
#include "include/defines.h"

typedef struct task_node {
    void (*function)(void *);
    void *argument;
    struct task_node *next;
} TaskNode;

typedef struct thread_pool {
    pthread_t *threads;
    int thread_count;
    TaskNode *task_head;
    TaskNode *task_tail;
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_cond;
    pthread_cond_t working_cond;
    int working_count;
    int thread_count_alive;
    volatile int shutdown;
} ThreadPool;

static void *thread_pool_worker(void *arg);

ThreadPool *thread_pool_create(int num_threads) {
    if (num_threads <= 0) {
        num_threads = DEFAULT_THREADS;
    }
    if (num_threads > MAX_THREADS) {
        num_threads = MAX_THREADS;
    }
    
    ThreadPool *pool = malloc(sizeof(ThreadPool));
    if (!pool) return NULL;
    
    pool->threads = malloc(sizeof(pthread_t) * num_threads);
    if (!pool->threads) {
        free(pool);
        return NULL;
    }
    
    pool->thread_count = num_threads;
    pool->task_head = NULL;
    pool->task_tail = NULL;
    pool->working_count = 0;
    pool->thread_count_alive = 0;
    pool->shutdown = 0;
    
    pthread_mutex_init(&pool->queue_mutex, NULL);
    pthread_cond_init(&pool->queue_cond, NULL);
    pthread_cond_init(&pool->working_cond, NULL);
    
    // Create worker threads
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&pool->threads[i], NULL, thread_pool_worker, pool) != 0) {
            thread_pool_destroy(pool);
            return NULL;
        }
        pool->thread_count_alive++;
    }
    
    // Wait for threads to start
    while (pool->thread_count_alive != num_threads) {
        usleep(1000);
    }
    
    return pool;
}

void thread_pool_destroy(ThreadPool *pool) {
    if (!pool) return;
    
    pthread_mutex_lock(&pool->queue_mutex);
    pool->shutdown = 1;
    pthread_cond_broadcast(&pool->queue_cond);
    pthread_mutex_unlock(&pool->queue_mutex);
    
    // Wait for all threads to finish
    for (int i = 0; i < pool->thread_count; i++) {
        if (pool->threads[i]) {
            pthread_join(pool->threads[i], NULL);
        }
    }
    
    // Free remaining tasks
    TaskNode *node = pool->task_head;
    while (node) {
        TaskNode *next = node->next;
        free(node);
        node = next;
    }
    
    pthread_mutex_destroy(&pool->queue_mutex);
    pthread_cond_destroy(&pool->queue_cond);
    pthread_cond_destroy(&pool->working_cond);
    
    free(pool->threads);
    free(pool);
}

void thread_pool_add_task(ThreadPool *pool, void (*task)(void *), void *arg) {
    if (!pool || !task || pool->shutdown) return;
    
    TaskNode *node = malloc(sizeof(TaskNode));
    if (!node) return;
    
    node->function = task;
    node->argument = arg;
    node->next = NULL;
    
    pthread_mutex_lock(&pool->queue_mutex);
    
    if (pool->task_head == NULL) {
        pool->task_head = node;
        pool->task_tail = node;
    } else {
        pool->task_tail->next = node;
        pool->task_tail = node;
    }
    
    pthread_cond_signal(&pool->queue_cond);
    pthread_mutex_unlock(&pool->queue_mutex);
}

static void *thread_pool_worker(void *arg) {
    ThreadPool *pool = (ThreadPool *)arg;
    
    while (1) {
        pthread_mutex_lock(&pool->queue_mutex);
        
        // Wait for task or shutdown
        while (pool->task_head == NULL && !pool->shutdown) {
            pthread_cond_wait(&pool->queue_cond, &pool->queue_mutex);
        }
        
        if (pool->shutdown) {
            pool->thread_count_alive--;
            pthread_mutex_unlock(&pool->queue_mutex);
            break;
        }
        
        // Get task from queue
        TaskNode *task = pool->task_head;
        if (task == NULL) {
            pthread_mutex_unlock(&pool->queue_mutex);
            continue;
        }
        
        if (task == pool->task_tail) {
            pool->task_head = NULL;
            pool->task_tail = NULL;
        } else {
            pool->task_head = task->next;
        }
        
        pool->working_count++;
        pthread_mutex_unlock(&pool->queue_mutex);
        
        // Execute task
        if (task->function) {
            task->function(task->argument);
        }
        
        // Cleanup task
        free(task);
        
        pthread_mutex_lock(&pool->queue_mutex);
        pool->working_count--;
        if (pool->working_count == 0 && pool->task_head == NULL) {
            pthread_cond_signal(&pool->working_cond);
        }
        pthread_mutex_unlock(&pool->queue_mutex);
    }
    
    return NULL;
}

int thread_pool_get_tasks_pending(ThreadPool *pool) {
    if (!pool) return 0;
    
    pthread_mutex_lock(&pool->queue_mutex);
    int count = 0;
    TaskNode *node = pool->task_head;
    while (node) {
        count++;
        node = node->next;
    }
    pthread_mutex_unlock(&pool->queue_mutex);
    
    return count;
}

int thread_pool_get_working_count(ThreadPool *pool) {
    if (!pool) return 0;
    
    pthread_mutex_lock(&pool->queue_mutex);
    int count = pool->working_count;
    pthread_mutex_unlock(&pool->queue_mutex);
    
    return count;
}

void thread_pool_wait(ThreadPool *pool) {
    if (!pool) return;
    
    pthread_mutex_lock(&pool->queue_mutex);
    while (pool->working_count > 0 || pool->task_head != NULL) {
        pthread_cond_wait(&pool->working_cond, &pool->queue_mutex);
    }
    pthread_mutex_unlock(&pool->queue_mutex);
}