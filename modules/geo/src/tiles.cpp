
#include "../../hacc/inc/everything.h"
#include "../../util/inc/math.h"
#include "../../util/inc/debug.h"
#include "../inc/tiles.h"

using namespace geo;
HCB_BEGIN(TileDef)
    type_name("geo::TileDef");
    attr("nature", member(&TileDef::nature));
    attr("vertices", member(&TileDef::vertices));
HCB_END(TileDef)

HCB_BEGIN(Tileset)
    type_name("geo::Tileset");
    attr("tiles", member(&Tileset::tiles));
HCB_END(Tileset)

HCB_BEGIN(Tilemap)
    type_name("geo::Tilemap");
    base<Furniture>("Tilemap");
    attr("tileset", member(&Tilemap::tileset));
    attr("texture", member(&Tilemap::texture));
    attr("pos", member(&Tilemap::pos));
    attr("width", member(&Tilemap::width));
    attr("height", member(&Tilemap::height));
    attr("tiles", member(&Tilemap::tiles));
HCB_END(Tilemap)

namespace geo {

static phys::BodyDef*& tilemap_bdf () {
    static phys::BodyDef* r = hacc::reference_file<phys::BodyDef>("modules/geo/res/tilemap.bdf");
    return r;
}

static Logger tilemap_logger ("tilemap");

Links<Tilemap> active_tilemaps;

Tilemap::Tilemap () : phys::Object(tilemap_bdf()) { }
void Tilemap::emerge () { materialize(); link(active_tilemaps); }
void Tilemap::reclude () { dematerialize(); unlink(); }


 // What follows is an algorithm to optimize tile geometry.

struct TileEdge {
    Vec v1;
    Vec v2;
    TileEdge* next;
    TileEdge* prev;
    TileDef* def = NULL;
};

const uint MAX_EDGES = 6;

struct TileData {
    TileEdge edges [MAX_EDGES];
};

static inline bool vecs_are_close (Vec a, Vec b) {
    return (a.x - b.x)*(a.x - b.x)
         + (a.y - b.y)*(a.y - b.y)
         < 0.01;
}
static inline void try_connecting_edges (TileEdge* te, TileEdge* pe, uint* count) {
    if (te->def && pe->def) {
         // Vertices should always wind in CCW direction, so
         //  compatible edges will be backwards from one another
        if (vecs_are_close(te->v1, pe->v2))
        if (vecs_are_close(te->v2, pe->v1)) {
            te->next = pe->next;
            te->prev = pe->prev;
            pe->def = NULL;  // disable old edge
            (*count)++;
        }
    }
}
static inline void try_connecting_tiles (TileData* td, TileData* pd, uint* count) {
    for (TileEdge* te = td->edges; te < td->edges + MAX_EDGES; te++)
    for (TileEdge* pe = pd->edges; pe < pd->edges + MAX_EDGES; pe++) {
        try_connecting_edges(te, pe, count);
    }
}
static inline void try_merging_edges (TileEdge* te, TileEdge* ne, uint* count) {
    if (te->def) {  // skip disabled edges
        if (te->def == ne->def) {  // don't merge edges with different natures
            if (ang_diff((te->v2 - te->v1).ang(), (ne->v2 - ne->v1).ang()) < 0.01) {
                te->next = ne->next;
                te->next->prev = te;
                te->v2 = ne->v2;
                ne->def = NULL;  // disable next edge
                (*count)++;
            }
        }
    }
}
static inline void try_creating_edge (b2Body* b2body, TileEdge* te, uint* count) {
    if (te->def) {
        b2EdgeShape b2es;
        b2es.m_hasVertex0 = true;
        b2es.m_vertex0 = te->prev->v1;
        b2es.m_vertex1 = te->v1;
        b2es.m_vertex2 = te->v2;
        b2es.m_hasVertex3 = true;
        b2es.m_vertex3 = te->next->v2;
        b2FixtureDef b2fdf = te->def->nature.b2;
        b2fdf.shape = &b2es;
        b2fdf.userData = te->def;
        b2body->CreateFixture(&b2fdf);
        (*count)++;
    }
}

void Tilemap::start () {
     // Build up all the edges
    TileData* tilegeoms = new TileData [width * height];
    uint raw_count = 0;
    uint connect_count = 0;
    uint merge_count = 0;
    uint final_count = 0;
    for (uint y = 0; y < height; y++)
    for (uint x = 0; x < width; x++) {
        TileData* td = &tilegeoms[y*width + x];
        TileDef* def = tileset->tiles[tiles[y*width+x]];
        if (def) {
            uint n_edges = def->vertices.size();
             // Create the edges for this tile
            for (uint e = 0; e < n_edges; e++) {
                td->edges[e].v1 = def->vertices[e] + pos + Vec(x, y);
                td->edges[e].v2 = def->vertices[e+1 == n_edges ? e+1 : 0] + pos + Vec(x, y);
                td->edges[e].next = &td->edges[e+1 == n_edges ? e+1 : 0];
                td->edges[e].prev = &td->edges[e < 0 ? n_edges-1 : e-1];
                td->edges[e].def = def;
                raw_count++;
            }
             // Connect tiles
            if (x > 0)
                try_connecting_tiles(td, &tilegeoms[y*width + (x - 1)], &connect_count);
            if (y > 0)
                try_connecting_tiles(td, &tilegeoms[(y - 1)*width + x], &connect_count);
        }
    }
     // Merge edges with straight corners
    for (uint y = 0; y < height; y++)
    for (uint x = 0; x < width; x++)
    for (uint e = 0; e < MAX_EDGES; e++) {
        TileEdge* te = &tilegeoms[y*width + x].edges[e];
        try_merging_edges(te, te->next, &merge_count);
    }
     // The bodydef has already been manifested through phys::Object
     // Add the fixtures
    for (uint y = 0; y < height; y++)
    for (uint x = 0; x < width; x++)
    for (uint e = 0; e < MAX_EDGES; e++) {
        try_creating_edge(b2body, &tilegeoms[y*width + x].edges[e], &final_count);
    }
    tilemap_logger.log("Created tilemap...raw edges: %u, connections: %u, merges: %u, final edges: %u\n",
        raw_count, connect_count, merge_count, final_count
    );
     // And...we're done.
}

}  // namespace geo

