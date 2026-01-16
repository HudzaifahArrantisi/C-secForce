#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "include/cyberforce.h"
#include "include/defines.h"

int load_proxy_list(const char *filename, char ***proxy_list) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return 0;
    }
    
    int capacity = 100;
    int count = 0;
    char **list = malloc(capacity * sizeof(char *));
    if (!list) {
        fclose(fp);
        return 0;
    }
    
    char line[BUFFER_MEDIUM];
    while (fgets(line, sizeof(line), fp)) {
        // Remove newline
        line[strcspn(line, "\n\r")] = 0;
        
        // Skip empty lines and comments
        if (strlen(line) == 0 || line[0] == '#') continue;
        
        // Check if we need to resize
        if (count >= capacity) {
            capacity *= 2;
            list = realloc(list, capacity * sizeof(char *));
            if (!list) {
                fclose(fp);
                return 0;
            }
        }
        
        list[count] = strdup(line);
        if (!list[count]) {
            // Cleanup on allocation failure
            for (int i = 0; i < count; i++) {
                free(list[i]);
            }
            free(list);
            fclose(fp);
            return 0;
        }
        count++;
    }
    
    fclose(fp);
    *proxy_list = list;
    return count;
}

const char *get_next_proxy(Config *config) {
    if (!config || config->proxy_count == 0) {
        return config ? config->proxy : NULL;
    }
    
    static int current_proxy = 0;
    
    if (current_proxy >= config->proxy_count) {
        current_proxy = 0;
    }
    
    return config->proxy_list[current_proxy++];
}

void rotate_proxy(Config *config) {
    if (!config || config->proxy_count == 0) return;
    
    static int rotation_count = 0;
    
    // Every 10 requests, switch proxy
    if (++rotation_count % 10 == 0) {
        // Already handled in get_next_proxy
    }
}