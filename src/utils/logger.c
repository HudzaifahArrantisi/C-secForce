#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include "include/cyberforce.h"
#include "include/defines.h"

void log_message(const char *message, Config *config) {
    if (!message || !config || config->verbose < 1) return;
    
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    // Log to console
    if (config->verbose >= 2) {
        printf("[%s] %s\n", timestamp, message);
    }
    
    // Log to file if output file specified
    if (config->output_file) {
        FILE *fp = fopen(config->output_file, "a");
        if (fp) {
            fprintf(fp, "[%s] %s\n", timestamp, message);
            fclose(fp);
        }
    }
}

void log_success(Config *config, const char *username, const char *password, 
                 long http_code, const char *response) {
    if (!config) return;
    
    char message[BUFFER_LARGE];
    snprintf(message, sizeof(message), "SUCCESS - Username: %s, Password: %s, HTTP: %ld",
             username, password, http_code);
    
    log_message(message, config);
    
    // Also save to results file
    if (config->output_file) {
        char results_file[BUFFER_MEDIUM];
        snprintf(results_file, sizeof(results_file), "%s.results", config->output_file);
        
        FILE *fp = fopen(results_file, "a");
        if (fp) {
            time_t now = time(NULL);
            struct tm *tm_info = localtime(&now);
            char timestamp[20];
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
            
            fprintf(fp, "[%s] SUCCESS - Target: %s, Username: %s, Password: %s, ",
                    timestamp, config->target, username, password);
            
            if (http_code > 0) {
                fprintf(fp, "HTTP: %ld", http_code);
            }
            
            if (response) {
                // Truncate response for logging
                char truncated[256];
                strncpy(truncated, response, sizeof(truncated) - 1);
                truncated[sizeof(truncated) - 1] = '\0';
                fprintf(fp, ", Response: %s", truncated);
            }
            
            fprintf(fp, "\n");
            fclose(fp);
        }
    }
}

void log_failure(Config *config, const char *username, const char *password, 
                 int error_code, const char *error_msg) {
    if (!config || config->verbose < 3) return;
    
    char message[BUFFER_LARGE];
    if (error_msg) {
        snprintf(message, sizeof(message), "FAILURE - Username: %s, Password: %s, Error: %s",
                 username, password, error_msg);
    } else {
        snprintf(message, sizeof(message), "FAILURE - Username: %s, Password: %s, Code: %d",
                 username, password, error_code);
    }
    
    log_message(message, config);
}

