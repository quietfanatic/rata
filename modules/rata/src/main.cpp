
#include <GL/glfw.h>
#include "../../hacc/inc/everything.h"
#include "../../core/inc/game.h"
#include "../../core/inc/input.h"
#include "../../core/inc/state.h"
#include "../../vis/inc/vis.h"

using vis::Image;
using vis::SubImg;

struct Test_Actor : core::Stateful, vis::Single_Image {
    Vec pos;
    Vec img_pos () { return pos; }
    void exist () { appear(); }
    Image* img_image () { static Image* r = hacc::require_id<Image>("vis/test.png"); return r; }
    SubImg* img_sub () {
        static SubImg r = hacc::value_from_file<std::unordered_map<std::string, SubImg>>(
            "modules/vis/test.hacc"
        ).at("green");
        return &r;
    }
    static Test_Actor* the;
    Test_Actor () { the = this; }
    ~Test_Actor () { the = NULL; }
};
Test_Actor* Test_Actor::the;

HCB_BEGIN(Test_Actor)
    base<core::Stateful>("test_actor");
    attr("pos", member(&Test_Actor::pos));
HCB_END(Test_Actor)

struct Test_Phase : core::Phase {
    Test_Phase () : core::Phase(core::game_phases(), "T.M") { }
    void run () {
        if (!Test_Actor::the) return;
        if (core::get_key(GLFW_KEY_LEFT))
            Test_Actor::the->pos.x -= 1*PX;
        if (core::get_key(GLFW_KEY_DOWN))
            Test_Actor::the->pos.y -= 1*PX;
        if (core::get_key(GLFW_KEY_RIGHT))
            Test_Actor::the->pos.x += 1*PX;
        if (core::get_key(GLFW_KEY_UP))
            Test_Actor::the->pos.y += 1*PX;
    }
} test_phase;

int main () {
    core::init();
    core::load("modules/rata/test_state.hacc");
    core::play();
}

