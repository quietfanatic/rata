#include <stdexcept>
#include <GL/gl.h>
#include <GL/glfw.h>
#include <SOIL/SOIL.h>
#include "../../hacc/inc/everything.h"
#include "../inc/vis.h"
#include "../../core/inc/game.h"
#include "../../util/inc/debug.h"
#include "../inc/shaders.h"

namespace vis {

    GLenum diagnose_opengl () {
        GLenum err = glGetError();
        if (err)
            fprintf(stderr, "OpenGL error: %04x\n", err);
        return err;
    }

    void Image::reload () {
        int iw; int ih; int ich;
        uint8* data = SOIL_load_image((name).c_str(), &iw, &ih, &ich, 4);
        if (!data) {
            throw std::logic_error(SOIL_last_result());
        }
        GLuint newtex;
        glGenTextures(1, &newtex);
        glBindTexture(GL_TEXTURE_2D, newtex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iw, ih, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        free(data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        if (diagnose_opengl()) {
            throw std::logic_error("Please look up the above OpenGL error code.");
        }
        unload();
        tex = newtex; size = Vec(iw, ih);
    }
    void Image::unload () {
        if (tex) {
            glDeleteTextures(1, &tex);
            tex = 0;
        }
    }
    Image::Image (std::string name) : Resource(name) { reload(); }

    static Logger draw_img_logger ("draw_img", false);

    void draw_img (Image* img, SubImg* sub, Vec p, bool fliph, bool flipv) {
        if (draw_img_logger.on) {
            draw_img_logger.log("img: %s sub: [%g %g] [%g %g %g %g] p: [%g %g] fliph: %u flipv: %u",
                img ? img->name.c_str() : "NULL", sub ? sub->pos.x : 0/0.0, sub ? sub->pos.y : 0/0.0,
                sub ? sub->box.l : 0/0.0, sub ? sub->box.b : 0/0.0, sub ? sub->box.r : 0/0.0, sub ? sub->box.t : 0/0.0,
                p.x, p.y, fliph, flipv
            );
        }
        if (!img || !sub) return;
        float tl, tb, tr, tt, vl, vb, vr, vt;
        if (fliph) {
            tl = sub->pos.x - sub->box.r;
            tr = sub->pos.x - sub->box.l;
            vl = p.x - sub->box.r * PX;
            vr = p.x - sub->box.l * PX;
        }
        else {
            tl = sub->pos.x + sub->box.l;
            tr = sub->pos.x + sub->box.r;
            vl = p.x + sub->box.l * PX;
            vr = p.x + sub->box.r * PX;
        }
        if (flipv) {
            tb = sub->pos.y - sub->box.t;
            tt = sub->pos.y - sub->box.b;
            vb = p.y - sub->box.t * PX;
            vt = p.y - sub->box.b * PX;
        }
        else {
            tb = sub->pos.y + sub->box.b;
            tt = sub->pos.y + sub->box.t;
            vb = p.y + sub->box.b * PX;
            vt = p.y + sub->box.t * PX;
        }
        tl /= img->size.x;
        tb /= img->size.y;
        tr /= img->size.x;
        tt /= img->size.y;
        if (draw_img_logger.on) {
            draw_img_logger.log("tex: [%g %g %g %g] vert [%g %g %g %g]",
                tl, tb, tr, tt, vl, vb, vr, vt
            );
        }
        hacc::require_id<Program>("modules/vis/res/test-red.prog")->use();
        glBindTexture(GL_TEXTURE_2D, img->tex);
         // Direct Mode is still the easiest for drawing individual images.
        glBegin(GL_QUADS);
            glTexCoord2f(tl, 1-tb); glVertex2f(vl, vb);
            glTexCoord2f(tr, 1-tb); glVertex2f(vr, vb);
            glTexCoord2f(tr, 1-tt); glVertex2f(vr, vt);
            glTexCoord2f(tl, 1-tt); glVertex2f(vl, vt);
        glEnd();
    }

    struct Camera_Setup_Layer : core::Layer {
        Camera_Setup_Layer () : core::Layer("B.M", "camera_setup") { }
        void run () {
            glClearColor(0.5, 0.5, 0.5, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glTranslatef(-1, -1, 0);  // 0,0 starts out in the center
            glScalef(1/10.0, 1/7.5, 1);
             // Make coordinates point to pixels, not the corners between pixels
            glTranslatef(0.45*PX/2, 0.45*PX/2, 0);
            glEnable(GL_TEXTURE_2D);
             // Blending in lieu of real shaders
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    } csl;

    struct Test_Layer : core::Layer {
        Test_Layer () : core::Layer("C.M", "test") { }
        void run () {
            static vis::Image* test_image = hacc::require_id<vis::Image>("modules/vis/res/test.png");
            static auto layout = hacc::new_from_file<Hash<SubImg>>("modules/vis/res/test.hacc");
            static vis::SubImg* white = &layout->at("white");
            static vis::SubImg* red = &layout->at("red");
            static vis::SubImg* green = &layout->at("green");
            static vis::SubImg* blue = &layout->at("blue");

            vis::draw_img(test_image, white, Vec(2, 2), false, false);
            vis::draw_img(test_image, red, Vec(18, 2), false, false);
            vis::draw_img(test_image, green, Vec(18, 13), false, false);
            vis::draw_img(test_image, blue, Vec(2, 13), false, false);
        }
    } test_layer;

    void Image_Drawer::appear () {
        link(image_drawers);
    }
    void Image_Drawer::disappear () {
        unlink();
    }

    Links<Image_Drawer> image_drawers;

    struct Image_Drawer_Layer : core::Layer {
        Image_Drawer_Layer () : core::Layer("D.M", "image_drawing") { }
        void run () {
            for (Image_Drawer* p = image_drawers.first(); p; p = p->next()) {
                p->draw();
            }
        }
    } idl;

}

using namespace vis;

static ResourceGroup images ("images");
HCB_BEGIN(Image)
    using namespace vis;
    type_name("vis::Image");
    resource_haccability<Image, &images>();
HCB_END(vis::Image)

HCB_BEGIN(SubImg)
    using namespace vis;
    type_name("vis::SubImg");
    attr("pos", member(&SubImg::pos));
    elem(member(&SubImg::pos));
    attr("box", member(&SubImg::box));
    elem(member(&SubImg::box));
HCB_END(vis::SubImg)

HCB_INSTANCE(std::unordered_map<std::string HCB_COMMA vis::SubImg>);

