#include "../inc/common.h"
#include "../inc/color.h"
#include "../inc/images.h"
#include "../inc/text.h"
#include "../inc/tiles.h"
#include "../../hacc/inc/everything.h"
#include "../../core/inc/opengl.h"
#include "../../core/inc/window.h"
#include "../../core/inc/commands.h"
#include "../../util/inc/debug.h"

namespace vis {

    Logger logger ("vis");

    using namespace core;

    Vec camera_pos = Vec(20, 15);
    Vec camera_size = Vec(20, 15);
    Vec global_camera_pos = camera_pos;
    Vec global_camera_size = camera_size;

    bool initted = false;
    GLuint world_fb = 0;
    GLuint world_tex = 0;
    GLuint world_depth_rb = 0;
    Vec rtt_camera_size = Vec(NAN, NAN);

    RGBf ambient_light = RGBf{1, 1, 1};
    RGBf diffuse_light = RGBf{1, 1, 1};
    RGBf radiant_light = RGBf{1, 1, 1};
    int light_debug_type = 0;

    struct Light_Program : Cameraed_Program {
        GLint model_pos;
        GLint ambient;
        GLint diffuse;
        GLint radiant;
        GLint materials_length;
        GLint debug_type;
        void finish () {
            Cameraed_Program::finish();
            glUniform1i(require_uniform("tex"), 0);
            glUniform1i(require_uniform("materials"), 1);
            model_pos = require_uniform("model_pos");
            ambient = require_uniform("ambient");
            diffuse = require_uniform("diffuse");
            radiant = require_uniform("radiant");
            materials_length = require_uniform("materials_length");
            debug_type = require_uniform("debug_type");
        }
        void Program_begin () override {
            Cameraed_Program::Program_begin();
            glUniform1i(debug_type, light_debug_type);
        }
    };
    Light_Program* light_program = NULL;

    void Materials::update () {
        if (items.size() > 256) {
            throw hacc::X::Logic_Error(
                "Material palette is too large: " + std::to_string(items.size()) + " > 256"
            );
        }
        if (!tex) {
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
         // The palette texture is internally laid out vertically.
        uint8 pdat [256 * 4 * 3];
        size_t j = 0;
        for (size_t i = 0; i < items.size(); i++) {
            pdat[j++] = items[i].ambient >> 24;
            pdat[j++] = items[i].ambient >> 16;
            pdat[j++] = items[i].ambient >> 8;
            pdat[j++] = items[i].ambient;
            pdat[j++] = items[i].diffuse >> 24;
            pdat[j++] = items[i].diffuse >> 16;
            pdat[j++] = items[i].diffuse >> 8;
            pdat[j++] = items[i].diffuse;
            pdat[j++] = items[i].radiant >> 24;
            pdat[j++] = items[i].radiant >> 16;
            pdat[j++] = items[i].radiant >> 8;
            pdat[j++] = items[i].radiant;
        }
        for (size_t i = items.size(); i < 256; i++) {
            pdat[j++] = 0x00;
            pdat[j++] = 0x00;
            pdat[j++] = 0x00;
            pdat[j++] = 0x00;
            pdat[j++] = 0x00;
            pdat[j++] = 0x00;
            pdat[j++] = 0x00;
            pdat[j++] = 0x00;
            pdat[j++] = 0x00;
            pdat[j++] = 0xff;
            pdat[j++] = 0x00;
            pdat[j++] = 0xff;
        }
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 3, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, pdat);
        diagnose_opengl("after setting palette");
    }

    Materials::~Materials () {
        if (tex) glDeleteTextures(1, &tex);
    }

    Materials* materials = NULL;

