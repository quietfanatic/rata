
#include <stdexcept>
#include "../inc/phys.h"
#include "../../core/inc/game.h"
#include "../../hacc/inc/haccable_standard.h"
#include "../../hacc/inc/haccable_pointers.h"
#include "../../util/inc/debug.h"

using namespace phys;

 // This is so satisfying

std::vector<Collision_Rule*> coll_rules;
HCB_BEGIN(Collision_Rule)
    type_name("phys::Collision_Rule");
    get_id([](const Collision_Rule& cr){ return cr.name(); });
    find_by_id([](std::string name){
        for (auto rule : coll_rules)
            if (rule->name() == name)
                return rule;
        return (Collision_Rule*)NULL;
    });
HCB_END(Collision_Rule)

HCB_BEGIN(b2Vec2)
    type_name("b2Vec2");
    elem(member(&b2Vec2::x));
    elem(member(&b2Vec2::y));
HCB_END(b2Vec2)

HCB_BEGIN(b2CircleShape)
    type_name("b2CircleShape");
    base<b2Shape>("circle");
    attr("c", member(&b2CircleShape::m_p));
    attr("r", member((float b2CircleShape::*)&b2CircleShape::m_radius));
HCB_END(b2CircleShape)

HCB_BEGIN(b2PolygonShape)
    type_name("b2PolygonShape");
    base<b2Shape>("polygon");
    attr("radius", member((float b2PolygonShape::*)&b2PolygonShape::m_radius, def(b2_polygonRadius)));
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
    type_name("b2EdgeShape");
    base<b2Shape>("edge");
    attr("v1", member((b2Vec2 b2EdgeShape::*)&b2EdgeShape::m_vertex1));
    attr("v2", member((b2Vec2 b2EdgeShape::*)&b2EdgeShape::m_vertex2));
    elem(member((b2Vec2 b2EdgeShape::*)&b2EdgeShape::m_vertex1));
    elem(member((b2Vec2 b2EdgeShape::*)&b2EdgeShape::m_vertex2));
HCB_END(b2EdgeShape)

HCB_BEGIN(b2FixtureDef)
    type_name("b2FixtureDef");
    attr("shape", member(&b2FixtureDef::shape));
    attr("friction", member(&b2FixtureDef::friction, def(0.f)));
    attr("restitution", member(&b2FixtureDef::restitution, def(0.f)));
    attr("density", member(&b2FixtureDef::density, def(0.f)));
    attr("is_sensor", member(&b2FixtureDef::isSensor, def(false)));
//    attr("filter", member(&b2FixtureDef::filter))
HCB_END(b2FixtureDef)

static std::vector<Collision_Rule*> coll_b2v (uint64 b) {
    std::vector<Collision_Rule*> v;
    for (uint i = 0; b; i++, b >>= 1) {
        if (b & 1) v.push_back(coll_rules[i]);
    }
    return v;
}
static uint64 coll_v2b (const std::vector<Collision_Rule*>& v) {
    uint64 b = 0;
    for (auto rule : v)
        b |= rule->bit();
    return b;
}

HCB_BEGIN(FixtureDef)
    using namespace phys;
    attr("name", member(&FixtureDef::name, def(std::string(""))));
    attr("b2", member(&FixtureDef::b2));
    attr("coll_a", value_functions<std::vector<Collision_Rule*>>(
        [](const FixtureDef& fdf){ return coll_b2v(fdf.coll_a); },
        [](FixtureDef& fdf, std::vector<Collision_Rule*> rules){
            fdf.coll_a = coll_v2b(rules);
        },
        optional<std::vector<Collision_Rule*>>()
    ));
    attr("coll_b", value_functions<std::vector<Collision_Rule*>>(
        [](const FixtureDef& fdf){ return coll_b2v(fdf.coll_b); },
        [](FixtureDef& fdf, std::vector<Collision_Rule*> rules){
            fdf.coll_b = coll_v2b(rules);
        },
        optional<std::vector<Collision_Rule*>>()
    ));
HCB_END(FixtureDef)

HCB_BEGIN(b2BodyType)
    type_name("b2BodyType");
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
    type_name("b2BodyDef");
    attr("type", member(&b2BodyDef::type, def(b2_dynamicBody)));
//    attr("pos", member(&b2BodyDef::position));  This shouldn't be set through this.
//    attr("vel", member(&b2BodyDef::linearVelocity));  Nor this
    attr("damping", member(&b2BodyDef::linearDamping, def(0.f)));
    attr("gravity", member(&b2BodyDef::gravityScale, def(1.f)));
//    attr("unrotatable", member(&b2BodyDef::fixed_rotation), true);
//    attr("ang", member(&b2BodyDef::angle), 0.f);
//    attr("ang_vel", member(&b2BodyDef::angularVelocity), 0.f);
HCB_END(b2BodyDef)

