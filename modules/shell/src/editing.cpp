#include <sstream>
#include "../inc/editing.h"
#include "../inc/main.h"
#include "../../hacc/inc/files.h"
#include "../../hacc/inc/documents.h"
#include "../../hacc/inc/strings.h"
#include "../../core/inc/commands.h"
#include "../../vis/inc/color.h"
#include "../../util/inc/debug.h"
#include "../../ent/inc/control.h"

using namespace util;
using namespace vis;
using namespace core;
using namespace geo;

namespace shell {

    Logger logger ("editing");

    Room_Editor* room_editor = NULL;
    Tile_Editor* tile_editor = NULL;

     // Get pointer of most derived type from Resident*
    hacc::Pointer res_realp (Resident* res) {
        if (!res) return nullptr;
        void* derived = hacc::Pointer(res).address_of_type(hacc::Type(typeid(*res)));
        return hacc::Pointer(hacc::Type(typeid(*res)), derived);
    }

     // Draw outlines around all residents
    void Room_Editor::Drawn_draw (Overlay) {
        size_t unpositioned_residents = 0;
        for (auto& room : all_rooms()) {
            color_offset(Vec(0, 0));
            draw_color(0xff00ffff);
            draw_rect(room.boundary);
            if (tile_editor && tile_editor->active)
                continue;
            if (room.observer_count) {
                for (auto& res : room.residents) {
                    Vec pos = res.Resident_get_pos();
                    const Rect& boundary = res.Resident_boundary();
                    if (!pos.is_defined() || !boundary.is_defined()) {
                        pos = room.boundary.lt() + Vec(0.5, -0.5);
                        pos.x += unpositioned_residents++;
                    }
                    color_offset(pos);
                    draw_color(&res == selected ? 0xff0000ff : 0x00ff00ff);
                    draw_rect(boundary);
                }
            }
        }
    }

     // Draw status messages and other info
    void Room_Editor::Drawn_draw (Dev) {
        if (!status.empty()) {
            color_offset(Vec(0, 0));
            draw_color(0x0000007f);
            Vec size = text_size(status, font);
            draw_solid_rect(Rect(0, size.y, size.x, 0));
            draw_text(status, font, Vec(0, 0), Vec(1, -1));
        }
         // Show position in various coordinate spaces
        Vec world_pos = camera->window_to_world(window->cursor_x, window->cursor_y);
        Vec hud_pos = camera->window_to_hud(window->cursor_x, window->cursor_y);
        Vec dev_pos = camera->window_to_dev(window->cursor_x, window->cursor_y);
        draw_text(
            "window: " + std::to_string(window->cursor_x) +
                  ", " + std::to_string(window->cursor_y) + "\n" +
            "world: " + std::to_string(world_pos.x) +
                 ", " + std::to_string(world_pos.y) + "\n" +
            "hud: " + std::to_string(hud_pos.x) +
               ", " + std::to_string(hud_pos.y) + "\n" +
            "dev: " + std::to_string(dev_pos.x) +
               ", " + std::to_string(dev_pos.y),
            font, camera->window_to_dev(window->width, 0),
            Vec(-1, 1)
        );
    }

