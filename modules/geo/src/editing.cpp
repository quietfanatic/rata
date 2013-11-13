#include "../inc/editing.h"
#include "../inc/rooms.h"
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
            if (room.observer_count) {
                for (auto& res : room.residents) {
                    Vec pos = res.Resident_get_pos();
                    if (!pos.is_defined()) {
                        pos = room.boundary.rt() + Vec(0.5, -0.5);
                        pos.x += unpositioned_residents++;
                    }
                    const Rect& boundary = res.Resident_boundary();
                    color_offset(pos);
                    draw_color(0xffffffff);
                    Vec corners [4];
                    corners[0] = boundary.lb();
                    corners[1] = boundary.rb();
                    corners[2] = boundary.rt();
                    corners[3] = boundary.lt();
                    draw_loop(4, corners);
                }
            }
        }
        color_offset(Vec(0, 0));
    }
    Resident_Editor::Resident_Editor () { resident_editor = this; }
    Resident_Editor::~Resident_Editor () {
        if (resident_editor == this)
            resident_editor = NULL;
    }

    void Resident_Editor::activate () {
        logger.log("Activating editor.");
        appear();
        active = true;
    }
    void Resident_Editor::deactivate () {
        logger.log("Deactivating editor.");
        disappear();
        active = false;
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
