#ifndef HAVE_GEO_EDITING_H
#define HAVE_GEO_EDITING_H

#include <string>
#include "shell/inc/menus.h"
#include "core/inc/window.h"
#include "geo/inc/camera.h"
#include "vis/inc/common.h"
namespace geo { struct Room; struct Resident; struct Tilemap; }
namespace vis { struct Font; struct Texture; }

namespace shell {

    struct Room_Editor : vis::Drawn<vis::Overlay>, vis::Drawn<vis::Dev>, core::Listener {
         // Associated utilities
        Menu<vis::Dev>* res_menu = NULL;  // Set on construction
        Menu<vis::Dev>* room_menu = NULL;  // Set on construction
        geo::Free_Camera fc;
        vis::Font* font = NULL;
         // Selected items
        geo::Resident* hovering = NULL;
        geo::Resident* selected = NULL;
        std::string selected_type_name;
        geo::Room* hovering_room = NULL;
        geo::Room* selected_room = NULL;
         // Modes
        bool editing_pts = false;
         // Cursor control
        bool clicking = false;
        bool dragging = false;
        util::Vec drag_origin;
        util::Vec drag_offset;
        util::Vec menu_world_pos;
        int dragging_pt = -1;
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
        void re_edit_room ();
        void re_reload_room ();
        void re_save_room ();
        void re_new_actor (hacc::Type type, hacc::Tree data);
        void re_new_furniture (hacc::Type type, hacc::Tree data);

        void Drawn_draw (vis::Overlay) override;
        void Drawn_draw (vis::Dev) override;
        bool Listener_event (SDL_Event*) override;
    };
    extern Room_Editor* room_editor;

    struct Tile_Editor : vis::Drawn<vis::Map>, core::Listener {
        geo::Tilemap* tilemap = NULL;
        uint16 tile = 0x0001;
        bool clicking = false;
        bool showing_selector = false;
        util::Vec selector_pos = util::Vec(-10020, -9985);

        geo::Free_Camera selector_camera;

        Tile_Editor ();
        ~Tile_Editor ();
        void activate ();
        void deactivate ();

        bool in_bounds (util::Vec);
        uint16& tile_at (util::Vec);
        void draw (util::Vec);
        void pick (util::Vec);

        void Drawn_draw (vis::Map) override;
        bool Listener_event (SDL_Event* event) override;
    };
    extern Tile_Editor* tile_editor;

    struct Texture_Tester : vis::Drawn<vis::Map>, vis::Drawn<vis::Sprites>, vis::Drawn<vis::Overlay>, core::Listener {
        vis::Texture* tex = NULL;
        util::Vec pos = util::Vec(10000, 10000);
        uint layer = 1;
        geo::Free_Camera camera;

        Texture_Tester ();
        ~Texture_Tester ();

        void activate ();
        void deactivate ();

        void Drawn_draw (vis::Map) override;
        void Drawn_draw (vis::Sprites) override;
        void Drawn_draw (vis::Overlay) override;

        bool Listener_event (SDL_Event*) override;
    };
    extern Texture_Tester* texture_tester;

}

#endif
