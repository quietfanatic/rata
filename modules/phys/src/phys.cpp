
#include <stdexcept>
#include "../inc/phys.h"
#include "../../core/inc/phases.h"
#include "../../hacc/inc/haccable_standard.h"
#include "../../util/inc/debug.h"
#include "../../vis/inc/graffiti.h"

namespace phys {

     // Collision rules

    static Logger cr_logger ("collision_rules");

    INIT_SAFE(std::vector<Collision_Rule*>, coll_rules);
    INIT_SAFE(uint, n_coll_rules, = 0);

    Collision_Rule::Collision_Rule () : index(n_coll_rules()++) {
        if (index < 64) {
            coll_rules().push_back(this);
            cr_logger.log("Declared collision rule: %u", index);
        }
        else throw hacc::X::Logic_Error("Too many Collision_Rules were created (> 64)");
    }

     // Box2D callbacks
    struct myCL : b2ContactListener {
        void PreSolve (b2Contact* contact, const b2Manifold* old_manifold) override {
            b2Fixture* a = contact->GetFixtureA();
            b2Fixture* b = contact->GetFixtureB();
            FixtureDef* afdf = (FixtureDef*)a->GetUserData();
            FixtureDef* bfdf = (FixtureDef*)b->GetUserData();
            uint64 coll_ab = afdf->coll_a & bfdf->coll_b;
            for (uint i = 0; coll_ab; i++) {
                if (coll_ab & 1)
                    coll_rules()[i]->Collision_Rule_presolve(contact, a, b);
                coll_ab >>= 1;
            }
            uint64 coll_ba = afdf->coll_b & bfdf->coll_a;
            for (uint i = 0; coll_ba; i++) {
                if (coll_ba & 1)
                    coll_rules()[i]->Collision_Rule_presolve(contact, b, a);
                coll_ba >>= 1;
            }
        }
        void EndContact (b2Contact* contact) override {
            b2Fixture* a = contact->GetFixtureA();
            b2Fixture* b = contact->GetFixtureB();
            FixtureDef* afdf = (FixtureDef*)a->GetUserData();
            FixtureDef* bfdf = (FixtureDef*)b->GetUserData();
            uint64 coll_ab = afdf->coll_a & bfdf->coll_b;
            for (uint i = 0; coll_ab; i++) {
                if (coll_ab & 1)
                    coll_rules()[i]->Collision_Rule_end(contact, a, b);
                coll_ab >>= 1;
            }
            uint64 coll_ba = afdf->coll_b & bfdf->coll_a;
            for (uint i = 0; coll_ba; i++) {
                if (coll_ba & 1)
                    coll_rules()[i]->Collision_Rule_end(contact, b, a);
                coll_ba >>= 1;
            }
        }
    } mycl;

    struct myCF : b2ContactFilter {
        bool ShouldCollide (b2Fixture* a, b2Fixture* b) override {
            FixtureDef* adef = (FixtureDef*)a->GetUserData();
            FixtureDef* bdef = (FixtureDef*)b->GetUserData();
            const Filter& afilt = reinterpret_cast<const Filter&>(a->GetFilterData());
            const Filter& bfilt = reinterpret_cast<const Filter&>(b->GetFilterData());
            return afilt.active && bfilt.active
                && afilt.mask & bfilt.mask
                && !(afilt.unmask & bfilt.unmask)
                && (adef->coll_a & bdef->coll_b || adef->coll_b & bdef->coll_a);
        }
    } mycf;

     // Space handling

    static Logger space_logger ("space");

    Space space;

    Space::Space () : core::Phase("D.M", "space") { }
    void Space::Phase_start () {
        space_logger.log("Creating the spacetime continuum.  Well, the space part anyway.");
        b2world = new b2World(
            b2Vec2(0, -30)
        );
        b2world->SetContactListener(&mycl);
    }
    void Space::Phase_run () {
        for (b2Body* b2b = b2world->GetBodyList(); b2b; b2b = b2b->GetNext()) {
            if (Object* obj = (Object*)b2b->GetUserData())
                if (b2b->IsActive())
                    obj->before_move();
        }
        b2world->Step(1/60.0, 10, 10);
        for (b2Body* b2b = b2world->GetBodyList(); b2b; b2b = b2b->GetNext()) {
            if (Object* obj = (Object*)b2b->GetUserData()) {
                if (b2b->IsActive())
                    obj->after_move();
                else obj->while_intangible();
            }
        }
    }
    Space::~Space () {
        space_logger.log("Destroying space.");
        delete b2world;
    }

