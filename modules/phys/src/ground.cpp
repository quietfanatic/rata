
#include "../inc/ground.h"

using namespace phys;
HCB_BEGIN(Grounded)
    type_name("phys::Grounded");
    attr("ground", member(&Grounded::ground));
    attr("ground_fix_index", member(&Grounded::ground_fix_index, def((uint)0)));
HCB_END(Grounded)

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
                    grd->ground_fix_index = grd->ground->fix_index(ground);
                }
            }
        }
        void end (b2Contact* contact, b2Fixture* grounded, b2Fixture* ground) {
            Grounded* grd = dynamic_cast<Grounded*>(
                (Object*)grounded->GetBody()->GetUserData()
            );
            if (grd && grd->get_ground_fix() == ground) {
                grd->ground = NULL;
                grd->ground_fix_index = 0;
            }
        }
    } gr;
    Collision_Rule* ground_rule = &gr;

}

