#include "../inc/haccable_standard.h"



struct SuperThing {
    virtual float number () = 0;
    virtual ~SuperThing () { }
};

HCB_BEGIN(SuperThing)
    pointee_policy(FOLLOW)
HCB_END(SuperThing)

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

struct SubThing3 : SuperThing {
    int val;
    float number () { return val; }
};

HCB_BEGIN(SubThing3)
    base<SuperThing>("sub3");
    elem(member(&SubThing3::val));
HCB_END(SubThing3)

SuperThing the_p;

struct Reffable {
    int i;
};

Reffable r1 {14};
Reffable r2 {28};

HCB_BEGIN(Reffable)
    find_by_id([](hacc::String s){
        if (s == "r1") return &r1;
        if (s == "r2") return &r2;
        return (Reffable*)NULL;
    });
    get_id([](const Reffable& r) -> hacc::String {
        if (&r == &r1) return "r1";
        if (&r == &r2) return "r2";
        return "";
    });
HCB_END(Reffable)

Reffable* rp2;

struct Self_Ref {
    int x;
    Self_Ref* self;
};

HCB_BEGIN(Self_Ref)
    attr("x", member(&Self_Ref::x));
    attr("self", member(&Self_Ref::self));
HCB_END(Self_Ref)

Self_Ref myself { 3, &myself };
hacc::Hacc* haccself = NULL;
Self_Ref yourself { 4, NULL };

#include "../../tap/inc/tap.h"
tap::Tester haccable_pointers_tester ("haccable_pointers", [](){
    using namespace hacc;
    using namespace tap;
    plan(18);
    doesnt_throw([](){ update_from_hacc(the_p, new_hacc({new_attr("sub1", 54)})); }, "Can update_from_hacc on polymorphic canonical_ptr...");
    is(the_p->number(), 54.f, "...which works correctly");
    is(to_hacc(the_p)->form(), OBJECT, "polymorphic canonical_pointer produces an object hacc");
    is(to_hacc(the_p)->as_object()->attr("sub1")->get_integer(), 54, "to_hacc on polymorphic canonical_pointer works.");
    doesnt_throw([](){ update_from_hacc(the_p, new_hacc({new_attr("sub2", 20.f)})); }, "Can update_from_hacc on polymorphic canonical_ptr...");
    is(the_p->number(), 20.f, "...which works correctly");
    is(to_hacc(the_p)->form(), OBJECT, "polymorphic canonical_pointer produces an object hacc");
    is(to_hacc(the_p)->as_object()->attr("sub2")->get_float(), 20.f, "to_hacc on polymorphic canonical_pointer works.");
    doesnt_throw([](){ update_from_hacc(the_p, new_hacc({new_hacc(Ref("sub3")), new_hacc(32)})); }, "polymorphic canonical_ptr can accept array-type union notation...");
    is(the_p->number(), 32.f, "...which works correctly");
    Reffable* rp = &r1;
    is(to_hacc(rp)->form(), REF, "pointers become REFs");
    is(to_hacc(rp)->as_ref()->r.id, String("r1"), "to_hacc on pointer uses get_id");
    rp2 = NULL;
    doesnt_throw([](){ update_from_hacc(rp2, new_hacc(Ref("r2"))); }, "Can update_from_hacc a pointer");
    ok(rp2, "...said pointer was in fact updated");
    is(rp2->i, (int)28, "...and was updated correctly!");
    doesnt_throw([](){ haccself = to_hacc(myself); }, "Can to_hacc a self-pointing thing");
    doesnt_throw([](){ update_from_hacc(yourself, haccself); }, "Can update_from_hacc a self-pointing thing");
    is(yourself.self, &yourself, "...it works!");
    
});
