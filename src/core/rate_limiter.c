#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

#ifdef _WIN32
    #include <errno.h>
#else
    #include <unistd.h>
#endif

#include "include/cyberforce.h"
#include "include/defines.h"

typedef struct rate_limiter {
    int requests_per_minute;
    int requests_per_second;
    time_t last_request_time;
    int request_count;
    int burst_limit;
    int burst_count;
    pthread_mutex_t mutex;
    struct timespec last_burst_reset;
} RateLimiter;

RateLimiter *rate_limiter_create(int requests_per_minute) {
    if (requests_per_minute <= 0) return NULL;
    
    RateLimiter *limiter = malloc(sizeof(RateLimiter));
    if (!limiter) return NULL;
    
    limiter->requests_per_minute = requests_per_minute;
    limiter->requests_per_second = ceil(requests_per_minute / 60.0);
    limiter->last_request_time = time(NULL);
    limiter->request_count = 0;
    limiter->burst_limit = limiter->requests_per_second * 2; // Allow 2x burst
    limiter->burst_count = 0;
    pthread_mutex_init(&limiter->mutex, NULL);
    clock_gettime(CLOCK_MONOTONIC, &limiter->last_burst_reset);
    
    return limiter;
}

void rate_limiter_destroy(RateLimiter *limiter) {
    if (!limiter) return;
    
    pthread_mutex_destroy(&limiter->mutex);
    free(limiter);
}

void rate_limiter_wait(RateLimiter *limiter) {
    if (!limiter) return;
    
    pthread_mutex_lock(&limiter->mutex);
    
    time_t current_time = time(NULL);
    time_t elapsed = current_time - limiter->last_request_time;
    
    // Reset counter if more than 1 minute has passed
    if (elapsed >= 60) {
        limiter->request_count = 0;
        limiter->last_request_time = current_time;
    }
    
    // Check burst limit
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double burst_elapsed = (now.tv_sec - limiter->last_burst_reset.tv_sec) +
                          (now.tv_nsec - limiter->last_burst_reset.tv_nsec) / 1e9;
    
    if (burst_elapsed >= 1.0) { // Reset burst counter every second
        limiter->burst_count = 0;
        limiter->last_burst_reset = now;
    }
    
    // Calculate wait time
    double wait_time = 0;
    
    // Check minute rate limit
    if (limiter->request_count >= limiter->requests_per_minute) {
        wait_time = 60 - elapsed;
    }
    // Check second rate limit (burst)
    else if (limiter->burst_count >= limiter->burst_limit) {
        wait_time = 1.0 - burst_elapsed;
    }
    // Check average rate
    else {
        double target_interval = 60.0 / limiter->requests_per_minute;
        double actual_interval = elapsed / (limiter->request_count + 1);
        
        if (actual_interval < target_interval) {
            wait_time = target_interval - actual_interval;
        }
    }
    
    if (wait_time > 0) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        
        ts.tv_sec += (time_t)wait_time;
        ts.tv_nsec += (long)((wait_time - (time_t)wait_time) * 1e9);
        
        if (ts.tv_nsec >= 1e9) {
            ts.tv_sec++;
            ts.tv_nsec -= 1e9;
        }
        
        pthread_mutex_unlock(&limiter->mutex);
        
        // Sleep with high precision
        struct timespec remaining;
        while (nanosleep(&ts, &remaining) == -1 && errno == EINTR) {
            ts = remaining;
        }
        
        pthread_mutex_lock(&limiter->mutex);
        current_time = time(NULL);
    }
    
    // Update counters
    limiter->request_count++;
    limiter->burst_count++;
    limiter->last_request_time = current_time;
    
    pthread_mutex_unlock(&limiter->mutex);
}

void rate_limiter_set_rate(RateLimiter *limiter, int requests_per_minute) {
    if (!limiter || requests_per_minute <= 0) return;
    
    pthread_mutex_lock(&limiter->mutex);
    limiter->requests_per_minute = requests_per_minute;
    limiter->requests_per_second = ceil(requests_per_minute / 60.0);
    limiter->burst_limit = limiter->requests_per_second * 2;
    pthread_mutex_unlock(&limiter->mutex);
}

int rate_limiter_get_count(RateLimiter *limiter) {
    if (!limiter) return 0;
    
    pthread_mutex_lock(&limiter->mutex);
    int count = limiter->request_count;
    pthread_mutex_unlock(&limiter->mutex);
    
    return count;
}

void rate_limiter_reset(RateLimiter *limiter) {
    if (!limiter) return;
    
    pthread_mutex_lock(&limiter->mutex);
    limiter->request_count = 0;
    limiter->burst_count = 0;
    limiter->last_request_time = time(NULL);
    clock_gettime(CLOCK_MONOTONIC, &limiter->last_burst_reset);
    pthread_mutex_unlock(&limiter->mutex);
}
