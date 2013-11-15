#include <sstream>
#include "../inc/editing.h"
#include "../inc/rooms.h"
#include "../inc/camera.h"
#include "../../hacc/inc/files.h"
#include "../../hacc/inc/documents.h"
#include "../../core/inc/commands.h"
#include "../../vis/inc/color.h"
#include "../../vis/inc/text.h"
#include "../../util/inc/debug.h"

using namespace util;
using namespace vis;
using namespace core;

namespace geo {

    Logger logger ("editing");

    Resident_Editor* resident_editor = NULL;

    hacc::Pointer res_realp (Resident* res) {
        void* derived = hacc::Pointer(res).address_of_type(hacc::Type(typeid(*res)));
        return hacc::Pointer(hacc::Type(typeid(*res)), derived);
    }

    void Resident_Editor::Drawn_draw (Overlay) {
        Vec cursor_pos = camera->window_to_world(window->cursor_x, window->cursor_y);
        size_t unpositioned_residents = 0;
        Resident* new_hovering = NULL;
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
                    if (boundary.covers(cursor_pos - pos)) {
                        new_hovering = &res;
                    }
                }
            }
        }
        color_offset(Vec(0, 0));
        if (new_hovering) {
            if (new_hovering != hovering) {
                auto realp = res_realp(new_hovering);
                std::ostringstream ss;
                ss << realp.address << " " << hacc::address_to_path(realp);
                status = ss.str();
            }
        }
        else {
            status = "";
        }
        hovering = new_hovering;
    }
    void Resident_Editor::Drawn_draw (Dev) {
        if (!status.empty()) {
            draw_color(0x0000007f);
            Vec size = text_size(status, font);
            draw_solid_rect(Rect(0, size.y, size.x, 0));
            draw_text(status, font, Vec(0, 0), Vec(1, -1));
        }
    }

    bool Resident_Editor::Listener_button (int code, int action) {
        if (code == GLFW_MOUSE_BUTTON_LEFT) {
            Vec realpos = geo::camera->window_to_world(window->cursor_x, window->cursor_y);
            if (action == GLFW_PRESS) {
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
                if (!picked) return false;
                Vec pos = picked->Resident_get_pos();
                selected = picked;
                drag_origin = pos;
                drag_offset = realpos - pos;
                logger.log("Dragging @%lx", (size_t)picked);
                clicking = true;
                return true;
            }
            else {
                dragging = NULL;
                clicking = false;
                return true;
            }
        }
        return false;
    }
    void Resident_Editor::Listener_cursor_pos (int x, int y) {
        Vec realpos = geo::camera->window_to_world(x, y);
        if (clicking && selected && (drag_origin - realpos).mag2() > 0.2)
            dragging = selected;
        if (dragging) {
            dragging->Resident_set_pos(realpos - drag_offset);
        }
    }

    Resident_Editor::Resident_Editor () { resident_editor = this; }
    Resident_Editor::~Resident_Editor () {
        if (resident_editor == this)
            resident_editor = NULL;
    }

    void Resident_Editor::activate () {
        logger.log("Activating editor.");
        Listener::activate();
        fc.pos = geo::camera->Camera_pos();
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
    }

     // Context menu actions
    void Resident_Editor::re_delete () {
        if (!selected) return;
        auto realp = res_realp(selected);
        if (hacc::Document* doc = hacc::get_document_containing(realp.address)) {
            realp.type.destruct(realp.address);
            doc->dealloc(realp.address);
        }
        else {
            printf("Could not re_delete: this object does not belong to a document.");
        }
    }

} using namespace geo;

HACCABLE(Resident_Editor) {
    name("geo::Resident_Editor");
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

void _re_delete () {
    if (!resident_editor) return;
    resident_editor->re_delete();
}
New_Command _re_delete_cmd ("re_delete", "Delete the selected object.", 0, _re_delete);
