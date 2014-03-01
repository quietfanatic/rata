
 // A TAP outputting test library for C++
 //
 // Instructions:
 //
 // 1. Declare Testers at the bottom of each cpp file, or wherever you want.  They
 //     should look something like:
 //
 //     #ifndef DISABLE_TESTS
 //     #include "tap.h"
 //     static tap::Tester universe_tests ("universe/universe.cpp" [](){
 //         using namespace tap;
 //         plan(3);
 //         ok(init_universe(), "Everything starts up right");
 //         is(get_answer(), 42, "Just in case");
 //         within(entropy(), 0.1, 0, "Not too hot");
 //     });
 //     #endif
 //
 //     Make sure you provide a unique name for each tester.  They don't have to
 //     be the filename of the file in which they reside.
 //
 // 2. At the front of your main function, put this:
 //
 //     tap::allow_testing(argc, argv, "--test");
 //
 //     This will cause your program to respond to those command-line arguments.
 //     If you give your program "--test universe/universe.cpp" as arguments, it
 //     will run the test you installed with than name *and then exit*.  If you give
 //     it "--test" without arguments, it will print a list of command lines in the
 //     form of "./my_program --test universe/universe.cpp" for each test you've
 //     declared.  You can change the "--test" to "-t" or anything you want.  If you
 //     pass an empty string as the third argument, then running your program with
 //     a single argument or no arguments will produce the above behavior.  Doing so
 //     is probably a bad idea unless you're compiling separate testing and usage
 //     executables.  You can optionally provide a fourth argument to allow_testing,
 //     to have a different option for listing tests than running tests, like
 //     "--list-tests" or something.
 //
 // 3. Compile tap.cpp and link tap.o with your program, or if tap is a shared
 //     library somewhere, link to it.  Unless you're disabling tests, such as for
 //     an optimized release, in which case you should define DISABLE_TESTS and not
 //     link to tap.o (though you may still include tap.h and call allow_testing).
 //
 // 4. To run the tests, do this:
 //
 //     .PHONY : test
 //     test : my_program
 //         ./my_program --test | prove -e '' -
 //
 // Advanced usage:  DISCLAIMER: I haven't actually tried this.
 //    If you want to set up on-site testing, you might want to link separate
 //     testing and usage executables, while having only one compile phase.  To do
 //     so, you will have to separate your tests from your code, such as in
 //     corresponding .t.cpp files, or through an #ifdef.  When building, compile the
 //     file containing the main function once with DISABLE_TESTS and once without.
 //     Compile the test files either into separate objects or into one object.
 //     Link your DISABLE_TESTS main with all your normal objects, and without the
 //     test objects and without the tap object.  Link your non-DISABLE_TESTS main
 //     with all the objects: normal, testing and tap.  Now run tests on the testing
 //     executable; if they succeed, provide the user with the usage executable.
 //

#ifndef HAVE_TAP_H
#define HAVE_TAP_H
#define __STDC_FORMAT_MACROS
#include <exception>
#include <typeinfo>

#ifdef DISABLE_TESTS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
namespace tap {
     // Provide dummy testing functions
    static void run_test (const char* name) {
        printf("1..0 # Skipping: This program was compiled with tests disabled.\n");
    }
    static void run_all_tests () { run_test(""); }
    static void allow_testing (int argc, char** argv, const char* test_flag, const char* list_flag = "") {
        if (argc > 1 && 0==strcmp(argv[0], test_flag)) {
            if (argc > 2) run_test("");
            exit(0);
        }
    }
}
#else

namespace tap {
using namespace std;

 // This is the struct you use to declare a set of tests.
struct Tester {
    const char* name;
    double order;
    void (* code )();
    Tester* next;
     // The constructor will register the test at init-time.
    Tester (const char* name, void (* code )());
     // You can optionally give the tester an order.  Testers with a lower
     //  order will run first.  Testers without order will be run after tests
     //  with order, in a random order.  This does not mean you can set up
     //  data in an earlier test to use in a later test.  You can't, because
     //  in the usual case, each tester is run in a different process.
    Tester (const char* name, double order, void (* code )());
};

