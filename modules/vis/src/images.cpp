#include <stdexcept>
#include <GL/gl.h>
#include <GL/glfw.h>
#include <SOIL/SOIL.h>
#include "../../hacc/inc/everything.h"
#include "../inc/images.h"
#include "../inc/shaders.h"  // for glproc

namespace vis {

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

    struct Layout_VBO_Data {
        Vec lbp;
        Vec lbt;
        Vec rbp;
        Vec rbt;
        Vec rtp;
        Vec rtt;
        Vec ltp;
        Vec ltt;
    };

    void Layout::finish () {
        for (Frame& f : frames)
            f.parent = this;
        static auto glGenBuffers = glproc<void (GLsizei, GLuint*)>("glGenBuffers");
        static auto glBindBuffer = glproc<void (GLenum, GLuint)>("glBindBuffer");
        static auto glBufferData = glproc<void (GLenum, GLsizeiptr, const GLvoid*, GLenum)>("glBufferData");
        static auto glGenVertexArrays = glproc<void (GLsizei, GLuint*)>("glGenVertexArrays");
        static auto glBindVertexArray = glproc<void (GLuint)>("glBindVertexArray");
        static auto glEnableVertexAttribArray = glproc<void (GLuint)>("glEnableVertexAttribArray");
        static auto glVertexAttribPointer = glproc<void (GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*)>("glVertexAttribPointer");
         // Create OpenGL VBO
        Layout_VBO_Data data [frames.size()];
        for (uint i = 0; i < frames.size(); i++) {
            data[i].lbp = frames[i].box.lb() * PX;
            data[i].lbt = frames[i].offset + frames[i].box.lb();
            data[i].lbt.x /= size.x;
            data[i].lbt.y /= size.y;
            data[i].lbt.y = 1 - data[i].lbt.y;
            data[i].rbp = frames[i].box.rb() * PX;
            data[i].rbt = frames[i].offset + frames[i].box.rb();
            data[i].rbt.x /= size.x;
            data[i].rbt.y /= size.y;
            data[i].rbt.y = 1 - data[i].rbt.y;
            data[i].rtp = frames[i].box.rt() * PX;
            data[i].rtt = frames[i].offset + frames[i].box.rt();
            data[i].rtt.x /= size.x;
            data[i].rtt.y /= size.y;
            data[i].rtt.y = 1 - data[i].rtt.y;
            data[i].ltp = frames[i].box.lt() * PX;
            data[i].ltt = frames[i].offset + frames[i].box.lt();
            data[i].ltt.x /= size.x;
            data[i].ltt.y /= size.y;
            data[i].ltt.y = 1 - data[i].ltt.y;
        }
        printf("Some texture coordinates: [%g %g] [%g %g] [%g %g] [%g %g]\n",
            data[0].lbt.x, data[0].lbt.y, data[0].rbt.x, data[0].rbt.y,
            data[0].rtt.x, data[0].rtt.y, data[0].ltt.x, data[0].ltt.y
        );
        glGenBuffers(1, &vbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferData(GL_ARRAY_BUFFER, frames.size() * sizeof(Layout_VBO_Data), data, GL_STATIC_DRAW);
         // And create the VAO.
        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
         // index, n_elements, type, normalize, stride, offset
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Layout_VBO_Data) / 4, (void*)offsetof(Layout_VBO_Data, lbp));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Layout_VBO_Data) / 4, (void*)offsetof(Layout_VBO_Data, lbt));

    }
    Layout::~Layout () {
        static auto glDeleteBuffers = glproc<void (GLsizei, const GLuint*)>("glDeleteBuffers");
        static auto glDeleteVertexArrays = glproc<void (GLsizei, const GLuint*)>("glDeleteVertexArrays");
        glDeleteBuffers(1, &vbo_id);
        glDeleteVertexArrays(1, &vao_id);
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
    finish([](Layout& layout){
        layout.finish();
    });
HCB_END(Layout)
 // For convenience
HCB_INSTANCE(std::unordered_map<std::string HCB_COMMA Layout>)


