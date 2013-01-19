#include <stdexcept>
#include <GL/gl.h>
#include <GL/glfw.h>
#include <SOIL/SOIL.h>
#include "../../hacc/inc/everything.h"
#include "../inc/images.h"

namespace vis {

    static GLenum diagnose_opengl (std::string s = "") {
        GLenum err = glGetError();
        if (err)
            fprintf(stderr, "OpenGL error %04x %s\n", err, s.c_str());
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


