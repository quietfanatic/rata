
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "tap/inc/tap.h"

int main (int argc, char** argv) {
     // Change directory to program
    char* lastslash = NULL;
    for (char* p = argv[0]; *p; p++) {
        if (*p == '/') lastslash = p;
    }
    if (lastslash) {
        char mydir [lastslash - argv[0] + 1];
        memcpy((void*)mydir, (void*)argv[0], lastslash - argv[0]);
        mydir[lastslash - argv[0]] = 0;
        if (chdir(mydir) == -1) {
            printf("chdir() failed with %d (%s)\n", errno, strerror(errno));
        }
    }
    tap::allow_testing(argc, argv, "--test");
    return 0;
}

