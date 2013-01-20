
#include "../../hacc/inc/everything.h"
#include "../../core/inc/state.h"
#include "../../phys/inc/phys.h"
#include "../../vis/inc/sprites.h"

const char* test_bdf = "modules/ent/test_actor_def.hacc";
struct Test_Actor : core::Stateful, phys::Physical_With<&test_bdf>, vis::Sprite {
    vis::Texture* texture;
    vis::Frame* frame;
    Vec sprite_pos () { return pos(); }
    vis::Texture* sprite_texture () { return texture; }
    vis::Frame* sprite_frame () { return frame; }
    void start () { phys::Physical::start(); vis::Sprite::appear(); }
};

HCB_BEGIN(Test_Actor)
    base<core::Stateful>("Test_Actor");
    attr("physical", supertype<phys::Physical>());
    attr("texture", member(&Test_Actor::texture));
    attr("frame", member(&Test_Actor::frame));
HCB_END(Test_Actor)

const char* boundary_bdf = "modules/ent/boundary.hacc";
struct Boundary : core::Stateful, phys::Physical_With<&boundary_bdf> {
    void start () { phys::Physical::start(); }
};

HCB_BEGIN(Boundary)
    base<core::Stateful>("Boundary");
    attr("physical", supertype<phys::Physical>());
HCB_END(Boundary)

