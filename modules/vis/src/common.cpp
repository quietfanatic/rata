#include "vis/src/common_internal.h"

#include "core/inc/commands.h"
#include "core/inc/opengl.h"
#include "core/inc/window.h"
#include "hacc/inc/everything.h"
#include "util/inc/debug.h"
#include "vis/inc/color.h"
#include "vis/inc/images.h"
#include "vis/inc/light.h"
#include "vis/inc/text.h"
#include "vis/inc/tiles.h"
using namespace core;
using namespace util;

namespace vis {

    Vec global_camera_pos = Vec(10, 7.5);
    Vec global_camera_size = Vec(20, 15);

    bool initted = false;
    GLuint world_fb = 0;
    GLuint world_tex = 0;
    GLuint world_depth_rb = 0;
    Vec rtt_camera_size = Vec(NAN, NAN);

    RGBf ambient_light = 1;
    RGBf diffuse_light = 1;
    RGBf radiant_light = 1;

     // Set up the requirements for a render-to-texture step
    void setup_rtt () {
        if (rtt_camera_size != global_camera_size) {
            rtt_camera_size = global_camera_size;
            log("vis", "setting up world framebuffer: %dx%d", (int)(rtt_camera_size.x/PX), (int)(rtt_camera_size.y/PX));
            if (world_fb) glDeleteFramebuffers(1, &world_fb);
            if (world_tex) glDeleteTextures(1, &world_tex);
            if (world_depth_rb) glDeleteRenderbuffers(1, &world_depth_rb);
            glGenFramebuffers(1, &world_fb);
            glBindFramebuffer(GL_FRAMEBUFFER, world_fb);
            glGenTextures(1, &world_tex);
            glBindTexture(GL_TEXTURE_2D, world_tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rtt_camera_size.x/PX, rtt_camera_size.y/PX, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glBindTexture(GL_TEXTURE_2D, 0);
            glGenRenderbuffers(1, &world_depth_rb);
            glBindRenderbuffer(GL_RENDERBUFFER, world_depth_rb);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, rtt_camera_size.x/PX, rtt_camera_size.y/PX);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, world_depth_rb);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, world_tex, 0);
            diagnose_opengl("after setting up render-to-texture capability");
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                throw hacc::X::Logic_Error("Framebuffer creationg failed!\n");
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }

    void init () {
        if (!initted) {
            color_init();
            images_init();
            text_init();
            tiles_init();
            light_init();
            initted = true;
        }
    }

    void render () {
        init();
        setup_rtt();
         // Start render to texture
        glBindFramebuffer(GL_FRAMEBUFFER, world_fb);
        glViewport(0, 0, rtt_camera_size.x/PX, rtt_camera_size.y/PX);
         // For now, clear to default background material
        glClearColor(1 / 255.0, 0, 0, 0);
        glClearDepth(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
         // Map rendering uses depth and no blend
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_GEQUAL);
        set_draw_phase(true);
        Program::unuse();
         // Use camera
        if (camera) {
            global_camera_pos.x = round(camera->pos.x/PX) * PX;
            global_camera_pos.y = round(camera->pos.y/PX) * PX;
            global_camera_size = camera->size;
        }
        else {
            global_camera_pos = Vec(10, 7.5);
            global_camera_size = Vec(20, 15);
        }
        for (auto& i : Map::items)
            i.Drawn_draw(Map());
         // Sprite rendering uses blend for shadows
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_SRC_ALPHA);
        for (auto& i : Sprites::items)
            i.Drawn_draw(Sprites());
         // Now render from world fb to window fb
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        set_draw_phase(false);
         // Light renders blend by adding.
        if (light_debug_type == 1)
            glDisable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
         // Clear to black, to make blending work.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        diagnose_opengl("before setting world tex");
        light_texture(world_tex);
        diagnose_opengl("after setting world tex");

        for (auto& i : Lights::items)
            i.Drawn_draw(Lights());
         // One global light render finally
        set_ambient(ambient_light);
        set_diffuse(diffuse_light);
        set_radiant(radiant_light);
        light_offset(Vec(0, 0));
        diagnose_opengl("after setting light");
        Vec pts [4];
        pts[0] = global_camera_pos - global_camera_size/2;
        pts[2] = global_camera_pos + global_camera_size/2;
        pts[1] = Vec(pts[2].x, pts[0].y);
        pts[3] = Vec(pts[0].x, pts[2].y);
        draw_light(4, pts);
         // Overlay rendering uses blend and no depth
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        Program::unuse();
        for (auto& i : Overlay::items)
            i.Drawn_draw(Overlay());
         // Turn off camera position
        global_camera_pos = Vec(10, 7.5);
        Program::unuse();
        for (auto& i : Hud::items)
            i.Drawn_draw(Hud());
         // Draw normal camera boundaries if camera_size is too big
        if (global_camera_size != Vec(20, 15)) {
            color_offset(Vec(10, 7.5));
            float h = 10 + 0.5*PX;
            float v = 7.5 + 0.5*PX;
            draw_color(0x0000007f);
            draw_rect(Rect(-h, -v, h, v));
        }
         // Turn off camera scaling
        global_camera_size = Vec(core::window->width, core::window->height)*PX;
        global_camera_pos = global_camera_size / 2;
        Program::unuse();
         // Zoom the viewport
        glDisable(GL_BLEND);
        glPixelZoom(window->width*PX/rtt_camera_size.x, window->height*PX/rtt_camera_size.y);
        glCopyPixels(0, 0, rtt_camera_size.x/PX, rtt_camera_size.y/PX, GL_COLOR);
         // Draw dev layer
        glEnable(GL_BLEND);
        glViewport(0, 0, window->width, window->height);
        for (auto& i : Dev::items)
            i.Drawn_draw(Dev());
    }

    Links<Drawn<Map>> Map::items;
    Links<Drawn<Sprites>> Sprites::items;
    Links<Drawn<Lights>> Lights::items;
    Links<Drawn<Overlay>> Overlay::items;
    Links<Drawn<Hud>> Hud::items;
    Links<Drawn<Dev>> Dev::items;

    Camera* camera = NULL;

    void Camera::activate () {
        prev = camera;
        camera = this;
        active = true;
    }
    void Camera::deactivate () {
        for (Camera** cp = &camera; *cp; cp = &(*cp)->prev) {
            if (*cp == this) {
                *cp = prev;
                break;
            }
        }
        active = false;
    }

} using namespace vis;

HACCABLE(vis::Map) { name("vis::Map"); }
HACCABLE(vis::Sprites) { name("vis::Sprites"); }
HACCABLE(vis::Lights) { name("vis::Lights"); }
HACCABLE(vis::Overlay) { name("vis::Overlay"); }
HACCABLE(vis::Hud) { name("vis::Hud"); }
HACCABLE(vis::Dev) { name("vis::Dev"); }

static void _global_lighting (const RGBf& amb, const RGBf& dif, bool relative) {
    if (relative) {
        ambient_light += amb;
        diffuse_light += dif;
    }
    else {
        ambient_light = amb;
        diffuse_light = dif;
    }
}
core::New_Command _global_lighting_cmd (
    "global_lighting", "Set or adjust the lighting; <1> ambient, <2> diffuse, <3> relative",
    2, _global_lighting
);

