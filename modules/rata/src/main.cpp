
#include "../../hacc/inc/everything.h"
#include "../../core/inc/game.h"
#include "../../core/inc/state.h"
#include "../../vis/inc/vis.h"

using vis::Image;
using vis::SubImg;

HCB_INSTANCE(std::unordered_map<std::string HCB_COMMA vis::SubImg>);

struct Test_Actor : core::Stateful, vis::Single_Image {
    void exist () { appear(); }
    Image* img_image () { static Image* r = hacc::require_id<Image>("vis/test.png"); return r; }
    SubImg* img_sub () {
        static SubImg r = hacc::value_from_file<std::unordered_map<std::string, SubImg>>(
            "modules/vis/test.hacc"
        ).at("green");
        return &r;
    }
    Vec img_pos () { return Vec(10, 7.5); }
};

HCB_BEGIN(Test_Actor)
    base<core::Stateful>("test_actor");
    empty();
HCB_END(Test_Actor)

int main () {
    core::init();
    core::load("modules/rata/test_state.hacc");
    core::play();
}

