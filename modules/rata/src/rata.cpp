#include "GL/glfw.h"
#include "../../hacc/inc/everything.h"
#include "../../core/inc/state.h"
#include "../../core/inc/input.h"
#include "../../phys/inc/aux.h"
#include "../../phys/inc/ground.h"
#include "../../vis/inc/sprites.h"
#include "../../vis/inc/models.h"
#include "../../geo/inc/rooms.h"

 // Here are some parameters.
 // TODO: make these changable with stats and equipment.
const float WALK_FRICTION = 8;
const float STOP_FRICTION = 8;
const float SKID_FRICTION = 12;
const float AIR_FORCE = 3;
const float AIR_SPEED = 3;
const float WALK_SPEED = 4;
const float JUMP_IMPULSE = 6;

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

    Feet feet;
    Model model;
    int8 direction = 1;

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
                feet.disable();
                 // I'd like to be able to do a mutual push between the ground and
                 //  the character, but Box2D isn't well-equipped for that.
                impulse(Vec(0, JUMP_IMPULSE));
                ground = NULL;
            }
            else {
                feet.enable();
                if (get_key('A') && !get_key('D')) {
                    feet.ambulate_x(this, -WALK_SPEED);
                    if (vel().x <= 0) direction = -1;
                    if (vel().x < -WALK_SPEED)
                        feet.ambulate_force(WALK_FRICTION);
                    else feet.ambulate_force(SKID_FRICTION);
                }
                else if (get_key('D')) {
                    feet.ambulate_x(this, WALK_SPEED);
                    if (vel().x >= 0) direction = 1;
                    if (vel().x > WALK_SPEED)
                        feet.ambulate_force(WALK_FRICTION);
                    else feet.ambulate_force(SKID_FRICTION);
                }
                else {
                    feet.ambulate_force(STOP_FRICTION);
                    feet.ambulate_x(this, 0);
                }
            }
        }
        else {
            feet.disable();
             // If you were in a 2D platformer, you'd be able to push against the air too.
            if (get_key('A') && !get_key('D')) {
                force(Vec(-AIR_FORCE, 0));
            }
            else if (get_key('D')) {
                force(Vec(AIR_FORCE, 0));
            }
        }
    }
    void after_move () {
        reroom(pos());
    }

    Rata () : Object(bdf()), feet(this), model(skel()) { }
    void emerge () { printf("Emerging\n"); materialize(); feet.enable(); appear(); }
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


