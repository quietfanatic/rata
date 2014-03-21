
#ifdef DISABLE_TESTS
#error tap.cpp cannot be compiled with DISABLE_TESTS
#else

#include "tap/inc/tap.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>
#include <unordered_map>
#include <typeinfo>
#include <type_traits>

namespace tap {
using namespace std;

static Tester*& first_tester () {
    static Tester* first_tester = NULL;
    return first_tester;
}
static Tester**& tester_tail () {
    static Tester** tester_tail = &first_tester();
    return tester_tail;
}

static unsigned planned = 0;
static unsigned tested = 0;
static unsigned todoing = 0;
static const char* todo_string = NULL;
static bool inverted = false;

Tester::Tester (const char* name, void (* code )()) :
    name(name), order(1/0.0), code(code), next(NULL)
{
    *tester_tail() = this;
    tester_tail() = &next;
}
Tester::Tester (const char* name, double order, void (* code )()) :
    name(name), order(order), code(code), next(NULL)
{
    if (order <= 1/0.0) {
        for (Tester** t = &first_tester(); *t; t = &(*t)->next) {
            if (order < (*t)->order) {
                next = *t;
                *t = this;
                return;
            }
        }
    } // we're last
    *tester_tail() = this;
    tester_tail() = &next;
}

void run_test (const char* name) {
    if (!first_tester()) {
        printf("1..0 # SKIP this program was compiled with testing disabled\n");
        return;
    }
    for (Tester* t = first_tester(); t; t = t->next) {
        if (0==strcmp(t->name, name)) {
            t->code();
            return;
        }
    }
    printf("1..1\nnot ok 1 - No test named %s has been compiled.\n", name);
}

void list_tests (int argc, char** argv, const char* test_flag) {
    if (!first_tester()) {
        printf("%s %s '(testing disabled)'\n", argv[0], test_flag);
        return;
    }
    for (Tester* t = first_tester(); t; t = t->next) {
        printf("%s %s '%s'\n", argv[0], test_flag, t->name);
    }
}

void allow_testing (int argc, char** argv, const char* test_flag) {
    if (test_flag && test_flag[0]) {
        if (argc >= 2 && 0==strcmp(argv[1], test_flag)) {
            if (argc >= 3) {
                run_test(argv[2]);
                exit(0);
            }
            else {
                list_tests(argc, argv, test_flag);
                exit(0);
            }
        }
        return;  // escape here if no testing arguments.
    }
    else if (argc >= 2) {
        run_test(argv[1]);
        exit(0);
    }
    else {
        list_tests(argc, argv, test_flag);
        exit(0);
    }
}

void plan (unsigned num_tests) {
    planned = num_tests;
    tested = 0;
    todoing = 0;
    printf("1..%u\n", num_tests);
}

void done_testing (unsigned num_tests) {
    plan(num_tests);  // I think it's just the same.
}

bool ok (bool succeeded, const char* name) {
    return succeeded ? pass(name) : fail(name);
}
static bool ok_handler (bool (* code ()), const char* name) {
    return ok(code(), name);
}
bool ok (bool (* code )(), const char* name) {
    using namespace internal;
    return glove_1((void*)ok_handler, (void*)code, name);
}

bool is_strcmp(const char* got, const char* expected, const char* name) {
    using namespace internal;
    if (!got) {
        if (!expected) return pass(name);
        else {
            fail(name);
            diag_unexpected(got, expected);
            return false;
        }
    }
    else if (!expected) {
        fail(name);
        diag_unexpected(got, expected);
        return false;
    }
    else if (0==strcmp(got, expected)) return pass(name);
    else {
        fail(name);
        diag_unexpected(got, expected);
        return false;
    }
}
static bool is_strcmp_handler (const char* (* code )(), const char* expected, const char* name) {
    return is_strcmp(code(), expected, name);
}
bool is_strcmp(const char* (* code )(), const char* expected, const char* name) {
    using namespace internal;
    return glove_2((void*)is_strcmp_handler, (void*)code, (void*)expected, name);
}

struct plusminus {
    double range;
    double center;
};

bool within (double got, double range, double expected, const char* name) {
    if (range < 0) range = -range;
    if (got >= expected - range && got <= expected + range) {
        return pass(name);
    }
    else {
        fail(name);
        plusminus pm = {range, expected};
        diag_unexpected(got, pm);
        return false;
    }
}
static bool within_handler (double (* code )(), double* range, double* expected, const char* name) {
    return within(code(), *range, *expected, name);
}
bool within (double (* code )(), double range, double expected, const char* name) {
    using namespace internal;
    return glove_3((void*)within_handler, (void*)code, &range, &expected, name);
}

bool about (double got, double expected, const char* name) {
    return within(got, expected*0.01, expected, name);
}
bool about (double (* code )(), double expected, const char* name) {
    return within(code, expected*0.01, expected, name);
}
static bool doesnt_throw_handler (void (* code )(), const char* name) {
    code(); return pass(name);
}
bool doesnt_throw (void (* code )(), const char* name) {
    using namespace internal;
    return glove_1((void*)doesnt_throw_handler, (void*)code, name);
}


static void pass_fail_message (bool succeeded, unsigned int num) {
    if (inverted ? succeeded : !succeeded) {
        printf("not ");
    }
    printf("ok %d", num);
}

bool invert_success () {
    return inverted = !inverted;
}

bool pass (const char* name) {
    tested += 1;
    pass_fail_message(true, tested);
    if (name && name[0]) {
        printf(" - %s", name);
    };
    if (todoing) {
        todoing--;
        printf(" # TODO");
        if (todo_string && todo_string[0]) {
            printf(" %s", todo_string);
        }
    }
    putchar('\n');
    return true;
}
bool fail (const char* name) {
    tested += 1;
    pass_fail_message(false, tested);
    if (name && name[0]) {
        printf(" - %s", name);
    };
    if (todoing) {
        todoing--;
        printf(" # TODO");
        if (todo_string && todo_string[0]) {
            printf(" %s", todo_string);
        }
    }
    putchar('\n');
    return false;
}


void todo (unsigned num, const char* reason) {
    todoing = num;
    todo_string = reason;
}

void skip (unsigned num, const char* reason) {
    for (unsigned int i = 0; i < num; i++) {
        tested++;
        printf("ok %d # SKIP", tested);
        if (reason && reason[0]) {
            printf(" %s", reason);
        }
        printf("\n");
    }
}

void BAIL_OUT (const char* reason) {
    printf("Bail out!");
    if (reason && reason[0]) {
        printf(" %s\n", reason);
    }
    exit(1);
}

static void diag_dyn (const char* prefix, const type_info& t, void* p);
static void diag_extype (const char* prefix, const type_info& t);
static void diag_ex (const char* prefix, std::exception& e);


namespace internal {
    static bool catch_weird (const char* name) {
        fail(name);
        diag("threw a non-standard exception");
        return false;
    }
    static bool diffex_weird (const type_info& et, const char* name) {
        fail(name);
        diag_extype("expected", et);
        diag(" but got a non-standard exception");
        return false;
    }
    static bool catch_std (std::exception& e, const char* name) {
        fail(name);
        diag_ex("threw", e);
        return false;
    }
    static bool diffex_std (const type_info& et, std::exception& e, const char* name) {
        fail(name);
        diag_extype("expected", et);
        diag_ex(" but got", e);
        return false;
    }
    static bool catch_scary (scary_exception& e, const char* name) {
        catch_std(e, name);
        BAIL_OUT("Caught a scary_exception");
        return false;
    }
    static bool diffex_scary (const type_info& et, scary_exception& e, const char* name) {
        diffex_std(et, e, name);
        BAIL_OUT("Caught a scary_exception");
        return false;
    }
    bool glove_1 (void* code, void* p1, const char* name) {
        try { return reinterpret_cast<bool(*)(void*,const char*)>(code)(p1, name); }
        catch (scary_exception& e) { return catch_scary(e, name); }
        catch (std::exception& e) { return catch_std(e, name); }
        catch (...) { return catch_weird(name); }
    }
    bool glove_2 (void* code, void* p1, void* p2, const char* name) {
        try { return reinterpret_cast<bool(*)(void*,void*,const char*)>(code)(p1, p2, name); }
        catch (scary_exception& e) { return catch_scary(e, name); }
        catch (std::exception& e) { return catch_std(e, name); }
        catch (...) { return catch_weird(name); }
    }
    bool glove_3 (void* code, void* p1, void* p2, void* p3, const char* name) {
        try { return reinterpret_cast<bool(*)(void*,void*,void*,const char*)>(code)(p1, p2, p3, name); }
        catch (scary_exception& e) { return catch_scary(e, name); }
        catch (std::exception& e) { return catch_std(e, name); }
        catch (...) { return catch_weird(name); }
    }
    bool glove_1_wanted (const type_info& et, void* code, void* p1, const char* name) {
        try { return reinterpret_cast<bool(*)(void*,const char*)>(code)(p1, name); }
        catch (scary_exception& e) { return diffex_scary(et, e, name); }
        catch (std::exception& e) { return diffex_std(et, e, name); }
        catch (...) { return diffex_weird(et, name); }
    }
    bool glove_2_wanted (const type_info& et, void* code, void* p1, void* p2, const char* name) {
        try { return reinterpret_cast<bool(*)(void*,void*,const char*)>(code)(p1, p2, name); }
        catch (scary_exception& e) { return diffex_scary(et, e, name); }
        catch (std::exception& e) { return diffex_std(et, e, name); }
        catch (...) { return diffex_weird(et, name); }
    }
}


std::unordered_map<std::string, void (*)(void*)> printers = {
    {typeid(bool).name(), [](void* p){ printf("%lu", (unsigned long)*(bool*)p); } },
    {typeid(char).name(), [](void* p){ printf("'%c'", *(char*)p); } },
    {typeid(signed char).name(), [](void* p){ printf("%hhd", *(signed char*)p); } },
    {typeid(unsigned char).name(), [](void* p){ printf("%hhu", *(unsigned char*)p); } },
    {typeid(signed short).name(), [](void* p){ printf("%hd", *(signed short*)p); } },
    {typeid(unsigned short).name(), [](void* p){ printf("%hu", *(unsigned short*)p); } },
    {typeid(signed).name(), [](void* p){ printf("%d", *(signed*)p); } },
    {typeid(unsigned).name(), [](void* p){ printf("%u", *(unsigned*)p); } },
    {typeid(signed long).name(), [](void* p){ printf("%ld", *(signed long*)p); } },
    {typeid(unsigned long).name(), [](void* p){ printf("%lu", *(unsigned long*)p); } },
    {typeid(signed long long).name(), [](void* p){ printf("%lld", *(signed long long*)p); } },
    {typeid(unsigned long long).name(), [](void* p){ printf("%llu", *(unsigned long long*)p); } },
    {typeid(float).name(), [](void* p){ printf("%g", *(float*)p); } },
    {typeid(double).name(), [](void* p){ printf("%lg", *(double*)p); } },
    {typeid(const char*).name(), [](void* p){ if (*(void**)p) printf("\"%s\"", *(const char**)p); else printf("NULL"); } },
    {typeid(std::string).name(), [](void* p){ printf("\"%s\"", ((std::string*)p)->c_str()); } },
    {typeid(plusminus).name(), [](void* p){ printf("within %g of %g", ((plusminus*)p)->range, ((plusminus*)p)->center); } },
};

bool default_can_print (const type_info& t, void* p) {
    return printers.count(t.name()) || t.name()[0] == 'P';  // This is pretty bad, I know.
}
void default_print (const type_info& t, void* p) {
    auto iter = printers.find(t.name());
    if (iter != printers.end()) {
        iter->second(p);
        return;
    }
    else if (t.name()[0] == 'P') {
        if (*(void**)p) printf("0x%lx", *(long*)p);
        else printf("NULL");
        return;
    }
}

void diag (const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf(" # ");
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

static void diag_dyn (const char* prefix, const type_info& t, void* v) {
    printf(" # %s ", prefix);
    if (default_can_print(t, v)) {
        default_print(t, v);
        printf("\n");
    }
    else {
        printf("<unprintable value of mangled-type %s>\n", t.name());
    }
}
static void diag_extype (const char* prefix, const type_info& t) {
    diag("%s an exception of mangled-type %s", prefix, t.name());
}
static void diag_ex (const char* prefix, std::exception& e) {
    diag_extype(prefix, typeid(e));
    diag("    %s", e.what());
}

namespace internal {
    void diag_unexpected_dyn (const type_info& gt, void* g, const type_info& et, void* e) {
        diag_dyn("expected", et, e);
        diag_dyn(" but got", gt, g);
    }
    bool fail_is (const type_info& gt, void* g, const type_info& et, void* e, const char* name) {
        fail(name);
        diag_unexpected_dyn(gt, g, et, e);
        return false;
    }
    bool fail_didnt_throw (const char* name) {
        fail(name);
        diag("didn't throw an exception");
        return false;
    }
    bool fail_extest (const char* name) {
        fail(name);
        diag("the thrown exception failed the exception test");
        return false;
    }
}





}  // namespace tap

#endif  // DISABLE_TESTS