    void set_materials (Materials* m) {
        if (materials != m) {
            materials = m;
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, materials->tex);
            glActiveTexture(GL_TEXTURE0);
        }
    }

     // Set up the requirements for a render-to-texture step
    void setup_rtt () {
        if (rtt_camera_size != camera_size) {
            rtt_camera_size = camera_size;
            logger.log("setting up world framebuffer: %dx%d", (int)(rtt_camera_size.x/PX), (int)(rtt_camera_size.y/PX));
            if (world_fb) glDeleteFramebuffers(1, &world_fb);
            if (world_tex) glDeleteTextures(1, &world_tex);
            if (world_depth_rb) glDeleteRenderbuffers(1, &world_depth_rb);
            glGenFramebuffers(1, &world_fb);
            glBindFramebuffer(GL_FRAMEBUFFER, world_fb);
            glGenTextures(1, &world_tex);
            glBindTexture(GL_TEXTURE_2D, world_tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rtt_camera_size.x/PX, rtt_camera_size.y/PX, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glBindTexture(GL_TEXTURE_2D, 0);
            glGenRenderbuffers(1, &world_depth_rb);
            glBindRenderbuffer(GL_RENDERBUFFER, world_depth_rb);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, rtt_camera_size.x/PX, rtt_camera_size.y/PX);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, world_depth_rb);
             // Do we have to unbind world_tex first?
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, world_tex, 0);
             // Do we have to do this here or when rendering?
            auto ca = GL_COLOR_ATTACHMENT0;
            glDrawBuffers(1, (const GLenum*)&ca);
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
            light_program = hacc::File("vis/res/light.prog").data().attr("prog");
            hacc::manage(&light_program);
            hacc::manage(&materials);
            diagnose_opengl("after loading world.prog");
            initted = true;
            light_program->use();
            glUniform1f(light_program->materials_length, 7);  // TODO: This is not dynamic
        }
    }

    void render () {
        init();
        setup_rtt();
        if (!materials) {
            throw hacc::X::Logic_Error("No materials were set!\n");
        }
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, materials->tex);
        glActiveTexture(GL_TEXTURE0);
         // Start render to texture
        glBindFramebuffer(GL_FRAMEBUFFER, world_fb);
        glViewport(0, 0, rtt_camera_size.x/PX, rtt_camera_size.y/PX);
         // For now, clear to default background material
        glClearColor(1 / 255.0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
         // Map and sprite rendering uses depth and no blend
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        Program::unuse();
         // Use camera
        global_camera_pos.x = round(camera_pos.x/PX) * PX;
        global_camera_pos.y = round(camera_pos.y/PX) * PX;
        global_camera_size = camera_size;
        for (auto& i : Map::items)
            i.Drawn_draw(Map());
        for (auto& i : Sprites::items)
            i.Drawn_draw(Sprites());
         // Now render from world fb to window fb
        glDisable(GL_DEPTH_TEST);
         // Light renders blend by adding.
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
         // Clear to black, to make blending work.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        light_program->use();
        glUniform3fv(light_program->ambient, 1, &ambient_light.r);
        glUniform3fv(light_program->diffuse, 1, &diffuse_light.r);
        glUniform3fv(light_program->radiant, 1, &radiant_light.r);
        glUniform2f(light_program->model_pos, 0, 0);
        diagnose_opengl("after setting light");
        glBindTexture(GL_TEXTURE_2D, world_tex);
        Vec pts [4];
        pts[0] = global_camera_pos - global_camera_size/2;
        pts[2] = global_camera_pos + global_camera_size/2;
        pts[1] = Vec(pts[2].x, pts[0].y);
        pts[3] = Vec(pts[0].x, pts[2].y);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec), pts);
        glDrawArrays(GL_QUADS, 0, 4);
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
        if (camera_size != Vec(20, 15)) {
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
    Links<Drawn<Overlay>> Overlay::items;
    Links<Drawn<Hud>> Hud::items;
    Links<Drawn<Dev>> Dev::items;

} using namespace vis;

HACCABLE(vis::Map) { name("vis::Map"); }
HACCABLE(vis::Sprites) { name("vis::Sprites"); }
HACCABLE(vis::Overlay) { name("vis::Overlay"); }
HACCABLE(vis::Hud) { name("vis::Hud"); }
HACCABLE(vis::Dev) { name("vis::Dev"); }

HACCABLE(RGBf) {
    name("vis::RGBf");
    elem(member(&RGBf::r));
    elem(member(&RGBf::g));
    elem(member(&RGBf::b));
}

HACCABLE(Material) {
    name("vis::Material");
    elem(member(&Material::ambient));
    elem(member(&Material::diffuse));
    elem(member(&Material::radiant));
}

HACCABLE(Materials) {
    name("vis::Materials");
    delegate(member(&Materials::items));
    finish([](Materials& v){ v.update(); });
}

HACCABLE(Light_Program) {
    name("vis::Light_Program");
    delegate(base<Program>());
    finish(&Light_Program::finish);
}

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

static void _light_debug () {
    light_debug_type++;
    if (light_debug_type > 3)
        light_debug_type = 0;
    print_to_console("Light debug type: " + std::to_string(light_debug_type) + "\n");
}

core::New_Command _light_debug_cmd (
    "light_debug", "Toggle between light debug modes",
    0, _light_debug
);
