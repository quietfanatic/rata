
#include "../../hacc/inc/everything.h"
#include "../../core/inc/state.h"
#include "../../core/inc/input.h"
#include "../../phys/inc/aux.h"
#include "../../phys/inc/ground.h"
#include "../../vis/inc/sprites.h"
#include "../../vis/inc/models.h"

using namespace phys;
using namespace vis;
using namespace core;

 // For now we're skipping having a generic Human class
static BodyDef*& bdf () {
     static auto bdf = hacc::reference_file<BodyDef>("modules/rata/res/rata.bdf");
     return bdf;
}
static Skel*& skel () {
    static auto skel = hacc::reference_file<Skel>("modules/rata/res/rata.skel");
    return skel;
}

struct Rata : Stateful, Object, Grounded, Draws_Sprites {

    Ambulator legs;
    Model model;

    void draw () {
        static auto stand = skel()->poses.named("stand");
        static auto walk1 = skel()->poses.named("walk1");
        static auto base = hacc::reference_file<vis::Skin>("modules/rata/res/rata-base.skin");
        if (ground)
            model.apply_pose(stand);
        else
            model.apply_pose(walk1);
        model.apply_skin(base);
        model.draw(pos());
    }

    void before_move () {
        legs.ambulate_force(8);
        if (get_key('A')) {
            legs.ambulate_x(this, -4);
        }
        else if (get_key('D')) {
            legs.ambulate_x(this, 4);
        }
        else {
            legs.ambulate_x(this, 0);
        }
    }
    void after_move () {
    }

    Rata () : Object(bdf()), legs(this), model(skel()) { }
    void start () { materialize(); legs.enable(); appear(); }
};

HCB_BEGIN(Rata)
    type_name("Rata");
    base<Stateful>("Rata");
    attr("object", supertype<Object>());
HCB_END(Rata)


