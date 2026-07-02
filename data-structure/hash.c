#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TABLE_SIZE 4

typedef struct Node {
    char *key;
    int value;
    struct Node *next;
} Node;

typedef struct HashTable {
    Node *buckets[TABLE_SIZE];
} HashTable;

void ht_init(HashTable *table) {
    for (int i = 0; i < TABLE_SIZE; i++)
        table->buckets[i] = NULL;
}

unsigned long hash_string(const char *str) {
    unsigned long hash = 5381;
    while (*str != '\0') {
        hash = hash * 33 + (unsigned char)(*str);
        str++;
    }
    return hash;
}

char *str_duplicate(const char *s) {
    char *copy = malloc(strlen(s) + 1);
    if (copy == NULL) {
        perror("malloc failed");
        exit(1);
    }

    strcpy(copy, s);
    return copy;
}

void ht_set(HashTable *table, const char *key, int value) {
    unsigned long hash = hash_string(key);
    int index = hash % TABLE_SIZE;

    Node *current = table->buckets[index];

    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            current->value = value;
            return;
        }
        current = current->next;
    }

    Node *new_node = malloc(sizeof(Node));
    if (new_node == NULL) {
        perror("malloc failed");
        exit(1);
    }

    new_node->key = str_duplicate(key);
    new_node->value = value;
    new_node->next = table->buckets[index];
    table->buckets[index] = new_node;
}

int ht_get(HashTable *table, const char *key, int *out_value) {
    unsigned long hash = hash_string(key);
    int index = hash % TABLE_SIZE;

    Node *current = table->buckets[index];

    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            *out_value = current->value;
            return 1;
        }
        current = current->next;
    }

    return 0;
}

int ht_delete(HashTable *table, const char *key) {
    unsigned long hash = hash_string(key);
    int index = hash % TABLE_SIZE;

    Node *current = table->buckets[index];
    Node *prev = NULL;

    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            if (prev == NULL)
                table->buckets[index] = current->next;
            else
                prev->next = current->next;

            free(current->key);
            free(current);
            return 1;
        }
        prev = current;
        current = current->next;
    }

    return 0;
}

void ht_print(HashTable *table) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        printf("bucket[%d]: ", i);
        Node *current = table->buckets[i];

        while (current != NULL) {
            printf("(%s: %d) -> ", current->key, current->value);
            current = current->next;
        }

        printf("NULL\n");
    }
}

void ht_free(HashTable *table) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Node *current = table->buckets[i];
        
        while (current != NULL) {
            Node *next = current->next;
            free(current->key);
            free(current);
            current = next;
        }

        table->buckets[i] = NULL;
    }
}

int main() {
    HashTable table;
    ht_init(&table);

    while (1) {
        char line[16];
        char cmd[sizeof line];
        char key[sizeof line];
        int value;

        printf("\n\ncommands list:\ns: set hash table\ng: get hash table\nd: delete key\np: print hash table\nq: quit\n\n");
        if (fgets(line, sizeof line, stdin) == NULL) break;

        if (strchr(line, '\n') == NULL) {
            int c = getchar();
            if (c != EOF) {
                while (c != '\n' && c != EOF)
                    c = getchar();
                printf("\ninput too long, ignored\n");
                continue;
            }
            /* EOF right after a full read: the last line just has no
               trailing newline, so fall through and process it */
        }

        if (sscanf(line, "%s", cmd) != 1)
            continue;

        if (cmd[0] == 's') {
            if (sscanf(line, "%*s %s %d", key, &value) == 2)
                ht_set(&table, key, value);
            else
                printf("\nusage: s <key> <value>\n");
        } else if (cmd[0] == 'g') {
            if (sscanf(line, "%*s %s", key) == 1) {
                if (ht_get(&table, key, &value)) {
                    printf("%s = %d\n", key, value);
                } else {
                    printf("%s not found\n", key);
                }
            } else {
                printf("\nusage: g <key>\n");
            }
        } else if (cmd[0] == 'd') {
            if (sscanf(line, "%*s %s", key) == 1) {
                if (ht_delete(&table, key))
                    printf("%s deleted\n", key);
                else
                    printf("%s not found\n", key);
            } else {
                printf("\nusage: d <key>\n");
            }
        } else if (cmd[0] == 'p') {
            printf("\nHash table contents\n");
            ht_print(&table);
        } else if (cmd[0] == 'q') {
            break;
        } else {
            printf("\n\ninvalid command\n\n");
        }
    }

    ht_free(&table);

    return 0;
}
