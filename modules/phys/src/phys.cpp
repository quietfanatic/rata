
#include "../inc/phys.h"
#include "../../core/inc/game.h"
#include "../../hacc/inc/haccable_standard.h"
#include "../../hacc/inc/haccable_pointers.h"
#include "../../util/inc/debug.h"

 // This is so satisfying

HCB_BEGIN(b2Vec2)
    elem(member(&b2Vec2::x));
    elem(member(&b2Vec2::y));
HCB_END(b2Vec2)

HCB_BEGIN(b2CircleShape)
    base<b2Shape>("circle");
    attr("c", member(&b2CircleShape::m_p));
    attr("r", member((float b2CircleShape::*)&b2CircleShape::m_radius));
HCB_END(b2CircleShape)

HCB_BEGIN(b2PolygonShape)
    base<b2Shape>("polygon");
    attr("radius", member((float b2PolygonShape::*)&b2PolygonShape::m_radius, hacc::Defaulter1<float>(b2_polygonRadius)));
    attr("verts", value_functions<std::vector<b2Vec2>>(
        [](const b2PolygonShape& ps){
            return std::vector<b2Vec2>(ps.m_vertices, ps.m_vertices + ps.m_vertexCount);
        },
        [](b2PolygonShape& ps, std::vector<b2Vec2> v){
            ps.Set(v.data(), v.size());
        }
    ));
HCB_END(b2PolygonShape)

HCB_BEGIN(b2EdgeShape)
    base<b2Shape>("edge");
    attr("v1", member((b2Vec2 b2EdgeShape::*)&b2EdgeShape::m_vertex1));
    attr("v2", member((b2Vec2 b2EdgeShape::*)&b2EdgeShape::m_vertex2));
    elem(member((b2Vec2 b2EdgeShape::*)&b2EdgeShape::m_vertex1));
    elem(member((b2Vec2 b2EdgeShape::*)&b2EdgeShape::m_vertex2));
HCB_END(b2EdgeShape)

HCB_BEGIN(b2FixtureDef)
    attr("shape", member(&b2FixtureDef::shape));
    attr("friction", member<float>(&b2FixtureDef::friction, 0.f));
    attr("restitution", member<float>(&b2FixtureDef::restitution, 0.f));
    attr("density", member<float>(&b2FixtureDef::density, 0.f));
    attr("is_sensor", member<bool>(&b2FixtureDef::isSensor, false));
//    attr("filter", member(&b2FixtureDef::filter))
HCB_END(b2FixtureDef)

HCB_BEGIN(phys::FixtureDef)
    using namespace phys;
    attr("b2", member(&FixtureDef::b2));
HCB_END(phys::FixtureDef)

HCB_BEGIN(b2BodyType)
    value_name([](const b2BodyType& bt) -> std::string {
        switch (bt) {
            case b2_staticBody: return "static";
            case b2_dynamicBody: return "dynamic";
            case b2_kinematicBody: return "kinematic";
            default: return "";
        }
    });
    value("static", b2_staticBody);
    value("dynamic", b2_dynamicBody);
    value("kinematic", b2_kinematicBody);
HCB_END(b2BodyType)

HCB_BEGIN(b2BodyDef)
    attr("type", member<b2BodyType>(&b2BodyDef::type, b2_dynamicBody));
//    attr("pos", member(&b2BodyDef::position));  This shouldn't be set through this.
//    attr("vel", member(&b2BodyDef::linearVelocity));  Nor this
    attr("damping", member<float>(&b2BodyDef::linearDamping, 0.f));
    attr("gravity", member<float>(&b2BodyDef::gravityScale, 1.f));
//    attr("unrotatable", member(&b2BodyDef::fixed_rotation), true);
//    attr("ang", member(&b2BodyDef::angle), 0.f);
//    attr("ang_vel", member(&b2BodyDef::angularVelocity), 0.f);
HCB_END(b2BodyDef)

HCB_BEGIN(phys::BodyDef)
    using namespace phys;
    attr("b2", member(&BodyDef::b2));
    attr("fixtures", member(&BodyDef::fixtures));
HCB_END(phys::BodyDef)

HCB_BEGIN(phys::Physical)
    using namespace phys;
    attr("pos", value_methods<Vec>(&Physical::pos, &Physical::set_pos));
    attr("vel", value_methods<Vec>(&Physical::vel, &Physical::set_vel, Vec(0, 0)));
HCB_END(phys::Physical)
namespace phys {

    b2World* sim = NULL;

    Links<Actor> all_actors;
    void Actor::start () {
        link(all_actors);
    }


    struct Act_Phase : core::Phase {
        Act_Phase () : core::Phase("C.M", "act") { }
        void run () { // These actions should be parallelizable.
            for (Actor* p = all_actors.first(); p; p = p->next())
                p->act();
        }
    } act_phase;

    Logger phys_logger ("phys");

    struct Sim_Phase : core::Phase {
        Sim_Phase () : core::Phase("D.M", "sim") { }
        void init () {
            phys_logger.log("Creating the main sim.");
            sim = new b2World(
                b2Vec2(0, -20)
            );
        }
        void start () {
        }
        void run () {
            sim->Step(1/60.0, 10, 10);
        }
        void stop () {
            delete sim;
            init();
        }
    } sim_phase;

    struct React_Phase : core::Phase {
        React_Phase () : core::Phase("E.M", "react") { }
        void run () { // These actions should be parallelizable.
            for (Actor* p = all_actors.first(); p; p = p->next())
                p->react();
        }

    } react_phase;


    b2Fixture* FixtureDef::manifest (b2Body* b2b) {
        b2Fixture* b2f = b2b->CreateFixture(&b2);
        b2f->SetUserData(this);
        return b2f;
    }

    b2Body* BodyDef::manifest (b2World* sim, Vec pos, Vec vel) {
        b2Body* b2b = sim->CreateBody(&b2);
        phys_logger.log("Sim now has %d bodies.\n", sim->GetBodyCount());
        for (FixtureDef& fix : fixtures) {
            fix.manifest(b2b);
        }
        return b2b;
    }

    void Physical::activate () { body->SetActive(true); }
    void Physical::deactivate () { body->SetActive(false); }

}

