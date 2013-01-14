
#include <GL/gl.h>
#include <SOIL/SOIL.h>
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
        uint8* data = SOIL_load_image(name.c_str(), &iw, &ih, &ich, 4);
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
        tex = newtex; w = iw; h = ih;
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

    void draw_image (Image* img, Vec p, bool fliph, bool flipv) {
        if (!img) return;
        glBindTexture(GL_TEXTURE_2D, img->tex);
         // Direct Mode is still the easiest for drawing individual images.
        glBegin(GL_QUADS);
            glTexCoord2f(!fliph, !flipv); glVertex2f(p.x, p.y);
            glTexCoord2f(!!fliph, !flipv); glVertex2f(p.x + img->w*PX, p.y);
            glTexCoord2f(!!fliph, !!flipv); glVertex2f(p.x + img->w*PX, p.y + img->h*PX);
            glTexCoord2f(!fliph, !!flipv); glVertex2f(p.x, p.y + img->h*PX);
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

