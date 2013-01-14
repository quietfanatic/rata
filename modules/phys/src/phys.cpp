
#include "../inc/phys.h"

 // This is so satisfying

HCB_BEGIN(b2Vec2)
    elem(member(&b2Vec2::x))
    elem(member(&b2Vec2::y))
HCB_END(b2Vec2)

HCB_BEGIN(b2CircleShape)
    base<b2Shape>("circle");
    attr("p", member(&b2CircleShape::m_p));
    attr("r", member(&b2CircleShape::m_radius));
HCB_END(b2CircleShape)

HCB_BEGIN(b2PolygonShape)
    base<b2Shape>("polygon");
    delegate(value_functions(
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




