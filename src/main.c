#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #define sleep(x) Sleep((x) * 1000)
    #define access _access
    #define F_OK 0
    // getopt implementation for Windows
    char *optarg = NULL;
    int optind = 1;
    int opterr = 1;
    int optopt = 0;
    int getopt(int argc, char *const argv[], const char *optstring) {
        return -1; // Stub for now
    }
#else
    #include <getopt.h>
    #include <unistd.h>
    #include <sys/stat.h>
#endif

#ifdef HAS_CURL
    #include <curl/curl.h>
#endif

#include "include/cyberforce.h"
#include "include/defines.h"

Config g_config;
volatile sig_atomic_t g_running = 1;
char *g_program_name = "cyberforce";

void print_banner() {
    printf("\n");
    printf(COLOR_CYAN "╔══════════════════════════════════════════════════════════╗\n" COLOR_RESET);
    printf(COLOR_CYAN "║" COLOR_RESET "                 " COLOR_YELLOW "CYBERFORCE" COLOR_RESET " v%s                " COLOR_CYAN "║\n" COLOR_RESET, CYBERFORCE_VERSION_STRING);
    printf(COLOR_CYAN "║" COLOR_RESET "         Advanced Security Testing Framework         " COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "║" COLOR_RESET "           [Academic Project - Semester 3]           " COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "║" COLOR_RESET "          Cybersecurity Specialization              " COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "╚══════════════════════════════════════════════════════════╝\n" COLOR_RESET);
    printf("\n");
}

void print_usage() {
    printf("Usage: %s [OPTIONS] -t TARGET\n\n", g_program_name);
    
    printf("Required:\n");
    printf("  -t, --target HOST          Target host or URL\n");
    printf("  -U, --user FILE/USER       Username or userlist file\n");
    printf("  -P, --pass FILE/PASS       Password or passlist file\n\n");
    
    printf("Basic Options:\n");
    printf("  -m, --module MODULE        Attack module (http, ftp, ssh, mysql)\n");
    printf("  -p, --port PORT            Port number (default: protocol specific)\n");
    printf("  -o, --output FILE          Output file for results\n");
    printf("  -v, --verbose LEVEL        Verbosity level (0-5, default: 2)\n");
    printf("  -h, --help                 Display this help message\n");
    printf("      --version              Display version information\n\n");
    
    printf("HTTP Options:\n");
    printf("      --method METHOD        HTTP method (GET, POST, PUT, DELETE)\n");
    printf("      --data DATA            POST data with {USER} and {PASS} placeholders\n");
    printf("      --cookie COOKIE        HTTP Cookie header\n");
    printf("      --referer REFERER      HTTP Referer header\n");
    printf("      --user-agent AGENT     User-Agent string or 'random'\n");
    printf("      --no-ssl-verify        Disable SSL certificate verification\n");
    printf("      --follow-redirects     Follow HTTP redirects\n\n");
    
    printf("Performance Options:\n");
    printf("  -T, --threads NUM          Number of threads (default: %d)\n", DEFAULT_THREADS);
    printf("      --delay MS             Delay between attempts in milliseconds\n");
    printf("      --jitter MS            Random jitter to add to delay\n");
    printf("      --timeout SEC          Connection timeout in seconds\n");
    printf("      --max-retries NUM      Maximum retry attempts\n");
    printf("      --rate-limit RPM       Maximum requests per minute\n\n");
    
    printf("Evasion Options:\n");
    printf("      --proxy PROXY          Proxy server (http://host:port, socks5://...)\n");
    printf("      --proxies FILE         File containing list of proxies\n");
    printf("      --tor                  Use Tor network (requires Tor running)\n");
    printf("      --randomize            Randomize attack order\n");
    printf("      --rotate-ua            Rotate User-Agent strings\n");
    printf("      --rotate-proxy         Rotate through proxy list\n\n");
    
    printf("Detection Options:\n");
    printf("      --success PATTERN      Pattern indicating success in response\n");
    printf("      --failure PATTERN      Pattern indicating failure in response\n");
    printf("      --success-code CODE    HTTP status code indicating success\n");
    printf("      --failure-code CODE    HTTP status code indicating failure\n\n");
    
    printf("Output Options:\n");
    printf("      --json                 Output results in JSON format\n");
    printf("      --csv                  Output results in CSV format\n");
    printf("      --xml                  Output results in XML format\n");
    printf("      --silent               Suppress all output except results\n");
    printf("      --color / --no-color   Enable/disable colored output\n\n");
    
    printf("Wordlist Generation:\n");
    printf("      --generate-wordlist    Generate wordlist from base word\n");
    printf("      --base-word WORD       Base word for generation\n");
    printf("      --rules RULES          Comma-separated list of rules\n");
    printf("                             (lower,upper,leet,numbers,years,etc)\n\n");
    
    printf("Examples:\n");
    printf("  %s -t http://target.com/login -U users.txt -P pass.txt -m http\n", g_program_name);
    printf("  %s -t ftp://fileserver.com -U admin -P rockyou.txt -m ftp -T 20\n", g_program_name);
    printf("  %s -t 192.168.1.100 -p 22 -U root -P passwords.txt -m ssh\n", g_program_name);
    printf("  %s --generate-wordlist --base-word admin --output custom.txt\n", g_program_name);
}

