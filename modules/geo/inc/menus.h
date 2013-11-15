#ifndef HAVE_GEO_MENUS_H
#define HAVE_GEO_MENUS_H

#include "rooms.h"
#include "../../core/inc/window.h"
#include "../../core/inc/commands.h"
#include "../../vis/inc/common.h"
#include "../../vis/inc/text.h"

namespace geo {

     // Menu items use HUD positioning, not Overlay or Dev positioning.
    struct Menu : core::Listener {
        Room room;
        Resident* hovering = NULL;

        void activate ();
        void deactivate ();

        void Listener_cursor_pos (int, int) override;
        bool Listener_button (int, int) override;
    };

    struct Text_Button : Resident, vis::Drawn<vis::Hud> {
        Vec pos = Vec(0, 0);
        Vec align = Vec(1, -1);
        vis::Font* font;
        std::string text;
        core::Command on_click;
        uint32 color = 0xffffffff;
        uint32 background_color = 0x0000007f;
        uint32 hover_color = 0xffffffff;
        uint32 hover_background_color = 0x000000cf;

        void set_text (std::string text);

        Rect boundary;
        bool hovering = false;
        void finish ();

        Vec Resident_get_pos () override;
        void Resident_set_pos (Vec) override;
        Rect Resident_boundary () override;
        void Resident_hover (Vec) override;
        void Resident_click (Vec) override;

        void Drawn_draw (vis::Hud) override;
    };

}

#endif
