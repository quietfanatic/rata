#include "vis/inc/tiles.h"

#include "core/inc/commands.h"
#include "core/inc/opengl.h"
#include "hacc/inc/everything.h"
#include "vis/inc/common.h"
using namespace core;
using namespace util;

namespace vis {

    struct Tile_Vertex {
        uint16 px;  // None of these are normalized
        uint16 py;
        uint16 tx;  // divided by tex size in shader
        uint16 ty;
        Tile_Vertex (uint16 px, uint16 py, uint16 tx, uint16 ty) : px(px), py(py), tx(tx), ty(ty) { }
        Tile_Vertex () { }
    };

    void Tiles::finish () {
        if (tiles.size() != width * height) {
            throw hacc::X::Logic_Error(
                "Tiles of incorrect size: given " + std::to_string(tiles.size()) +
                " instead of " + std::to_string(width) + "x" + std::to_string(height)
            );
        }
        if (vbo_id) {
            glDeleteBuffers(1, &vbo_id);
        }
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
        vbo_size = vdat_i * 4;
        glGenBuffers(1, &vbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferData(GL_ARRAY_BUFFER, vbo_size * 4 * sizeof(Tile_Vertex), vdats, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        diagnose_opengl("after Tiles::finish");
        delete[] vdats;
    }
    Tiles::~Tiles () {
        if (vbo_id)
            glDeleteBuffers(1, &vbo_id);
    }

    void Tiles::resize (uint w, uint h) {
        std::vector<uint16> new_tiles (w * h);
        for (uint y = 0; y < h; y++)
        for (uint x = 0; x < w; x++) {
            if (x < width && y < height) {
                new_tiles[y*w+x] = tiles[y*width+x];
            }
            else {
                new_tiles[y*w+x] = 0;
            }
        }
        width = w;
        height = h;
        tiles = std::move(new_tiles);
        finish();
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
    };
    static Tiles_Program* prog = NULL;
    void tiles_init () {
        prog = hacc::File("vis/res/tiles.prog").attr("prog");
        hacc::manage(&prog);
    }

    void draw_tiles (Tiles* tiles, Texture* tex, Vec pos) {
        prog->use();
        glUniform2f(prog->model_pos, round(pos.x/PX)*PX, round(pos.y/PX)*PX);
        glUniform2f(prog->tileset_size, tex->size.x, tex->size.y);
        glBindTexture(GL_TEXTURE_2D, tex->id);
        glBindBuffer(GL_ARRAY_BUFFER, tiles->vbo_id);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
         // index, n_elements, type, normalize, stride, offset
        glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(Tile_Vertex), (void*)offsetof(Tile_Vertex, px));
        glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(Tile_Vertex), (void*)offsetof(Tile_Vertex, tx));
        glDrawArrays(GL_QUADS, 0, tiles->vbo_size);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        core::diagnose_opengl("after draw_tiles");
    }

    void draw_tile (uint16 tile, Texture* tex, Vec pos) {
        prog->use();
        glUniform2f(prog->model_pos, round(pos.x/PX)*PX, round(pos.y/PX)*PX);
        glUniform2f(prog->tileset_size, tex->size.x, tex->size.y);
        glBindTexture(GL_TEXTURE_2D, tex->id);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        Tile_Vertex verts [4];
        uint tx = (tile & 0x3fff) % 16 * 16;
        uint ty = (tile & 0x3fff) / 16 * 16;
        bool flipx = !!(tile & 0x8000);
        bool flipy = !!(tile & 0x4000);
        verts[0] = Tile_Vertex(0, 0, tx+(16* flipx), ty+(16*!flipy));
        verts[1] = Tile_Vertex(1, 0, tx+(16*!flipx), ty+(16*!flipy));
        verts[2] = Tile_Vertex(1, 1, tx+(16*!flipx), ty+(16* flipy));
        verts[3] = Tile_Vertex(0, 1, tx+(16* flipx), ty+(16* flipy));
        glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(Tile_Vertex), &verts[0].px);
        glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(Tile_Vertex), &verts[0].tx);
        glDrawArrays(GL_QUADS, 0, 4);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);
        core::diagnose_opengl("after draw_tile");
    }

} using namespace vis;

HACCABLE(Tiles) {
    name("vis::Tiles");
    attr("width", member(&Tiles::width));
    attr("height", member(&Tiles::height));
    attr("tiles", member(&Tiles::tiles));
    finish([](Tiles& v){ v.finish(); });
}

HACCABLE(Tiles_Program) {
    name("vis::Tiles_Program");
    delegate(base<core::Program>());
    finish(&Tiles_Program::finish);
}

namespace {

hacc::Special_Filetype _tiles_ft ("tiles",
    [] (std::string filename) -> hacc::Dynamic {
        std::string s = hacc::string_from_file(filename);
        if (s.size() == 0) {
            return hacc::Dynamic(std::move(Tiles()));
        }
        if (s.size() < 8) {
            throw hacc::X::Logic_Error(
                "\"" + filename + "\" isn't large enough for a .tiles file."
            );
        }
        if (s.size() % 2) {
            throw hacc::X::Logic_Error(
                "\"" + filename + "\" is not of even size (its size is " + std::to_string(s.size()) + ")"
            );
        }
        Tiles r;
        r.width = s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3];
        r.height = s[4] << 24 | s[5] << 16 | s[6] << 8 | s[7];
        if ((s.size() - 8) / 2 != r.width * r.height) {
            throw hacc::X::Logic_Error(
                "\"" + filename + "\" is not of correct size for its given width and height: " +
                std::to_string((s.size() - 8) / 2) + " != " + std::to_string(r.width) + " " + std::to_string(r.height)
            );
        }
        r.tiles.resize(r.width * r.height);
        for (uint i = 0; i < r.tiles.size(); i++) {
            r.tiles[i] = (s[8 + i*2] << 8) | (s[8 + i*2 + 1]);
        }
        r.finish();
        return hacc::Dynamic(std::move(r));
    },
    [] (std::string filename, const hacc::Dynamic& dyn) {
        Tiles* val = dyn.address();
        std::string s;
        s.resize(8 + val->tiles.size() * 2);
        s[0] = val->width >> 24;
        s[1] = val->width >> 16;
        s[2] = val->width >> 8;
        s[3] = val->width;
        s[4] = val->height >> 24;
        s[5] = val->height >> 16;
        s[6] = val->height >> 8;
        s[7] = val->height;
        for (uint i = 0; i < val->tiles.size(); i++) {
            s[8 + i*2] = val->tiles[i] >> 8;
            s[8 + i*2 + 1] = val->tiles[i];
        }
        hacc::string_to_file(s, filename);
    }
);

void _resize_tiles (Tiles* tiles, uint w, uint h) {
    if (tiles)
        tiles->resize(w, h);
}

core::New_Command _resize_tiles_cmd (
    "resize_tiles", "Resize a vis::Tiles object.  Will lose tiles if shrunk.",
    3, _resize_tiles
);

}
