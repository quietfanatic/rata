
#include "../inc/tap.h"
#include <string>
#include <stdexcept>

bool truth () { return true; }
bool lie () { return false; }
void live () { }
void die () { throw std::logic_error("ACK!"); }
bool deadly_lie () { throw std::logic_error("false"); return true; }
int get_32 () { return 32; }
int deadly_int () { throw std::logic_error("X"); return 32; }
const char* get_asdf () { return "asdf"; }
double get_1_4 () { return 1.4; }
void throw_3 () { throw (int)3; }
bool is_3 (int x) { return x == 3; }
bool is_5 (int x) { return x == 5; }

void tap_test_f () {
    using namespace tap;
    plan(48);

    pass("pass passes");
    ok(true, "ok on true passes");
    ok(truth, "ok can take function");
    is((int)32, (int)32, "is on equal ints passes");
    is(get_32, (int)32, "is can take functions");
    is((float)32, (float)32, "is on equal floats passes");
    is((double)32, (double)32, "is on equal floats passes");
    is_strcmp("asdf", "asdf", "is_strcmp on equal strings passes");
    is_strcmp(get_asdf, "asdf", "is_strcmp can take functions");
    is_strcmp((const char*)NULL, (const char*)NULL, "is_strcmp on NULLS passes");
    is("asdf", "asdf", "is on equal strings passes");
    is((const char*)NULL, (const char*)NULL, "is on const char* NULLS passes");
    is((int*)NULL, (int*)NULL, "is on int* NULLS passes");
     int heyguys = 9;
    is(&heyguys, &heyguys, "is can compare pointers");
    is(std::string("asdf"), std::string("asdf"), "is on equal std::strings passes");
    is(std::string("asdf"), "asdf", "is on equal std::string and const char* passes");
    within(1.0, 0.1, 1.001, "within can pass");
    within(get_1_4, 0.1, 1.399, "within can take functions");
    about(1.0, 1.001, "about can pass");
    about(get_1_4, 1.4004, "about can take functions");
    about(-25, -25.003, "about can take negative numbers");
    doesnt_throw(live, "doesnt_throw can pass");
    throws<int>(throw_3, "throws<int> can pass");
    throws<int>(throw_3, "throws<int> can pass");
    throws(throw_3, is_3, "throws can test the exception");
     skip(6, "This should skip 6 tests");
     todo(17, "Testing failure (and todo)");
    fail("fail fails");
    ok(false, "ok on false fails");
    ok(lie, "ok can fail on functions");
    is((int)5, (int)3245, "is can fail");
    is_strcmp("asdf", "fdsa", "is_strcmp can fail");
    is_strcmp("sadf", NULL, "is_strcmp fails on single NULL");
    is_strcmp((const char*)NULL, "sadf", "is_strcmp fails on single NULL");
     int nope = -9999;
    is(&heyguys, &nope, "is fails on different pointers");
    is(std::string("sadf"), std::string("qwert"), "is fails on different std::strings");
    within(1.0, 0.1, 1.11, "within can fail");
    within(get_1_4, 0.3, 1, "withing can fail with functions");
    about(1.0, 1.1, "about can fail");
    doesnt_throw(die, "doesnt_throw catches and fails on exception");
    throws<int>(die, "throws fails on wrong kind of exception");
    throws(throw_3, is_5, "throws can fail the exception test");
    ok(deadly_lie, "ok catches and fails on exception");
    is(deadly_int, 32, "is catches and fails on exception");
}
tap::Tester tap_test ("tap", tap_test_f);

