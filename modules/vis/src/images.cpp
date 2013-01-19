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
    Image::~Image () { unload(); }

    SubImg* Layout::sub_named (std::string name) {
        for (SubImg& s : subimgs)
            if (s.name == name)
                return &s;
        return NULL;
    }

    void Layout::reload () {
        Layout&& tmp = hacc::value_from_file<Layout>(name);
        subimgs = tmp.subimgs;
    }
    Layout::Layout (std::string name) : Resource(name) { reload(); }
    Layout::Layout () { }

}

using namespace vis;

static ResourceGroup images ("images");
HCB_BEGIN(Image)
    type_name("vis::Image");
    resource_haccability<Image, &images>();
HCB_END(vis::Image)

HCB_BEGIN(SubImg)
    type_name("vis::SubImg");
    attr("name", member(&SubImg::name));
    attr("pos", member(&SubImg::pos));
    attr("box", member(&SubImg::box));
    attr("points", member(&SubImg::points, optional<decltype(((SubImg*)NULL)->points)>()));
    chain_find_by_id<SubImg, Layout>([](Layout* layout, std::string id){
        return layout->sub_named(id);
    });
HCB_END(vis::SubImg)

static ResourceGroup layouts ("layouts");
HCB_BEGIN(Layout)
    type_name("vis::Layout");
    attr("subimgs", member(&Layout::subimgs));
    resource_haccability<Layout, &layouts>();
HCB_END(Layout)

