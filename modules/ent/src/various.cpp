
#include "../../hacc/inc/everything.h"
#include "../../phys/inc/phys.h"
#include "../../vis/inc/common.h"
#include "../../vis/inc/images.h"

static phys::BodyDef*& test_bdf () {
    static phys::BodyDef* test_bdf = hacc::File("ent/res/test_actor.bdf").data().attr("bdf");
    hacc::manage(&test_bdf);
    return test_bdf;
}
struct Test_Actor : phys::Object, vis::Drawn<vis::Sprites> {
    vis::Texture* texture;
    vis::Frame* frame;

    void Drawn_draw (vis::Sprites) {
        vis::draw_frame(frame, texture, pos());
    }

    Test_Actor () : Object() { test_bdf(); }
    void finish () {
        set_def(test_bdf());
        materialize();
        appear();
    }
};

HACCABLE(Test_Actor) {
    name("ent::Test_Actor");
    attr("Object", base<phys::Object>());
    attr("texture", member(&Test_Actor::texture));
    attr("frame", member(&Test_Actor::frame));
    finish([](Test_Actor& ta){ ta.finish(); });
}

static phys::BodyDef*& boundary_bdf () {
    static phys::BodyDef* boundary_bdf = hacc::File("ent/res/boundary.bdf").data().attr("bdf");
    hacc::manage(&boundary_bdf);
    return boundary_bdf;
}
struct Boundary : phys::Object {
    Boundary () : Object() { boundary_bdf(); }
    void finish () {
        set_def(boundary_bdf());
        materialize();
    }
};

HACCABLE(Boundary) {
    name("ent::Boundary");
    attr("Object", base<phys::Object>());
    finish([](Boundary& b){ b.finish(); });
}

