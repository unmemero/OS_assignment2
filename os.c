#include <stdio.h>

int main() {
    #if defined(_WIN32) || defined(_WIN64)
        printf("Operating System: Windows\n");
    #elif defined(__linux__)
        printf("Operating System: Linux\n");
    #elif defined(__APPLE__) || defined(__MACH__)
        printf("Operating System: macOS\n");
    #elif defined(__unix__)
        printf("Operating System: UNIX\n");
    #elif defined(__FreeBSD__)
        printf("Operating System: FreeBSD\n");
    #else
        printf("Operating System: Unknown\n");
    #endif
    return 0;
}
