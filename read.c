#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LINE_LENGTH 100
#define MAX_SERVERS 100

typedef struct {
    int num;
    char servers[MAX_SERVERS][MAX_LINE_LENGTH];
    int num_servers;
} ServerList;

void readServerList(char* filename, ServerList* serverList) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return;
    }
    char line[MAX_LINE_LENGTH];
    int lineNum = 0;
    while (fgets(line, MAX_LINE_LENGTH, file)) {
        char* token;
        token = strtok(line, ",");
        while(token) {
            int num = atoi(token);
            if (num != 0) {
                if (lineNum > 0) {
                    serverList[lineNum - 1] = *serverList;
                    serverList++;
                }
                serverList->num = num;
                serverList->num_servers = 0;
            } else {
                if (serverList->num_servers < MAX_SERVERS) {
                    strcpy(serverList->servers[serverList->num_servers], token);
                    serverList->num_servers++;
                }
            }
            token = strtok(NULL, ",");
        }
        lineNum++;
    }
    fclose(file);
}

int main() {
    ServerList serverList[MAX_SERVERS];
    readServerList("list.txt", serverList);

    // Loop through the ServerList elements
    for (int i = 0; i < MAX_SERVERS; i++) {
        if (serverList[i].num_servers == 0) {
            // End of data
            break;
        }
        printf("Server list #%d:\n", serverList[i].num);
        // Loop through the server strings in the ServerList
        for (int j = 0; j < serverList[i].num_servers; j++) {
            printf(" - %s\n", serverList[i].servers[j]);
        }
    }
    return 0;
}
