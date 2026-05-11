#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>

int main() {
    FILE *fp = fopen("5.json", "rb");
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
    cJSON *network = cJSON_GetObjectItem(root, "network");

    cJSON *ip = cJSON_GetObjectItem(network, "ip");
    if (cJSON_IsString(ip)) {
        printf("ip = %s\n", ip->valuestring);
    }

    cJSON *gateway = cJSON_GetObjectItem(network, "gateway");
    if (cJSON_IsString(gateway)) {
        printf("gateway = %s\n", gateway->valuestring);
    }

    cJSON *mask = cJSON_GetObjectItem(network, "mask");
    if (cJSON_IsString(mask)) {
        printf("mask = %s\n", mask->valuestring);
    }
    
    cJSON_Delete(root);
    free(json_data);
    
    return 0;
}
