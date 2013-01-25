#include "GL/glfw.h"
#include "../../hacc/inc/everything.h"
#include "../../core/inc/state.h"
#include "../../core/inc/input.h"
#include "../../phys/inc/aux.h"
#include "../../phys/inc/ground.h"
#include "../../vis/inc/sprites.h"
#include "../../vis/inc/models.h"
#include "../../geo/inc/rooms.h"

using namespace phys;
using namespace vis;
using namespace core;
using namespace geo;

 // For now we're skipping having a generic Human class
static BodyDef*& bdf () {
     static auto bdf = hacc::reference_file<BodyDef>("modules/rata/res/rata.bdf");
     return bdf;
}
static Skel*& skel () {
    static auto skel = hacc::reference_file<Skel>("modules/rata/res/rata.skel");
    return skel;
}

struct Rata : Stateful, Object, Grounded, Resident, Draws_Sprites {

    Ambulator legs;
    Model model;
    uint8 direction = 1;

    void draw () {
        static auto stand = skel()->poses.named("stand");
        static auto walk1 = skel()->poses.named("walk1");
        static auto base = hacc::reference_file<vis::Skin>("modules/rata/res/rata-base.skin");
        if (ground)
            model.apply_pose(stand);
        else
            model.apply_pose(walk1);
        model.apply_skin(base);
        model.draw(pos(), direction < 0);
    }

    void before_move () {
         // Control logic takes so much space always
        if (ground) {
            if (get_key(GLFW_KEY_SPACE)) {
                legs.disable();
                 // I'd like to be able to do a mutual push between the ground and
                 //  the character, but Box2D isn't well-equipped for that.
                impulse(Vec(0, 12));
                ground = NULL;
            }
            else {
                legs.enable();
                if (get_key('A') && !get_key('D')) {
                    legs.ambulate_x(this, -4);
                    if (vel().x <= 0) direction = -1;
                    if (vel().x < -4)
                        legs.ambulate_force(8);
                    else legs.ambulate_force(12);
                }
                else if (get_key('D')) {
                    legs.ambulate_x(this, 4);
                    if (vel().x >= 0) direction = 1;
                    if (vel().x > 4)
                        legs.ambulate_force(8);
                    else legs.ambulate_force(12);
                }
                else {
                    legs.ambulate_force(8);
                    legs.ambulate_x(this, 0);
                }
            }
        }
        else {
            legs.disable();
             // If you were in a 2D platformer, you'd be able to push against the air too.
            if (get_key('A') && !get_key('D')) {
                force(Vec(-2, 0));
            }
            else if (get_key('D')) {
                force(Vec(2, 0));
            }
        }
    }
    void after_move () {
        reroom(pos());
    }

    Rata () : Object(bdf()), legs(this), model(skel()) { }
    void emerge () { printf("Emerging\n"); materialize(); legs.enable(); appear(); }
    void reclude () { }
    void start () { beholder = this; }
};

HCB_BEGIN(Rata)
    type_name("Rata");
    base<Stateful>("Rata");
    attr("object", supertype<Object>());
    attr("resident", supertype<Resident>());
    attr("direction", member(&Rata::direction));
    attr("grounded", supertype<Grounded>());
HCB_END(Rata)


