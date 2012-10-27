

#define HEADER
#include "index.cpp"
#undef HEADER
#define IMPLEMENTATION
#include "index.cpp"
#undef IMPLEMENTATION


int main (int argc, char** argv) {
    if (argc > 2 && 0==strcmp(argv[1], "--test")) {
        run_test(argv[2]);
        exit(0);
    }
    printf("hello\n");
    return 0;
}





