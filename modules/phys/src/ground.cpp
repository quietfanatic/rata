
#include "../inc/ground.h"
#include "../../hacc/inc/haccable_standard.h"

namespace phys {

    struct Ground_Rule : Collision_Rule {
        std::string name () const { return "ground"; }
        void post (b2Contact* contact, b2Fixture* grounded, b2Fixture* ground) {
            b2WorldManifold wm;
            contact->GetWorldManifold(&wm);
            if (contact->GetFixtureA() == grounded
                ? wm.normal.y < -0.7
                : wm.normal.y > 0.7
            ) {
                Grounded* grd = dynamic_cast<Grounded*>(
                    (Object*)grounded->GetBody()->GetUserData()
                );
                if (grd) {
                    grd->ground = (Object*)ground->GetBody()->GetUserData();
                    grd->ground_fixdef = (FixtureDef*)ground->GetUserData();
                }
            }
        }
        void end (b2Contact* contact, b2Fixture* grounded, b2Fixture* ground) {
            Grounded* grd = dynamic_cast<Grounded*>(
                (Object*)grounded->GetBody()->GetUserData()
            );
            if (grd && grd->ground_fixdef == (FixtureDef*)ground->GetUserData()) {
                grd->ground = NULL;
                grd->ground_fixdef = NULL;
            }
        }
    } ground_rule;

} using namespace phys;

HCB_BEGIN(Grounded)
    name("phys::Grounded");
    attr("ground", member(&Grounded::ground).optional());
    attr("ground_fixdef", member(&Grounded::ground_fixdef).optional());
HCB_END(Grounded)

