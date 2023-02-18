// yuhangs-MacBook-Air.local
#include <unistd.h>
#include <stdio.h>

int main() {
    char hostname[256];
    int result = gethostname(hostname, sizeof(hostname));
    if (result == 0) {
        printf("Hostname: %s\n", hostname);
    } else {
        perror("gethostname");
        return 1;
    }
    return 0;
}