     // Process cursor position; hover, drag
    void Room_Editor::Listener_cursor_pos (int x, int y) {
        Vec world_pos = camera->window_to_world(x, y);
        if (clicking) {
            if (selected) {
                if ((drag_origin - world_pos).mag2() > 0.2)
                    dragging = true;
                if (dragging) {
                    selected->Resident_set_pos(world_pos - drag_offset);
                }
            }
        }
        else {
             // Prefer residents with lower tops
            float lowest_res_t = INF;
            Resident* lowest_res = NULL;
            float lowest_room_t = INF;
            Room* lowest_room = NULL;
            size_t unpositioned_residents = 0;
            for (auto& room : all_rooms()) {
                if (room.observer_count) {
                    if (room.boundary.covers(world_pos)) {
                        if (room.boundary.t < lowest_room_t) {
                            lowest_room_t = room.boundary.t;
                            lowest_room = &room;
                        }
                    }
                    for (auto& res : room.residents) {
                        Vec pos = res.Resident_get_pos();
                        const Rect& boundary = res.Resident_boundary();
                        if (!pos.is_defined() || !boundary.is_defined()) {
                            pos = room.boundary.lt() + Vec(0.5, -0.5);
                            pos.x += unpositioned_residents++;
                        }
                        if (boundary.covers(world_pos - pos)) {
                            if (boundary.t < lowest_res_t) {
                                lowest_res_t = boundary.t;
                                lowest_res = &res;
                            }
                        }
                    }
                }
            }
             // Set status message about hovered object
            if (lowest_res) {
                if (lowest_res != hovering) {
                    auto realp = res_realp(lowest_res);
                    auto path = hacc::address_to_path(realp);
                    std::ostringstream ss;
                    ss << realp.address << " " << hacc::path_to_string(path);
                    status = ss.str();
                }
            }
            else {
                status = "";
            }
            hovering = lowest_res;
            hovering_room = lowest_room;
        }
    }
     // Clicking behavior
    bool Room_Editor::Listener_button (int code, int action) {
        if (action == GLFW_PRESS) {
            Vec realpos = camera->window_to_world(window->cursor_x, window->cursor_y);
            dragging = NULL;
             // Just upgrade hovering to selected
            selected = hovering;
            selected_room = hovering_room;
            if (selected) {
                Vec pos = selected->Resident_get_pos();
                if (code == GLFW_MOUSE_BUTTON_LEFT) {
                    drag_origin = pos;
                    drag_offset = realpos - pos;
                    logger.log("Selected " + hacc::Reference(res_realp(selected)).show());
                    clicking = true;
                }
                else if (code == GLFW_MOUSE_BUTTON_RIGHT) {
                    if (action == GLFW_PRESS) {
                        selected_type_name = res_realp(selected).type.name();
                        menu_world_pos = camera->window_to_world(window->cursor_x, window->cursor_y);
                        Vec area = camera->window_to_dev(window->width, 0);
                        res_menu->size = res_menu->root->Menu_Item_size(area);
                        Vec pos = camera->window_to_dev(window->cursor_x, window->cursor_y);
                        res_menu->pos = pos - Vec(-1*PX, res_menu->size.y + 1*PX);
                        res_menu->activate();
                    }
                }
                return true;
            }
            else if (selected_room) {
                if (code == GLFW_MOUSE_BUTTON_RIGHT) {
                    if (action == GLFW_PRESS) {
                        menu_world_pos = camera->window_to_world(window->cursor_x, window->cursor_y);
                        Vec area = camera->window_to_dev(window->width, 0);
                        room_menu->size = room_menu->root->Menu_Item_size(area);
                        Vec pos = camera->window_to_dev(window->cursor_x, window->cursor_y);
                        room_menu->pos = pos - Vec(-1*PX, room_menu->size.y + 1*PX);
                        room_menu->activate();
                    }
                }
            }
        }
        else {  // GLFW_RELEASE
            if (code == GLFW_MOUSE_BUTTON_LEFT) {
                dragging = false;
                clicking = false;
            }
            return true;
        }
        return false;
    }
    bool Room_Editor::Listener_key (int code, int action) {
        if (action == GLFW_PRESS && code == GLFW_KEY_ESC) {
            deactivate();
            return true;
        }
        return false;
    }

    Room_Editor::Room_Editor () :
        res_menu(hacc::File("shell/res/re_menus.hacc").data().attr("res_menu")),
        room_menu(hacc::File("shell/res/re_menus.hacc").data().attr("room_menu"))
    {
        room_editor = this;
    }
    Room_Editor::~Room_Editor () {
        if (active) deactivate();
        if (room_editor == this)
            room_editor = NULL;
    }

    void Room_Editor::activate () {
        logger.log("Activating room editor.");
        Listener::activate();
        fc.pos = camera->Camera_pos();
        fc.size = Vec(40, 30);
        fc.activate();
        Drawn<Overlay>::appear();
        Drawn<Dev>::appear();
    }
    void Room_Editor::deactivate () {
        logger.log("Deactivating room editor.");
        hovering = NULL;
        selected = NULL;
        dragging = false;
        clicking = false;
        Listener::deactivate();
        fc.deactivate();
        Drawn<Overlay>::disappear();
        Drawn<Dev>::disappear();
        res_menu->deactivate();
        room_menu->deactivate();
    }

