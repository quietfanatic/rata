
#include "../inc/ground.h"

using namespace phys;
HCB_BEGIN(Grounded)
    type_name("phys::Grounded");
    attr("ground", member(&Grounded::ground));
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
                if (grd) grd->ground = (Object*)ground->GetBody()->GetUserData();
            }
        }
    } gr;
    Collision_Rule* ground_rule = &gr;

}

