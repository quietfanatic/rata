#include "../inc/common.h"
#include "../inc/tiles.h"
#include "../inc/sprites.h"
#include "../inc/graffiti.h"
#include "../inc/text.h"
#include "../../hacc/inc/everything.h"
#include "../../core/inc/opengl.h"

namespace vis {

    using namespace core;

    Vec camera_pos = Vec(10, 7.5);

    struct Renderers {
        Tiles_Renderer tiles;
        Sprites_Renderer sprites;
        Graffiti_Renderer graffiti;
        Text_Renderer text;
        void run () {
            glClearColor(0.5, 0.5, 0.5, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            tiles.run();
            sprites.run();
            graffiti.run();
            text.run();
        }
    };
    Renderers* renderers = NULL;

    void init () {
        if (!renderers)
            renderers = new Renderers;
    }
    void render () {
        init();
        renderers->run();
    }

} using namespace vis;

