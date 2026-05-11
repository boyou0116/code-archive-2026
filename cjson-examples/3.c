#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>

int main() {
    FILE *fp = fopen("3.json", "rb");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp); // fseek(fp, 0, SEEK_SET)

    char *json_data = malloc(file_size + 1);
    if (json_data == NULL) {
        perror("malloc");
        fclose(fp);
        return 1;
    }

    fread(json_data, 1, file_size, fp);
    json_data[file_size] = '\0';

    fclose(fp);

    cJSON *root = cJSON_Parse(json_data);

    if (root == NULL) {
        
        printf("JSON parse error\n");

        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            printf("Error before: %s\n", error_ptr);
        }

        free(json_data);
        return 1;
    }

    // parse JSON
    cJSON *ports = cJSON_GetObjectItem(root, "ports");
    if (!cJSON_IsArray(ports)) {
        printf("ports is not an Array\n");
        cJSON_Delete(root);
        free(json_data);
        return 1;
    }

    int count = cJSON_GetArraySize(ports);

    printf("count = %d\n", count);

    for (int i = 0; i < 4; i++) {
        cJSON *pid = cJSON_GetArrayItem(ports, i);
        if (cJSON_IsNumber(pid)) {
            printf("port[%d] = %d\n", i, pid->valueint);
        }
    }
    
    cJSON_Delete(root);
    free(json_data);
    
    return 0;
}
