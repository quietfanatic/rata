#include "../inc/editing.h"
#include "../inc/rooms.h"
#include "../inc/camera.h"
#include "../../core/inc/commands.h"
#include "../../vis/inc/color.h"
#include "../../util/inc/debug.h"

using namespace util;
using namespace vis;
using namespace core;

namespace geo {

    Logger logger ("editing");

    Resident_Editor* resident_editor = NULL;

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
                    draw_color(0x00ff00ff);
                    draw_rect(boundary);
                }
            }
        }
        color_offset(Vec(0, 0));
    }

    Resident* dragging = NULL;
    Vec dragging_offset;

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
                dragging = picked;
                dragging_offset = realpos - pos;
                logger.log("Dragging @%lx", (size_t)picked);
                return true;
            }
            else {
                dragging = NULL;
                return true;
            }
        }
        return false;
    }
    void Resident_Editor::Listener_cursor_pos (int x, int y) {
        if (dragging) {
            Vec realpos = geo::camera->window_to_world(x, y);
            dragging->Resident_set_pos(realpos - dragging_offset);
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
        appear();
    }
    void Resident_Editor::deactivate () {
        logger.log("Deactivating editor.");
        Listener::deactivate();
        fc.deactivate();
        disappear();
    }

} using namespace geo;

HACCABLE(Resident_Editor) {
    name("geo::Resident_Editor");
}

void _resident_editor () {
    if (!resident_editor) return;
    if (resident_editor->active)
        resident_editor->deactivate();
    else
        resident_editor->activate();
}

New_Command _resident_editor_cmd ("resident_editor", "Toggle the Resident_Editor interface.", 0, _resident_editor);
