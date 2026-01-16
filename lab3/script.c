#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <openssl/sha.h>

#define MAX_PATH 4096
#define TABLE_SIZE 4096

typedef struct HashEntry {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    char* first_path;
    struct HashEntry* next;
} HashEntry;

HashEntry* hash_table[TABLE_SIZE];

unsigned int get_table_index(unsigned char hash[SHA256_DIGEST_LENGTH]) {
    unsigned int h;
    memcpy(&h, hash, sizeof(int));
    return h % TABLE_SIZE;
}

char* find_or_add(unsigned char hash[SHA256_DIGEST_LENGTH], const char* path) {
    unsigned int idx = get_table_index(hash);
    HashEntry* entry = hash_table[idx];
    while (entry != NULL) {
        if (memcmp(entry->hash, hash, SHA256_DIGEST_LENGTH) == 0) {
            return entry->first_path;
        }
        entry = entry->next;
    }
    HashEntry* new_node = malloc(sizeof(HashEntry));
    new_node->hash[0] = 0; 
    memcpy(new_node->hash, hash, SHA256_DIGEST_LENGTH);
    new_node->first_path = strdup(path);
    new_node->next = hash_table[idx];
    hash_table[idx] = new_node;
    return NULL;
}

int compute_sha256(const char* filepath, unsigned char out[SHA256_DIGEST_LENGTH]) {
    FILE* fp = fopen(filepath, "rb");
    if (!fp) return -1;
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    unsigned char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) SHA256_Update(&ctx, buf, n);
    SHA256_Final(out, &ctx);
    fclose(fp);
    return 0;
}

void process_dir(const char* base_path) {
    DIR* dir = opendir(base_path);
    if (!dir) return;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s/%s", base_path, entry->d_name);
        struct stat st;
        if (lstat(full_path, &st) != 0) continue;
        if (S_ISDIR(st.st_mode)) {
            process_dir(full_path);
            continue;
        }
        if (!S_ISREG(st.st_mode)) continue;
        unsigned char hash[SHA256_DIGEST_LENGTH];
        if (compute_sha256(full_path, hash) != 0) continue;
        char* existing_path = find_or_add(hash, full_path);
        if (existing_path) {
            struct stat first_st;
            if (stat(existing_path, &first_st) == 0) {
                if (st.st_dev == first_st.st_dev && st.st_ino != first_st.st_ino) {
                    if (unlink(full_path) == 0 && link(existing_path, full_path) == 0) {
                        printf("%s -> %s\n", full_path, existing_path);
                    }
                }
            }
        }
    }
    closedir(dir);
}

int main(int argc, char* argv[]) {
    if (argc != 2) return 1;
    memset(hash_table, 0, sizeof(hash_table));
    process_dir(argv[1]);
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashEntry* entry = hash_table[i];
        while (entry) {
            HashEntry* tmp = entry;
            entry = entry->next;
            free(tmp->first_path);
            free(tmp);
        }
    }
    return 0;
}
