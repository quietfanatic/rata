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

    void Texture::load (Image* image) {
        if (!size.is_defined()) size = image->size;
        GLuint newtex;
        glGenTextures(1, &newtex);
        glBindTexture(GL_TEXTURE_2D, newtex);
        if (diagnose_opengl("after generating and binding texture")) {
            throw std::logic_error("OpenGL error");
        }
        glPixelStorei(GL_UNPACK_ROW_LENGTH, image->size.x);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, offset.x);
         // We're storing textures upside-down
        glPixelStorei(GL_UNPACK_SKIP_ROWS, image->size.y - (offset.y + size.y));
        if (diagnose_opengl("after setting PixelStore")) {
            throw std::logic_error("OpenGL error");
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data);
        if (!smooth) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        if (diagnose_opengl("after loading a texture")) {
            throw std::logic_error("OpenGL error");
        }
        if (newtex) tex = newtex;
    }
    void Texture::unload () {
        if (tex) {
            glDeleteTextures(1, &tex);
            tex = 0;
        }
    }
    Texture::~Texture () { unload(); }

    void Image::load () {
        int iw; int ih; int ich;
        data = SOIL_load_image(filename.c_str(), &iw, &ih, &ich, 4);
        if (!data) {
            throw std::logic_error(SOIL_last_result());
        }
        size = Vec(iw, ih);
        for (Texture& t : textures) {
            if (t.offset.x < 0 || t.offset.y < 0
             || t.offset.x + t.size.x > size.x
             || t.offset.y + t.size.y > size.y)
                throw std::logic_error("Texture specified outside of image");
        }
        for (Texture& t : textures) {
            t.load(this);
        }
        free(data);
    }

    void Image::unload () {
        for (Texture& t : textures) {
            t.unload();
        }
    }

    Texture* Image::texture_named (std::string name) {
        for (Texture& t : textures)
            if (t.name == name)
                return &t;
        return NULL;
    }

    Frame* Layout::frame_named (std::string name) {
        for (Frame& s : frames)
            if (s.name == name)
                return &s;
        return NULL;
    }

}

using namespace vis;

HCB_BEGIN(Texture)
    type_name("vis::Texture");
    attr("name", member(&Texture::name, def(std::string("ALL"))));
    attr("offset", member(&Texture::offset, def(Vec(0, 0))));
    attr("size", member(&Texture::size, def(Vec())));
    attr("layout", member(&Texture::layout, def((Layout*)NULL)));
HCB_END(Texture)

HCB_BEGIN(Image)
    type_name("vis::Image");
    attr("filename", member(&Image::filename));
    attr("textures", member(&Image::textures, def(std::vector<Texture>(1, Texture()))));
    get_attr([](Image& image, std::string name){
        return image.texture_named(name);
    });
    finish([](Image& i){ i.load(); });
HCB_END(vis::Image)

HCB_BEGIN(Frame)
    type_name("vis::Frame");
    attr("name", member(&Frame::name));
    attr("offset", member(&Frame::offset));
    attr("box", member(&Frame::box));
    attr("points", member(&Frame::points, optional<decltype(((Frame*)NULL)->points)>()));
HCB_END(vis::Frame)

HCB_BEGIN(Layout)
    type_name("vis::Layout");
    attr("size", member(&Layout::size));
    attr("frames", member(&Layout::frames));
    get_attr([](Layout& layout, std::string name){
        return layout.frame_named(name);
    });
HCB_END(Layout)