    static void general_edit (hacc::Reference ref) {
        auto tree = ref.to_tree();
        auto tmp = tmpnam(NULL);
        auto str = " // Editing " + ref.show() + "\n"
                 + hacc::tree_to_string(tree, tmp, 3);
        hacc::string_to_file(str, tmp);
        auto editor = (const char*)getenv("EDITOR");
        if (!editor) {
            if (system("which gedit") == 0)
                editor = "gedit";
            else if (system("which mate-text-editor") == 0)
                editor = "mate-text-editor";
            else
                editor = "vim";
        }
        system((std::string(editor) + " " + tmp).c_str());
        auto new_str = hacc::string_from_file(tmp);
        if (new_str != str) {
            logger.log("Updating");
            ref.from_tree(hacc::tree_from_string(new_str));
        }
        else {
            logger.log("Not updating");
        }
        remove(tmp);
    }

     // Context menu actions
    void Room_Editor::re_edit () {
        if (!selected) return;
        general_edit(res_realp(selected));
    }

    void Room_Editor::re_duplicate () {
        if (!selected) return;
        auto realp = res_realp(selected);
        if (hacc::Document* doc = hacc::get_document_containing(realp.address)) {
            void* newp = NULL;
            if (realp.type.can_copy_assign()) {
                newp = doc->alloc(realp.type);
                realp.type.copy_assign(newp, realp);
            }
            else {
                hacc::Tree tree = hacc::Reference(realp).to_tree();
                newp = doc->alloc(realp.type);
                try {
                    realp.type.construct(newp);
                    hacc::Reference(realp.type, newp).from_tree(tree);
                }
                catch (...) {
                    realp.type.destruct(newp);
                    doc->dealloc(newp);
                    throw;
                }
            }
            Resident* old_res = hacc::Pointer(realp.type, realp.address);
            Resident* new_res = hacc::Pointer(realp.type, newp);
            Vec pos = old_res->Resident_get_pos();
            pos += Vec(frand()*2-1, 0.8);
            new_res->Resident_set_pos(pos);
             // TODO: set pos to nearby
        }
        else {
            throw hacc::X::Logic_Error("Could not re_duplicate: this object does not belong to a document.");
        }
    }
    void Room_Editor::re_delete () {
        if (!selected) return;
        auto realp = res_realp(selected);
        if (hacc::Document* doc = hacc::get_document_containing(realp.address)) {
            realp.type.destruct(realp.address);
            doc->dealloc(realp.address);
        }
        else {
            throw hacc::X::Logic_Error("Could not re_delete: this object does not belong to a document.");
        }
    }
    void Room_Editor::re_edit_room () {
        if (!selected_room) return;
        general_edit(selected_room);
    }
    void Room_Editor::re_reload_room () {
        if (!selected_room) return;
        auto filename = hacc::address_to_path(selected_room).root();
        window->before_next_frame([filename](){hacc::reload(filename);});
    }
    void Room_Editor::re_save_room () {
        if (!selected_room) return;
        auto filename = hacc::address_to_path(selected_room).root();
        window->before_next_frame([filename](){hacc::save(filename);});
    }
    void Room_Editor::re_new_actor (hacc::Type type, hacc::Tree data) {
        hacc::Document* st = current_state.data();
        void* newp = st->alloc(type);
        type.construct(newp);
        hacc::Reference(type, newp).from_tree(data);
        Resident* resp = (Resident*)hacc::Pointer(type, newp).address_of_type(hacc::Type::CppType<Resident>());
        if (resp) {
            resp->Resident_set_pos(menu_world_pos);
            resp->set_room(selected_room);
        }
    }
    void Room_Editor::re_new_furniture (hacc::Type type, hacc::Tree data) {
        if (!selected_room) return;
        hacc::Document* doc = hacc::get_document_containing(selected_room);
        void* newp = doc->alloc(type);
        type.construct(newp);
        hacc::Reference(type, newp).from_tree(data);
        Resident* resp = (Resident*)hacc::Pointer(type, newp).address_of_type(hacc::Type::CppType<Resident>());
        if (resp) {
            resp->Resident_set_pos(menu_world_pos);
            resp->set_room(selected_room);
        }
    }

