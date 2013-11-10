#include "../inc/tiles.h"
#include "../inc/common.h"
#include "../../core/inc/opengl.h"
#include "../../hacc/inc/everything.h"

namespace vis {
    using namespace core;

    struct Tile_Vertex {
        uint16 px;  // None of these are normalized
        uint16 py;
        uint16 tx;  // divided by tex size in shader
        uint16 ty;
        Tile_Vertex (uint16 px, uint16 py, uint16 tx, uint16 ty) : px(px), py(py), tx(tx), ty(ty) { }
        Tile_Vertex () { }
    };

    void Tiles::finish (uint32 width, uint32 height, const uint16* tiles) {
        auto vdats = new Tile_Vertex [height * width][4];
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
                vdats[vdat_i][0] = Tile_Vertex(px+0, py+0, tx+(16* flipx), ty+(16*!flipy));
                vdats[vdat_i][1] = Tile_Vertex(px+1, py+0, tx+(16*!flipx), ty+(16*!flipy));
                vdats[vdat_i][2] = Tile_Vertex(px+1, py+1, tx+(16*!flipx), ty+(16* flipy));
                vdats[vdat_i][3] = Tile_Vertex(px+0, py+1, tx+(16* flipx), ty+(16* flipy));
                vdat_i++;
            }
        }
        vao_size = vdat_i * 4;
        glGenBuffers(1, &vbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferData(GL_ARRAY_BUFFER, vao_size * 4 * sizeof(Tile_Vertex), vdats, GL_STATIC_DRAW);
        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
         // index, n_elements, type, normalize, stride, offset
        glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(Tile_Vertex), (void*)offsetof(Tile_Vertex, px));
        glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(Tile_Vertex), (void*)offsetof(Tile_Vertex, tx));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        delete[] vdats;
    }

    struct Tiles_Program : Cameraed_Program {
        GLint model_pos = 0;
        GLint tileset_size = 0;
        GLint tex = 0;
        void finish () {
            Cameraed_Program::finish();
            model_pos = require_uniform("model_pos");
            tileset_size = require_uniform("tileset_size");
            tex = require_uniform("tex");
            glUniform1i(tex, 0);
        }
        void Program_begin () override {
            Cameraed_Program::Program_begin();
        }
    };
    static Tiles_Program* prog = NULL;
    void tiles_init () {
        prog = hacc::File("vis/res/tiles.prog").data().attr("prog");
        hacc::manage(&prog);
    }

    void draw_tiles (Tiles* tiles, Texture* tex, Vec pos) {
        prog->use();
        glUniform2f(prog->model_pos, pos.x, pos.y);
        glUniform2f(prog->tileset_size, tex->size.x, tex->size.y);
        glBindTexture(GL_TEXTURE_2D, tex->id);
        glBindVertexArray(tiles->vao_id);
        glDrawArrays(GL_QUADS, 0, tiles->vao_size);
        core::diagnose_opengl("after draw_tiles");
    }

} using namespace vis;

HCB_BEGIN(Tiles_Program)
    name("vis::Tiles_Program");
    delegate(base<core::Program>());
    finish(&Tiles_Program::finish);
HCB_END(Tiles_Program)

