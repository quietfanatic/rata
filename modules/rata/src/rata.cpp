
#include "../../hacc/inc/everything.h"
#include "../../core/inc/state.h"
#include "../../core/inc/input.h"
#include "../../phys/inc/aux.h"
#include "../../vis/inc/sprites.h"
#include "../../vis/inc/models.h"

using namespace phys;
using namespace vis;
using namespace core;

 // For now we're skipping having a generic Human class
static phys::BodyDef*& bdf () {
     static auto bdf = hacc::reference_file<phys::BodyDef>("modules/rata/res/rata.bdf");
     return bdf;
}
static vis::Skel*& skel () {
    static auto skel = hacc::reference_file<vis::Skel>("modules/rata/res/rata.skel");
    return skel;
}

struct Rata : Stateful, Ambulator, Draws_Sprites {
     // For drawing
    Model model;
    void draw () {
        static auto stand = skel()->poses.named("stand");
        static auto base = hacc::reference_file<vis::Skin>("modules/rata/res/rata-base.skin");
        model.apply_pose(stand);
        model.apply_skin(base);
        model.draw(pos());
    }

    void before_move () {
        ambulate_force(6);
        if (core::get_key('A')) {
            ambulate_x(-4);
        }
        else if (core::get_key('D')) {
            ambulate_x(4);
        }
        else {
            ambulate_x(0);
        }
    }
    void after_move () {
    }

    Rata () : Ambulator(bdf()), model(skel()) { }
    void start () { materialize(); appear(); }
};

HCB_BEGIN(Rata)
    type_name("Rata");
    base<core::Stateful>("Rata");
    attr("object", supertype<Object>());
HCB_END(Rata)