    struct Type_Specific_Menu : Menu_Item {
        std::unordered_map<std::string, Menu_Item*> items;

        Vec Menu_Item_size (Vec area) override {
            auto iter = items.find(room_editor->selected_type_name);
            if (iter != items.end())
                return iter->second->Menu_Item_size(area);
            else
                return Vec(0, 0);
        }
        void Menu_Item_draw (Vec pos, Vec size) override {
            auto iter = items.find(room_editor->selected_type_name);
            if (iter != items.end())
                iter->second->Menu_Item_draw(pos, size);
        }
        bool Menu_Item_hover (Vec pos, Vec size) override {
            auto iter = items.find(room_editor->selected_type_name);
            if (iter != items.end())
                return iter->second->Menu_Item_hover(pos, size);
            else
                return false;
        }
        bool Menu_Item_click (Vec pos, Vec size) override {
            auto iter = items.find(room_editor->selected_type_name);
            if (iter != items.end())
                return iter->second->Menu_Item_click(pos, size);
            else
                return false;
        }
    };

    Tile_Editor::Tile_Editor () {
        tile_editor = this;
    }
    Tile_Editor::~Tile_Editor () {
        if (active) deactivate();
        if (tile_editor == this)
            tile_editor = NULL;
    }

    void Tile_Editor::activate () {
        logger.log("Activating tile editor");
        Listener::activate();
        Drawn<vis::Map>::appear();
    }
    void Tile_Editor::deactivate () {
        logger.log("Deactivating tile editor");
        Listener::deactivate();
        Drawn<vis::Map>::disappear();
    }


