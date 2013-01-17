
#include "../../hacc/inc/everything.h"
#include "../../core/inc/state.h"
#include "../../phys/inc/phys.h"
#include "../../vis/inc/vis.h"

const char* test_bdf = "modules/ent/test_actor_def.hacc";
struct Test_Actor : core::Stateful, phys::Physical_With<&test_bdf>, vis::Single_Image {
    vis::Image* image;
    vis::SubImg sub;
    Vec img_pos () { return pos(); }
    vis::Image* img_image () { return image; }
    vis::SubImg* img_sub () { return &sub; }
    void start () { phys::Physical::start(); vis::Single_Image::appear(); }
};

HCB_BEGIN(Test_Actor)
    base<core::Stateful>("Test_Actor");
    attr("physical", supertype<phys::Physical>());
    attr("image", member(&Test_Actor::image));
    attr("sub", member(&Test_Actor::sub));
HCB_END(Test_Actor)

const char* boundary_bdf = "modules/ent/boundary.hacc";
struct Boundary : core::Stateful, phys::Physical_With<&boundary_bdf> {
    void start () { phys::Physical::start(); }
};

HCB_BEGIN(Boundary)
    base<core::Stateful>("Boundary");
    attr("physical", supertype<phys::Physical>());
HCB_END(Boundary)

