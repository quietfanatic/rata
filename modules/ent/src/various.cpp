
#include "../../hacc/inc/everything.h"
#include "../../core/inc/state.h"
#include "../../phys/inc/phys.h"
#include "../../vis/inc/sprites.h"

static phys::BodyDef*& test_bdf () {
    static auto test_bdf = hacc::reference_file<phys::BodyDef>("modules/ent/res/test_actor.bdf");
    return test_bdf;
}
struct Test_Actor : core::Stateful, phys::Object, vis::Draws_Sprites {
    core::Texture* texture;
    vis::Frame* frame;

    void draws_sprites () {
        draw_sprite(frame, texture, pos());
    }

    Test_Actor () : Object(test_bdf()) { }
    void start () { materialize(); vis::Draws_Sprites::activate(); }
};

HCB_BEGIN(Test_Actor)
    name("ent::Test_Actor");
    attr("Object", base<phys::Object>());
    attr("texture", member(&Test_Actor::texture));
    attr("frame", member(&Test_Actor::frame));
HCB_END(Test_Actor)

static phys::BodyDef*& boundary_bdf () {
    static auto boundary_bdf = hacc::reference_file<phys::BodyDef>("modules/ent/res/boundary.bdf");
    return boundary_bdf;
}
struct Boundary : core::Stateful, phys::Object {
    Boundary () : Object(boundary_bdf()) { }
    void start () { materialize(); }
};

HCB_BEGIN(Boundary)
    name("ent::Boundary");
    attr("Object", base<phys::Object>());
HCB_END(Boundary)

