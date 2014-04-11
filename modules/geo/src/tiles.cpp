#include "geo/inc/tiles.h"

#include <stdexcept>
#include "core/inc/opengl.h"
#include "hacc/inc/everything.h"
#include "util/inc/debug.h"
#include "util/inc/geometry.h"
using namespace phys;
using namespace util;

namespace geo {

    void Tilemap::finish () {
        Object::finish();
        Resident::finish();
        physicalize();
    }

    void Tilemap::Resident_emerge () {
        materialize();
        appear();
    }
    void Tilemap::Resident_reclude () {
        dematerialize();
        disappear();
    }

    void Tilemap::Drawn_draw (vis::Map) {
        auto def = get_def();
        if (!def->tiles || !def->texture) return;
        vis::draw_tiles(def->tiles, def->texture, Object::get_pos());
    }


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

    static bool vecs_are_close (Vec a, Vec b) {
        return (a.x - b.x)*(a.x - b.x)
             + (a.y - b.y)*(a.y - b.y)
             < 0.01;
    }
    static void connect_edges (TileEdge* te, TileEdge* pe, uint* cancelled) {
        if (vecs_are_close(te->v1, pe->v2)) {
            te->prev = pe;
            pe->next = te;
        }
        if (vecs_are_close(te->v2, pe->v1)) {
            te->next = pe;
            pe->prev = te;
        }
        if (te->next == pe && te->prev == pe) {
            te->def = NULL;
            pe->def = NULL;
            *cancelled += 2;
        }
    }
    static bool merge_edges (TileEdge* te, uint* merged, uint* eaten) {
        TileEdge* ne = te->next;
        if (ne == te)
            throw std::logic_error("The tile edge merging algorithm went wrong somewhere.\n");
        float ang = angle_diff(angle(te->v2 - te->v1), angle(ne->v2 - ne->v1));
        if (ang < 0.01 && te->def == ne->def) {
             // merge edges only with the same nature
            te->next = ne->next;
            te->next->prev = te;
            te->v2 = ne->v2;
            ne->def = NULL;
            (*merged)++;
            return true;
        }
        else if (ang > PI - 0.01) {
             // Different natured edges can eat one another though
            te->next = ne->next;
            te->next->prev = te;
            te->v2 = ne->v2;
            if (length2(te->v2 - te->v1) < length2(ne->v2 - ne->v1)) {
                te->def = ne->def;
            }
            ne->def = NULL;
            (*eaten)++;
            return true;
        }
        return false;
    }
    static void create_edge (b2Body* b2body, TileEdge* te, uint* final) {
        b2EdgeShape b2es;
        b2es.m_hasVertex0 = true;
        b2es.m_vertex0 = te->prev->v1;
        b2es.m_vertex1 = te->v1;
        b2es.m_vertex2 = te->v2;
        b2es.m_hasVertex3 = true;
        b2es.m_vertex3 = te->next->v2;
        b2FixtureDef b2fdf = te->def->nature.b2;
        b2fdf.filter = phys::Filter();  // Default filter should be okay
        b2fdf.shape = &b2es;
        b2fdf.userData = te->def;
        b2body->CreateFixture(&b2fdf);
        (*final)++;
    }

    void Tilemap::physicalize () {
        auto def = get_def();
        if (!def->tiles || !def->tileset) return;
        auto width = def->tiles->width;
        auto height = def->tiles->height;
        auto es = new TileEdge [height * width][MAX_EDGES];
        uint initial = 0;
        uint cancelled = 0;
        uint merged = 0;
        uint eaten = 0;
        uint final = 0;
        for (uint y = 0; y < height; y++)
        for (uint x = 0; x < width; x++) {
            TileDef* td = def->tileset->tiles.at(def->tiles->tiles[y*width+x] & 0x3fff);
            if (td) {
                uint n_edges = td->vertices.size();
                 // Create the edges for this tile
                for (uint e = 0; e < n_edges; e++) {
                    es[y * width + x][e].v1 = td->vertices[e] + Vec(x, height - y - 1);
                    es[y * width + x][e].v2 = td->vertices[e+1 == n_edges ? 0 : e+1] + Vec(x, height - y - 1);
                    es[y * width + x][e].next = &es[y * width + x][e+1 == n_edges ? 0 : e+1];
                    es[y * width + x][e].prev = &es[y * width + x][e == 0 ? n_edges-1 : e-1];
                    es[y * width + x][e].def = td;
                    initial++;
                     // Connect edges
                    if (x > 0)
                        for (uint pe = 0; pe < MAX_EDGES; pe++)
                            if (es[y * width + x-1][pe].def)
                                connect_edges(&es[y * width + x][e], &es[y * width + x-1][pe], &cancelled);
                    if (y > 0)
                        for (uint pe = 0; pe < MAX_EDGES; pe++)
                            if (es[(y-1) * width + x][pe].def)
                                connect_edges(&es[y * width + x][e], &es[(y-1) * width + x][pe], &cancelled);
                }
            }
        }
         // Merge edges
        for (uint y = 0; y < height; y++)
        for (uint x = 0; x < width; x++)
        for (uint e = 0; e < MAX_EDGES; e++) {
            if (es[y * width + x][e].def) {
                while(merge_edges(&es[y * width + x][e], &merged, &eaten)) { }
            }
        }
         // The bodydef has already been manifested through phys::Object
         // Add the fixtures
        for (uint y = 0; y < height; y++)
        for (uint x = 0; x < width; x++)
        for (uint e = 0; e < MAX_EDGES; e++) {
            if (es[y * width + x][e].def) {
                create_edge(b2body, &es[y * width + x][e], &final);
            }
        }
        log("tilemap", "Optimized tilemap edges.  initial:%u cancelled:%u merged:%u eaten:%u final:%u",
            initial, cancelled, merged, eaten, final
        );
         // And...we're done with the edges.
        delete[] es;
    }

} using namespace geo;

HACCABLE(TileDef) {
    name("geo::TileDef");
    attr("nature", member(&TileDef::nature));
    attr("vertices", member(&TileDef::vertices));
}

HACCABLE(Tileset) {
    name("geo::Tileset");
    attr("tiles", member(&Tileset::tiles));
}

HACCABLE(Tilemap_Def) {
    name("geo::Tilemap_Def");
    attr("Object_Def", base<Object_Def>().collapse());
    attr("tileset", member(&Tilemap_Def::tileset));
    attr("texture", member(&Tilemap_Def::texture));
    attr("tiles", member(&Tilemap_Def::tiles).optional());
}

HACCABLE(Tilemap) {
    name("geo::Tilemap");
    attr("Resident", base<Resident>().collapse());
    attr("Object", base<phys::Object>().optional().collapse());
    finish(&Tilemap::finish);
}


