#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>

int main() {
    FILE *fp = fopen("2.json", "rb");
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
    cJSON *ip = cJSON_GetObjectItem(root, "ip");

    if (!cJSON_IsString(ip)) {
        printf("ip is not a string\n");
        cJSON_Delete(root);
        free(json_data);
        return 1;
    }

    printf("ip = %s\n", ip->valuestring);

    cJSON *port = cJSON_GetObjectItem(root, "port");

    if (!cJSON_IsNumber(port)) {
        printf("ip is not a number\n");
        cJSON_Delete(root);
        free(json_data);
        return 1;
    }

    printf("port = %d\n", (int)(port->valuedouble));

    cJSON *debug = cJSON_GetObjectItem(root, "debug");
    
    if (!cJSON_IsBool(debug)) {
        printf("ip is not a bool\n");
        cJSON_Delete(root);
        free(json_data);
        return 1;
    }
    
    printf("debug = %s\n", cJSON_IsTrue(debug) ? "true" : "false");

    
    cJSON_Delete(root);
    free(json_data);
    
    return 0;
}
