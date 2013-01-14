
#include <GL/gl.h>
#include <SOIL/SOIL.h>
#include "../../hacc/inc/haccable_standard.h"
#include "../inc/vis.h"

namespace vis {

    GLenum diagnose_opengl () {
        GLenum err = glGetError();
        if (err)
            fprintf(stderr, "OpenGL error: %04x\n", err);
        return err;
    }

    ResourceGroup Image::all ("images");

    bool Image::load () {
        int iw; int ih; int ich;
        uint8* data = SOIL_load_image(("modules/" + name).c_str(), &iw, &ih, &ich, 4);
        if (!data) {
            fprintf(stderr, "Failed to load image %s: %s\n", name.c_str(), SOIL_last_result());
            return false;
        }
        GLuint newtex;
        glGenTextures(1, &newtex);
        glBindTexture(GL_TEXTURE_2D, newtex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iw, ih, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        free(data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        if (diagnose_opengl()) {
            fprintf(stderr, "Aborting load of %s\n", name.c_str());
            return false;
        }
        unload();
        tex = newtex; size = Vec(iw, ih);
        return true;
    }
    bool Image::reload () { return load(); }
    bool Image::unload () {
        if (tex) {
            glDeleteTextures(1, &tex);
            tex = 0;
        }
        return true;
    }

    void start_draw () {
        glClearColor(0.5, 0.5, 0.5, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glTranslatef(-1, -1, 0);  // 0,0 starts out in the center
        glScalef(1/10.0/2, 1/7.5/2, 1);
         // Make coordinates point to pixels, not the corners between pixels
        glTranslatef(0.45*PX/2, 0.45*PX/2, 0);
        glEnable(GL_TEXTURE_2D);
    }

    void draw_img (Image* img, SubImg* sub, Vec p, bool fliph, bool flipv) {
        if (!img || !sub) return;
        glBindTexture(GL_TEXTURE_2D, img->tex);
        float tl, tb, tr, tt, vl, vb, vr, vt;
        if (fliph) {
            tl = sub->pos.x + sub->size.x;
            tr = sub->pos.x;
            vl = p.x - sub->size.x + sub->center.x;
            vr = p.x + sub->center.x;
        }
        else {
            tl = sub->pos.x;
            tr = sub->pos.x + sub->size.x;
            vl = p.x - sub->center.x;
            vr = p.x + sub->size.x - sub->center.x;
        }
        if (flipv) {
            tb = sub->pos.y + sub->size.y;
            tt = sub->pos.y;
            vb = p.y - sub->size.y + sub->center.y;
            vt = p.y + sub->center.y;
        }
        else {
            tb = sub->pos.y;
            tt = sub->pos.y + sub->size.y;
            vb = p.y - sub->center.y;
            vt = p.y + sub->size.y - sub->center.y;
        }
        tl /= img->size.x;
        tb /= img->size.y;
        tr /= img->size.x;
        tt /= img->size.y;
         // Direct Mode is still the easiest for drawing individual images.
        glBegin(GL_QUADS);
            glTexCoord2f(tl, 1-tb); glVertex2f(vl, vb);
            glTexCoord2f(tr, 1-tb); glVertex2f(vr, vb);
            glTexCoord2f(tr, 1-tt); glVertex2f(vr, vt);
            glTexCoord2f(tl, 1-tt); glVertex2f(vl, vt);
        glEnd();
    }

    void finish_draw () {
         // Leave flipping to core, so we don't have to include glfw
    }

}

HCB_BEGIN(vis::Image)
    using namespace vis;
    get_id([](const Image& i){ return i.name; });
    find_by_id([](std::string id) -> Image* {
        Image*& p = reinterpret_cast<Image*&>(Image::all.loaded[id]);
        if (!p) {
            p = new Image (id);
            if (!p->load()) {
                fprintf(stderr, "Load of image failed.\n");
                Image::all.loaded.erase(id);
                return NULL;
            }
            
        }
        return p;
    });
HCB_END(vis::Image)

HCB_BEGIN(vis::SubImg)
    using namespace vis;
    attr("pos", member(&SubImg::pos));
    elem(member(&SubImg::pos));
    attr("size", member(&SubImg::size));
    elem(member(&SubImg::size));
    attr("center", member(&SubImg::center));
    elem(member(&SubImg::center));
HCB_END(vis::SubImg)

HCB_BEGIN(vis::Layout1D)
    delegate(member(&vis::Layout1D::subs));
HCB_END(vis::Layout1D)
HCB_INSTANCE(std::unordered_map<std::string HCB_COMMA vis::SubImg>);

