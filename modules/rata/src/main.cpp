
#include <GL/glfw.h>
#include "../../hacc/inc/everything.h"
#include "../../core/inc/game.h"
#include "../../core/inc/input.h"
#include "../../core/inc/state.h"
#include "../../vis/inc/vis.h"
#include "../../phys/inc/phys.h"

using vis::Image;
using vis::SubImg;
using phys::Physical;

struct Test_Actor : core::Stateful, vis::Single_Image, Physical {
    Vec img_pos () { return pos(); }
    void start () { appear(); activate(); }
    Image* img_image () { static Image* r = hacc::require_id<Image>("vis/test.png"); return r; }
    SubImg* img_sub () {
        static SubImg r = hacc::value_from_file<std::unordered_map<std::string, SubImg>>(
            "modules/vis/test.hacc"
        ).at("green");
        return &r;
    }
    static Test_Actor* the;
    phys::BodyDef* bd () {
        static phys::BodyDef bd = hacc::value_from_file<phys::BodyDef>("modules/rata/test_actor_def.hacc");
        return &bd;
    }
    Test_Actor () : Physical(bd()) { the = this; }
    ~Test_Actor () { the = NULL; }
};
Test_Actor* Test_Actor::the;

HCB_BEGIN(Test_Actor)
    base<core::Stateful>("test_actor");
    attr("physical", supertype<Physical>());
HCB_END(Test_Actor)






 // ONE LINE, that's right, ONE LINE!
int main () {
    core::load("modules/rata/test_state.hacc");
}




