
#include "../inc/phys.h"

 // This is so satisfying

HCB_BEGIN(b2Vec2)
    elem(member(&b2Vec2::x))
    elem(member(&b2Vec2::y))
HCB_END(b2Vec2)

HCB_BEGIN(b2CircleShape)
    base<b2Shape>("circle");
    attr("c", member(&b2CircleShape::m_p));
    attr("r", member(&b2CircleShape::m_radius));
HCB_END(b2CircleShape)

HCB_BEGIN(b2PolygonShape)
    base<b2Shape>("polygon");
    attr("radius", member(&b2PolygonShape::m_radius), b2_polygonRadius);
    attr("verts", value_functions(
        [](const b2PolygonShape& ps){
            return std::vector<b2Vec2>(ps.m_vertices, ps.m_vertexCount);
        },
        [](b2PolygonShap& ps, std::vector<b2Vec2> v){
            ps.Set(v.data(), v.size());
        },
    ))
HCB_END(b2PolygonShape)

HCB_BEGIN(b2EdgeShape)
    base<b2Shape>("edge");
    elem(member(&b2EdgeShape::v1));
    elem(member(&b2EdgeShape::v2));
HCB_END(b2EdgeShape)

HCB_BEGIN(b2FixtureDef)
    attr("shape", member(&b2FixtureDef::shape));
    attr("friction", member(&b2FixtureDef::friction), 0.f);
    attr("restitution", member(&b2FixtureDef::restitution), 0.f);
    attr("density", member(&b2FixtureDef::density), 0.f);
    attr("sensor", member(&b2FixtureDef::isSensor), false);
//    attr("filter", member(&b2FixtureDef::filter))
HCB_END(b2FixtureDef)

HCB_BEGIN(phys::FixtureDef)
    using namespace phys;
    attr("b2", member(&FixtureDef::b2));
HCB_END(phys::FixtureDef)

HCB_BEGIN(b2BodyDef)
    attr("type", member(&b2BodyDef::type), b2_dynamicBody);
//    attr("pos", member(&b2BodyDef::position));  This shouldn't be set through this.
//    attr("vel", member(&b2BodyDef::linearVelocity));  Nor this
    attr("damping", member(&b2BodyDef::linearDamping), 0.f);
    attr("gravity", member(&b2BodyDef::gravityScale), 0.f);
    attr("unrotatable", member(&b2BodyDef::fixed_rotation), true);
//    attr("ang", member(&b2BodyDef::angle), 0.f);
//    attr("ang_vel", member(&b2BodyDef::angularVelocity), 0.f);
HCB_END(b2BodyDef)

HCB_BEGIN(phys::BodyDef)
    using namespace phys;
    attr("b2", member(&BodyDef::b2))
    attr("fixtures", member(&BodyDef::fixtures))
HCB_END(phys::BodyDef)

namespace phys {
    b2World* sim;

    void init () {
        sim = new b2World(
            b2Vec2(-10, 0)
        );
    }

    struct Sim_Phase : core::Phase {
        Sim_Phase () : core::Phase(core::game_phases(), "B.M") { }
        void run () {
            sim->step(1/60.0, 10, 10);
        }
    } sim_phase;


    b2Fixture* FixtureDef::manifest (b2Body* b2b) {
        b2Fixture* b2f = b2b->createFixture(b2);
        b2f->setUserData(this);
    }

    b2Body* BodyDef::manifest (b2World* sim, Vec pos, Vec vel = Vec(0, 0)) {
        b2Body* b2b = sim->createBody(&b2);
        for (FixtureDef& fix : fixtures) {
            fix->manifest(b2b);
        }
    }

}

