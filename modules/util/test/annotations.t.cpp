#include <string>
#include "../inc/annotations.h"
#include "../../tap/inc/tap.h"

struct IntAnnotation {
    int x;
};
struct StringAnnotation {
    std::string s = "asdf";
};

tap::Tester annotations_tester ("annotations", [](){
    using namespace tap;
    using namespace util;
    plan(6);
    is(get_annotation<int, IntAnnotation>(), (IntAnnotation*)NULL, "get_annotation returns NULL if there is none");
    set_annotation<int>(IntAnnotation{42});
    ok(get_annotation<int, IntAnnotation>() != (IntAnnotation*)NULL, "get_annotation is non-NULL is there is one");
    is(get_annotation<int, IntAnnotation>()->x, 42, "get_annotation gets correct value");
    is(annotation<int, IntAnnotation>().x, 42, "annotation() gets preexisting value");
    annotation<int, IntAnnotation>().x = 144;
    is(annotation<int, IntAnnotation>().x, 144, "annotation() can be used to set the value");
    is(annotation<int, StringAnnotation>().s, std::string("asdf"), "annotation() can call the default constructor");
});
