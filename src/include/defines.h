#ifndef DEFINES_H
#define DEFINES_H

// Version information
#define CYBERFORCE_VERSION_MAJOR 2
#define CYBERFORCE_VERSION_MINOR 0
#define CYBERFORCE_VERSION_PATCH 0
#define CYBERFORCE_VERSION_STRING "2.0.0"

// Default values
#define DEFAULT_THREADS 10
#define DEFAULT_TIMEOUT 30
#define DEFAULT_DELAY 100
#define DEFAULT_MAX_RETRIES 3
#define DEFAULT_RATE_LIMIT 60

// Wordlist generation rules
#define RULE_NONE        0x00
#define RULE_LOWERCASE   0x01
#define RULE_UPPERCASE   0x02
#define RULE_CAPITALIZE  0x04
#define RULE_LEET        0x08
#define RULE_NUMBERS     0x10
#define RULE_PREPEND     0x20
#define RULE_APPEND      0x40
#define RULE_REVERSE     0x80
#define RULE_DOUBLE      0x100
#define RULE_YEARS       0x200
#define RULE_ALL         0xFFFF

// Error codes
#define ERROR_SUCCESS 0
#define ERROR_INVALID_ARGUMENT 1
#define ERROR_FILE_NOT_FOUND 2
#define ERROR_NETWORK_FAILURE 3
#define ERROR_MEMORY_ALLOCATION 4
#define ERROR_THREAD_FAILURE 5
#define ERROR_TIMEOUT 6
#define ERROR_AUTH_FAILED 7
#define ERROR_RATE_LIMITED 8
#define ERROR_PROXY_FAILED 9
#define ERROR_SSL_FAILED 10

// Status codes
#define STATUS_IDLE 0
#define STATUS_RUNNING 1
#define STATUS_PAUSED 2
#define STATUS_STOPPED 3
#define STATUS_COMPLETED 4

// Buffer sizes
#define BUFFER_SMALL 256
#define BUFFER_MEDIUM 1024
#define BUFFER_LARGE 4096
#define BUFFER_XLARGE 16384

// Time constants
#define SECOND_IN_US 1000000
#define MINUTE_IN_US 60000000
#define HOUR_IN_US 3600000000

// Color codes for output   
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"

// Log levels
#define LOG_NONE 0
#define LOG_ERROR 1
#define LOG_WARNING 2
#define LOG_INFO 3
#define LOG_DEBUG 4
#define LOG_TRACE 5

// Platform detection
#ifdef _WIN32
    #define PLATFORM_WINDOWS 1
    #define PLATFORM_LINUX 0
    #define PLATFORM_MACOS 0
#elif __APPLE__
    #define PLATFORM_WINDOWS 0
    #define PLATFORM_LINUX 0
    #define PLATFORM_MACOS 1
#elif __linux__
    #define PLATFORM_WINDOWS 0
    #define PLATFORM_LINUX 1
    #define PLATFORM_MACOS 0
#else
    #define PLATFORM_WINDOWS 0
    #define PLATFORM_LINUX 0
    #define PLATFORM_MACOS 0
#endif

// Compiler hints
#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define ALWAYS_INLINE __attribute__((always_inline))
#define NO_INLINE __attribute__((noinline))
#define PACKED __attribute__((packed))

// Safe string operations
#define STRNCPY(dest, src, n) \
    do { \
        strncpy((dest), (src), (n)); \
        (dest)[(n)-1] = '\0'; \
    } while(0)

#define SAFE_FREE(ptr) \
    do { \
        if ((ptr) != NULL) { \
            free((ptr)); \
            (ptr) = NULL; \
        } \
    } while(0)

#define SAFE_CLOSE(fd) \
    do { \
        if ((fd) >= 0) { \
            close((fd)); \
            (fd) = -1; \
        } \
    } while(0)

// Debug macros
#ifdef DEBUG
    #define DEBUG_PRINT(fmt, ...) \
        fprintf(stderr, "[DEBUG] %s:%d:%s(): " fmt, \
                __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...)
#endif

// Assert macro
#define CYBERFORCE_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "[ASSERT] %s:%d:%s(): %s\n", \
                    __FILE__, __LINE__, __func__, message); \
            abort(); \
        } \
    } while(0)

#endif