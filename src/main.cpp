
#include <stdio.h>

#define HEADER
#include "index.cpp"
#undef HEADER
#define IMPLEMENTATION
#include "index.cpp"
#undef IMPLEMENTATION

int main () {
    printf("hello\n");
    return 0;
}





