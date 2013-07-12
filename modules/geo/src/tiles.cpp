
#include <stdexcept>
#include "../inc/tiles.h"
#include "../../hacc/inc/everything.h"
#include "../../util/inc/geometry.h"
#include "../../util/inc/debug.h"
#include "../../core/inc/opengl.h"
#include "../../core/inc/phases.h"
#include "../../vis/inc/sprites.h"

namespace geo {
    using namespace util;

    static phys::BodyDef*& tilemap_bdf () {
        static phys::BodyDef* r = hacc::File("modules/geo/res/tilemap.bdf").data();
        return r;
    }

    static Logger tilemap_logger ("tilemap");

    Links<Tilemap> active_tilemaps;

    Tilemap::Tilemap () : phys::Object() { tilemap_bdf(); }

    void Tilemap::Resident_emerge () {
        materialize();
        Linkable<Tilemap>::link(active_tilemaps);
    }
    void Tilemap::Resident_reclude () {
        dematerialize();
        Linkable<Tilemap>::unlink();
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
        float angle = angle_diff((te->v2 - te->v1).ang(), (ne->v2 - ne->v1).ang());
        if (angle < 0.01 && te->def == ne->def) {
             // merge edges only with the same nature
            te->next = ne->next;
            te->next->prev = te;
            te->v2 = ne->v2;
            ne->def = NULL;
            (*merged)++;
            return true;
        }
        else if (angle > PI - 0.01) {
             // Different natured edges can eat one another though
            te->next = ne->next;
            te->next->prev = te;
            te->v2 = ne->v2;
            if ((te->v2 - te->v1).mag2() < (ne->v2 - ne->v1).mag2()) {
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
        b2fdf.shape = &b2es;
        b2fdf.userData = te->def;
        b2body->CreateFixture(&b2fdf);
        (*final)++;
    }

    struct Tilemap_Vertex {
        uint16 px;  // None of these are normalized
        uint16 py;
        uint16 tx;  // divided by tex size in shader
        uint16 ty;
        Tilemap_Vertex (uint16 px, uint16 py, uint16 tx, uint16 ty) : px(px), py(py), tx(tx), ty(ty) { }
        Tilemap_Vertex () { }
    };

    void Tilemap::finish () {
        apply_bdf(tilemap_bdf());
         // Build up all the edges
        auto es = new TileEdge [height][width][MAX_EDGES];
        uint initial = 0;
        uint cancelled = 0;
        uint merged = 0;
        uint eaten = 0;
        uint final = 0;
        if (tiles.size() != width * height) {
            throw hacc::X::Logic_Error(
                "Tilemap has wrong number of tiles (" + std::to_string(tiles.size())
              + " != " + std::to_string(width * height)
              + " == " + std::to_string(width)
              + " * " + std::to_string(height)
              + ")"
            );
        }
        for (uint y = 0; y < height; y++)
        for (uint x = 0; x < width; x++) {
            TileDef* def = tileset->tiles.at(tiles[y*width+x] & 0x3fff);
            if (def) {
                uint n_edges = def->vertices.size();
                 // Create the edges for this tile
                for (uint e = 0; e < n_edges; e++) {
                    es[y][x][e].v1 = def->vertices[e] + Vec(x, height - y - 1);
                    es[y][x][e].v2 = def->vertices[e+1 == n_edges ? 0 : e+1] + Vec(x, height - y - 1);
                    es[y][x][e].next = &es[y][x][e+1 == n_edges ? 0 : e+1];
                    es[y][x][e].prev = &es[y][x][e == 0 ? n_edges-1 : e-1];
                    es[y][x][e].def = def;
                    initial++;
                     // Connect edges
                    if (x > 0)
                        for (uint pe = 0; pe < MAX_EDGES; pe++)
                            if (es[y][x-1][pe].def)
                                connect_edges(&es[y][x][e], &es[y][x-1][pe], &cancelled);
                    if (y > 0)
                        for (uint pe = 0; pe < MAX_EDGES; pe++)
                            if (es[y-1][x][pe].def)
                                connect_edges(&es[y][x][e], &es[y-1][x][pe], &cancelled);
                }
            }
        }
         // Merge edges
        for (uint y = 0; y < height; y++)
        for (uint x = 0; x < width; x++)
        for (uint e = 0; e < MAX_EDGES; e++) {
            if (es[y][x][e].def) {
                while(merge_edges(&es[y][x][e], &merged, &eaten));
            }
        }
         // The bodydef has already been manifested through phys::Object
         // Add the fixtures
        for (uint y = 0; y < height; y++)
        for (uint x = 0; x < width; x++)
        for (uint e = 0; e < MAX_EDGES; e++) {
            if (es[y][x][e].def) {
                create_edge(b2body, &es[y][x][e], &final);
            }
        }
        tilemap_logger.log("Optimized tilemap edges.  initial:%u cancelled:%u merged:%u eaten:%u final:%u",
            initial, cancelled, merged, eaten, final
        );
         // And...we're done with the edges.
        delete[] es;
         // Now for the graphics buffers
        auto vdats = new Tilemap_Vertex [height * width][4];
        uint vdat_i = 0;
        for (uint y = 0; y < height; y++)
        for (uint x = 0; x < width; x++) {
            if (uint tile = tiles[y*width+x]) {
                uint px = x;
                uint py = height - y - 1;
                uint tx = (tile & 0x3fff) % 16 * 16;
                uint ty = (tile & 0x3fff) / 16 * 16;
                bool flipx = !!(tile & 0x8000);
                bool flipy = !!(tile & 0x4000);
                vdats[vdat_i][0] = Tilemap_Vertex(px+0, py+0, tx+(16* flipx), ty+(16*!flipy));
                vdats[vdat_i][1] = Tilemap_Vertex(px+1, py+0, tx+(16*!flipx), ty+(16*!flipy));
                vdats[vdat_i][2] = Tilemap_Vertex(px+1, py+1, tx+(16*!flipx), ty+(16* flipy));
                vdats[vdat_i][3] = Tilemap_Vertex(px+0, py+1, tx+(16* flipx), ty+(16* flipy));
                vdat_i++;
            }
        }
        vao_size = vdat_i * 4;
        glGenBuffers(1, &vbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferData(GL_ARRAY_BUFFER, vao_size * 4 * sizeof(Tilemap_Vertex), vdats, GL_STATIC_DRAW);
        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
         // index, n_elements, type, normalize, stride, offset
        glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(Tilemap_Vertex), (void*)offsetof(Tilemap_Vertex, px));
        glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(Tilemap_Vertex), (void*)offsetof(Tilemap_Vertex, tx));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
         // Done.
        delete[] vdats;

    }

     // Now for drawing tilemaps.

    struct Tilemap_Layer : core::Layer, core::Renderer {
        core::Program* program;
        int tex;
        int camera_pos;
        int model_pos;
        int tileset_size;

        Tilemap_Layer () : core::Layer("E.M", "tilemaps") { }
        void Layer_start () override {
            program = hacc::File("modules/vis/res/tiles.prog").data().attr("prog");
            tex = program->require_uniform("tex");
            camera_pos = program->require_uniform("camera_pos");
            model_pos = program->require_uniform("model_pos");
            tileset_size = program->require_uniform("tileset_size");
            glUseProgram(program->glid);
            glUniform1i(tex, 0);  // Texture unit 0
            if (core::diagnose_opengl("after creating tilemap renderer")) {
                throw std::logic_error("tilemaps layer init failed due to GL error");
            }
        }

         // Renderer
        void start_rendering () override {
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            glUseProgram(program->glid);
            glUniform2f(camera_pos, vis::camera_pos.x, vis::camera_pos.y);
        }
        void Layer_run () override {
            use();
            for (Tilemap* map = active_tilemaps.first(); map; map = map->Linkable<Tilemap>::next()) {
                Vec pos = map->Object::pos();
                glUniform2f(model_pos, pos.x, pos.y);
                Vec ts = map->texture->size;
                glUniform2f(tileset_size, ts.x, ts.y);
                glBindTexture(GL_TEXTURE_2D, map->texture->tex);
                glBindVertexArray(map->vao_id);
                glDrawArrays(GL_QUADS, 0, map->vao_size);
                core::diagnose_opengl("After rendering a tilemap");
            }
        }
    } tilemap_layer;

} using namespace geo;

HCB_BEGIN(TileDef)
    name("geo::TileDef");
    attr("nature", member(&TileDef::nature));
    attr("vertices", member(&TileDef::vertices));
HCB_END(TileDef)

HCB_BEGIN(Tileset)
    name("geo::Tileset");
    attr("tiles", member(&Tileset::tiles));
HCB_END(Tileset)

HCB_BEGIN(Tilemap)
    name("geo::Tilemap");
    attr("Resident", base<Resident>());
    attr("Object", base<phys::Object>().optional());
    attr("tileset", member(&Tilemap::tileset));
    attr("texture", member(&Tilemap::texture));
    attr("width", member(&Tilemap::width));
    attr("height", member(&Tilemap::height));
    attr("tiles", member(&Tilemap::tiles));
    finish([](Tilemap& t){
        t.Resident::finish();
        t.finish();
    });
HCB_END(Tilemap)




