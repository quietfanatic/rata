#include <stdexcept>
#include <GL/gl.h>
#include <GL/glfw.h>
#include <SOIL/SOIL.h>
#include "../../hacc/inc/everything.h"
#include "../../core/inc/phases.h"
#include "../../core/inc/opengl.h"
#include "../../util/inc/debug.h"
#include "../inc/sprites.h"

namespace vis {

    Vec camera_pos = Vec(10, 7.5);

    using namespace core;

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
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        diagnose_opengl("after creating a layout vao");

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

    static Links<Draws_Sprites> sprite_drawers;

    void Draws_Sprites::activate () {
        link(sprite_drawers);
    }
    void Draws_Sprites::deactivate () {
        unlink();
    }

    struct Sprite_Layer : Layer, Renderer {
        Program* program;
        GLint tex;
        GLint camera_pos;
        GLint model_pos;
        GLint model_scale;
        void (* glUseProgram )(GLuint);
        void (* glUniform1i )(GLint, GLint);
        void (* glUniform2f )(GLint, GLfloat, GLfloat);
        void (* glUniform3f )(GLint, GLfloat, GLfloat, GLfloat);
        void (* glBindVertexArray )(GLuint);

        Sprite_Layer () : Layer("C.M", "sprites") { }
        void Layer_start () override {
            glUseProgram = glproc<void (GLuint)>("glUseProgram");
            glUniform1i = glproc<void (GLint, GLint)>("glUniform1i");
            glUniform2f = glproc<void (GLint, GLfloat, GLfloat)>("glUniform2f");
            glUniform3f = glproc<void (GLint, GLfloat, GLfloat, GLfloat)>("glUniform3f");
            glBindVertexArray = glproc<void (GLuint)>("glBindVertexArray");
            program = hacc::File("modules/vis/res/sprite.prog").data().attr("prog");
            tex = program->require_uniform("tex");
            camera_pos = program->require_uniform("camera_pos");
            model_pos = program->require_uniform("model_pos");
            model_scale = program->require_uniform("model_scale");
            glUseProgram(program->glid);
            glUniform1i(tex, 0);  // Texture unit 0
            if (diagnose_opengl("after setting uniforms and stuff")) {
                throw std::logic_error("sprites init failed due to GL error");
            }
        }
         // Renderer
        void start_rendering () override {
            glDisable(GL_BLEND);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST); // Depth buffer is awesome
            glUseProgram(program->glid);
            glUniform2f(camera_pos, vis::camera_pos.x, vis::camera_pos.y);
        }
         // Layer
        void Layer_run () override {
            glClearColor(0.5, 0.5, 0.5, 0);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

            for (Draws_Sprites* p = sprite_drawers.first(); p; p = p->next()) {
                p->draws_sprites();
            }
        }
    } sprite_layer;

    static Logger draw_sprite_logger ("draw_sprite", false);

    void Draws_Sprites::draw_sprite (Frame* frame, core::Texture* tex, Vec p, bool fliph, bool flipv, float z) {
        if (draw_sprite_logger.on) {
            draw_sprite_logger.log("tex: %s frame: [%g %g] [%g %g %g %g] p: [%g %g] fliph: %u flipv: %u, z: %g",
                tex ? tex->name.c_str() : "NULL", frame ? frame->offset.x : 0/0.0, frame ? frame->offset.y : 0/0.0,
                frame ? frame->box.l : 0/0.0, frame ? frame->box.b : 0/0.0, frame ? frame->box.r : 0/0.0, frame ? frame->box.t : 0/0.0,
                p.x, p.y, fliph, flipv, z
            );
        }

        sprite_layer.use();

        sprite_layer.glUniform3f(sprite_layer.model_pos, p.x, p.y, z);
        sprite_layer.glUniform2f(sprite_layer.model_scale, fliph ? -1.0 : 1.0, flipv ? -1.0 : 1.0);
        glBindTexture(GL_TEXTURE_2D, tex->tex);
        sprite_layer.glBindVertexArray(frame->parent->vao_id);
        glDrawArrays(GL_QUADS, 4 * (frame - frame->parent->frames.data()), 4);

        diagnose_opengl("After rendering a sprite");
    }

    struct Sprite_Test : Draws_Sprites {
        void finish () { Draws_Sprites::activate(); }
        void draws_sprites () override {
            static Image* image = hacc::File("modules/vis/res/test.hacc").data().attr("img");
            static Texture* texture = image->texture_named("ALL");
            static Layout* layout = hacc::File("modules/vis/res/test.hacc").data().attr("layout");
            static Frame* white = layout->frame_named("white");
            static Frame* red = layout->frame_named("red");
            static Frame* green = layout->frame_named("green");
            static Frame* blue = layout->frame_named("blue");

            draw_sprite(white, texture, Vec(2, 2), false, false, 0);
            draw_sprite(red, texture, Vec(18, 2), false, false, 0);
            draw_sprite(green, texture, Vec(18, 13), false, false, 0);
            draw_sprite(blue, texture, Vec(2, 13), false, false, 0);
        }
    };

} using namespace vis;

HCB_BEGIN(Frame)
    name("vis::Frame");
    attr("name", member(&Frame::name).prepare());
    attr("offset", member(&Frame::offset));
    attr("box", member(&Frame::box));
    attr("points", member(&Frame::points).optional());
HCB_END(vis::Frame)

HCB_BEGIN(Layout)
    name("vis::Layout");
    attr("size", member(&Layout::size));
    attr("frames", member(&Layout::frames));
    attrs([](Layout& layout, std::string name){
        Frame* r = layout.frame_named(name);
        if (r) return hacc::Reference(r);
        else throw hacc::X::No_Attr(hacc::Type::CppType<Layout>(), name);
    });
    finish([](Layout& layout){ layout.finish(); });
HCB_END(Layout)
 // For convenience
HCB_INSTANCE(std::unordered_map<std::string HCB_COMMA Layout>)

HCB_BEGIN(Sprite_Test)
    name("vis::Sprite_Test");
    finish([](Sprite_Test& st){ st.finish(); });
HCB_END(Sprite_Test)