    void Tile_Editor::Drawn_draw (vis::Map) {
        if (!tilemap) return;
         // display current tile in corner
        draw_tile(tile, tilemap->texture, camera->window_to_world(0, 0) - Vec(0, 1));
    }
    bool Tile_Editor::in_bounds (Vec pos) {
        return pos.x >= 0 && pos.x < tilemap->tiles->width
            && pos.y >= 0 && pos.y < tilemap->tiles->height;
    }
    uint16& Tile_Editor::tile_at (Vec pos) {
        if (!in_bounds(pos)) {
            throw hacc::X::Logic_Error("Position (%f,%f) is out of bounds for this tilemap\n");
        }
        size_t x = floor(pos.x);
        size_t y = tilemap->tiles->height - floor(pos.y) - 1;
        return tilemap->tiles->tiles[tilemap->tiles->width * y + x];
    }
    void Tile_Editor::draw (Vec pos) {
        if (!in_bounds(pos))
            return;
        uint16& selected = tile_at(pos);
        if (selected != tile) {
            selected = tile;
            tilemap->tiles->finish();
             // Update all relevant tilemaps
            for (auto b = phys::space.b2world->GetBodyList(); b; b = b->GetNext()) {
                if (auto tm = dynamic_cast<Tilemap*>((phys::Object*)b->GetUserData())) {
                    if (tm->tiles == tilemap->tiles) {
                        tm->set_tiles(tilemap->tiles);
                    }
                }
            }
        }
    }
    void Tile_Editor::pick (Vec pos) {
        if (!in_bounds(pos))
            return;
        tile = tile_at(pos);
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
        if (clicking) {
            draw(camera->window_to_world(x, y));
        }
    }
    bool Tile_Editor::Listener_key (int code, int action) {
        if (action == GLFW_PRESS) {
            switch (code) {
                case GLFW_KEY_ESC:
                    deactivate();
                    return true;
                case 'S': {
                    for (auto& f : hacc::loaded_files()) {
                        if (f.data().address() == tilemap->tiles) {
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
                case GLFW_KEY_LEFT:
                    if ((tile & 0x3fff) == 0) {
                        tile += tilemap->tileset->tiles.size() - 1;
                    }
                    else {
                        tile -= 1;
                    }
                    return true;
                case GLFW_KEY_RIGHT:
                    if ((tile & 0x3fff) >= tilemap->tileset->tiles.size() - 1) {
                        tile = 0;
                    }
                    else {
                        tile += 1;
                    }
                    return true;
                case GLFW_KEY_UP: {
                    Vec size = tilemap->texture->size*PX;
                    if ((tile & 0x3fff) < size.y) {
                        tile += size.y - size.x;
                        if ((tile & 0x3fff) >= tilemap->tileset->tiles.size() - 1) {
                            tile -= size.x;
                        }
                    }
                    else {
                        tile -= size.x;
                    }
                    return true;
                }
                case GLFW_KEY_DOWN: {
                    Vec size = tilemap->texture->size*PX;
                    if ((tile & 0x3fff) >= size.y - size.x) {
                        tile -= size.y - size.x;
                    }
                    else if ((tile & 0x3fff) >= tilemap->tileset->tiles.size() - size.x) {
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

} using namespace shell;

HACCABLE(Room_Editor) {
    name("shell::Room_Editor");
    attr("font", member(&Room_Editor::font).optional());
    attr("res_menu", member(&Room_Editor::res_menu).optional());
    attr("room_menu", member(&Room_Editor::room_menu).optional());
}

HACCABLE(Type_Specific_Menu) {
    name("shell::Type_Specific_Menu");
    attr("Menu_Item", base<Menu_Item>().collapse());
    delegate(member(&Type_Specific_Menu::items));
}

HACCABLE(Tile_Editor) {
    name("shell::Tile_Editor");
}

namespace {

void _re_toggle () {
    if (!room_editor) return;
    if (room_editor->active)
        room_editor->deactivate();
    else
        room_editor->activate();
}

New_Command _re_toggle_cmd ("re_toggle", "Toggle the Room_Editor interface.", 0, _re_toggle);

void _re_edit () {
    if (!room_editor) return;
    room_editor->re_edit();
}
New_Command _re_edit_cmd ("re_edit", "Edit the selected object's textual representation with $EDITOR.", 0, _re_edit);
void _re_duplicate () {
    if (!room_editor) return;
    room_editor->re_duplicate();
}
New_Command _re_duplicate_cmd ("re_duplicate", "Duplicate the selected object.", 0, _re_duplicate);
void _re_delete () {
    if (!room_editor) return;
    room_editor->re_delete();
}
New_Command _re_delete_cmd ("re_delete", "Delete the selected object.", 0, _re_delete);

void _re_edit_room () {
    if (!room_editor) return;
    room_editor->re_edit_room();
}
New_Command _re_edit_room_cmd ("re_edit_room", "Text-edit the selected room.", 0, _re_edit_room);

void _re_reload_room () {
    if (!room_editor) return;
    room_editor->re_reload_room();
}
New_Command _re_reload_room_cmd ("re_reload_room", "Reload the selected room from disk.", 0, _re_reload_room);

void _re_save_room () {
    if (!room_editor) return;
    room_editor->re_save_room();
}
New_Command _re_save_room_cmd ("re_save_room", "Save the selected room to disk.", 0, _re_save_room);

void _re_new_actor (std::string type, hacc::Tree data) {
    if (!room_editor) return;
    room_editor->re_new_actor(hacc::Type(type), data);
}
New_Command _re_new_actor_cmd ("re_new_actor", "Add a new actor to the current state.", 2, _re_new_actor);

void _re_new_furniture (std::string type, hacc::Tree data) {
    if (!room_editor) return;
    room_editor->re_new_furniture(hacc::Type(type), data);
}
New_Command _re_new_furniture_cmd ("re_new_furniture", "Add a new object to the current room.", 2, _re_new_furniture);

void _re_control_this () {
    if (!room_editor || !ent::player) return;
    if (auto cont = dynamic_cast<ent::Controllable*>(room_editor->selected))
        ent::player->set_character(cont);
}
New_Command _re_control_this_cmd ("re_control_this", "Transfer keyboard control to this Biped.", 0, _re_control_this);

void _re_start_te () {
    if (!room_editor || !tile_editor) return;
    if (auto tilemap = dynamic_cast<Tilemap*>(room_editor->selected)) {
        room_editor->status = "Editing "
                            + hacc::path_to_string(hacc::address_to_path(tilemap->tiles));
        tile_editor->tilemap = tilemap;
        tile_editor->activate();
    }
}
New_Command _re_start_te_cmd ("re_start_te", "Start tilemap editor from room editor.", 0, _re_start_te);

}
