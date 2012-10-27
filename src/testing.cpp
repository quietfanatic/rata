
#ifdef HEADER
#include "tap.hpp"

struct Tester : Named<Tester> {
    void (* code )();
    Tester (CStr name, void (* code )()) : Named(name), code(code) { }
};

void run_test (CStr name) {
#ifdef DISABLE_TESTS
    printf("1..0 # Skipping: This program instance has not been compiled with tests.\n");
#else
    if (Tester* t = Named<Tester>::lookup(name)) {
        if (t->code)
            t->code();
        else printf("1..1\nnot ok 1 - The tester named %s has real code\n", name);
    }  // The convention for tests we're using produces false statements when tests fail.
    else printf("1..1\nnot ok 1 - There is a tester named %s\n", name);
#endif
}

#endif

#ifdef IMPLEMENTATION

#ifndef DISABLE_TESTS

Tester testing_tester ("testing", [](){
    plan(2);
    is(Named<Tester>::lookup("testing"), &testing_tester, "This tester was registered when declared");
    ok(Named<Tester>::lookup("testing")->code, "This tester has code");
});

#endif

#endif
