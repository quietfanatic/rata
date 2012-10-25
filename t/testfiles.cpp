
#include "tap.hpp"

bool has_tests = false;

struct Testfile : Hashed<Testfile> {
    void (* test ) ();
    Testfile (CStr name, void (* test ) ()) : Hashed(name), test(test) { has_tests = true; }
};

void do_test (CStr name) {
    if (has_tests) {
        Testfile* test = Hashed<Testfile>::table.lookup(name);
        if (!test) printf("not ok 1 - No test %s was compiled\n", name);
        else test->test();
    }
    else {
        printf("0..1 # Skipped: This program has not been compiled with tests\n");
    }
};


