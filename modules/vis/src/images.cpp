#include "vis/inc/images.h"

#include <string>
#include <vector>
#include <stdexcept>
#include <GL/glew.h>
#include <SOIL/SOIL.h>
#include "core/inc/opengl.h"
#include "core/inc/window.h"
#include "hacc/inc/everything.h"
#include "util/inc/integration.h"
#include "vis/src/common_internal.h"
using namespace util;

namespace vis {

    void Texture::load (Image* image) {
        if (!size.is_defined()) size = image->size;
        GLuint newid;
        glGenTextures(1, &newid);
        glBindTexture(GL_TEXTURE_2D, newid);
        if (core::diagnose_opengl("after generating and binding texture")) {
            throw std::logic_error("OpenGL error");
        }
        glPixelStorei(GL_UNPACK_ROW_LENGTH, image->size.x);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, offset.x);
         // We're storing textures upside-down
        glPixelStorei(GL_UNPACK_SKIP_ROWS, image->size.y - (offset.y + size.y));
        if (core::diagnose_opengl("after setting PixelStore")) {
            throw std::logic_error("OpenGL error");
        }
        uint32* data = use_palettes ? image->processed_data : image->data;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        if (!smooth) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        if (core::diagnose_opengl("after loading a texture")) {
            throw std::logic_error("OpenGL error");
        }
        if (newid) id = newid;
        glBindTexture(GL_TEXTURE_2D, 0);
         // Reset these to avoid confusing other functions
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    }
    void Texture::unload () {
        if (id) {
            glDeleteTextures(1, &id);
            id = 0;
        }
    }
    Texture::~Texture () { unload(); }

    void Image::load () {
         // Can't use OpenGL if there isn't a window, bleh
        if (!core::window) {
            throw hacc::X::Logic_Error("Tried to call Image::load when no window has been created.");
        }
        core::window->open();
        int iw; int ih; int ich;
        std::string abs_fn = rel2abs(filename);
        data = (uint32*)SOIL_load_image(abs_fn.c_str(), &iw, &ih, &ich, 4);
        if (!data) {
            throw hacc::X::Logic_Error("Couldn't open image \"" + abs_fn + "\": " + SOIL_last_result());
        }
        if (ich != 4) {
            throw hacc::X::Logic_Error("Tried to use an image with " + std::to_string(ich) + " channels, but we can only use images with 4 channels.");
        }
        size = Vec(iw, ih);
        for (Texture& t : textures) {
            if (t.offset.x < 0 || t.offset.y < 0
             || t.offset.x + t.size.x > size.x
             || t.offset.y + t.size.y > size.y)
                throw hacc::X::Logic_Error("Error processing image \"" + filename + "\": Texture boundary is outside of image");
        }
        if (!palettes.empty()) {
             // Read palettes into a hash for quick lookup
            typedef std::unordered_map<uint32, uint32> Palette_Hash;
            std::vector<Palette_Hash> substs (palettes.size());
            for (size_t i = 0; i < palettes.size(); i++) {
                auto& p = palettes[i];
                size_t x = p.offset.x;
                size_t y = ih - p.offset.y - 1;
                if (p.vertical) {
                    size_t max = p.length ? y + p.length : (size_t)-1;
                    if (max > (size_t)ih) max = ih;
                    while (y < max) {
                        uint32* px = data + (y*iw) + x;
                        uint32 left = px[0] & p.mask;
                        uint32 right = px[1] & p.mask;
                        if (!left && !right) break;
                        substs[i].emplace(left, right);
                        y++;
                    }
                }
                else {
                    size_t max = p.length ? x + p.length : (size_t)-1;
                    if (max > (size_t)iw) max = iw;
                    while (x < max) {
                        uint32* px = data + (y*iw) + x;
                        uint32 left = px[0] & p.mask;
                        uint32 right = px[iw] & p.mask;
                        if (!left && !right) break;
                        substs[i].emplace(left, right);
                        x++;
                    }
                }
            }
            processed_data = (uint32*)malloc(iw*ih*sizeof(int32));
            for (size_t i = 0; i < (size_t)iw*ih; i++) {
                for (size_t pi = 0; pi < palettes.size(); pi++) {
                    auto iter = substs[pi].find(data[i] & palettes[pi].mask);
                    if (iter != substs[pi].end()) {
                        uint32 new_val = (iter->second & palettes[pi].mask)
                                       | (data[i] & ~palettes[pi].mask);
                        processed_data[i] = new_val;
                        break;
                    }
                    else {
                        processed_data[i] = data[i];
                    }
                }
            }
        }
        else processed_data = data;
        for (Texture& t : textures) {
            t.load(this);
        }
        if (processed_data != data) free(processed_data);
        free(data);
        processed_data = data = NULL;
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
         // Create OpenGL VBO
        auto data = new Layout_VBO_Data [frames.size()];
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
        glGenBuffers(1, &vbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferData(GL_ARRAY_BUFFER, frames.size() * sizeof(Layout_VBO_Data), data, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        core::diagnose_opengl("after creating a layout vbo");
        delete[] data;
    }

    Layout::~Layout () {
        if (vbo_id)
            glDeleteBuffers(1, &vbo_id);
    }

    Frame* Layout::frame_named (std::string name) {
        for (Frame& s : frames)
            if (s.name == name)
                return &s;
        return NULL;
    }

    struct Images_Program : Cameraed_Program {
        GLint model_pos = 0;
        GLint model_scale = 0;
        GLint tex = 0;
        GLint draw_phase = 0;
        void finish () {
            Cameraed_Program::finish();
            model_pos = require_uniform("model_pos");
            model_scale = require_uniform("model_scale");
            tex = require_uniform("tex");
            draw_phase = require_uniform("draw_phase");
            glUniform1i(tex, 0);
        }
    };

    static Images_Program* prog = NULL;
    static Frame* plain_frame = NULL;
    void images_init () {
        auto prog_doc = hacc::File("vis/res/images.prog").data();
        prog = prog_doc.attr("prog");
        plain_frame = prog_doc.attr("plain_layout").attr("ALL");
        hacc::manage(&prog);
        hacc::manage(&plain_frame);
    }

    void set_draw_phase (bool phase) {
        prog->use();
        glUniform1i(prog->draw_phase, phase);
    }


    void draw_frame (Frame* frame, Texture* texture, Vec pos, Vec scale, float z) {
        prog->use();
        glUniform3f(prog->model_pos, round(pos.x/PX)*PX, round(pos.y/PX)*PX, z);
        glUniform2f(prog->model_scale, scale.x, scale.y);
        glBindTexture(GL_TEXTURE_2D, texture->id);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, frame->parent->vbo_id);
         // Set up format of the buffer
         // index, n_elements, type, normalize, stride, offset
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Layout_VBO_Data) / 4, (void*)offsetof(Layout_VBO_Data, lbp));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Layout_VBO_Data) / 4, (void*)offsetof(Layout_VBO_Data, lbt));
        glDrawArrays(GL_QUADS, 4 * (frame - frame->parent->frames.data()), 4);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        core::diagnose_opengl("after draw_frame");
    }
    void draw_texture (Texture* texture, Rect area, float z) {
        draw_frame(plain_frame, texture, area.lb(), (area.rt() - area.lb())/PX, z);
    }

} using namespace vis;

