
#include "../../hacc/inc/everything.h"
#include "../../core/inc/state.h"
#include "../../phys/inc/phys.h"
#include "../../vis/inc/vis.h"

static phys::BodyDef test_actor_bd = hacc::value_from_file<phys::BodyDef>("modules/ent/test_actor_def.hacc");
struct Test_Actor : core::Stateful, phys::Physical, vis::Single_Image {
    vis::Image* image;
    vis::SubImg sub;
    Vec img_pos () { return pos(); }
    vis::Image* img_image () { return image; }
    vis::SubImg* img_sub () { return &sub; }
    Test_Actor () :
        Physical(&test_actor_bd)
    { }
    void start () { phys::Physical::start(); vis::Single_Image::appear(); }
};

HCB_BEGIN(Test_Actor)
    base<core::Stateful>("Test_Actor");
    attr("physical", supertype<phys::Physical>());
    attr("image", member(&Test_Actor::image));
    attr("sub", member(&Test_Actor::sub));
HCB_END(Test_Actor)

