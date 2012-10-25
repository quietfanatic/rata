#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
typedef unsigned int uint;
typedef int32_t int32;
typedef uint32_t uint32;

 // test_more.hpp
 // A perlish TAP tester for C++, comparable to Test::More

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
    planned = 0;
    tested = 0;
    printf("1..%u\n", p);
}

void todo (uint num, const char* reason) {
    todoing = num;
    todo_reason = reason;
}

void BAIL_OUT (const char* reason) {
    printf("Bail out!  %s\n", reason);
}

void pass (const char* name) {
    tested++;
    printf("ok %u - %s", tested, name);
    if (todo_reason) {
        printf(" # TODO %s\n", todo_reason);
        if (!--todoing) todo_reason = NULL;
    }
    else printf("\n");
}
void fail (const char* name) {
    tested++;
    printf("not ok %u - %s", tested, name);
    if (todo_reason) {
        printf(" # TODO %s\n", todo_reason);
        if (!--todoing) todo_reason = NULL;
    }
    else printf("\n");
}

void ok (bool t, const char* name) {
    if (t) pass(name);
    else fail(name);
}


template <class A, class B>
void is (A a, B b, const char* name) {
    ok(a == b, name);
}

 // special cased for diagnostic convenience.
 // Eventually we'll want to run a and b through RSON to diagnose.
void is (int32 a, int32 b, const char* name) {
    if (a == b) pass(name);
    else {
        fail(name);
        printf(" # expected %" PRIi32 "\n #  but got %" PRIi32 "\n", b, a);
    }
}
void is (uint32 a, uint32 b, const char* name) {
    if (a == b) pass(name);
    else {
        fail(name);
        printf(" # expected %" PRIu32 "\n #  but got %" PRIu32 "\n", b, a);
    }
}
void is (double a, double b, const char* name) {
    if (a == b) pass(name);
    else {
        fail(name);
        printf(" # expected %lg\n #  but got %lg\n", b, a);
    }
}
void is (const char* a, const char* b, const char* name) {
    if (0==strcmp(a, b)) pass(name);
    else {
        fail(name);
        printf(" # expected \"%s\"\n #  but got \"%s\"\n", b, a);
    }
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
