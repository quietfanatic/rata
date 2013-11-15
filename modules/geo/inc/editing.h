#ifndef HAVE_GEO_EDITING_H
#define HAVE_GEO_EDITING_H

#include "camera.h"
#include "rooms.h"
#include "../../vis/inc/common.h"
#include "../../vis/inc/text.h"
#include "../../core/inc/window.h"

namespace geo {

    struct Resident_Editor : vis::Drawn<vis::Overlay>, vis::Drawn<vis::Dev>, core::Listener {
        Free_Camera fc;
        vis::Font* font = NULL;
        Resident* hovering = NULL;
        Resident* dragging = NULL;
        Vec drag_offset;
        std::string status;
        void activate ();
        void deactivate ();
        Resident_Editor ();
        ~Resident_Editor ();

        void Drawn_draw (vis::Overlay) override;
        void Drawn_draw (vis::Dev) override;
        bool Listener_button (int, int) override;
        void Listener_cursor_pos (int x, int y) override;
    };
    extern Resident_Editor* resident_editor;

}

#endif