     // Objects
    void Object::apply_bdf (BodyDef* bdf) {
        b2body->SetType(bdf->type);
        b2body->SetLinearDamping(bdf->damping);
        b2body->SetGravityScale(bdf->gravity_scale);
        if (bdf->mass == bdf->mass) {
            b2MassData md {bdf->mass, Vec(0, 0), 0};
            b2body->SetMassData(&md);
        }
        for (auto fix : bdf->fixtures) {
            add_fixture(fix);
        }
    }
    b2Fixture* Object::add_fixture (FixtureDef* fdf) {
        b2Fixture* b2f = b2body->CreateFixture(&fdf->b2);
        b2f->SetUserData(fdf);
        return b2f;
    }
    b2Fixture* Object::get_fixture (FixtureDef* fd) {
        for (b2Fixture* fix = b2body->GetFixtureList(); fix; fix = fix->GetNext()) {
            if ((FixtureDef*)fix->GetUserData() == fd)
                return fix;
        }
        return NULL;
    }

    void Object::foreach_contact (const std::function<void (b2Fixture*, b2Fixture*)>& f) {
        for (auto ce = b2body->GetContactList(); ce; ce = ce->next) {
            if (ce->contact->IsEnabled() && ce->contact->IsTouching()) {
                auto fix_a = ce->contact->GetFixtureA();
                auto fix_b = ce->contact->GetFixtureB();
                if ((Object*)fix_a->GetBody()->GetUserData() == this)
                    f(fix_a, fix_b);
                else
                    f(fix_b, fix_a);
            }
        }
    }

    Object::Object () {
        b2BodyDef b2bd;
        b2bd.active = false;
        b2bd.fixedRotation = true;
        b2bd.type = b2_dynamicBody;
        b2bd.userData = this;
        b2body = space.b2world->CreateBody(&b2bd);
        space_logger.log("%d objects in space.", space.b2world->GetBodyCount());
    }

    void Object::materialize () { b2body->SetActive(true); }
    void Object::dematerialize () { b2body->SetActive(false); }

     // Debug fixture drawing
    
    struct Phys_Debug_Layer : core::Layer {
        Phys_Debug_Layer () : core::Layer("G.M", "phys_debug", false) { }
        void Layer_start () { }
        void Layer_run () {
            for (b2Body* b2b = space.b2world->GetBodyList(); b2b; b2b = b2b->GetNext()) {
                if (b2b->IsActive()) {
                    uint32 color = 0xffffff7f;
                    switch (b2b->GetType()) {
                        case b2_staticBody: color = 0x00ff007f; break;
                        case b2_dynamicBody: color = 0x0000ff7f; break;
                        case b2_kinematicBody: color = 0x00ffff7f; break;
                        default: color = 0xffffff7f; break;  // shouldn't happen
                    }
                    vis::graffiti_pos(b2b->GetPosition());
                    for (b2Fixture* b2f = b2b->GetFixtureList(); b2f; b2f = b2f->GetNext()) {
                        uint32 fcolor = b2f->IsSensor() ? 0xff00ff7f : color;
                        b2Shape* b2s = b2f->GetShape();
                        switch (b2s->m_type) {
                            case b2Shape::e_circle: {
                                auto b2cs = static_cast<b2CircleShape*>(b2s);
                                Vec pts [4];  // Just draw a diamond until draw_circle exists
                                pts[0] = Vec(b2cs->m_p.x - b2cs->m_radius, b2cs->m_p.y);
                                pts[1] = Vec(b2cs->m_p.x, b2cs->m_p.y - b2cs->m_radius);
                                pts[2] = Vec(b2cs->m_p.x + b2cs->m_radius, b2cs->m_p.y);
                                pts[3] = Vec(b2cs->m_p.x, b2cs->m_p.y + b2cs->m_radius);
                                vis::draw_loop(4, pts, fcolor);
                                break;
                            }
                            case b2Shape::e_polygon: {
                                auto b2ps = static_cast<b2PolygonShape*>(b2s);
                                 // I love binary compatibility
                                vis::draw_loop(b2ps->m_count, (Vec*)b2ps->m_vertices, fcolor);
                                break;
                            }
                            case b2Shape::e_edge: {
                                auto b2es = static_cast<b2EdgeShape*>(b2s);
                                vis::draw_line(b2es->m_vertex1, b2es->m_vertex2, fcolor);
                                if (b2es->m_hasVertex0)
                                    vis::draw_line(Vec(b2es->m_vertex0)-Vec(.25,.25), b2es->m_vertex1, (fcolor&~0xff)|0x3f);
                                if (b2es->m_hasVertex3)
                                    vis::draw_line(b2es->m_vertex2, Vec(b2es->m_vertex3)+Vec(.25,.25), (fcolor&~0xff)|0x3f);
                                break;
                            }
                            case b2Shape::e_chain: {
                                auto b2cs = static_cast<b2ChainShape*>(b2s);
                                vis::draw_chain(b2cs->m_count, (Vec*)b2cs->m_vertices, fcolor);
                                break;
                            }
                            default: { }  // shouldn't happen
                        }
                    }
                }
            }
        }
    } phys_debug_layer;

} using namespace phys;

 // This is so satisfying

HCB_BEGIN(Collision_Rule*)
    name("phys::Collision_Rule*");
    hacc::hacc_pointer_by_method(&Collision_Rule::name, coll_rules(), true);
HCB_END(Collision_Rule*)

