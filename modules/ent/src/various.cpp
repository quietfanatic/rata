
#include "../../hacc/inc/everything.h"
#include "../../core/inc/state.h"
#include "../../phys/inc/phys.h"
#include "../../vis/inc/sprites.h"

auto test_bdf = hacc::reference_file<phys::BodyDef>("modules/ent/res/test_actor.bdf");
struct Test_Actor : core::Stateful, phys::Physical, vis::Sprite {
    vis::Texture* texture;
    vis::Frame* frame;
    Vec sprite_pos () { return pos(); }
    vis::Texture* sprite_texture () { return texture; }
    vis::Frame* sprite_frame () { return frame; }


    Test_Actor () : Physical(test_bdf) { }
    void start () { phys::Physical::start(); vis::Sprite::appear(); }
};

HCB_BEGIN(Test_Actor)
    base<core::Stateful>("Test_Actor");
    attr("physical", supertype<phys::Physical>());
    attr("texture", member(&Test_Actor::texture));
    attr("frame", member(&Test_Actor::frame));
HCB_END(Test_Actor)

auto boundary_bdf = hacc::reference_file<phys::BodyDef>("modules/ent/res/boundary.bdf");
struct Boundary : core::Stateful, phys::Physical {
    Boundary () : Physical(boundary_bdf) { }
    void start () { phys::Physical::start(); }
};

HCB_BEGIN(Boundary)
    base<core::Stateful>("Boundary");
    attr("physical", supertype<phys::Physical>());
HCB_END(Boundary)

