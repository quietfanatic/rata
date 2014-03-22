#include "shell/inc/editing.h"

#include <string>
#include <sstream>
#include <GL/glfw.h>
#include "core/inc/commands.h"
#include "ent/inc/control.h"
#include "geo/inc/rooms.h"
#include "geo/inc/tiles.h"
#include "hacc/inc/documents.h"
#include "hacc/inc/files.h"
#include "hacc/inc/strings.h"
#include "shell/inc/main.h"
#include "util/inc/debug.h"
#include "vis/inc/color.h"
#include "vis/inc/text.h"
#include "vis/inc/tiles.h"

using namespace util;
using namespace vis;
using namespace core;
using namespace geo;

namespace shell {

    Tile_Editor* tile_editor = NULL;

    Tile_Editor::Tile_Editor () {
        tile_editor = this;
    }
    Tile_Editor::~Tile_Editor () {
        if (active) deactivate();
        if (tile_editor == this)
            tile_editor = NULL;
    }

    void Tile_Editor::activate () {
        log("editing", "Activating tile editor");
        Listener::activate();
        Drawn<vis::Map>::appear();
        clicking = false;
        showing_selector = false;
    }
    void Tile_Editor::deactivate () {
        log("editing", "Deactivating tile editor");
        Listener::deactivate();
        Drawn<vis::Map>::disappear();
        selector_camera.deactivate();
    }


    void Tile_Editor::Drawn_draw (vis::Map) {
        if (!tilemap || !tilemap->get_def()->tileset) return;
        auto def = tilemap->get_def();
        if (showing_selector) {
            size_t width = def->texture->size.x*PX;
            size_t size = def->tileset->tiles.size();
            for (size_t i = 0; i < size; i++) {
                draw_tile(
                    i | (tile & 0xc000), def->texture,
                    selector_pos + Vec(i % width, -(float)(i / width + 1))
                );
            }
        }
         // display current tile in corner
        draw_tile(tile, def->texture, camera->window_to_world(0, 0) - Vec(0, 1));
    }
    bool Tile_Editor::in_bounds (Vec pos) {
        auto def = tilemap->get_def();
        return pos.x >= 0 && pos.x < def->tiles->width
            && pos.y >= 0 && pos.y < def->tiles->height;
    }
    uint16& Tile_Editor::tile_at (Vec pos) {
        if (!in_bounds(pos)) {
            throw hacc::X::Logic_Error("Position (%f,%f) is out of bounds for this tilemap\n");
        }
        auto def = tilemap->get_def();
        size_t x = floor(pos.x);
        size_t y = def->tiles->height - floor(pos.y) - 1;
        return def->tiles->tiles[def->tiles->width * y + x];
    }
    void Tile_Editor::draw (Vec pos) {
        if (!in_bounds(pos))
            return;
        auto def = tilemap->get_def();
        uint16& selected = tile_at(pos);
        if (selected != tile) {
            selected = tile;
            def->tiles->finish();
             // Update all relevant tilemaps
            for (auto b = phys::space.b2world->GetBodyList(); b; b = b->GetNext()) {
                if (auto tm = dynamic_cast<Tilemap*>((phys::Object*)b->GetUserData())) {
                    if (tm->get_def()->tiles == def->tiles) {
                        tm->get_def()->tiles = def->tiles;
                        tm->finish();
                    }
                }
            }
        }
    }
    void Tile_Editor::pick (Vec pos) {
        auto def = tilemap->get_def();
        if (in_bounds(pos))
            tile = tile_at(pos);
        else if (def->tileset && def->texture
              && pos.x >= selector_pos.x && pos.x < (selector_pos.x + def->texture->size.x*PX)
              && pos.y <= selector_pos.y && pos.y > (selector_pos.y - def->texture->size.y*PX)) {
            uint16 index = pos.x - selector_pos.x
                         + (pos.y - selector_pos.y) / def->texture->size.y*PX;
            if (index < def->tileset->tiles.size()) {
                tile = (tile & 0xc000) | index;
            }
        }
    }
    bool Tile_Editor::Listener_button (int btn, int action) {
        if (!tilemap) return false;
        if (action == GLFW_PRESS) {
            Vec pos = camera->window_to_world(window->cursor_x, window->cursor_y) - tilemap->pos();
            if (btn == GLFW_MOUSE_BUTTON_LEFT) {
                draw(pos);
                clicking = true;
            }
            else if (btn == GLFW_MOUSE_BUTTON_RIGHT) {
                pick(pos);
            }
        }
        else {  // GLFW_RELEASE
            if (btn == GLFW_MOUSE_BUTTON_LEFT) {
                clicking = false;
            }
        }
        return true;
    }
    void Tile_Editor::Listener_cursor_pos (int x, int y) {
        if (!tilemap) return;
        if (clicking) {
            draw(camera->window_to_world(x, y) - tilemap->pos());
        }
    }
    bool Tile_Editor::Listener_key (int code, int action) {
        if (action == GLFW_PRESS) {
            auto def = tilemap->get_def();
            switch (code) {
                case GLFW_KEY_ESC:
                    if (showing_selector) {
                        showing_selector = false;
                        selector_camera.deactivate();
                    }
                    else {
                        deactivate();
                    }
                    return true;
                case 'S': {
                    for (auto& f : hacc::loaded_files()) {
                        if (f.data().address() == def->tiles) {
                            hacc::save(f);
                            return true;
                        }
                    }
                    fprintf(stderr, "Could not save this tiles object, because it doesn't belong to a file.\n");
                    return true;
                }
                case 'H':
                    tile ^= 0x8000;
                    return true;
                case 'V':
                    tile ^= 0x4000;
                    return true;
                case '/':
                    showing_selector = !showing_selector;
                    if (showing_selector) {
                        selector_camera.pos = selector_pos + Vec(10, -7.5);
                        selector_camera.size = Vec(40, 30);
                        selector_camera.activate();
                    }
                    else {
                        selector_camera.deactivate();
                    }
                    return true;
                case GLFW_KEY_LEFT:
                    if ((tile & 0x3fff) == 0) {
                        tile += def->tileset->tiles.size() - 1;
                    }
                    else {
                        tile -= 1;
                    }
                    return true;
                case GLFW_KEY_RIGHT:
                    if ((tile & 0x3fff) >= def->tileset->tiles.size() - 1) {
                        tile = 0;
                    }
                    else {
                        tile += 1;
                    }
                    return true;
                case GLFW_KEY_UP: {
                    Vec size = def->texture->size*PX;
                    if ((tile & 0x3fff) < size.y) {
                        tile += size.y - size.x;
                        if ((tile & 0x3fff) >= def->tileset->tiles.size() - 1) {
                            tile -= size.x;
                        }
                    }
                    else {
                        tile -= size.x;
                    }
                    return true;
                }
                case GLFW_KEY_DOWN: {
                    Vec size = def->texture->size*PX;
                    if ((tile & 0x3fff) >= size.y - size.x) {
                        tile -= size.y - size.x;
                    }
                    else if ((tile & 0x3fff) >= def->tileset->tiles.size() - size.x) {
                        tile -= size.y - size.x - size.x;
                    }
                    else {
                        tile += size.x;
                    }
                    return true;
                }
                default:
                    return false;
            }
        }
         // TODO: h and v flip current tile
        return false;
    }