HCB_BEGIN(BodyDef)
    type_name("phys::BodyDef");
    attr("b2", member(&BodyDef::b2));
    attr("fixtures", member(&BodyDef::fixtures));
HCB_END(BodyDef)

HCB_BEGIN(Object)
    type_name("phys::Object");
    attr("pos", value_methods(&Object::pos, &Object::set_pos));
    attr("vel", value_methods(&Object::vel, &Object::set_vel, def(Vec(0, 0))));
HCB_END(Object)

namespace phys {

     // Collision rules

    static uint& n_coll_rules () {
        static uint n_coll_rules = 0;
        return n_coll_rules;
    }

    Collision_Rule::Collision_Rule () : index(n_coll_rules()++) {
        if (index < 64)
            coll_rules.push_back(this);
        else throw std::logic_error("Too many Collision_Rules were created (> 64)");
    }

    struct myCL : b2ContactListener {
        void PostSolve (b2Contact*, const b2ContactImpulse*);
        void EndContact (b2Contact*);
    } mycl;
    void myCL::PostSolve (b2Contact* contact, const b2ContactImpulse* ci) {
        b2Fixture* a = contact->GetFixtureA();
        b2Fixture* b = contact->GetFixtureB();
        FixtureDef* afdf = (FixtureDef*)a->GetUserData();
        FixtureDef* bfdf = (FixtureDef*)b->GetUserData();
        uint64 coll_ab = afdf->coll_a & bfdf->coll_b;
        for (uint i = 0; coll_ab; i++) {
            if (coll_ab & 1) coll_rules[i]->post(contact, a, b);
            coll_ab >>= 1;
        }
        uint64 coll_ba = afdf->coll_b & bfdf->coll_a;
        for (uint i = 0; coll_ba; i++) {
            if (coll_ba & 1) coll_rules[i]->post(contact, b, a);
            coll_ba >>= 1;
        }
    }
    void myCL::EndContact (b2Contact* contact) {
        b2Fixture* a = contact->GetFixtureA();
        b2Fixture* b = contact->GetFixtureB();
        FixtureDef* afdf = (FixtureDef*)a->GetUserData();
        FixtureDef* bfdf = (FixtureDef*)b->GetUserData();
        uint64 coll_ab = afdf->coll_a & bfdf->coll_b;
        for (uint i = 0; coll_ab; i++) {
            if (coll_ab & 1) coll_rules[i]->end(contact, a, b);
            coll_ab >>= 1;
        }
        uint64 coll_ba = afdf->coll_b & bfdf->coll_a;
        for (uint i = 0; coll_ba; i++) {
            if (coll_ba & 1) coll_rules[i]->end(contact, b, a);
            coll_ba >>= 1;
        }
    }


     // Space handling

    b2World* space = NULL;

    static Logger space_logger ("space");

    struct Space_Phase : core::Phase {
        Space_Phase () : core::Phase("D.M", "space") { }
        void init () {
            space_logger.log("Creating the spacetime continuum.  Well, the space part anyway.");
            space = new b2World(
                b2Vec2(0, -20)
            );
            space->SetContactListener(&mycl);
        }
        void start () {
        }
        void run () {
            for (b2Body* b2b = space->GetBodyList(); b2b; b2b = b2b->GetNext()) {
                if (Object* obj = (Object*)b2b->GetUserData())
                    if (b2b->IsActive())
                        obj->before_move();
            }
            space->Step(1/60.0, 10, 10);
            for (b2Body* b2b = space->GetBodyList(); b2b; b2b = b2b->GetNext()) {
                if (Object* obj = (Object*)b2b->GetUserData()) {
                    if (b2b->IsActive())
                        obj->after_move();
                    else obj->while_intangible();
                }
            }
        }
        void stop () {
            space_logger.log("Destroying space.");
            delete space;
            init();
        }
    } space_phase;

     // Definitions

    b2Fixture* FixtureDef::manifest (b2Body* b2b) {
        b2Fixture* b2f = b2b->CreateFixture(&b2);
        b2f->SetUserData(this);
        return b2f;
    }

    b2Body* BodyDef::manifest (Object* owner, b2World* sim, Vec pos, Vec vel) {
        b2Body* b2b = space->CreateBody(&b2);
        b2b->SetUserData(owner);
        space_logger.log("%d objects in space.", space->GetBodyCount());
        for (FixtureDef& fix : fixtures) {
            fix.manifest(b2b);
        }
        return b2b;
    }

     // Objects

    void Object::materialize () { b2body->SetActive(true); }
    void Object::dematerialize () { b2body->SetActive(false); }

}

