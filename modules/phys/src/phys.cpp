#include "phys/inc/phys.h"

#include <vector>
#include <stdexcept>
#include "core/inc/commands.h"
#include "hacc/inc/haccable_standard.h"
#include "util/inc/debug.h"
#include "vis/inc/color.h"
#include "vis/inc/common.h"
using namespace util;

namespace phys {

     // Collision rules

    INIT_SAFE(std::vector<Collision_Rule*>, coll_rules);
    INIT_SAFE(uint, n_coll_rules, = 0);

    Collision_Rule::Collision_Rule () : index(n_coll_rules()++) {
        if (index < 64) {
            coll_rules().push_back(this);
            log("collision_rules", "Declared collision rule: %u", index);
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
            const Filter& afilt = reinterpret_cast<const Filter&>(a->GetFilterData());
            const Filter& bfilt = reinterpret_cast<const Filter&>(b->GetFilterData());
            return afilt.active && bfilt.active
                && (afilt.mask & bfilt.mask)
                && !(afilt.unmask & bfilt.unmask);
        }
    } mycf;

     // Space handling

    Space space;

    Space::Space () { }
    void Space::start () {
        log("space", "Creating the spacetime continuum.  Well, the space part anyway.");
        b2world = new b2World(
            b2Vec2(0, -30)
        );
        b2world->SetContactListener(&mycl);
        b2world->SetContactFilter(&mycf);
    }
    void Space::run () {
        for (b2Body* b2b = b2world->GetBodyList(); b2b; b2b = b2b->GetNext()) {
            if (Object* obj = (Object*)b2b->GetUserData())
                if (b2b->IsActive())
                    obj->Object_before_move();
        }
        b2world->Step(1/60.0, 10, 10);
        for (b2Body* b2b = b2world->GetBodyList(); b2b; b2b = b2b->GetNext()) {
            if (Object* obj = (Object*)b2b->GetUserData()) {
                if (b2b->IsActive())
                    obj->Object_after_move();
                else obj->Object_while_intangible();
            }
        }
    }
    void Space::stop () {
        log("space", "Destroying space.");
        delete b2world;
    }
    Space::~Space () { stop(); }

    void Object::finish () {
        b2body->SetType(def->type);
        b2body->SetLinearDamping(def->damping);
        b2body->SetGravityScale(def->gravity_scale);
        b2MassData md {def->mass, Vec(0, 0), 0};
        b2body->SetMassData(&md);
         // Remove old fixtures
        while (auto fix = b2body->GetFixtureList())
            b2body->DestroyFixture(fix);
         // Add new fixtures
        for (auto& fix : def->fixtures) {
            add_fixture(&fix);
        }
    }
    b2Fixture* Object::add_fixture (FixtureDef* fdf) {
        fdf->b2.filter = fdf->filter;
        b2Fixture* b2f = b2body->CreateFixture(&fdf->b2);
        b2f->SetUserData(fdf);
        log("space", "Adding fixture @%lx to @%lx", (size_t)fdf, (size_t)this);
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
        log("space", "Added @%lx; %d objects in space.", (size_t)this, space.b2world->GetBodyCount());
    }
    Object::~Object () {
        if (b2body) {
            space.b2world->DestroyBody(b2body);
            log("space", "Removed @%lx; %d objects in space.", (size_t)this, space.b2world->GetBodyCount());
        }
    }

    void Object::materialize () { b2body->SetActive(true); }
    void Object::dematerialize () { b2body->SetActive(false); }

     // Debug fixture drawing

