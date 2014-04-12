#include "geo/inc/ground.h"

#include <string>
#include "hacc/inc/haccable_standard.h"

namespace geo {

    struct Ground_Rule : Collision_Rule {
        std::string name () const { return "ground"; }
        void Collision_Rule_presolve (b2Contact* contact, b2Fixture* grounded, b2Fixture* ground) override {
            b2WorldManifold wm;
            contact->GetWorldManifold(&wm);
            if (contact->GetFixtureA() == grounded
                ? wm.normal.y < -0.7  // TODO: make this value configurable
                : wm.normal.y > 0.7
            ) {
                Grounded* grdd = dynamic_cast<Grounded*>(
                    (Object*)grounded->GetBody()->GetUserData()
                );
                if (grdd) {
                    bool had_ground = grdd->ground;
                    grdd->ground = (Object*)ground->GetBody()->GetUserData();
                    grdd->ground_fixdef = (FixtureDef*)ground->GetUserData();
                     // Allow walking via friction
                    float vel = grdd->Grounded_velocity();
                    float fric = grdd->Grounded_friction();
                    contact->SetTangentSpeed(vel);
                    if (had_ground && fric == fric && fric >= 0)
                        contact->SetFriction(b2MixFriction(fric, ground->GetFriction()));
                }
            }
        }
        void Collision_Rule_end (b2Contact* contact, b2Fixture* grounded, b2Fixture* ground) override {
            Grounded* grdd = dynamic_cast<Grounded*>(
                (Object*)grounded->GetBody()->GetUserData()
            );
            if (grdd && grdd->ground_fixdef == (FixtureDef*)ground->GetUserData()) {
                grdd->ground = NULL;
                grdd->ground_fixdef = NULL;
            }
        }
    } ground_rule;

} using namespace geo;

HACCABLE(Grounded) {
    name("geo::Grounded");
    attr("ground", member(&Grounded::ground).optional());
    attr("ground_fixdef", member(&Grounded::ground_fixdef).optional());
}

