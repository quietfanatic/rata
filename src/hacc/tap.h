
 // A TAP outputter for C++
 //
 // Instructions for separate compilation:
 // 1.  Put something like this at the bottom of each cpp file:
 //
 // #ifndef DISABLE_TESTS
 // #include "tap.h"
 // static tap::Tester mytester ("mytester", [](){
 //     using namespace tap;
 //
 //     plan(3);  // This is an example test set.
 //     ok(init_stuff(), "We can start up properly");
 //     is(get_42(), 42, "Just in case");
 //     within(entropy(), 0, 0.1, "Not too hot");
 //
 // });
 // #endif
 //
 // 2. Compile each cpp file to an object as normal.  Also compile tap_main.cpp
 //     to tap_main.o
 // 3. As a separate operation from your main program's linking phase, link each
 //     compiled object (separately) with tap_main.o to produce one test executable
 //     for each object.  Standard practice is to name them *.t and put them in a
 //     t/ directory.
 // 4. Run prove -e '' on the test executables.
 //
 // Instructions for whole-program multi-file compilation:
 //  (I used to do this, it's a little weird, but effective)
 // 1. Declare Testers at the bottom of each cpp file, like above.  Make sure you
 //     provide a string as the unique name of the tester.
 // 2. In your program's main function, check for a "--test NAME" command-line
 //     option.  If it is present, call tap::run_test_named(NAME) and exit(0).
 // 3. Fill your t/ directory with scripts that call my_program --test NAME
 //
 // Instructions for whole-program single-file compilation:
 // 1. Just put all the test code in your main(), protected by a command-line
 //     option if you want.  Since you're probably not running a lot of tests,
 //     you don't need to use prove.
 //
 // Note: You may have more than one Tester object per cpp file.  They will be
 //  executed top to bottom.  However, you must only have one call to plan() to
 //  make prove happy.

#ifndef HAVE_TAP_HPP
#define HAVE_TAP_HPP
#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#ifndef TAP_CAN_PRINT
template <class C>
bool default_can_print (const C& x) { return false; }

#define TAP_CAN_PRINT(x) default_can_print(x)
#endif
#ifndef TAP_PRINT
template <class C>
void default_print (const C& x) { }
#define TAP_PRINT(x) default_print(x)
#endif





#include <string>
 // C++ needs an abstraction like Haskell's Show.
 // I guess that's part of what HACC is for.

namespace tap {

typedef unsigned int uint;
typedef int32_t int32;
typedef uint32_t uint32;

 // tap.hpp
 // A perlish TAP-generating tester for C++

uint planned = 0;
uint tested = 0;
uint todoing = 0;
const char* todo_reason = NULL;

void plan (uint p) {
    planned = p;
    tested = 0;
    printf("1..%u\n", planned);
}

void done_testing (uint p) {
    if (!planned) 
        printf("1..%u\n", p);
    planned = 0;
    tested = 0;
}

void todo (uint num, const char* reason) {
    todoing = num;
    todo_reason = reason;
}

void BAIL_OUT (const char* reason) {
    printf("Bail out!  %s\n", reason);
}

bool pass (const char* name) {
    tested++;
    printf("ok %u - %s", tested, name);
    if (todo_reason) {
        printf(" # TODO %s\n", todo_reason);
        if (!--todoing) todo_reason = NULL;
    }
    else printf("\n");
    return true;
}
bool fail (const char* name) {
    tested++;
    printf("not ok %u - %s", tested, name);
    if (todo_reason) {
        printf(" # TODO %s\n", todo_reason);
        if (!--todoing) todo_reason = NULL;
    }
    else printf("\n");
    return false;
}

bool ok (bool t, const char* name) {
    return t ? pass(name) : fail(name);
}


template <class A, class B>
bool is (A a, B b, const char* name) {
    if (ok(a == b, name)) return true;
    else if (TAP_CAN_PRINT(a) && TAP_CAN_PRINT(b)) {
        printf(" # expected ");
        TAP_PRINT(b);
        printf("\n #  but got ");
        TAP_PRINT(a);
        printf("\n");
    }
    return false;
}

template <class A, class B>
void isnt (A a, B b, const char* name) {
    ok(a != b, name);  // No need for expected/got here
}

void within (double a, double b, double range, const char* name) {
    if (a - range <= b && b - range <= a) pass(name);
    else {
        fail(name);
        printf(" # expected %lg +- %lg\n #  but got %lg\n", b, range, a);
    }
}

void about (double a, double b, const char* name) {
    within(a, b, 0.01, name);  // Quite tolerant
}

struct Tester {
    const char* name;
    void (* code )();
    Tester* next;
    Tester (const char*, void (*)());
};

 // We are including all the actual testing code in the tested .o
 // Otherwise to compile without tap_main.o you'd have to use another tap.o

Tester* first_tester = NULL;
Tester** tester_tail = &first_tester;

Tester::Tester (const char* name, void (* code )()) :
    name(name), code(code), next(NULL)
{
    *tester_tail = this;
    tester_tail = &next;
}

void run_tester_named(const char* name) {
    for (Tester* t = first_tester; t; t = t->next) {
        if (0==strcmp(t->name, name)) {
            t->code();
            exit(0);
        }
    }
    printf("1..1\nnot ok 1 - No tester was compiled named %s\n", name);
}
void run_all_testers () {
    for (Tester* t = first_tester; t; t = t->next) {
        t->code();
    }
}


}

#endif