    struct Phys_Debug_Layer : vis::Drawn<vis::Overlay> {
        void Drawn_draw (vis::Overlay) override {
            using namespace vis;
            for (b2Body* b2b = space.b2world->GetBodyList(); b2b; b2b = b2b->GetNext()) {
                if (b2b->IsActive()) {
                    uint32 color = 0xffffff7f;
                    switch (b2b->GetType()) {
                        case b2_staticBody: color = 0x00ff007f; break;
                        case b2_dynamicBody: color = 0x0000ff7f; break;
                        case b2_kinematicBody: color = 0x00ffff7f; break;
                        default: color = 0xffffff7f; break;  // shouldn't happen
                    }
                    color_offset(b2b->GetPosition());
                    for (b2Fixture* b2f = b2b->GetFixtureList(); b2f; b2f = b2f->GetNext()) {
                        uint32 fcolor = b2f->IsSensor() ? 0xff00ff7f : color;
                        draw_color(fcolor);
                        b2Shape* b2s = b2f->GetShape();
                        switch (b2s->m_type) {
                            case b2Shape::e_circle: {
                                auto b2cs = static_cast<b2CircleShape*>(b2s);
                                Vec pts [4];  // Just draw a diamond until draw_circle exists
                                pts[0] = Vec(b2cs->m_p.x - b2cs->m_radius, b2cs->m_p.y);
                                pts[1] = Vec(b2cs->m_p.x, b2cs->m_p.y - b2cs->m_radius);
                                pts[2] = Vec(b2cs->m_p.x + b2cs->m_radius, b2cs->m_p.y);
                                pts[3] = Vec(b2cs->m_p.x, b2cs->m_p.y + b2cs->m_radius);
                                draw_loop(4, pts);
                                break;
                            }
                            case b2Shape::e_polygon: {
                                auto b2ps = static_cast<b2PolygonShape*>(b2s);
                                 // I love binary compatibility
                                draw_loop(b2ps->m_count, (Vec*)b2ps->m_vertices);
                                break;
                            }
                            case b2Shape::e_edge: {
                                auto b2es = static_cast<b2EdgeShape*>(b2s);
                                draw_line(b2es->m_vertex1, b2es->m_vertex2);
                                if (b2es->m_hasVertex0) {
                                    draw_color((fcolor&~0xff)|0x3f);
                                    draw_line(Vec(b2es->m_vertex0)-Vec(.25, .25), b2es->m_vertex1);
                                }
                                if (b2es->m_hasVertex3) {
                                    draw_color((fcolor&~0xff)|0x3f);
                                    draw_line(b2es->m_vertex2, Vec(b2es->m_vertex3)+Vec(.25, .25));
                                }
                                break;
                            }
                            case b2Shape::e_chain: {
                                auto b2cs = static_cast<b2ChainShape*>(b2s);
                                draw_chain(b2cs->m_count, (Vec*)b2cs->m_vertices);
                                break;
                            }
                            default: { }  // shouldn't happen
                        }
                    }
                }
            }
            color_offset(Vec(0, 0));
        }
    } phys_debug_layer;

} using namespace phys;

 // This is so satisfying

HACCABLE(b2Vec2) {
    name("b2Vec2");
    elem(member(&b2Vec2::x));
    elem(member(&b2Vec2::y));
}

HACCABLE(b2Shape) {
    name("b2Shape");
}

HACCABLE(b2CircleShape) {
    name("b2CircleShape");
    attr("b2Shape", base<b2Shape>().collapse());
    attr("c", member(&b2CircleShape::m_p));
    attr("r", member((float b2CircleShape::*)&b2CircleShape::m_radius));
}

struct Hack_b2PolygonShape_Verts : b2PolygonShape { };

HACCABLE(b2PolygonShape) {
    name("b2PolygonShape");
    attr("b2Shape", base<b2Shape>().collapse());
    attr("radius", member((float b2PolygonShape::*)&b2PolygonShape::m_radius).optional());
    attr("verts", base<Hack_b2PolygonShape_Verts>());
}
HACCABLE(Hack_b2PolygonShape_Verts) {
    name("Hack_b2PolygonShape_Verts");
    array();
    length(value_funcs<size_t>(
        [](const Hack_b2PolygonShape_Verts& v)->size_t{
            return v.m_count;
        },
        [](Hack_b2PolygonShape_Verts& v, size_t size){
            if (size >= 3 && size <= b2_maxPolygonVertices) {
                v.m_count = size;
            }
            else
                throw hacc::X::Wrong_Size(hacc::Pointer(&v), size, b2_maxPolygonVertices);
        }
    ));
    elems([](Hack_b2PolygonShape_Verts& v, size_t index){
        if ((int)index <= v.m_count)
            return hacc::Reference(v.m_vertices + index);
        else
            throw hacc::X::Out_Of_Range(hacc::Pointer(&v), index, v.m_count);
    });
     // We have to call Set during fill and not finish, because another object's
     //  finish relies on this object having Set called.
    fill([](Hack_b2PolygonShape_Verts& v, hacc::Tree t){
        auto a = t.as<hacc::Array>();
        b2Vec2 tmp_verts [b2_maxPolygonVertices];
        for (uint i = 0; i < a.size(); i++)
            hacc::Reference(&tmp_verts[i]).fill(a[i]);
        v.Set(tmp_verts, a.size());
    });
}