HACCABLE(Texture) {
    name("vis::Texture");
    attr("name", member(&Texture::name).optional().prepare());
    attr("offset", member(&Texture::offset).optional());
    attr("size", member(&Texture::size).optional());
    attr("smooth", member(&Texture::smooth).optional());
    attr("use_palettes", member(&Texture::use_palettes).optional());
}

HACCABLE(Palette) {
    name("vis::Palette");
    attr("offset", member(&Palette::offset));
    attr("vertical", member(&Palette::vertical).optional());
    attr("length", member(&Palette::length).optional());
    attr("mask", member(&Palette::mask).optional());
}

HACCABLE(Image) {
    name("vis::Image");
    attr("filename", member(&Image::filename));
    attr("textures", member(&Image::textures).optional());
    attr("palettes", member(&Image::palettes).optional());
    attrs([](Image& image, std::string name){
        Texture* r = image.texture_named(name);
        if (r) return hacc::Reference(r);
        else throw hacc::X::No_Attr(hacc::Pointer(&image), name);
    });
    finish([](Image& i){ i.load(); });
}

HACCABLE(Layout) {
    name("vis::Layout");
    attr("size", member(&Layout::size));
    attr("frames", member(&Layout::frames));
    attrs([](Layout& layout, std::string name){
        Frame* r = layout.frame_named(name);
        if (r) return hacc::Reference(r);
        else throw hacc::X::No_Attr(hacc::Pointer(&layout), name);
    });
    finish([](Layout& layout){ layout.finish(); });
}
 // For convenience
HCB_INSTANCE(std::unordered_map<std::string HCB_COMMA Layout>)

HACCABLE(Frame) {
    name("vis::Frame");
    attr("name", member(&Frame::name).prepare());
    attr("offset", member(&Frame::offset));
    attr("box", member(&Frame::box));
    attr("points", member(&Frame::points).optional());
}

HACCABLE(Images_Program) {
    name("vis::Images_Program");
    delegate(base<core::Program>());
    finish(&Images_Program::finish);
}
