#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <time.h>
#include <ctype.h>
#include "include/cyberforce.h"
#include "include/defines.h"

char *md5_hash(const char *input) {
    if (!input) return NULL;
    
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5((unsigned char*)input, strlen(input), digest);
    
    char *result = malloc(MD5_DIGEST_LENGTH * 2 + 1);
    if (!result) return NULL;
    
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(&result[i*2], "%02x", (unsigned int)digest[i]);
    }
    
    return result;
}

char *sha256_hash(const char *input) {
    if (!input) return NULL;
    
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)input, strlen(input), digest);
    
    char *result = malloc(SHA256_DIGEST_LENGTH * 2 + 1);
    if (!result) return NULL;
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&result[i*2], "%02x", (unsigned int)digest[i]);
    }
    
    return result;
}

char *base64_encode(const char *input) {
    if (!input) return NULL;
    
    BIO *bmem, *b64;
    BUF_MEM *bptr;
    
    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    
    BIO_write(b64, input, strlen(input));
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);
    
    char *result = malloc(bptr->length + 1);
    if (!result) {
        BIO_free_all(b64);
        return NULL;
    }
    
    memcpy(result, bptr->data, bptr->length);
    result[bptr->length] = '\0';
    
    BIO_free_all(b64);
    return result;
}

char *base64_decode(const char *input) {
    if (!input) return NULL;
    
    BIO *b64, *bmem;
    size_t length = strlen(input);
    char *buffer = malloc(length);
    if (!buffer) return NULL;
    
    memset(buffer, 0, length);
    
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bmem = BIO_new_mem_buf(input, -1);
    bmem = BIO_push(b64, bmem);
    
    int len = BIO_read(bmem, buffer, length);
    BIO_free_all(bmem);
    
    if (len <= 0) {
        free(buffer);
        return NULL;
    }
    
    buffer[len] = '\0';
    return buffer;
}

char *generate_random_string(int length) {
    if (length <= 0) return NULL;
    
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    char *result = malloc(length + 1);
    if (!result) return NULL;
    
    srand(time(NULL));
    
    for (int i = 0; i < length; i++) {
        int key = rand() % (int)(sizeof(charset) - 1);
        result[i] = charset[key];
    }
    
    result[length] = '\0';
    return result;
}