HACCABLE(b2EdgeShape) {
    name("b2EdgeShape");
    attr("b2Shape", base<b2Shape>().collapse());
    attr("v1", member(&b2EdgeShape::m_vertex1));
    attr("v2", member(&b2EdgeShape::m_vertex2));
    elem(member(&b2EdgeShape::m_vertex1));
    elem(member(&b2EdgeShape::m_vertex2));
}

HACCABLE(b2FixtureDef) {
    name("b2FixtureDef");
    attr("shape", member(&b2FixtureDef::shape).optional());
    attr("friction", member(&b2FixtureDef::friction).optional());
    attr("restitution", member(&b2FixtureDef::restitution).optional());
    attr("density", member(&b2FixtureDef::density).optional());
    attr("is_sensor", member(&b2FixtureDef::isSensor).optional());
}

struct Collision_Rules {
    uint64 ui64;
};

HACCABLE(Collision_Rules) {
    name("phys::Collision_Rules");
    prepare([](Collision_Rules&, hacc::Tree){ });
    fill([](Collision_Rules& crs, hacc::Tree tree){
        uint64 r = 0;
        for (auto& e : tree.as<hacc::Array>()) {
            auto n = e.as<std::string>();
            for (auto rule : coll_rules()) {
                if (rule->name() == n) {
                    r |= rule->bit();
                }
                break;
            }
        }
        crs.ui64 = r;
    });
    to_tree([](const Collision_Rules& crs) {
        hacc::Array r;
        for (size_t i = 0; i < coll_rules().size(); i++) {
            if (crs.ui64 & (1<<i)) r.push_back(hacc::Tree(coll_rules()[i]->name()));
        }
        return hacc::Tree(std::move(r));
    });
}

HACCABLE(FixtureDef) {
    name("phys::FixtureDef");
    attr("b2", member(&FixtureDef::b2).collapse());
    attr("coll_a", member((Collision_Rules FixtureDef::*)&FixtureDef::coll_a).optional());
    attr("coll_b", member((Collision_Rules FixtureDef::*)&FixtureDef::coll_b).optional());
    attr("filter", member(&FixtureDef::filter).optional());
}

HACCABLE(Filter) {
    name("phys::Filter");
    attr("mask", member(&Filter::mask).optional());
    attr("unmask", member(&Filter::unmask).optional());
    attr("active", member(&Filter::active).optional());
}

HACCABLE(b2BodyType) {
    name("b2BodyType");
    value("static", b2_staticBody);
    value("dynamic", b2_dynamicBody);
    value("kinematic", b2_kinematicBody);
}

HACCABLE(Object_Def) {
    name("phys::Object_Def");
    attr("type", member(&Object_Def::type).optional());
    attr("mass", member(&Object_Def::mass).optional());
    attr("damping", member(&Object_Def::damping).optional());
    attr("gravity_scale", member(&Object_Def::gravity_scale).optional());
    attr("fixtures", member(&Object_Def::fixtures).optional());
}

HACCABLE(Object) {
    name("phys::Object");
    attr("def", member(&Object::def));
    attr("pos", value_methods(&Object::pos, &Object::set_pos).optional());
    attr("vel", value_methods(&Object::vel, &Object::set_vel).optional());
    finish(&Object::finish);
}

HACCABLE(Phys_Debug_Layer) {
    name("phys::Phys_Debug_Layer");
    finish([](Phys_Debug_Layer& pdl){
        pdl.appear();
    });
}

void _phys_debug () {
    if (phys_debug_layer.visible())
        phys_debug_layer.disappear();
    else
        phys_debug_layer.appear();
}
core::New_Command _phys_debug_cmd ("phys_debug", "Toggle contours around physical objects", 0, _phys_debug);

