

#define HEADER
#include "index.cpp"
#undef HEADER
#define IMPLEMENTATION
#include "index.cpp"
#undef IMPLEMENTATION


int main (int argc, char** argv) {
    if (argc >= 3) {
        if (0==strcmp(argv[1], "--test")) {
            tap::run_tester_named(argv[2]);
        }
    }
    printf("hello\n");
    return 0;
}





