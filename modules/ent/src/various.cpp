
#include "../../hacc/inc/everything.h"
#include "../../core/inc/state.h"
#include "../../phys/inc/phys.h"
#include "../../vis/inc/sprites.h"

auto test_bdf = hacc::reference_file<phys::BodyDef>("modules/ent/res/test_actor.bdf");
struct Test_Actor : core::Stateful, phys::Object, vis::Sprite {
    vis::Texture* texture;
    vis::Frame* frame;
    Vec sprite_pos () { return pos(); }
    vis::Texture* sprite_texture () { return texture; }
    vis::Frame* sprite_frame () { return frame; }


    Test_Actor () : Object(test_bdf) { }
    void start () { materialize(); appear(); }
};

HCB_BEGIN(Test_Actor)
    base<core::Stateful>("Test_Actor");
    attr("object", supertype<phys::Object>());
    attr("texture", member(&Test_Actor::texture));
    attr("frame", member(&Test_Actor::frame));
HCB_END(Test_Actor)

auto boundary_bdf = hacc::reference_file<phys::BodyDef>("modules/ent/res/boundary.bdf");
struct Boundary : core::Stateful, phys::Object {
    Boundary () : Object(boundary_bdf) { }
    void start () { materialize(); }
};

HCB_BEGIN(Boundary)
    base<core::Stateful>("Boundary");
    attr("object", supertype<phys::Object>());
HCB_END(Boundary)

