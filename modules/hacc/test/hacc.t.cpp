#include "../inc/hacc.h"

#include "../../tap/inc/tap.h"
tap::Tester hacc_tester ("hacc", [](){
    using namespace hacc;
    using namespace tap;
    plan(18);
    is(Hacc::Null().n, null, "null roundtrip");
    is(Hacc::Bool(true).b, true, "true roundtrip");
    is(Hacc::Bool(false).b, false, "false roundtrip");
    is(Hacc::Integer(45).i, 45, "integer roundtrip");
    is(Hacc::Integer(-45).i, -45, "negative integer roundtrip");
    is(Hacc::Float(32.f).f, 32.f, "float roundtrip");
    is(Hacc::Double(32.F).d, 32.F, "double roundtrip");
    is(Hacc::String(String("hello")).s, std::string("hello"), "string roundtrip");
    is(Hacc::Ref(Ref("id")).r, Ref("id"), "reference roundtrip");
    is(Hacc::Array(Array()).a, Array(), "Empty array roundtrip");
    is(Hacc::Array{}.a, Array(), "Empty array roundtrip w/ initializer_list");
    initializer_list<Hacc*> testlist = {new_hacc(3), new_hacc(4.f)};
    is(Hacc::Array(testlist).a, Array(testlist), "Full array roundtrip w/ initializer_list");
    is(Hacc::Array{new_hacc(4)}.elem(0)->form(), INTEGER, "Hacc::Array::elem appears to work");
    is(Hacc::Array{new_hacc(4)}.n_elems(), (size_t)1, "Hacc::Array::n_elems appears to work");
    is(Hacc::Object{{String("asdf"), new_hacc(4)}}.n_attrs(), (size_t)1, "Hacc::Object::n_attrs appears to work");
    is(Hacc::Object{{String("asdf"), new_hacc(4)}}.name_at(0), String("asdf"), "Hacc::Object::name_at appears to work");
    is(Hacc::Object{{String("asdf"), new_hacc(4)}}.value_at(0)->form(), INTEGER, "Hacc::Object::value_at appears to work");
    is(Hacc::Object{{String("asdf"), new_hacc(4)}}.attr("asdf")->form(), INTEGER, "Hacc::Object::attr appears to work");
});