void print_version() {
    printf("CyberForce v%s\n", CYBERFORCE_VERSION_STRING);
    printf("Copyright (c) 2024 Cybersecurity Student - Semester 3\n");
    printf("Academic project for Web Security course\n");
}

void signal_handler(int sig) {
    if (!g_running) {
        printf("\n[!] Force quitting...\n");
        exit(1);
    }
    
    printf("\n[!] Received signal %d. Shutting down gracefully...\n", sig);
    g_running = 0;
    attack_stop();
}

void cleanup() {
    cleanup_config(&g_config);
    curl_global_cleanup();
}

int check_file_exists(const char *filename) {
    struct stat st;
    return (stat(filename, &st) == 0);
}

int parse_arguments(int argc, char *argv[], Config *config) {
    static struct option long_options[] = {
        // Required
        {"target", required_argument, 0, 't'},
        {"user", required_argument, 0, 'U'},
        {"pass", required_argument, 0, 'P'},
        
        // Basic
        {"module", required_argument, 0, 'm'},
        {"port", required_argument, 0, 'p'},
        {"output", required_argument, 0, 'o'},
        {"threads", required_argument, 0, 'T'},
        {"verbose", required_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 256},
        
        // HTTP specific
        {"method", required_argument, 0, 257},
        {"data", required_argument, 0, 258},
        {"cookie", required_argument, 0, 259},
        {"referer", required_argument, 0, 260},
        {"user-agent", required_argument, 0, 261},
        {"no-ssl-verify", no_argument, 0, 262},
        {"follow-redirects", no_argument, 0, 263},
        
        // Performance
        {"delay", required_argument, 0, 264},
        {"jitter", required_argument, 0, 265},
        {"timeout", required_argument, 0, 266},
        {"max-retries", required_argument, 0, 267},
        {"rate-limit", required_argument, 0, 268},
        
        // Evasion
        {"proxy", required_argument, 0, 269},
        {"proxies", required_argument, 0, 270},
        {"tor", no_argument, 0, 271},
        {"randomize", no_argument, 0, 272},
        {"rotate-ua", no_argument, 0, 273},
        {"rotate-proxy", no_argument, 0, 274},
        
        // Detection
        {"success", required_argument, 0, 275},
        {"failure", required_argument, 0, 276},
        {"success-code", required_argument, 0, 277},
        {"failure-code", required_argument, 0, 278},
        
        // Output
        {"json", no_argument, 0, 279},
        {"csv", no_argument, 0, 280},
        {"xml", no_argument, 0, 281},
        {"silent", no_argument, 0, 282},
        {"color", no_argument, 0, 283},
        {"no-color", no_argument, 0, 284},
        
        // Wordlist generation
        {"generate-wordlist", no_argument, 0, 285},
        {"base-word", required_argument, 0, 286},
        {"rules", required_argument, 0, 287},
        
        {0, 0, 0, 0}
    };
    
    int opt, option_index = 0;
    bool generate_wordlist_mode = false;
    
    while ((opt = getopt_long(argc, argv, "t:U:P:m:p:o:T:v:h", 
                             long_options, &option_index)) != -1) {
        switch (opt) {
            case 't':
                config->target = strdup(optarg);
                break;
                
            case 'U':
                if (check_file_exists(optarg)) {
                    config->userlist_file = strdup(optarg);
                } else {
                    config->username = strdup(optarg);
                }
                break;
                
            case 'P':
                if (check_file_exists(optarg)) {
                    config->passlist_file = strdup(optarg);
                } else {
                    config->password = strdup(optarg);
                }
                break;
                
            case 'm':
                config->module_type = get_module_type(optarg);
                if (config->module_type == MODULE_HTTP) {
                    config->protocol = strdup("http");
                } else if (config->module_type == MODULE_FTP) {
                    config->protocol = strdup("ftp");
                } else if (config->module_type == MODULE_SSH) {
                    config->protocol = strdup("ssh");
                } else if (config->module_type == MODULE_MYSQL) {
                    config->protocol = strdup("mysql");
                }
                break;
                
            case 'p':
                config->port = atoi(optarg);
                break;
                
            case 'o':
                config->output_file = strdup(optarg);
                break;
                
            case 'T':
                config->threads = atoi(optarg);
                if (config->threads < 1) config->threads = 1;
                if (config->threads > MAX_THREADS) config->threads = MAX_THREADS;
                break;
                
            case 'v':
                config->verbose = atoi(optarg);
                break;
                
            case 'h':
                print_usage();
                exit(0);
                
            case 256: // --version
                print_version();
                exit(0);
                
            case 257: // --method
                config->method = strdup(optarg);
                break;
                
            case 258: // --data
                config->post_data = strdup(optarg);
                break;
                
            case 259: // --cookie
                config->cookie = strdup(optarg);
                break;
                
            case 260: // --referer
                config->referer = strdup(optarg);
                break;
                
            case 261: // --user-agent
                config->user_agent = strdup(optarg);
                break;
                
            case 262: // --no-ssl-verify
                config->ssl_verify = false;
                break;
                
            case 264: // --delay
                config->delay = atoi(optarg);
                break;
                
            case 265: // --jitter
                config->jitter = atoi(optarg);
                break;
                
            case 266: // --timeout
                config->timeout = atoi(optarg);
                break;
                
            case 267: // --max-retries
                config->max_retries = atoi(optarg);
                break;
                
            case 268: // --rate-limit
                config->rate_limit = atoi(optarg);
                break;
                
            case 269: // --proxy
                config->proxy = strdup(optarg);
                break;
                
            case 270: // --proxies
                config->proxy_count = load_proxy_list(optarg, &config->proxy_list);
                break;
                
            case 271: // --tor
                config->use_tor = true;
                if (!config->proxy) {
                    config->proxy = strdup("socks5://127.0.0.1:9050");
                }
                break;
                
            case 272: // --randomize
                config->randomize = true;
                break;
                
            case 273: // --rotate-ua
                config->rotate_ua = true;
                break;
                
            case 275: // --success
                config->success_pattern = strdup(optarg);
                break;
                
            case 276: // --failure
                config->failure_pattern = strdup(optarg);
                break;
                
            case 279: // --json
                config->output_format = OUTPUT_JSON;
                break;
                
            case 280: // --csv
                config->output_format = OUTPUT_CSV;
                break;
                
            case 281: // --xml
                config->output_format = OUTPUT_XML;
                break;
                
            case 285: // --generate-wordlist
                generate_wordlist_mode = true;
                break;
                
            case 286: // --base-word
                if (generate_wordlist_mode) {
                    config->username = strdup(optarg);
                }
                break;
                
            case 287: // --rules
                // Parse rules string
                break;
                
            default:
                fprintf(stderr, "Unknown option: %c\n", opt);
                return -1;
        }
    }
    
    // Handle wordlist generation mode
    if (generate_wordlist_mode) {
        if (!config->username || !config->output_file) {
            fprintf(stderr, "Error: --base-word and --output are required for wordlist generation\n");
            return -1;
        }
        generate_wordlist(config->username, config->output_file, RULE_ALL);
        exit(0);
    }
    
    // Validate required arguments
    if (!config->target) {
        fprintf(stderr, "Error: Target is required. Use -t or --target\n");
        return -1;
    }
    
    if (!config->username && !config->userlist_file) {
        fprintf(stderr, "Error: Username or userlist is required. Use -U or --user\n");
        return -1;
    }
    
    if (!config->password && !config->passlist_file) {
        fprintf(stderr, "Error: Password or passlist is required. Use -P or --pass\n");
        return -1;
    }
    
    // Set defaults
    if (config->threads == 0) config->threads = DEFAULT_THREADS;
    if (config->timeout == 0) config->timeout = DEFAULT_TIMEOUT;
    if (config->delay == 0) config->delay = DEFAULT_DELAY;
    if (config->max_retries == 0) config->max_retries = DEFAULT_MAX_RETRIES;
    if (config->rate_limit == 0) config->rate_limit = DEFAULT_RATE_LIMIT;
    if (config->verbose == 0) config->verbose = 2;
    
    // Set default module if not specified
    if (!config->protocol) {
        config->module_type = MODULE_HTTP;
        config->protocol = strdup("http");
    }
    
    // Set default port based on protocol
    if (config->port == 0) {
        if (strcmp(config->protocol, "http") == 0) config->port = 80;
        else if (strcmp(config->protocol, "https") == 0) config->port = 443;
        else if (strcmp(config->protocol, "ftp") == 0) config->port = 21;
        else if (strcmp(config->protocol, "ssh") == 0) config->port = 22;
        else if (strcmp(config->protocol, "mysql") == 0) config->port = 3306;
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    g_program_name = argv[0];
    
    // Register signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGQUIT, signal_handler);
    
    // Register cleanup function
    atexit(cleanup);
    
    // Initialize global configuration
    memset(&g_config, 0, sizeof(Config));
    g_config.ssl_verify = true;
    g_config.start_time = time(NULL);
    
    // Initialize CURL
    curl_global_init(CURL_GLOBAL_ALL);
    
    // Parse command line arguments
    if (parse_arguments(argc, argv, &g_config) != 0) {
        fprintf(stderr, "Failed to parse arguments. Use -h for help.\n");
        return 1;
    }
    
    // Show banner if not in silent mode
    if (g_config.verbose > 0) {
        print_banner();
    }
    
    // Display configuration
    if (g_config.verbose >= 2) {
        printf("[+] Configuration:\n");
        printf("    Target: %s\n", g_config.target);
        printf("    Protocol: %s\n", g_config.protocol);
        printf("    Port: %d\n", g_config.port);
        printf("    Threads: %d\n", g_config.threads);
        printf("    Timeout: %d seconds\n", g_config.timeout);
        printf("    Delay: %d ms\n", g_config.delay);
        if (g_config.jitter > 0) {
            printf("    Jitter: %d ms\n", g_config.jitter);
        }
        if (g_config.proxy) {
            printf("    Proxy: %s\n", g_config.proxy);
        }
        if (g_config.userlist_file) {
            printf("    Userlist: %s\n", g_config.userlist_file);
        } else {
            printf("    Username: %s\n", g_config.username);
        }
        if (g_config.passlist_file) {
            printf("    Passlist: %s\n", g_config.passlist_file);
        } else {
            printf("    Password: %s\n", g_config.password);
        }
        printf("\n");
    }
    
    // Start the attack
    printf("[+] Starting attack at %s", ctime(&g_config.start_time));
    
    int result = start_attack(&g_config);
    
    g_config.end_time = time(NULL);
    double elapsed = difftime(g_config.end_time, g_config.start_time);
    
    printf("\n[+] Attack completed in %.2f seconds\n", elapsed);
    
    return result;
}