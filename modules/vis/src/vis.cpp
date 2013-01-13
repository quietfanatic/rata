
#include <GL/gl.h>
#include <SOIL/SOIL.h>
#include "../inc/vis.h"

namespace vis {

    ResourceGroup Image::all ("images");

    bool Image::load () {
        GLuint newtex = SOIL_load_OGL_texture(
            name.c_str(), 4, tex, 0
        );
        if (!newtex) return false;
        glBindTexture(GL_TEXTURE_2D, newtex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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