 // Do this to allow command-line testing.
void allow_testing (int argc, char** argv, const char* test_flag);

 // Do this at the beginning of your testing.  Give it as an argument the
 //  number of tests you plan to run.  If you run a different number, the
 //  test set is considered to have failed.
void plan (unsigned num_tests);

 // Alternatively, do this at the end of your testing.
void done_testing (unsigned num_tests);

 // Run a test.  If succeeded is true, the test is successful, otherwise it
 //  is a failure.  The form that takes a function pointer (I recommend using
 //  a C++11 lambda) will also catch exceptions.  If it does, it fails the
 //  test and prints the exception's what().
bool ok (bool succeeded, const char* name = "");
bool ok (bool (* code )(), const char* name = "");

 // Run a test that succeeds if got == expected (with overloaded operator ==).
 //  If the test failed, it will try to tell you what it got vs. what it expected.
 //  You probably know that you shouldn't use == to compare floating point numbers,
 //  so for those, look at within() and about().
 // As a special case, you can use is() with const char* and it'll do a strcmp (with
 //  NULL checks).
template <class A, class B>
bool is (const A& got, const B& expected, const char* name = "");
template <class A, class B>
bool is (A (* code )(), const B& expected, const char* name = "");
 // You can call the special case directly if you want.
bool is_strcmp(const char* got, const char* expected, const char* name = "");
bool is_strcmp(const char* (* code )(), const char* expected, const char* name = "");

 // There isn't enough of a use case for isnt() to justify coding it.
 // Just use ok(a != b, "a isn't b");
 // Because there are no useful diagnostics to print if isnt() fails.  Maybe if a
 //  more sophisticated diagnostic system arises, there'll be a reason to include it,
 //  or if it turns out catching exceptions in the != is important.

 // Tests that got is within +/- range of expected.
bool within (double got, double range, double expected, const char* name = "");
bool within (double (* code )(), double range, double expected, const char* name = "");
 // Tests that got is within a factor of .001 of expected.
bool about (double got, double expected, const char* name = "");
bool about (double (* code )(), double expected, const char* name = "");

 // Tests that code throws an exception of class Except.  If a different kind of
 //  exception is thrown, the test fails.
template <class Except>
bool throws (void (* code )(), const char* name = "");

 // Like above, but fails if the thrown exception does not satisfy extest.
template <class Except>
bool throws (void (* code )(), bool (* extest )(Except&), const char* name = "");

 // Succeeds if no exception is thrown.
bool doesnt_throw (void (* code )(), const char* name = "");

 // Automatically pass a test with this name.  Only resort to this if you can't
 //  make your test work with the other testing functions.
bool pass (const char* name = "");
 // Likewise with fail.
bool fail (const char* name = "");

 // Makes it so passes are failures and failures are passes.
 // This is primarily for testing the testing library itself.
bool invert_success ();

 // Print a message as diagnostics.  This should only be a single line.
 //  It uses a printf format and arguments.
void diag (const char* fmt, ...);

 // Print a diagnostic about having expected one thing but gotten another.
 //  The types must be printable.
template <class A, class B>
void diag_unexpected (A got, B expected);

 // Mark the next num tests as todo.  You must still run the tests.  If only
 //  todo tests fail, the test set is still considered successful.
void todo (unsigned num, const char* reason = "");

 // Declare that you've skipped num tests.  You must not still run the tests.
void skip (unsigned num, const char* reason = "");

 // When everything is wrong and you can't even continue testing, do this.
void BAIL_OUT (const char* reason = "");


 // The following two functions are for if you're doing your own command-line
 // parsing and want testing to behave with it.
 // To run a tester manually, do this.  It will not exit unless BAIL_OUT is called.
void run_test (const char* name);
 // To list the tests manually, do this.  It will print to stdout.
void list_tests (int argc, char** argv);

