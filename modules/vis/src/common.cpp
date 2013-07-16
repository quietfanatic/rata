#include "../inc/common.h"
#include "../inc/color.h"
#include "../inc/images.h"
#include "../inc/text.h"
#include "../inc/tiles.h"
#include "../../hacc/inc/everything.h"
#include "../../core/inc/opengl.h"

namespace vis {

    using namespace core;

    Vec camera_pos = Vec(10, 7.5);
    Vec global_camera_pos = camera_pos;

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
    }

    Links<Drawn<Map>> Map::items;
    Links<Drawn<Sprites>> Sprites::items;
    Links<Drawn<Overlay>> Overlay::items;
    Links<Drawn<Hud>> Hud::items;

} using namespace vis;

