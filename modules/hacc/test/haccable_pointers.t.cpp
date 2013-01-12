#include "../inc/haccable_pointers.h"



struct SuperThing {
    virtual float number () = 0;
    virtual ~SuperThing () { }
};

struct SubThing1 : SuperThing {
    int subint;
    float number () { return subint; }
};

HCB_BEGIN(SubThing1)
    base<SuperThing>("sub1");
    delegate(member(&SubThing1::subint));
HCB_END(SubThing1)

struct HasAFloat {
    float myfloat;
    HasAFloat (float f = 0/0.0) : myfloat(f) { }
    virtual ~HasAFloat () { }
};

HCB_BEGIN(HasAFloat)
    delegate(member(&HasAFloat::myfloat));
HCB_END(HasAFloat)

 // Inherits from SuperThing second to prove that subtypes and supertypes
 //  don't have to have the same address.
struct SubThing2 : HasAFloat, SuperThing {
    SubThing2 (float f = 0/0.0) : HasAFloat(f) { }
    float number () { return myfloat; }
};

HCB_BEGIN(SubThing2)
    base<SuperThing>("sub2");
    delegate(supertype<HasAFloat>());
HCB_END(SubThing2)

hacc::canonical_ptr<SuperThing> the_p;

#include "../../tap/inc/tap.h"
tap::Tester haccable_pointers_tester ("haccable_pointers", [](){
    using namespace hacc;
    using namespace tap;
    plan(8);
    doesnt_throw([](){ update_from_hacc(the_p, new_hacc({new_attr("sub1", 54)})); }, "Can update_from_hacc on polymorphic canonical_ptr...");
    is(the_p->number(), 54.f, "...which works correctly");
    is(to_hacc(the_p)->form(), OBJECT, "polymorphic canonical_pointer produces an object hacc");
    is(to_hacc(the_p)->as_object()->attr("sub1")->get_integer(), 54, "to_hacc on polymorphic canonical_pointer works.");
    doesnt_throw([](){ update_from_hacc(the_p, new_hacc({new_attr("sub2", 20.f)})); }, "Can update_from_hacc on polymorphic canonical_ptr...");
    is(the_p->number(), 20.f, "...which works correctly");
    is(to_hacc(the_p)->form(), OBJECT, "polymorphic canonical_pointer produces an object hacc");
    is(to_hacc(the_p)->as_object()->attr("sub2")->get_float(), 20.f, "to_hacc on polymorphic canonical_pointer works.");
    
});
