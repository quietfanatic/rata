#include "../inc/hacc.h"

#include "../../tap/inc/tap.h"
tap::Tester hacc_tester ("hacc", [](){
    using namespace hacc;
    using namespace tap;
    plan(12);
    start();
    is(Hacc().form, NULLFORM, "nullary constructor makes null hacc");
    is(Hacc(true).b, true, "bool roundtrip");
    is(Hacc(false).b, false, "false roundtrip");
    is(Hacc(45).i, 45, "integer roundtrip");
    is(Hacc(-45).i, -45, "negative integer roundtrip");
    is(Hacc(32.f).f, 32.f, "float roundtrip");
    is(Hacc(32.0).d, 32.0, "double roundtrip");
    is(Hacc(String("hello")).s, std::string("hello"), "string roundtrip");
    is(Hacc(Var{"id"}).v, Var{"id"}, "reference roundtrip");
    is(*Hacc(Array()).a, Array(), "Empty array roundtrip");
    const Array& a = Array{new Hacc (3), new Hacc (4.f)};
    is(*Hacc(a).a, a, "Full array roundtrip");
    doesnt_throw([](){finish();});
//    is(Hacc({new Hacc (4)}).elem(0)->form(), INTEGER, "Hacc::Array::elem appears to work");
//    is(Hacc::Array{new_hacc(4)}.n_elems(), (size_t)1, "Hacc::Array::n_elems appears to work");
//    is(Hacc::Object{{String("asdf"), new_hacc(4)}}.n_attrs(), (size_t)1, "Hacc::Object::n_attrs appears to work");
//    is(Hacc::Object{{String("asdf"), new_hacc(4)}}.name_at(0), String("asdf"), "Hacc::Object::name_at appears to work");
//    is(Hacc::Object{{String("asdf"), new_hacc(4)}}.value_at(0)->form(), INTEGER, "Hacc::Object::value_at appears to work");
//    is(Hacc::Object{{String("asdf"), new_hacc(4)}}.attr("asdf")->form(), INTEGER, "Hacc::Object::attr appears to work");
});
