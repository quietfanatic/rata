#ifndef HAVE_GEO_EDITING_H
#define HAVE_GEO_EDITING_H

#include "menus.h"
#include "../../geo/inc/camera.h"
#include "../../geo/inc/rooms.h"
#include "../../vis/inc/common.h"
#include "../../vis/inc/text.h"
#include "../../core/inc/window.h"

namespace shell {
    using namespace util;

    struct Resident_Editor : vis::Drawn<vis::Overlay>, vis::Drawn<vis::Dev>, core::Listener {
        geo::Free_Camera fc;
        vis::Font* font = NULL;
        geo::Resident* hovering = NULL;
        geo::Resident* selected = NULL;
        geo::Resident* dragging = NULL;
        Menu<vis::Dev>* context_menu = NULL;
        bool clicking;
        Vec drag_origin;
        Vec drag_offset;
        std::string status;
        void activate ();
        void deactivate ();
        Resident_Editor ();
        ~Resident_Editor ();

         // Context menu actions
        void re_edit ();
        void re_duplicate ();
        void re_delete ();

        void Drawn_draw (vis::Overlay) override;
        void Drawn_draw (vis::Dev) override;
        bool Listener_button (int, int) override;
        void Listener_cursor_pos (int x, int y) override;
    };
    extern Resident_Editor* resident_editor;

}

#endif
