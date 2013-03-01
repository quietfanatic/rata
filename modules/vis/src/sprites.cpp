#include <stdexcept>
#include <GL/gl.h>
#include <GL/glfw.h>
#include <SOIL/SOIL.h>
#include "../../hacc/inc/everything.h"
#include "../inc/sprites.h"
#include "../../core/inc/phases.h"
#include "../../core/inc/state.h"
#include "../../util/inc/debug.h"
#include "../inc/shaders.h"

namespace vis {

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


    static GLenum diagnose_opengl (const char* when = "") {
        GLenum err = glGetError();
        if (err)
            fprintf(stderr, "OpenGL error: %04x %s\n", err, when);
        return err;
    }

    static Links<Draws_Sprites> sprite_drawers;

    void Draws_Sprites::activate () {
        link(sprite_drawers);
    }
    void Draws_Sprites::deactivate () {
        unlink();
    }


    struct Sprite_Layer : core::Layer, core::Game_Object, Renderer {
        Program* program = hacc::reference_file<Program>("modules/vis/res/sprite.prog");
        GLint tex = program->require_uniform("tex");
        GLint camera_pos = program->require_uniform("camera_pos");
        GLint model_pos = program->require_uniform("model_pos");
        GLint model_scale = program->require_uniform("model_scale");

        Sprite_Layer () : core::Layer("C.M", "sprites") {
            static auto glUniform1i = glproc<void (GLint, GLint)>("glUniform1i");
            static auto glUseProgram = glproc<void (GLuint)>("glUseProgram");
            glUseProgram(program->glid);
            glUniform1i(tex, 0);  // Texture unit 0
            if (diagnose_opengl("after setting uniforms and stuff")) {
                throw std::logic_error("sprites init failed due to GL error");
            }
        }
         // for Renderer
        void start_rendering () {
            static auto glUniform2f = glproc<void (GLint, GLfloat, GLfloat)>("glUniform2f");
            static auto glUseProgram = glproc<void (GLuint)>("glUseProgram");
            glDisable(GL_BLEND);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST); // Depth buffer is awesome
            glUseProgram(program->glid);
            glUniform2f(camera_pos, 10, 7.5);  // TODO: Control the camera with this
        }
         // for Layer
        void start () { }
        void run () {
            glClearColor(0.5, 0.5, 0.5, 0);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

            for (Draws_Sprites* p = sprite_drawers.first(); p; p = p->next()) {
                p->draws_sprites();
            }
        }
    };
    core::Celebrity<Sprite_Layer> sprite_layer;

    static Logger draw_sprite_logger ("draw_sprite", false);

    void Draws_Sprites::draw_sprite (Frame* frame, Texture* tex, Vec p, bool fliph, bool flipv, float z) {
        static auto glBindVertexArray = glproc<void (GLuint)>("glBindVertexArray");
        static auto glUniform2f = glproc<void (GLint, GLfloat, GLfloat)>("glUniform2f");
        static auto glUniform3f = glproc<void (GLint, GLfloat, GLfloat, GLfloat)>("glUniform3f");

        if (draw_sprite_logger.on) {
            draw_sprite_logger.log("tex: %s frame: [%g %g] [%g %g %g %g] p: [%g %g] fliph: %u flipv: %u, z: %g",
                tex ? tex->name.c_str() : "NULL", frame ? frame->offset.x : 0/0.0, frame ? frame->offset.y : 0/0.0,
                frame ? frame->box.l : 0/0.0, frame ? frame->box.b : 0/0.0, frame ? frame->box.r : 0/0.0, frame ? frame->box.t : 0/0.0,
                p.x, p.y, fliph, flipv, z
            );
        }

        sprite_layer->use();

        glUniform3f(sprite_layer->model_pos, p.x, p.y, z);
        glUniform2f(sprite_layer->model_scale, fliph ? -1.0 : 1.0, flipv ? -1.0 : 1.0);
        glBindTexture(GL_TEXTURE_2D, tex->tex);
        glBindVertexArray(frame->parent->vao_id);
        glDrawArrays(GL_QUADS, 4 * (frame - frame->parent->frames.data()), 4);

        diagnose_opengl("After rendering a sprite");
    }

    struct Sprite_Test : core::Stateful, Draws_Sprites {
        void start () { Draws_Sprites::activate(); }
        void draws_sprites () {
            static Image* image = hacc::reference_file<Image>("modules/vis/res/test.image");
            static Texture* texture = image->texture_named("ALL");
            static Layout* layout = hacc::reference_file<Layout>("modules/vis/res/test.layout");
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

}

using namespace vis;

HCB_BEGIN(Frame)
    type_name("vis::Frame");
    attr("name", member(&Frame::name));
    attr("offset", member(&Frame::offset));
    attr("box", member(&Frame::box));
    attr("points", member(&Frame::points)(optional));
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

HCB_BEGIN(Sprite_Test)
    type_name("vis::Sprite_Test");
    base<core::Stateful>("Sprite_Test");
    empty();
HCB_END(Sprite_Test)
