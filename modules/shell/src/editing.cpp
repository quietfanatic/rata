#include <sstream>
#include "../inc/editing.h"
#include "../../geo/inc/rooms.h"
#include "../../geo/inc/camera.h"
#include "../../hacc/inc/files.h"
#include "../../hacc/inc/documents.h"
#include "../../hacc/inc/strings.h"
#include "../../core/inc/commands.h"
#include "../../vis/inc/color.h"
#include "../../vis/inc/text.h"
#include "../../util/inc/debug.h"

using namespace util;
using namespace vis;
using namespace core;
using namespace geo;

namespace shell {

    Logger logger ("editing");

    Resident_Editor* resident_editor = NULL;

    hacc::Pointer res_realp (Resident* res) {
        void* derived = hacc::Pointer(res).address_of_type(hacc::Type(typeid(*res)));
        return hacc::Pointer(hacc::Type(typeid(*res)), derived);
    }

    void Resident_Editor::Drawn_draw (Overlay) {
        size_t unpositioned_residents = 0;
        for (auto& room : all_rooms()) {
            color_offset(Vec(0, 0));
            draw_color(0xff00ffff);
            draw_rect(room.boundary);
            if (room.observer_count) {
                for (auto& res : room.residents) {
                    Vec pos = res.Resident_get_pos();
                    if (!pos.is_defined()) {
                        pos = room.boundary.rt() + Vec(0.5, -0.5);
                        pos.x += unpositioned_residents++;
                    }
                    const Rect& boundary = res.Resident_boundary();
                    color_offset(pos);
                    draw_color(&res == selected ? 0xff0000ff : 0x00ff00ff);
                    draw_rect(boundary);
                }
            }
        }
    }
    void Resident_Editor::Drawn_draw (Dev) {
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

    void Resident_Editor::Listener_cursor_pos (int x, int y) {
        Vec world_pos = camera->window_to_world(x, y);
        if (clicking) {
            if (selected && (drag_origin - world_pos).mag2() > 0.2)
                dragging = selected;
            if (dragging) {
                dragging->Resident_set_pos(world_pos - drag_offset);
            }
        }
        else {
            Resident* new_hovering = NULL;
            size_t unpositioned_residents = 0;
            for (auto& room : all_rooms()) {
                if (room.observer_count) {
                    for (auto& res : room.residents) {
                        Vec pos = res.Resident_get_pos();
                        const Rect& boundary = res.Resident_boundary();
                        if (!pos.is_defined()) {
                            pos = room.boundary.rt() + Vec(0.5, -0.5);
                            pos.x += unpositioned_residents++;
                        }
                        if (boundary.covers(world_pos - pos)) {
                            new_hovering = &res;
                        }
                    }
                }
            }
            if (new_hovering) {
                if (new_hovering != hovering) {
                    auto realp = res_realp(new_hovering);
                    auto path = hacc::address_to_path(realp);
                    std::ostringstream ss;
                    ss << realp.address << " " << hacc::path_to_string(path);
                    status = ss.str();
                }
            }
            else {
                status = "";
            }
            hovering = new_hovering;
        }
    }
    bool Resident_Editor::Listener_button (int code, int action) {
        if (action == GLFW_PRESS) {
            Vec realpos = camera->window_to_world(window->cursor_x, window->cursor_y);
            dragging = NULL;
            std::vector<std::pair<float, Resident*>> matches;
             // Search for cursor overlap
            for (auto& room : all_rooms()) {
                if (room.observer_count) {
                    for (auto& res : room.residents) {
                        Vec pos = res.Resident_get_pos();
                        if (!pos.is_defined()) continue;
                        const Rect& boundary = res.Resident_boundary();
                        if (boundary.covers(realpos - pos)) {
                            matches.emplace_back(boundary.t, &res);
                        }
                    }
                }
            }
             // Resident with lowest top gets priority
            float lowest = INF;
            Resident* picked = NULL;
            for (auto& p : matches) {
                if (p.first < lowest) {
                    lowest = p.first;
                    picked = p.second;
                }
            }
            selected = picked;
            if (!picked) return false;
            Vec pos = picked->Resident_get_pos();
            if (code == GLFW_MOUSE_BUTTON_LEFT) {
                drag_origin = pos;
                drag_offset = realpos - pos;
                logger.log("Selected " + hacc::Reference(res_realp(picked)).show());
                clicking = true;
            }
            else if (code == GLFW_MOUSE_BUTTON_RIGHT) {
                if (action == GLFW_PRESS) {
                    Vec area = camera->window_to_dev(window->width, 0);
                    context_menu->size = context_menu->root->Menu_Item_size(area);
                    Vec pos = camera->window_to_dev(window->cursor_x, window->cursor_y);
                    context_menu->pos = pos - Vec(-1*PX, context_menu->size.y + 1*PX);
                    context_menu->activate();
                    return true;
                }
            }
            return true;
        }
        else {  // GLFW_RELEASE
            if (code == GLFW_MOUSE_BUTTON_LEFT) {
                dragging = NULL;
                clicking = false;
            }
            return true;
        }
        return false;
    }

    Resident_Editor::Resident_Editor () :
        context_menu(hacc::File("shell/res/re_context_menu.hacc").data().attr("cm"))
    {
        resident_editor = this;
    }
    Resident_Editor::~Resident_Editor () {
        if (resident_editor == this)
            resident_editor = NULL;
    }

    void Resident_Editor::activate () {
        logger.log("Activating editor.");
        Listener::activate();
        fc.pos = camera->Camera_pos();
        fc.size = Vec(40, 30);
        fc.activate();
        Drawn<Overlay>::appear();
        Drawn<Dev>::appear();
    }
    void Resident_Editor::deactivate () {
        logger.log("Deactivating editor.");
        Listener::deactivate();
        fc.deactivate();
        Drawn<Overlay>::disappear();
        Drawn<Dev>::disappear();
        context_menu->deactivate();
    }

     // Context menu actions
    void Resident_Editor::re_edit () {
        if (!selected) return;
        auto realp = res_realp(selected);
        auto tree = hacc::Reference(realp).to_tree();
        auto tmp = tmpnam(NULL);
        auto str = " // Editing " + hacc::Reference(realp).show() + "\n"
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
            printf("Updating\n");
            hacc::Reference(realp).from_tree(hacc::tree_from_string(new_str));
        }
        else {
            printf("Not updating\n");
        }
        remove(tmp);
    }
    void Resident_Editor::re_duplicate () {
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
    void Resident_Editor::re_delete () {
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

} using namespace shell;

HACCABLE(Resident_Editor) {
    name("shell::Resident_Editor");
    attr("font", member(&Resident_Editor::font).optional());
}

void _re_toggle () {
    if (!resident_editor) return;
    if (resident_editor->active)
        resident_editor->deactivate();
    else
        resident_editor->activate();
}

New_Command _re_toggle_cmd ("re_toggle", "Toggle the Resident_Editor interface.", 0, _re_toggle);

void _re_edit () {
    if (!resident_editor) return;
    resident_editor->re_edit();
}
New_Command _re_edit_cmd ("re_edit", "Edit the selected object's textual representation with $EDITOR.", 0, _re_edit);
void _re_duplicate () {
    if (!resident_editor) return;
    resident_editor->re_duplicate();
}
New_Command _re_duplicate_cmd ("re_duplicate", "Duplicate the selected object.", 0, _re_duplicate);
void _re_delete () {
    if (!resident_editor) return;
    resident_editor->re_delete();
}
New_Command _re_delete_cmd ("re_delete", "Delete the selected object.", 0, _re_delete);
