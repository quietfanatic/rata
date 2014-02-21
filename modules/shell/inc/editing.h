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

    struct Room_Editor : vis::Drawn<vis::Overlay>, vis::Drawn<vis::Dev>, core::Listener {
         // Associated utilities
        geo::Free_Camera fc;
        Menu<vis::Dev>* res_menu = NULL;  // Set on construction
        Menu<vis::Dev>* room_menu = NULL;  // Set on construction
        vis::Font* font = NULL;
         // Selected items
        geo::Resident* hovering = NULL;
        geo::Resident* selected = NULL;
        geo::Room* hovering_room = NULL;
        geo::Room* selected_room = NULL;
         // Cursor control
        bool clicking = false;
        bool dragging = false;
        Vec drag_origin;
        Vec drag_offset;
        Vec menu_world_pos;
         // Drawing
        std::string status;

        void activate ();
        void deactivate ();
        Room_Editor ();
        ~Room_Editor ();

         // Context menu actions
        void re_edit ();
        void re_duplicate ();
        void re_delete ();
        void re_reload_room ();
        void re_new_actor (hacc::Type type, hacc::Tree data);

        void Drawn_draw (vis::Overlay) override;
        void Drawn_draw (vis::Dev) override;
        bool Listener_button (int, int) override;
        void Listener_cursor_pos (int x, int y) override;
    };
    extern Room_Editor* room_editor;

}

#endif
