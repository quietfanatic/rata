#include "../inc/common.h"
#include "../inc/color.h"
#include "../inc/images.h"
#include "../inc/text.h"
#include "../inc/tiles.h"
#include "../../hacc/inc/everything.h"
#include "../../core/inc/opengl.h"
#include "../../core/inc/window.h"

namespace vis {

    using namespace core;

    Vec camera_pos = Vec(10, 7.5);
    Vec camera_size = Vec(10, 7.5);
    Vec global_camera_pos = camera_pos;
    Vec global_camera_size = camera_size;

    bool initted = false;

    void init () {
        if (!initted) {
            color_init();
            images_init();
            text_init();
            tiles_init();
            initted = true;
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    }
    void render () {
        init();
         // For now, clear to 50% grey
        glClearColor(0.5, 0.5, 0.5, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
         // Map and sprite rendering uses depth and no blend
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
         // Use camera
        global_camera_pos = camera_pos;
        global_camera_size = camera_size;
        Program::unuse();
        for (auto& i : Map::items)
            i.Drawn_draw(Map());
        for (auto& i : Sprites::items)
            i.Drawn_draw(Sprites());
         // Overlay rendering uses blend and no depth
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        for (auto& i : Overlay::items)
            i.Drawn_draw(Overlay());
         // Turn off camera
        global_camera_pos = Vec(10, 7.5);
        Program::unuse();
        for (auto& i : Hud::items)
            i.Drawn_draw(Hud());
         // Draw normal camera boundaries if camera_size is too big
        if (camera_size != Vec(20, 15)) {
            Vec pts [4];
            float h = 10 + 0.5*PX;
            float v = 7.5 + 0.5*PX;
            pts[0] = Vec(10-h, 7.5-v);
            pts[1] = Vec(10+h, 7.5-v);
            pts[2] = Vec(10+h, 7.5+v);
            pts[3] = Vec(10-h, 7.5+v);
            draw_color(0x0000007f);
            draw_loop(4, pts);
        }
         // Turn off camera scaling
        global_camera_size = Vec(core::window->width, core::window->height)*PX;
        global_camera_pos = global_camera_size / 2;
        Program::unuse();
        for (auto& i : Dev::items)
            i.Drawn_draw(Dev());
    }

    Links<Drawn<Map>> Map::items;
    Links<Drawn<Sprites>> Sprites::items;
    Links<Drawn<Overlay>> Overlay::items;
    Links<Drawn<Hud>> Hud::items;
    Links<Drawn<Dev>> Dev::items;

} using namespace vis;

