
#include <stdexcept>
#include "../../hacc/inc/everything.h"
#include "../../util/inc/math.h"
#include "../../util/inc/debug.h"
#include "../../vis/inc/shaders.h"
#include "../../core/inc/game.h"
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
    float angle = ang_diff((te->v2 - te->v1).ang(), (ne->v2 - ne->v1).ang());
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

void Tilemap::start () {
     // Build up all the edges
    auto es = new TileEdge [height][width][MAX_EDGES];
    uint initial = 0;
    uint cancelled = 0;
    uint merged = 0;
    uint eaten = 0;
    uint final = 0;
    for (uint y = 0; y < height; y++)
    for (uint x = 0; x < width; x++) {
        TileDef* def = tileset->tiles[tiles[y*width+x]];
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
    set_pos(pos);
     // Now for the graphics buffers
    using vis::glproc;
    static auto glGenBuffers = glproc<void (GLsizei, GLuint*)>("glGenBuffers");
    static auto glBindBuffer = glproc<void (GLenum, GLuint)>("glBindBuffer");
    static auto glBufferData = glproc<void (GLenum, GLsizeiptr, const GLvoid*, GLenum)>("glBufferData");
    static auto glGenVertexArrays = glproc<void (GLsizei, GLuint*)>("glGenVertexArrays");
    static auto glBindVertexArray = glproc<void (GLuint)>("glBindVertexArray");
    static auto glEnableVertexAttribArray = glproc<void (GLuint)>("glEnableVertexAttribArray");
    static auto glVertexAttribPointer = glproc<void (GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*)>("glVertexAttribPointer");
    auto vdats = new Tilemap_Vertex [height * width][4];
    uint vdat_i = 0;
    for (uint y = 0; y < height; y++)
    for (uint x = 0; x < width; x++) {
        if (uint tile = tiles[y*width+x]) {
            uint px = x;
            uint py = height - y - 1;
            uint tx = (tile & 0x3fff) % 16 * 16;
            uint ty = (tile & 0x3fff) / 16 * 16;
            vdats[vdat_i][0] = Tilemap_Vertex(px+0, py+0, tx+0, ty+0);
            vdats[vdat_i][1] = Tilemap_Vertex(px+1, py+0, tx+16, ty+0);
            vdats[vdat_i][2] = Tilemap_Vertex(px+1, py+1, tx+16, ty+16);
            vdats[vdat_i][3] = Tilemap_Vertex(px+0, py+1, tx+0, ty+16);
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
     // Done.
    delete[] vdats;

}

 // Now for drawing tilemaps.

static vis::Program* tiles_program;

struct Tilemap_Layer : core::Layer {
    Tilemap_Layer () : core::Layer("E.M", "tilemaps") { }
    void init () {
        static auto glUniform1i = vis::glproc<void (GLint, GLint)>("glUniform1i");
        static auto glUniform2f = vis::glproc<void (GLint, GLfloat, GLfloat)>("glUniform2f");
        tiles_program = hacc::reference_file<vis::Program>("modules/geo/res/tiles.prog");
        tiles_program->use();
        int tex_uni = tiles_program->require_uniform("tex");
        glUniform1i(tex_uni, 0);  // Texture unit 0
        camera_pos = tiles_program->require_uniform("camera_pos");
        glUniform2f(camera_pos, 10, 7.5);  // TODO make this dynamic
        if (vis::diagnose_opengl("after setting uniform")) {
            throw std::logic_error("tilemaps layer init failed due to GL error");
        }
        model_pos = tiles_program->require_uniform("model_pos");
        tileset_size = tiles_program->require_uniform("tileset_size");
    }
    int camera_pos;
    int model_pos;
    int tileset_size;
    void run () {
        static auto glUniform2f = vis::glproc<void (GLint, GLfloat, GLfloat)>("glUniform2f");
        static auto glBindVertexArray = vis::glproc<void (GLuint)>("glBindVertexArray");
        tiles_program->use();
        for (Tilemap* map = active_tilemaps.first(); map; map = map->next()) {
            Vec pos = map->Object::pos();
            glUniform2f(model_pos, pos.x, pos.y);
            Vec ts = map->texture->size;
            glUniform2f(tileset_size, ts.x, ts.y);
            glBindTexture(GL_TEXTURE_2D, map->texture->tex);
            glBindVertexArray(map->vao_id);
            glDrawArrays(GL_QUADS, 0, map->vao_size);
//            diagnose_opengl("After rendering a tilemap");
        }
    }
} tilemap_layer;



}  // namespace geo