    Texture_Tester* texture_tester;

    Texture_Tester::Texture_Tester () {
        texture_tester = this;
        static bool initted = false;
        if (!initted) {
            initted = true;
            hacc::manage(&texture_tester);
        }
    }
    Texture_Tester::~Texture_Tester () {
        if (texture_tester == this)
            texture_tester = NULL;
    }

    void Texture_Tester::activate () {
        deactivate();
        switch (layer) {
            case 0: Drawn<vis::Map>::appear(); break;
            case 1: Drawn<vis::Sprites>::appear(); break;
            case 3: Drawn<vis::Overlay>::appear(); break;
            default: return;
        }
        Listener::activate();
        camera.pos = pos + Vec(10, 7.5);
        camera.activate();
    }
    void Texture_Tester::deactivate () {
        Drawn<vis::Map>::disappear();
        Drawn<vis::Sprites>::disappear();
        Drawn<vis::Overlay>::disappear();
        Listener::deactivate();
        camera.deactivate();
    }

    void Texture_Tester::Drawn_draw (vis::Map) {
        if (!tex) return;
        draw_texture(tex, Rect(pos, pos + tex->size*PX));
    }
    void Texture_Tester::Drawn_draw (vis::Sprites) {
        if (!tex) return;
        draw_texture(tex, Rect(pos, pos + tex->size*PX));
    }
    void Texture_Tester::Drawn_draw (vis::Overlay) {
        if (!tex) return;
        draw_texture(tex, Rect(pos, pos + tex->size*PX));
    }

    bool Texture_Tester::Listener_key (int code, int action) {
        if (action == GLFW_PRESS && code == GLFW_KEY_ESC) {
            deactivate();
            return true;
        }
        return false;
    }

} using namespace shell;

HACCABLE(Tile_Editor) {
    name("shell::Tile_Editor");
}

HACCABLE(Texture_Tester) {
    name("shell::Texture_Tester");
    attr("tex", member(&Texture_Tester::tex).optional());
    attr("layer", member(&Texture_Tester::layer).optional());
}

void _texture_test (Texture* tex, uint layer) {
    if (!texture_tester) return;
    if (texture_tester->active
     && texture_tester->tex == tex
     && texture_tester->layer == layer) {
        texture_tester->deactivate();
    }
    else {
        texture_tester->tex = tex;
        texture_tester->layer = layer;
        texture_tester->activate();
    }
}
New_Command _texture_test_cmd ("texture_test", "Show a texture on the given-numbered layer.", 2, _texture_test);

