#ifndef HAVE_GEO_MENUS_H
#define HAVE_GEO_MENUS_H

#include <memory>
#include "rooms.h"
#include "camera.h"
#include "../../core/inc/window.h"
#include "../../core/inc/commands.h"
#include "../../vis/inc/common.h"
#include "../../vis/inc/text.h"

namespace geo {

     // Menu Items are always positioned by their lower-left corner.
     // Individual items do not get to choose what layer they are drawn on.
    struct Menu_Item {
        virtual Vec Menu_Item_size (Vec area) { return Vec(0, 0); }
        virtual void Menu_Item_draw (Vec pos, Vec area) = 0;
         // These should not check that pos is actually in
         //  their boundary.  That's the container's job.
         // If these return false, the event is cascaded up.
         // pos is relative to the item's lower-left corner.
        virtual bool Menu_Item_hover (Vec pos, Vec area) { return false; }
        virtual bool Menu_Item_click (Vec pos, Vec area) { return false; }
    };

    struct Menu_Base : core::Listener {
        Vec pos = Vec(0, 0);
        Vec size = Vec(INF, INF);
        bool deactivate_on_click = false;
        std::unique_ptr<Menu_Item> root;
        void draw ();

        void hover (Vec pos);
        bool click (Vec pos);
    };
    template <class Layer>
    struct Menu2 : Menu_Base, vis::Drawn<Layer> {
        void Drawn_draw (Layer) override { draw(); }
        void activate () { vis::Drawn<Layer>::appear(); core::Listener::activate(); }
        void deactivate () { vis::Drawn<Layer>::disappear(); core::Listener::deactivate(); }
        void Listener_cursor_pos (int x, int y) {
            hover(camera->window_to_layer<Layer>(x, y));
        }
        bool Listener_button (int x, int y) {
            return click(camera->window_to_layer<Layer>(x, y));
        }
    };

    struct Menu : core::Listener {
        Room room {true};
        Resident* hovering = NULL;
        bool dev_space = false;  // Otherwise use hud positioning

        void activate ();
        void deactivate ();

        bool active ();

        void Listener_cursor_pos (int, int) override;
        bool Listener_button (int, int) override;
    };

    struct Text_Button_Base : Resident {
        Vec pos = Vec(0, 0);
        Vec align = Vec(1, -1);
        vis::Font* font = NULL;
        std::string text;
        core::Command on_click;
        uint32 color = 0xffffffff;
        uint32 background_color = 0x0000007f;
        uint32 hover_color = 0xffffffff;
        uint32 hover_background_color = 0x000000ff;

        void set_text (std::string text);

        Rect boundary;
        bool hovering = false;
        void finish ();

        Vec Resident_get_pos () override;
        void Resident_set_pos (Vec) override;
        Rect Resident_boundary () override;
        void Resident_hover (Vec) override;
        void Resident_click (Vec) override;

        void draw ();
    };

    template <class Layer>
    struct Text_Button : Text_Button_Base, vis::Drawn<Layer> {
        void Drawn_draw (Layer) override {
            draw();
        }
        void Resident_emerge () override { vis::Drawn<Layer>::appear(); }
        void Resident_reclude () override { vis::Drawn<Layer>::disappear(); }
    };

    struct Button : Menu_Item {
        core::Command on_click;
        uint32 color = 0xffffffff;
        uint32 background_color = 0x00000000;
        uint32 hover_color = 0xffffffff;
        uint32 hover_background_color = 0x00000000;
        uint64 hovering_frame = -1;
        bool hovering () { return hovering_frame == core::window->frames_simulated; }
        bool Menu_Item_hover (Vec pos, Vec size) override {
            hovering_frame = core::window->frames_simulated;
            return true;
        }
        bool Menu_Item_click (Vec pos, Vec size) override {
            if (on_click) on_click();
            return true;
        }
        void Menu_Item_draw (Vec, Vec) override;
    };

    struct VBox : Button {
        std::vector<std::unique_ptr<Menu_Item>> contents;
        Vec cached_area;
        Vec cached_size;
        Vec Menu_Item_size (Vec area) override;
        void Menu_Item_draw (Vec, Vec) override;
        bool Menu_Item_hover (Vec, Vec) override;
        bool Menu_Item_click (Vec, Vec) override;

         // Type of contents confuses copy assignability testing
        VBox& operator = (const VBox&) = delete;
    };

    struct Text_Button2 : Button {
        std::string text;
        vis::Font* font = NULL;
        Vec cached_area;
        Vec cached_size;
        Vec Menu_Item_size (Vec area) override;
        void Menu_Item_draw (Vec, Vec) override;
    };

}

#endif
