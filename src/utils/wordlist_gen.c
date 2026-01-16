#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "include/cyberforce.h"
#include "include/defines.h"

void replace_char(char *str, char find, char replace) {
    for (int i = 0; str[i]; i++) {
        if (str[i] == find) {
            str[i] = replace;
        }
    }
}

void reverse_string(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

char **load_wordlist(const char *filename, int *count) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        *count = 0;
        return NULL;
    }
    
    // First pass: count lines
    int lines = 0;
    char buffer[MAX_WORD_LEN];
    while (fgets(buffer, sizeof(buffer), fp)) {
        lines++;
    }
    
    rewind(fp);
    
    // Allocate memory
    char **wordlist = malloc(lines * sizeof(char *));
    if (!wordlist) {
        fclose(fp);
        *count = 0;
        return NULL;
    }
    
    // Second pass: read lines
    int i = 0;
    while (fgets(buffer, sizeof(buffer), fp) && i < lines) {
        // Remove newline
        buffer[strcspn(buffer, "\n\r")] = 0;
        
        // Skip empty lines
        if (strlen(buffer) == 0) continue;
        
        wordlist[i] = strdup(buffer);
        if (!wordlist[i]) {
            // Cleanup on allocation failure
            for (int j = 0; j < i; j++) {
                free(wordlist[j]);
            }
            free(wordlist);
            fclose(fp);
            *count = 0;
            return NULL;
        }
        i++;
    }
    
    fclose(fp);
    *count = i;
    return wordlist;
}

void free_wordlist(char **wordlist, int count) {
    if (!wordlist) return;
    
    for (int i = 0; i < count; i++) {
        if (wordlist[i]) {
            free(wordlist[i]);
        }
    }
    free(wordlist);
}

void shuffle_wordlist(char **wordlist, int count) {
    if (count <= 1) return;
    
    srand(time(NULL));
    
    for (int i = count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        
        char *temp = wordlist[i];
        wordlist[i] = wordlist[j];
        wordlist[j] = temp;
    }
}

void generate_wordlist(const char *base_word, const char *output_file, int rules) {
    if (!base_word || !output_file) return;
    
    FILE *fp = fopen(output_file, "w");
    if (!fp) {
        fprintf(stderr, "[-] Failed to open output file: %s\n", output_file);
        return;
    }
    
    int word_count = 0;
    
    // Helper function to write word
    #define WRITE_WORD(word) do { \
        fprintf(fp, "%s\n", word); \
        word_count++; \
    } while(0)
    
    // Original word
    WRITE_WORD(base_word);
    
    // Lowercase
    if (rules & RULE_LOWERCASE) {
        char *lower = strdup(base_word);
        for (int i = 0; lower[i]; i++) {
            lower[i] = tolower(lower[i]);
        }
        WRITE_WORD(lower);
        free(lower);
    }
    
    // Uppercase
    if (rules & RULE_UPPERCASE) {
        char *upper = strdup(base_word);
        for (int i = 0; upper[i]; i++) {
            upper[i] = toupper(upper[i]);
        }
        WRITE_WORD(upper);
        free(upper);
    }
    
    // Capitalize
    if (rules & RULE_CAPITALIZE) {
        char *cap = strdup(base_word);
        if (cap[0]) {
            cap[0] = toupper(cap[0]);
            for (int i = 1; cap[i]; i++) {
                cap[i] = tolower(cap[i]);
            }
            WRITE_WORD(cap);
        }
        free(cap);
    }
    
    // Leetspeak variations
    if (rules & RULE_LEET) {
        char leet[MAX_WORD_LEN];
        
        // Individual replacements
        const char *leet_replacements[][2] = {
            {"a", "@"}, {"a", "4"},
            {"e", "3"},
            {"i", "1"}, {"i", "!"},
            {"o", "0"},
            {"s", "$"}, {"s", "5"},
            {"t", "7"},
            {"b", "8"},
            {"g", "9"},
            {NULL, NULL}
        };
        
        for (int i = 0; leet_replacements[i][0]; i++) {
            char *pos = strstr(base_word, leet_replacements[i][0]);
            if (pos) {
                strcpy(leet, base_word);
                replace_char(leet, leet_replacements[i][0][0], leet_replacements[i][1][0]);
                WRITE_WORD(leet);
            }
        }
        
        // Multiple replacements
        strcpy(leet, base_word);
        replace_char(leet, 'a', '@');
        replace_char(leet, 'e', '3');
        replace_char(leet, 'i', '!');
        replace_char(leet, 'o', '0');
        replace_char(leet, 's', '$');
        WRITE_WORD(leet);
    }
    
    // Append numbers
    if (rules & RULE_NUMBERS) {
        char temp[MAX_WORD_LEN];
        
        // Append 0-999
        for (int i = 0; i <= 999; i++) {
            snprintf(temp, sizeof(temp), "%s%d", base_word, i);
            WRITE_WORD(temp);
            
            if (i <= 99) {
                snprintf(temp, sizeof(temp), "%s%02d", base_word, i);
                WRITE_WORD(temp);
            }
            
            if (i <= 9) {
                snprintf(temp, sizeof(temp), "%s0%d", base_word, i);
                WRITE_WORD(temp);
            }
        }
        
        // Common number patterns
        const char *common_numbers[] = {
            "123", "1234", "12345", "123456",
            "111", "222", "333", "444", "555", "666", "777", "888", "999",
            "000", "001", "007", "008", "009",
            "100", "200", "300", "400", "500", "600", "700", "800", "900",
            NULL
        };
        
        for (int i = 0; common_numbers[i]; i++) {
            snprintf(temp, sizeof(temp), "%s%s", base_word, common_numbers[i]);
            WRITE_WORD(temp);
        }
    }
    
    // Prepend numbers
    if (rules & RULE_PREPEND) {
        char temp[MAX_WORD_LEN];
        
        for (int i = 0; i <= 9; i++) {
            snprintf(temp, sizeof(temp), "%d%s", i, base_word);
            WRITE_WORD(temp);
        }
        
        // Common prefixes
        const char *prefixes[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", NULL};
        for (int i = 0; prefixes[i]; i++) {
            snprintf(temp, sizeof(temp), "%s%s", prefixes[i], base_word);
            WRITE_WORD(temp);
        }
    }
    
    // Years
    if (rules & RULE_YEARS) {
        char temp[MAX_WORD_LEN];
        
        // Recent years
        for (int year = 2000; year <= 2025; year++) {
            snprintf(temp, sizeof(temp), "%s%d", base_word, year);
            WRITE_WORD(temp);
            
            snprintf(temp, sizeof(temp), "%d%s", year, base_word);
            WRITE_WORD(temp);
            
            // Last two digits
            snprintf(temp, sizeof(temp), "%s%02d", base_word, year % 100);
            WRITE_WORD(temp);
        }
    }
    
    // Reverse
    if (rules & RULE_REVERSE) {
        char *rev = strdup(base_word);
        reverse_string(rev);
        WRITE_WORD(rev);
        free(rev);
    }
    
    // Double
    if (rules & RULE_DOUBLE) {
        char temp[MAX_WORD_LEN * 2];
        snprintf(temp, sizeof(temp), "%s%s", base_word, base_word);
        WRITE_WORD(temp);
    }
    
    fclose(fp);
    printf("[+] Generated %d words to %s\n", word_count, output_file);
}