void save_results(Config *config, AttackResult *results, int count) {
    if (!config || !config->output_file || count <= 0) return;
    
    FILE *fp = fopen(config->output_file, "w");
    if (!fp) return;
    
    switch (config->output_format) {
        case OUTPUT_JSON:
            fprintf(fp, "{\n");
            fprintf(fp, "  \"attack\": {\n");
            fprintf(fp, "    \"target\": \"%s\",\n", config->target);
            fprintf(fp, "    \"port\": %d,\n", config->port);
            fprintf(fp, "    \"protocol\": \"%s\",\n", config->protocol);
            fprintf(fp, "    \"start_time\": %ld,\n", config->start_time);
            fprintf(fp, "    \"end_time\": %ld,\n", config->end_time);
            fprintf(fp, "    \"total_attempts\": %d,\n", count);
            fprintf(fp, "    \"results\": [\n");
            
            for (int i = 0; i < count; i++) {
                fprintf(fp, "      {\n");
                fprintf(fp, "        \"username\": \"%s\",\n", results[i].username);
                fprintf(fp, "        \"password\": \"%s\",\n", results[i].password);
                fprintf(fp, "        \"status\": %d,\n", results[i].status);
                fprintf(fp, "        \"timestamp\": %ld,\n", results[i].timestamp);
                
                if (results[i].http_code > 0) {
                    fprintf(fp, "        \"http_code\": %ld,\n", results[i].http_code);
                }
                
                if (results[i].response_time > 0) {
                    fprintf(fp, "        \"response_time\": %.3f,\n", results[i].response_time);
                }
                
                fprintf(fp, "        \"target\": \"%s\"\n", results[i].target);
                fprintf(fp, "      }%s\n", (i < count - 1) ? "," : "");
            }
            
            fprintf(fp, "    ]\n");
            fprintf(fp, "  }\n");
            fprintf(fp, "}\n");
            break;
            
        case OUTPUT_CSV:
            fprintf(fp, "timestamp,username,password,status,target,port,http_code,response_time,response_size\n");
            for (int i = 0; i < count; i++) {
                char timestamp[20];
                struct tm *tm_info = localtime(&results[i].timestamp);
                strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
                
                fprintf(fp, "%s,%s,%s,%d,%s,%d,%ld,%.3f,%zu\n",
                        timestamp,
                        results[i].username,
                        results[i].password,
                        results[i].status,
                        results[i].target,
                        results[i].port,
                        results[i].http_code,
                        results[i].response_time,
                        results[i].response_size);
            }
            break;
            
        case OUTPUT_XML:
            fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
            fprintf(fp, "<attack>\n");
            fprintf(fp, "  <target>%s</target>\n", config->target);
            fprintf(fp, "  <port>%d</port>\n", config->port);
            fprintf(fp, "  <protocol>%s</protocol>\n", config->protocol);
            fprintf(fp, "  <start_time>%ld</start_time>\n", config->start_time);
            fprintf(fp, "  <end_time>%ld</end_time>\n", config->end_time);
            fprintf(fp, "  <results>\n");
            
            for (int i = 0; i < count; i++) {
                fprintf(fp, "    <result>\n");
                fprintf(fp, "      <username>%s</username>\n", results[i].username);
                fprintf(fp, "      <password>%s</password>\n", results[i].password);
                fprintf(fp, "      <status>%d</status>\n", results[i].status);
                fprintf(fp, "      <timestamp>%ld</timestamp>\n", results[i].timestamp);
                
                if (results[i].http_code > 0) {
                    fprintf(fp, "      <http_code>%ld</http_code>\n", results[i].http_code);
                }
                
                if (results[i].response_time > 0) {
                    fprintf(fp, "      <response_time>%.3f</response_time>\n", results[i].response_time);
                }
                
                fprintf(fp, "      <target>%s</target>\n", results[i].target);
                fprintf(fp, "      <port>%d</port>\n", results[i].port);
                fprintf(fp, "    </result>\n");
            }
            
            fprintf(fp, "  </results>\n");
            fprintf(fp, "</attack>\n");
            break;
            
        case OUTPUT_PLAIN:
        default:
            fprintf(fp, "CyberForce Results\n");
            fprintf(fp, "==================\n\n");
            fprintf(fp, "Target: %s\n", config->target);
            fprintf(fp, "Port: %d\n", config->port);
            fprintf(fp, "Protocol: %s\n", config->protocol);
            fprintf(fp, "Start Time: %s", ctime(&config->start_time));
            fprintf(fp, "End Time: %s", ctime(&config->end_time));
            fprintf(fp, "Total Attempts: %d\n\n", count);
            
            fprintf(fp, "Successful Logins:\n");
            fprintf(fp, "------------------\n");
            
            int success_count = 0;
            for (int i = 0; i < count; i++) {
                if (results[i].status == 1) {
                    success_count++;
                    char timestamp[20];
                    struct tm *tm_info = localtime(&results[i].timestamp);
                    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
                    
                    fprintf(fp, "[%s] %s:%s", timestamp,
                            results[i].username, results[i].password);
                    
                    if (results[i].http_code > 0) {
                        fprintf(fp, " (HTTP: %ld)", results[i].http_code);
                    }
                    
                    fprintf(fp, "\n");
                }
            }
            
            if (success_count == 0) {
                fprintf(fp, "No successful logins found.\n");
            }
            break;
    }
    
    fclose(fp);
    printf("[+] Results saved to %s\n", config->output_file);
}