 // This will run all the testers, but is not that useful, since you should
 // probably be using prove.
void run_all_tests ();


 // Use this to allow tap to unmangle type names.
// void set_unmangler(const char* (* unmangler )(const type_info&));
 // Use this to change tap's behavior when printing things.
// void set_printer(bool (* can_print )(const type_info&), void (* print )(const type_info&, void*));


 // If your exception inherits from this, it will cause all testing functions
 // to bail out when it's thrown, though it'll still print diagnostics.
struct scary_exception : std::exception { };


 // BELOW THIS LINE IS IMPLEMENTATION DETAILS
 // =========================================

namespace internal {
     // These wrap typeless routines to catch exceptions.  They will call fail()
     // if they catch something, but will not call pass() if they don't.
    bool glove_1(void*, void*, const char*);
    bool glove_2(void*, void*, void*, const char*);
    bool glove_3(void*, void*, void*, void*, const char*);
     // Like above, but upon catching, claims to have wanted an exception
     // of the type of the first argument.
    bool glove_1_wanted(const type_info&, void*, void*, const char*);
    bool glove_2_wanted(const type_info&, void*, void*, void*, const char*);
     // fail() and use diag_unexpected_dyn
    bool fail_is(const type_info&, void*, const type_info&, void*, const char*);

    bool fail_didnt_throw(const char*);
    bool fail_extest(const char*);
    void diag_unexpected_dyn(const type_info&, void*, const type_info&, void*);

     // These implement the logic that goes inside a glove.
    template <class A, class B>
    bool is_handler_pp (const A* got, const B* expected, const char* name) {
        return *got == *expected
            ? pass(name)
            : fail_is(typeid(A), (void*)got, typeid(B), (void*)expected, name);
    }
    template <class A, class B>
    bool is_handler_cp (A (* code )(), const B* expected, const char* name) {
        A got = code();
        return is_handler_pp(&got, expected, name);
    }
    template <class E>
    bool picky_catcher (void (* code )(), const char* name) {
        try { code(); return fail_didnt_throw(name); }
        catch (E e) { return pass(name); }
    }
    template <class E>
    bool pickier_catcher (void (* code )(), bool (* extest )(E), const char* name) {
        try { code(); return fail_didnt_throw(name); }
        catch (E e) {
            return extest(e)
                ? pass(name)
                : fail_extest(name);
        }
    }
}


 // Filling out of the templated wrappers.
template <class A, class B>
inline bool is (const A& got, const B& expected, const char* name) {
    using namespace internal;
    return glove_2((void*)is_handler_pp<A, B>, (void*)&got, (void*)&expected, name);
}
template <class A, class B>
inline bool is (A (* code )(), const B& expected, const char* name) {
    using namespace internal;
    return glove_2((void*)is_handler_cp<A, B>, (void*)code, (void*)&expected, name);
}
 // simply aliases for is_strcmp
inline bool is (const char* got, const char* expected, const char* name) {
    return is_strcmp(got, expected, name);
}
inline bool is (const char* (* code )(), const char* expected, const char* name) {
    return is_strcmp(code, expected, name);
}

 // Catching specific exceptions
template <class E>
bool throws (void (* code )(), const char* name) {
    using namespace internal;
    return glove_1_wanted(typeid(E), (void*)picky_catcher<E>, (void*)code, name);
}

template <class E>
bool throws (void (* code )(), bool (* extest )(E), const char* name) {
    using namespace internal;
    return glove_2_wanted(typeid(E), (void*)pickier_catcher<E>, (void*)code, (void*)extest, name);
}

 // Diagnostics
template <class A, class B>
void diag_unexpected (A got, B expected) {
    using namespace internal;
    diag_unexpected_dyn(typeid(A), &got, typeid(B), &expected);
}

}  // namespace tap

#endif  // DISABLE_TESTS

#endif
