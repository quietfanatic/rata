#include <stdexcept>
#include <GL/gl.h>
#include <GL/glfw.h>
#include <SOIL/SOIL.h>
#include "../../hacc/inc/everything.h"
#include "../inc/opengl.h"
#include "../inc/images.h"

namespace core {

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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
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
} using namespace core;

HCB_BEGIN(Texture)
    name("core::Texture");
    attr("name", member(&Texture::name).optional().prepare());
    attr("offset", member(&Texture::offset).optional());
    attr("size", member(&Texture::size).optional());
HCB_END(Texture)

HCB_BEGIN(Image)
    name("core::Image");
    attr("filename", member(&Image::filename));
    attr("textures", member(&Image::textures).optional());
    attrs([](Image& image, std::string name){
        Texture* r = image.texture_named(name);
        if (r) return hacc::Reference(r);
        else throw hacc::X::No_Attr(hacc::Type::CppType<Image>(), name);
    });
    finish([](Image& i){ i.load(); });
HCB_END(core::Image)

