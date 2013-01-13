
#include <GL/gl.h>
#include <SOIL/SOIL.h>
#include "../../util/inc/Vec.h"
#include "../inc/vis.h"

namespace vis {

    ResourceGroup Image::all ("images");

    bool Image::load () {
        int w; int h; int ch;
        uint8* data = SOIL_load_image(name.c_str(), &w, &h, &ch, 4);
        if (!data) return false;
        GLuint newtex = SOIL_load_OGL_texture_from_memory(
            data, w*h*ch, 4, SOIL_CREATE_NEW_ID, 0
        );
        if (!newtex) return false;
        glBindTexture(GL_TEXTURE_2D, newtex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        unload();
        tex = newtex;
        return true;
    }
    bool Image::reload () { return load(); }
    bool Image::unload () {
        glDeleteTextures(1, &tex);
        tex = 0;
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

    void draw_image (Image* img, Vec p, bool fliph, bool flipv) {
        if (!img) return;
        glBindTexture(GL_TEXTURE_2D, img->tex);
         // Direct Mode is still the easiest for drawing individual images.
        glBegin(GL_QUADS);
            glTexCoord2f(!fliph, !!flipv); glVertex2f(p.x, p.y);
            glTexCoord2f(!!fliph, !!flipv); glVertex2f(p.x + img->w*PX, p.y);
            glTexCoord2f(!!fliph, !flipv); glVertex2f(p.x + img->w*PX, p.y + img->h*PX);
            glTexCoord2f(!fliph, !flipv); glVertex2f(p.x, p.y + img->h*PX);
        glEnd();
    }

    void finish_draw () {
         // Leave flipping to core, so we don't have to include glfw
    }

}

HCB_BEGIN(vis::Image)
    using namespace vis;
    get_id([](const Image& i){ return i.name; });
    find_by_id([](std::string id){
        Image* p = (Image*)(Resource*)Image::all.loaded[id];
        if (!p) {
            p = new Image (id);
            if (!p->load()) {
                delete p;
                return (Image*)NULL;
            }
            Image::all.loaded.emplace(id, p);
        }
        return p;
    });
HCB_END(vis::Image)

