#include "../inc/tiles.h"
#include "../inc/common.h"
#include "../../core/inc/opengl.h"
#include "../../core/inc/phases.h"
#include "../../hacc/inc/files.h"

namespace vis {
    using namespace core;

    Links<Tiles> tileses;
    void Tiles::appear () { link(tileses); }
    void Tiles::disappear () { unlink(); }

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

    struct Tiles_Renderer::Data {
        Program* program = hacc::File("modules/vis/res/tiles.prog").data().attr("prog");
        GLint tex = program->require_uniform("tex");
        GLint camera_pos = program->require_uniform("camera_pos");
        GLint model_pos = program->require_uniform("model_pos");
        GLint tileset_size = program->require_uniform("tileset_size");

        Data () {
            glUseProgram(program->glid);
            glUniform1i(tex, 0);  // Texture unit 0
            if (core::diagnose_opengl("after creating tilemap renderer")) {
                throw std::logic_error("tilemaps layer init failed due to GL error");
            }
        }
    };

    Tiles_Renderer::Tiles_Renderer () : data(new Data) { }

    void Tiles_Renderer::run () {
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glUseProgram(data->program->glid);
        glUniform2f(data->camera_pos, vis::camera_pos.x, vis::camera_pos.y);
        for (Tiles* t = tileses.first(); t; t = t->next()) {
            Vec pos = t->Tiles_pos();
            glUniform2f(data->model_pos, pos.x, pos.y);
            auto tex = t->Tiles_texture();
            Vec ts = tex->size;
            glUniform2f(data->tileset_size, ts.x, ts.y);
            glBindTexture(GL_TEXTURE_2D, tex->id);
            glBindVertexArray(t->vao_id);
            glDrawArrays(GL_QUADS, 0, t->vao_size);
            core::diagnose_opengl("After rendering a tilemap");
        }
    }

}
