#include "../inc/common.h"
#include "../../hacc/inc/everything.h"
#include "../../core/inc/opengl.h"

namespace vis {

    using namespace core;

    Vec camera_pos = Vec(10, 7.5);

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
        glDeleteBuffers(1, &vbo_id);
        glDeleteVertexArrays(1, &vao_id);
    }

    Frame* Layout::frame_named (std::string name) {
        for (Frame& s : frames)
            if (s.name == name)
                return &s;
        return NULL;
    }

} using namespace vis;

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

HCB_BEGIN(Frame)
    name("vis::Frame");
    attr("name", member(&Frame::name).prepare());
    attr("offset", member(&Frame::offset));
    attr("box", member(&Frame::box));
    attr("points", member(&Frame::points).optional());
HCB_END(vis::Frame)
