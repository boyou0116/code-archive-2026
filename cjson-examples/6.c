#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>

int main() {
    FILE *fp = fopen("6.json", "rb");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *json_data = malloc(file_size + 1);
    if(json_data == NULL) {
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
    cJSON *device_name = cJSON_GetObjectItem(root, "device_name");
    if (cJSON_IsString(device_name)) {
        printf("device_name = %s\n", device_name->valuestring);
    }

    cJSON *debug = cJSON_GetObjectItem(root, "debug");
    if (cJSON_IsBool(debug)) {
        printf("debug = %s\n", cJSON_IsTrue(debug) ? "true" : "false");
    }

    cJSON *timeout_ms = cJSON_GetObjectItem(root, "timeout_ms");
    if (cJSON_IsNumber(timeout_ms)) {
        printf("timeout_ms = %d\n", timeout_ms->valueint);
    }

    cJSON *baudrate = cJSON_GetObjectItem(root, "baudrate");
    if (cJSON_IsNumber(baudrate)) {
        printf("baudrate = %d\n", baudrate->valueint);
    }

    cJSON *ports = cJSON_GetObjectItem(root, "ports");

    int count = 0;
    if (cJSON_IsArray(ports)) {
        count = cJSON_GetArraySize(ports);
        for (int i = 0; i < count; i++) {
            cJSON *port_id = cJSON_GetArrayItem(ports, i);
            if (cJSON_IsNumber(port_id)) {
                printf("port[%d] = %d\n", i, port_id->valueint);
            }
        }
    }

    cJSON *network = cJSON_GetObjectItem(root, "network");
    if (cJSON_IsObject(network)) {
        printf("network:\n");
        cJSON *ip = cJSON_GetObjectItem(network, "ip");
        if (cJSON_IsString(ip)) {
            printf("\tip = %s\n", ip->valuestring);
        }
        cJSON *gateway = cJSON_GetObjectItem(network, "gateway");
        if (cJSON_IsString(gateway)) {
            printf("\tgateway = %s\n", gateway->valuestring);
        }

        cJSON *mask = cJSON_GetObjectItem(network, "mask");
        if (cJSON_IsString(mask)) {
            printf("\tmask = %s\n", mask->valuestring);
        }
    }
    
    return 0;
}
