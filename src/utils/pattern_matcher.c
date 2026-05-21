#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifndef _WIN32
    #include <regex.h>
#endif
#include "include/cyberforce.h"
#include "include/defines.h"

bool pattern_match(const char *text, const char *pattern, bool use_regex) {
    if (!text || !pattern) return false;
    
    if (!use_regex) {
        // Simple substring search
        return strstr(text, pattern) != NULL;
    } else {
#ifdef _WIN32
        // MinGW does not ship POSIX regex.h by default.
        return strstr(text, pattern) != NULL;
#else
        // Regular expression matching
        regex_t regex;
        int ret;
        
        ret = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
        if (ret != 0) {
            // Fall back to substring search if regex compilation fails
            return strstr(text, pattern) != NULL;
        }
        
        ret = regexec(&regex, text, 0, NULL, 0);
        regfree(&regex);
        
        return ret == 0;
#endif
    }
}

bool extract_pattern(const char *text, const char *pattern, char *result, size_t size) {
    if (!text || !pattern || !result || size == 0) return false;
    
    // Simple extraction: find pattern and copy what follows
    const char *pos = strstr(text, pattern);
    if (!pos) return false;
    
    pos += strlen(pattern);
    
    // Skip whitespace
    while (*pos && isspace(*pos)) pos++;
    
    // Copy until newline or end
    size_t i = 0;
    while (*pos && *pos != '\n' && *pos != '\r' && i < size - 1) {
        result[i++] = *pos++;
    }
    result[i] = '\0';
    
    return i > 0;
}
