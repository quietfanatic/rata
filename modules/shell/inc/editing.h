#ifndef HAVE_GEO_EDITING_H
#define HAVE_GEO_EDITING_H

#include <string>
#include "shell/inc/menus.h"
#include "core/inc/window.h"
#include "vis/inc/common.h"
namespace geo { struct Room; struct Resident; struct Tilemap; }
namespace vis { struct Font; struct Texture; }

namespace shell {

    struct Tile_Editor : vis::Drawn<vis::Map>, core::Listener {
        geo::Tilemap* tilemap = NULL;
        uint16 tile = 0x0001;
        bool clicking = false;
        bool showing_selector = false;
        util::Vec selector_pos = util::Vec(-10020, -9985);

        vis::Camera selector_camera;

        Tile_Editor ();
        ~Tile_Editor ();
        void activate ();
        void deactivate ();

        bool in_bounds (util::Vec);
        uint16& tile_at (util::Vec);
        void draw (util::Vec);
        void pick (util::Vec);

        void Drawn_draw (vis::Map) override;
        bool Listener_button (int, int) override;
        bool Listener_key (int, int) override;
        void Listener_cursor_pos (int, int) override;
        int Listener_trap_cursor () override { return selector_camera.active ? false : -1; }
    };
    extern Tile_Editor* tile_editor;

    struct Texture_Tester : vis::Drawn<vis::Map>, vis::Drawn<vis::Sprites>, vis::Drawn<vis::Overlay>, core::Listener {
        vis::Texture* tex = NULL;
        util::Vec pos = util::Vec(10000, 10000);
        uint layer = 1;
        vis::Camera camera;

        Texture_Tester ();
        ~Texture_Tester ();

        void activate ();
        void deactivate ();

        void Drawn_draw (vis::Map) override;
        void Drawn_draw (vis::Sprites) override;
        void Drawn_draw (vis::Overlay) override;

        bool Listener_key (int, int) override;
        void Listener_cursor_pos (int, int) override;
        int Listener_trap_cursor () override { return camera.active ? false : -1; }
    };
    extern Texture_Tester* texture_tester;

}

#endif
