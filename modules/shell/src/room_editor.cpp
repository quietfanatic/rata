
#include <string>
#include <sstream>
#include <SDL2/SDL_events.h>
#include "core/inc/commands.h"
#include "ent/inc/control.h"
#include "geo/inc/rooms.h"
#include "geo/inc/tiles.h"
#include "hacc/inc/documents.h"
#include "hacc/inc/files.h"
#include "hacc/inc/strings.h"
#include "shell/inc/editing.h"
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

     // Get pointer of most derived type from Resident*
    static hacc::Pointer res_realp (Resident* res) {
        if (!res) return nullptr;
        void* derived = hacc::Pointer(res).address_of_type(hacc::Type(typeid(*res)));
        return hacc::Pointer(hacc::Type(typeid(*res)), derived);
    }

    struct Room_Editor : vis::Drawn<vis::Overlay>, vis::Drawn<vis::Dev>, core::Listener {
         // Associated utilities
        Menu<vis::Dev>* res_menu = NULL;  // Set on construction
        Menu<vis::Dev>* room_menu = NULL;  // Set on construction
        vis::Camera camera;
        vis::Font* font = NULL;
         // Selected items
        geo::Resident* hovering = NULL;
        geo::Resident* selected = NULL;
        std::string selected_type_name;
        geo::Room* hovering_room = NULL;
        geo::Room* selected_room = NULL;
         // Modes
        bool editing_pts = false;
        bool editing_room = false;
         // Cursor control
        bool clicking = false;
        bool dragging = false;
        util::Vec drag_origin;
        util::Vec drag_offset;
        util::Vec menu_world_pos;
        int dragging_pt = -1;
         // Drawing
        std::string status;

        Room_Editor () { hacc::manage(this); }
        void activate () {
            if (!res_menu)
                res_menu = hacc::File("shell/res/re_menus.hacc").attr("res_menu");
            if (!room_menu)
                room_menu = hacc::File("shell/res/re_menus.hacc").attr("room_menu");
            log("editing", "Activating room editor.");
            Listener::activate();
            if (vis::camera)
                camera.pos = vis::camera->pos;
            else
                camera.pos = Vec(10, 7.5);
            camera.size = Vec(40, 30);
            camera.activate();
            Drawn<Overlay>::appear();
            Drawn<Dev>::appear();
            clicking = false;
            dragging = false;
        }
        void deactivate () {
            log("editing", "Deactivating room editor.");
            hovering = NULL;
            selected = NULL;
            dragging = false;
            clicking = false;
            Listener::deactivate();
            camera.deactivate();
            Drawn<Overlay>::disappear();
            Drawn<Dev>::disappear();
            res_menu->deactivate();
            room_menu->deactivate();
        }
        bool Listener_event (SDL_Event* event) override {
            switch (event->type) {
                case SDL_MOUSEMOTION: {
                    auto x = event->motion.x;
                    auto y = event->motion.y;
                     // Adjust camera
                    float move_speed = 1/256.0;
                    if (x < 64) {
                        camera.pos.x += (x - 64) * move_speed;
                    }
                    else if (x > window->width - 64) {
                        camera.pos.x += (x - window->width + 64) * move_speed;
                    }
                    if (y < 64) {
                        camera.pos.y -= (y - 64) * move_speed;
                    }
                    else if (y > window->height - 64) {
                        camera.pos.y -= (y - window->height + 64) * move_speed;
                    }
                    Vec world_pos = vis::camera->window_to_world(event->motion.x, event->motion.y);
                     // Snap to grid when pressing CTRL
                    if (SDL_GetModState() & KMOD_CTRL) {
                        world_pos.x = round(world_pos.x * 2) / 2;
                        world_pos.y = round(world_pos.y * 2) / 2;
                    }
                    if (editing_pts) {
                        if (dragging_pt >= 0) {
                            Vec r_pos = selected->Resident_get_pos();
                            selected->Resident_set_pt(dragging_pt, world_pos - r_pos - drag_offset);
                        }
                    }
                    else if (editing_room) {
                        if (dragging_pt >= 0) {
                            selected_room->set_pt(dragging_pt, world_pos - drag_offset);
                        }
                    }
                    else {
                        if (clicking) {
                            if (selected) {
                                if (length2(drag_origin - world_pos) > 0.2)
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
                                        if (!defined(pos) || !defined(boundary)) {
                                            pos = room.boundary.lt() + Vec(0.5, -0.5);
                                            pos.x += unpositioned_residents++;
                                        }
                                        if (covers(boundary, world_pos - pos)) {
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
                    return true;
                }
                case SDL_MOUSEBUTTONDOWN: {
                    int x = event->button.x;
                    int y = event->button.y;
                    Vec realpos = vis::camera->window_to_world(x, y);
                    if (editing_pts) {
                        if (event->button.button == SDL_BUTTON_LEFT) {
                            size_t n_pts = selected->Resident_n_pts();
                            Vec r_pos = selected->Resident_get_pos();
                            float lowest_pt_t = INF;
                            Vec lowest_pt_pos = Vec(0, 0);
                            int lowest_pt = -1;
                            for (size_t i = 0; i < n_pts; i++) {
                                Vec pos = selected->Resident_get_pt(i);
                                const Rect& boundary = pos + r_pos
                                                     + Rect(-0.25, -0.25, 0.25, 0.25);
                                if (covers(boundary, realpos)) {
                                    if (boundary.t < lowest_pt_t) {
                                        lowest_pt_t = boundary.t;
                                        lowest_pt_pos = pos;
                                        lowest_pt = i;
                                    }
                                }
                            }
                            dragging_pt = lowest_pt;
                            drag_offset = realpos - r_pos - lowest_pt_pos;
                        }
                    }
                    else if (editing_room) {
                        if (event->button.button == SDL_BUTTON_LEFT) {
                            size_t n_pts = selected_room->n_pts();
                            float lowest_pt_t = INF;
                            Vec lowest_pt_pos = Vec(0, 0);
                            int lowest_pt = -1;
                            for (size_t i = 0; i < n_pts; i++) {
                                Vec pos = selected_room->get_pt(i);
                                const Rect& boundary = pos + Rect(-0.25, -0.25, 0.25, 0.25);
                                if (covers(boundary, realpos)) {
                                    if (boundary.t < lowest_pt_t) {
                                        lowest_pt_t = boundary.t;
                                        lowest_pt_pos = pos;
                                        lowest_pt = i;
                                    }
                                }
                            }
                            dragging_pt = lowest_pt;
                            drag_offset = realpos - lowest_pt_pos;
                        }
                    }
                    else {
                         // Just upgrade hovering to selected
                        dragging = NULL;
                        selected = hovering;
                        selected_room = hovering_room;
                        if (selected) {
                            Vec pos = selected->Resident_get_pos();
                            if (event->button.button == SDL_BUTTON_LEFT) {
                                drag_origin = pos;
                                drag_offset = realpos - pos;
                                log("editing", "Selected " + hacc::Reference(res_realp(selected)).show());
                                clicking = true;
                            }
                            else if (event->button.button == SDL_BUTTON_RIGHT) {
                                selected_type_name = res_realp(selected).type.name();
                                menu_world_pos = vis::camera->window_to_world(x, y);
                                Vec area = vis::camera->window_to_dev(window->width, 0);
                                res_menu->size = res_menu->root->Menu_Item_size(area);
                                Vec pos = vis::camera->window_to_dev(x, y);
                                res_menu->pos = pos - Vec(-1*PX, res_menu->size.y + 1*PX);
                                res_menu->activate();
                            }
                        }
                        else if (selected_room) {
                            if (event->button.button == SDL_BUTTON_RIGHT) {
                                menu_world_pos = vis::camera->window_to_world(x, y);
                                Vec area = vis::camera->window_to_dev(window->width, 0);
                                room_menu->size = room_menu->root->Menu_Item_size(area);
                                Vec pos = vis::camera->window_to_dev(x, y);
                                room_menu->pos = pos - Vec(-1*PX, room_menu->size.y + 1*PX);
                                room_menu->activate();
                            }
                        }
                    }
                     // Snap to grid if pressing ctrl
                    if (SDL_GetModState() & KMOD_CTRL) {
                        drag_offset.x = round(drag_offset.x * 2) / 2;
                        drag_offset.y = round(drag_offset.y * 2) / 2;
                    }
                    return true;
                }
                case SDL_MOUSEBUTTONUP: {
                    if (event->button.button == SDL_BUTTON_LEFT) {
                        dragging = false;
                        clicking = false;
                        dragging_pt = -1;
                    }
                    return true;
                }
                case SDL_KEYDOWN: {
                    if (event->key.keysym.sym == SDLK_ESCAPE) {
                        if (editing_pts)
                            editing_pts = false;
                        else if (editing_room)
                            editing_room = false;
                        else
                            deactivate();
                        return true;
                    }
                    else return false;
                }
                default: return false;
            }
        }
        void Drawn_draw (Overlay) override {
            size_t unpositioned_residents = 0;
            if (editing_pts) {
                if (!selected) return;
                Vec r_pos = selected->Resident_get_pos();
                auto n_pts = selected->Resident_n_pts();
                for (size_t i = 0; i < n_pts; i++) {
                    Vec pos = r_pos + selected->Resident_get_pt(i);
                    if (defined(pos)) {
                        color_offset(pos);
                        draw_color((int)i == dragging_pt ? 0xff0000ff : 0xffff00ff);
                        draw_rect(Rect(-0.25, -0.25, 0.25, 0.25));
                    }
                }
            }
            if (editing_room) {
                if (!selected_room) return;
                auto n_pts = selected_room->n_pts();
                for (size_t i = 0; i < n_pts; i++) {
                    Vec pos = selected_room->get_pt(i);
                    if (defined(pos)) {
                        color_offset(pos);
                        draw_color((int)i == dragging_pt ? 0xff0000ff : 0xffff00ff);
                        draw_rect(Rect(-0.25, -0.25, 0.25, 0.25));
                    }
                }
            }
            for (auto& room : all_rooms()) {
                 // Draw outline around room
                color_offset(Vec(0, 0));
                draw_color(0xff0000ff);
                draw_rect(room.boundary);
                if (tile_editor && tile_editor->active)
                    continue;
                 // Draw lines to neighbors
                color_offset(Vec(0, 0));
                for (size_t i = 0; i < room.neighbors.size(); i++) {
                    if (room.neighbors[i]) {
                        draw_color(0xff0000ff);
                        Vec start = room.boundary.rb() + Vec(-0.5 - i, 0.5);
                        Vec end = room.neighbors[i]->boundary.lt();
                        draw_line(start, end);
                    }
                }
                 // Draw outlines around all residents
                if (room.observer_count) {
                    for (auto& res : room.residents) {
                        if (!editing_pts) {
                            Vec pos = res.Resident_get_pos();
                            const Rect& boundary = res.Resident_boundary();
                            if (!defined(pos) || !defined(boundary)) {
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
        }
        void Drawn_draw (Dev) override {
            if (!status.empty()) {
                color_offset(Vec(0, window->height*PX));
                draw_color(0x0000007f);
                Vec size = text_size(status, font);
                draw_solid_rect(Rect(0, -size.y, size.x, 0));
                draw_text(status, font, Vec(0, window->height*PX), Vec(1, 1));
            }
             // Show position in various coordinate spaces
            Vec world_pos = vis::camera->window_to_world(window->cursor_x, window->cursor_y);
            Vec hud_pos = vis::camera->window_to_hud(window->cursor_x, window->cursor_y);
            Vec dev_pos = vis::camera->window_to_dev(window->cursor_x, window->cursor_y);
            draw_text(
                "window: " + std::to_string(window->cursor_x) +
                      ", " + std::to_string(window->cursor_y) + "\n" +
                "world: " + std::to_string(world_pos.x) +
                     ", " + std::to_string(world_pos.y) + "\n" +
                "hud: " + std::to_string(hud_pos.x) +
                   ", " + std::to_string(hud_pos.y) + "\n" +
                "dev: " + std::to_string(dev_pos.x) +
                   ", " + std::to_string(dev_pos.y),
                font, vis::camera->window_to_dev(window->width, 0),
                Vec(-1, 1)
            );
        }
    };
    static Room_Editor room_editor;

    static void re_toggle () {
        if (room_editor.active)
            room_editor.deactivate();
        else
            room_editor.activate();
    }

    static void general_edit (hacc::Reference ref, std::string prefix = "") {
        auto tree = ref.to_tree();
        auto tmp = tmpnam(NULL);
        try {
            if (prefix.empty())
                prefix = " // Editing " + ref.show() + "\n";
            auto str = prefix + hacc::tree_to_string(tree, tmp, 3);
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
            if (system((std::string(editor) + " " + tmp).c_str()) != 0)
                log("editing", "Failed to open text editor.");
            auto new_str = hacc::string_from_file(tmp);
            if (new_str != str) {
                log("editing", "Updating");
                ref.from_tree(hacc::tree_from_string(new_str));
            }
            else {
                log("editing", "Not updating");
            }
            remove(tmp);
        }
        catch (...) {
            remove(tmp);
            throw;
        }
    }
    struct New_File {
        std::string filename;
        hacc::Tree data;
    };
    static void re_new_file (std::string rec_name, hacc::Tree rec_data) {
        New_File nf {rec_name, rec_data};
        general_edit(&nf, " // Creating new file\n");
        hacc::Dynamic d;
        hacc::Reference(&d).from_tree(nf.data);
        auto file = hacc::File(nf.filename, std::move(d));
        try {
            hacc::save(file);
        }
        catch (...) {
            hacc::unload(file);
            throw;
        }
    }
    struct Tilemap_Uniqueification {
        std::string tiles_filename;
        hacc::Tree Tilemap_Def;
    };
    static void re_uniqueify_tilemap (hacc::Tree def_data, hacc::Tree tiles_data) {
        if (!room_editor.selected) return;
        auto tilemap = dynamic_cast<Tilemap*>(room_editor.selected);
        if (!tilemap) return;
        auto realp = res_realp(tilemap);
        if (hacc::Document* doc = hacc::get_document_containing(realp.address)) {
            Tilemap_Uniqueification tu {"world/room/<unititled>.tiles", def_data};
            general_edit(&tu, " // Uniqueifying tilemap " + hacc::Reference(tilemap).show() + "\n");
            auto def = doc->create<geo::Tilemap_Def>();
            try {
                hacc::Reference(def).from_tree(tu.Tilemap_Def);
                hacc::Dynamic d;
                hacc::Reference(&d).from_tree(tiles_data);
                def->tiles = d.address();
                auto file = hacc::File(tu.tiles_filename, std::move(d));
                try {
                    hacc::save(file);
                }
                catch (...) {
                    hacc::unload(file);
                    throw;
                }
                tilemap->def = def;
            }
            catch (...) {
                doc->destroy(def);
                throw;
            }
        }
    }

    static void re_edit () {
        if (!room_editor.selected) return;
        general_edit(res_realp(room_editor.selected));
    }
    static void re_duplicate () {
        if (!room_editor.selected) return;
        auto realp = res_realp(room_editor.selected);
        if (hacc::Document* doc = hacc::get_document_containing(realp.address)) {
            void* newp = doc->alloc(realp.type);
            realp.type.construct(newp);
            try {
                if (realp.type.can_copy_assign()) {
                    realp.type.copy_assign(newp, realp);
                }
                else {
                    hacc::Tree tree = hacc::Reference(realp).to_tree();
                    hacc::Reference(realp.type, newp).from_tree(tree);
                }
            }
            catch (...) {
                realp.type.destruct(newp);
                doc->dealloc(newp);
            }
            Resident* old_res = hacc::Pointer(realp.type, realp.address);
            Resident* new_res = hacc::Pointer(realp.type, newp);
            Vec pos = old_res->Resident_get_pos();
            pos += Vec(frand()*2-1, 0.8);
            new_res->Resident_set_pos(pos);
        }
        else {
            throw hacc::X::Logic_Error("Could not re_duplicate: this object does not belong to a document.");
        }
    }
    static void re_delete () {
        if (!room_editor.selected) return;
        auto realp = res_realp(room_editor.selected);
        if (hacc::Document* doc = hacc::get_document_containing(realp.address)) {
            realp.type.destruct(realp.address);
            doc->dealloc(realp.address);
        }
        else {
            throw hacc::X::Logic_Error("Could not re_delete: this object does not belong to a document.");
        }
    }
    static void re_reload_room () {
        if (!room_editor.selected_room) return;
        auto filename = hacc::address_to_path(room_editor.selected_room).root();
        window->before_next_frame([filename](){hacc::reload(filename);});
    }
    static void re_save_room () {
        if (!room_editor.selected_room) return;
        auto filename = hacc::address_to_path(room_editor.selected_room).root();
        window->before_next_frame([filename](){hacc::save(filename);});
    }
    static void re_new_actor (hacc::Type type, hacc::Tree data) {
        if (!room_editor.selected_room) return;
        hacc::Document* st = current_state.data();
        void* newp = st->alloc(type);
        type.construct(newp);
        try {
            hacc::Reference(type, newp).from_tree(data);
            Resident* resp = (Resident*)hacc::Pointer(type, newp).address_of_type(hacc::Type::CppType<Resident>());
            if (resp) {
                resp->Resident_set_pos(room_editor.menu_world_pos);
                resp->set_room(room_editor.selected_room);
            }
        }
        catch (...) {
            type.destruct(newp);
            st->dealloc(newp);
            throw;
        }
    }
    static void re_new_furniture (hacc::Type type, hacc::Tree data) {
        if (!room_editor.selected_room) return;
        hacc::Document* doc = hacc::get_document_containing(room_editor.selected_room);
        if (!doc) {
            throw hacc::X::Logic_Error("Selected room is not in a document!?");
        }
        void* newp = doc->alloc(type);
        type.construct(newp);
        try {
            hacc::Reference(type, newp).from_tree(data);
            Resident* resp = (Resident*)hacc::Pointer(type, newp).address_of_type(hacc::Type::CppType<Resident>());
            if (resp) {
                resp->Resident_set_pos(room_editor.menu_world_pos);
                resp->set_room(room_editor.selected_room);
            }
        }
        catch (...) {
            type.destruct(newp);
            doc->dealloc(newp);
            throw;
        }
    }
    static void re_control_this () {
        if (!room_editor.selected || !ent::player) return;
        if (auto cont = dynamic_cast<ent::Controllable*>(room_editor.selected))
            ent::player->set_character(cont);
    }
    static void re_start_te () {
        if (!tile_editor) return;
        if (auto tilemap = dynamic_cast<Tilemap*>(room_editor.selected)) {
            room_editor.status = "Editing "
                               + hacc::path_to_string(hacc::address_to_path(tilemap->get_def()->tiles));
            tile_editor->tilemap = tilemap;
            tile_editor->activate();
        }
    }
    static void re_edit_room () {
        if (!room_editor.selected_room) return;
        room_editor.editing_room = !room_editor.editing_room;
    }
    static void re_edit_points () {
        if (!room_editor.selected) return;
        room_editor.editing_pts = !room_editor.editing_pts;
    }

    struct Type_Specific_Menu : Menu_Item {
        std::unordered_map<std::string, Menu_Item*> items;

        Vec Menu_Item_size (Vec area) override {
            auto iter = items.find(room_editor.selected_type_name);
            if (iter != items.end())
                return iter->second->Menu_Item_size(area);
            else
                return Vec(0, 0);
        }
        void Menu_Item_draw (Vec pos, Vec size) override {
            auto iter = items.find(room_editor.selected_type_name);
            if (iter != items.end())
                iter->second->Menu_Item_draw(pos, size);
        }
        bool Menu_Item_hover (Vec pos, Vec size) override {
            auto iter = items.find(room_editor.selected_type_name);
            if (iter != items.end())
                return iter->second->Menu_Item_hover(pos, size);
            else
                return false;
        }
        bool Menu_Item_click (Vec pos, Vec size) override {
            auto iter = items.find(room_editor.selected_type_name);
            if (iter != items.end())
                return iter->second->Menu_Item_click(pos, size);
            else
                return false;
        }
    };

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

HACCABLE(New_File) {
    name("shell::New_File");
    attr("filename", member(&New_File::filename));
    attr("data", member(&New_File::data));
}

HACCABLE(Tilemap_Uniqueification) {
    name("shell::Tilemap_Uniqueification");
    attr("tiles_filename", member(&Tilemap_Uniqueification::tiles_filename));
    attr("Tilemap_Def", member(&Tilemap_Uniqueification::Tilemap_Def));
}

New_Command _re_toggle_cmd ("re_toggle", "Toggle the Room_Editor interface.", 0, re_toggle);
New_Command _re_edit_cmd ("re_edit", "Edit the selected object's textual representation with $EDITOR.", 0, re_edit);
New_Command _re_duplicate_cmd ("re_duplicate", "Duplicate the selected object.", 0, re_duplicate);
New_Command _re_delete_cmd ("re_delete", "Delete the selected object.", 0, re_delete);
New_Command _re_edit_room_cmd ("re_edit_room", "Edit the selected room's boundary and neighbors.", 0, re_edit_room);
New_Command _re_reload_room_cmd ("re_reload_room", "Reload the selected room from disk.", 0, re_reload_room);
New_Command _re_save_room_cmd ("re_save_room", "Save the selected room to disk.", 0, re_save_room);
New_Command _re_new_actor_cmd ("re_new_actor", "Add a new actor to the current state.", 2, re_new_actor);
New_Command _re_new_furniture_cmd ("re_new_furniture", "Add a new object to the current room.", 2, re_new_furniture);
New_Command _re_control_this_cmd ("re_control_this", "Transfer keyboard control to this Biped.", 0, re_control_this);
New_Command _re_start_te_cmd ("re_start_te", "Start tilemap editor from room editor.", 0, re_start_te);
New_Command _re_edit_pts_cmd ("re_edit_pts", "Edit object-specific points.", 0, re_edit_points);
New_Command _re_new_file ("re_new_file", "Create new file with some default parameters", 2, re_new_file);
New_Command _re_uniqueify_tilemap_cmd (
    "re_uniqueify_tilemap", "Give selected tilemap a unique Tilemap_Def and tiles file.",
    2, re_uniqueify_tilemap
);