HCB_BEGIN(b2Vec2)
    name("b2Vec2");
    elem(member(&b2Vec2::x));
    elem(member(&b2Vec2::y));
HCB_END(b2Vec2)

HCB_BEGIN(b2Shape)
    name("b2Shape");
HCB_END(b2Shape)

HCB_BEGIN(b2CircleShape)
    name("b2CircleShape");
    attr("b2Shape", base<b2Shape>().optional());
    attr("c", member(&b2CircleShape::m_p));
    attr("r", member((float b2CircleShape::*)&b2CircleShape::m_radius));
HCB_END(b2CircleShape)

HCB_BEGIN(b2PolygonShape)
    name("b2PolygonShape");
    attr("b2Shape", base<b2Shape>().optional());
    attr("radius", member((float b2PolygonShape::*)&b2PolygonShape::m_radius).optional());
    attr("verts", mixed_funcs<std::vector<b2Vec2>>(
        [](const b2PolygonShape& ps){
            return std::vector<b2Vec2>(ps.m_vertices, ps.m_vertices + ps.m_count);
        },
        [](b2PolygonShape& ps, const std::vector<b2Vec2>& v){
            ps.Set(v.data(), v.size());
        }
    ).narrow());
HCB_END(b2PolygonShape)

HCB_BEGIN(b2EdgeShape)
    name("b2EdgeShape");
    attr("b2Shape", base<b2Shape>().optional());
    attr("v1", member(&b2EdgeShape::m_vertex1));
    attr("v2", member(&b2EdgeShape::m_vertex2));
    elem(member(&b2EdgeShape::m_vertex1));
    elem(member(&b2EdgeShape::m_vertex2));
HCB_END(b2EdgeShape)

HCB_BEGIN(b2FixtureDef)
    name("b2FixtureDef");
    attr("shape", member(&b2FixtureDef::shape).optional());
    attr("friction", member(&b2FixtureDef::friction).optional());
    attr("restitution", member(&b2FixtureDef::restitution).optional());
    attr("density", member(&b2FixtureDef::density).optional());
    attr("is_sensor", member(&b2FixtureDef::isSensor).optional());
    attr("filter", ref_func<Filter>([](b2FixtureDef& fd)->Filter&{ return reinterpret_cast<Filter&>(fd.filter); }).optional());
HCB_END(b2FixtureDef)

static std::vector<Collision_Rule*> coll_b2v (uint64 b) {
    std::vector<Collision_Rule*> v;
    for (uint i = 0; b; i++, b >>= 1) {
        if (b & 1) v.push_back(coll_rules()[i]);
    }
    return v;
}
static uint64 coll_v2b (const std::vector<Collision_Rule*>& v) {
    uint64 b = 0;
    for (auto rule : v) {
        if (!rule) throw hacc::X::Logic_Error("Somehow a null pointer got into a collision rule list");
        b |= rule->bit();
    }
    return b;
}

HCB_BEGIN(FixtureDef)
    name("phys::FixtureDef");
    attr("b2", member(&FixtureDef::b2));
    attr("coll_a", value_funcs<std::vector<Collision_Rule*>>(
        [](const FixtureDef& fdf){ return coll_b2v(fdf.coll_a); },
        [](FixtureDef& fdf, std::vector<Collision_Rule*> rules){
            fdf.coll_a = coll_v2b(rules);
        }
    ).optional().narrow());
    attr("coll_b", value_funcs<std::vector<Collision_Rule*>>(
        [](const FixtureDef& fdf){ return coll_b2v(fdf.coll_b); },
        [](FixtureDef& fdf, std::vector<Collision_Rule*> rules){
            fdf.coll_b = coll_v2b(rules);
        }
    ).optional().narrow());
HCB_END(FixtureDef)

HCB_BEGIN(Filter)
    name("phys::Filter");
    attr("mask", member(&Filter::mask).optional());
    attr("unmask", member(&Filter::unmask).optional());
    attr("active", member(&Filter::active).optional());
HCB_END(Filter)

HCB_BEGIN(b2BodyType)
    name("b2BodyType");
    value("static", b2_staticBody);
    value("dynamic", b2_dynamicBody);
    value("kinematic", b2_kinematicBody);
HCB_END(b2BodyType)

HCB_BEGIN(BodyDef)
    name("phys::BodyDef");
    attr("type", member(&BodyDef::type).optional());
    attr("mass", member(&BodyDef::mass).optional());
    attr("damping", member(&BodyDef::damping).optional());
    attr("gravity_scale", member(&BodyDef::gravity_scale).optional());
    attr("fixtures", member(&BodyDef::fixtures));
HCB_END(BodyDef)

HCB_BEGIN(Object)
    name("phys::Object");
    attr("pos", value_methods(&Object::pos, &Object::set_pos));
    attr("vel", value_methods(&Object::vel, &Object::set_vel).optional());
HCB_END(Object)

HCB_BEGIN(Phys_Debug_Layer)
    name("phys::Phys_Debug_Layer");
HCB_END(Phys_Debug_Layer)

