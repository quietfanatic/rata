
#include "../../hacc/inc/everything.h"
#include "../../phys/inc/phys.h"
#include "../../vis/inc/sprites.h"

static phys::BodyDef*& test_bdf () {
    static phys::BodyDef* test_bdf = hacc::File("modules/ent/res/test_actor.bdf").data().attr("bdf");
    return test_bdf;
}
struct Test_Actor : phys::Object, vis::Sprite {
    vis::Texture* texture;
    vis::Frame* frame;

    void Sprite_draw () {
        draw_sprite(frame, texture, pos());
    }

    Test_Actor () : Object() { test_bdf(); }
    void finish () {
        apply_bdf(test_bdf());
        materialize();
        appear();
    }
};

HCB_BEGIN(Test_Actor)
    name("ent::Test_Actor");
    attr("Object", base<phys::Object>());
    attr("texture", member(&Test_Actor::texture));
    attr("frame", member(&Test_Actor::frame));
    finish([](Test_Actor& ta){ ta.finish(); });
HCB_END(Test_Actor)

static phys::BodyDef*& boundary_bdf () {
    static phys::BodyDef* boundary_bdf = hacc::File("modules/ent/res/boundary.bdf").data().attr("bdf");
    return boundary_bdf;
}
struct Boundary : phys::Object {
    Boundary () : Object() { boundary_bdf(); }
    void finish () {
        apply_bdf(boundary_bdf());
        materialize();
    }
};

HCB_BEGIN(Boundary)
    name("ent::Boundary");
    attr("Object", base<phys::Object>());
    finish([](Boundary& b){ b.finish(); });
HCB_END(Boundary)

