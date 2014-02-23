#include "../inc/light.h"
#include "../../hacc/inc/files.h"
#include "../../core/inc/commands.h"

using namespace core;

namespace vis {

    int light_debug_type = 0;

    Materials* materials = NULL;

    struct Light_Program : Cameraed_Program {
        GLint model_pos;
        GLint ambient;
        GLint diffuse;
        GLint radiant;
        GLint materials_length;
        GLint debug_type;
        void finish () {
            Cameraed_Program::finish();
            glUniform1i(require_uniform("tex"), 0);
            glUniform1i(require_uniform("materials"), 1);
            model_pos = require_uniform("model_pos");
            ambient = require_uniform("ambient");
            diffuse = require_uniform("diffuse");
            radiant = require_uniform("radiant");
            materials_length = require_uniform("materials_length");
            debug_type = require_uniform("debug_type");
        }
        void Program_begin () override {
            Cameraed_Program::Program_begin();
            glUniform1i(debug_type, light_debug_type);
        }
    };
    Light_Program* light_program = NULL;

    void Materials::update () {
        if (items.size() > 256) {
            throw hacc::X::Logic_Error(
                "Material palette is too large: " + std::to_string(items.size()) + " > 256"
            );
        }
        if (!tex) {
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
         // The palette texture is internally laid out vertically.
        uint8 pdat [256 * 4 * 4];
        size_t j = 0;
        for (size_t i = 0; i < items.size(); i++) {
            pdat[j++] = items[i].ambient >> 24;
            pdat[j++] = items[i].ambient >> 16;
            pdat[j++] = items[i].ambient >> 8;
            pdat[j++] = items[i].ambient;
            pdat[j++] = items[i].diffuse >> 24;
            pdat[j++] = items[i].diffuse >> 16;
            pdat[j++] = items[i].diffuse >> 8;
            pdat[j++] = items[i].diffuse;
            pdat[j++] = items[i].specular >> 24;
            pdat[j++] = items[i].specular >> 16;
            pdat[j++] = items[i].specular >> 8;
            pdat[j++] = items[i].specular;
            pdat[j++] = items[i].radiant >> 24;
            pdat[j++] = items[i].radiant >> 16;
            pdat[j++] = items[i].radiant >> 8;
            pdat[j++] = items[i].radiant;
        }
        for (size_t i = items.size(); i < 256; i++) {
            pdat[j++] = 0x00;
            pdat[j++] = 0x00;
            pdat[j++] = 0x00;
            pdat[j++] = 0x00;
            pdat[j++] = 0x00;
            pdat[j++] = 0x00;
            pdat[j++] = 0x00;
            pdat[j++] = 0x00;
            pdat[j++] = 0x00;
            pdat[j++] = 0x00;
            pdat[j++] = 0x00;
            pdat[j++] = 0x00;
            pdat[j++] = 0x00;
            pdat[j++] = 0xff;  // Highlight invalid materials green.
            pdat[j++] = 0x00;
            pdat[j++] = 0xff;
        }
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 4, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, pdat);
        diagnose_opengl("after setting palette");
    }

    Materials::~Materials () {
        if (tex) glDeleteTextures(1, &tex);
    }

    void set_materials (Materials* m) {
        materials = m;
    }

    void set_ambient (const RGBf& amb) {
        light_program->use();
        glUniform3fv(light_program->ambient, 1, &amb.r);
    };
    void set_diffuse (const RGBf& dif) {
        light_program->use();
        glUniform3fv(light_program->diffuse, 1, &dif.r);
    };
    void set_radiant (const RGBf& rad) {
        light_program->use();
        glUniform3fv(light_program->radiant, 1, &rad.r);
    };

    void light_offset (Vec pos) {
        light_program->use();
        glUniform2f(light_program->model_pos, pos.x, pos.y);
    }

    void light_texture (GLuint tex) {
        light_program->use();
        glBindTexture(GL_TEXTURE_2D, tex);
    }

    void draw_light (size_t n, Vec* pts) {
        if (!materials) {
            throw hacc::X::Logic_Error("No materials were set!\n");
        }
        light_program->use();
        if (light_debug_type == 1) {
            glUniform1f(light_program->materials_length, materials->items.size());
        }
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, materials->tex);
        glActiveTexture(GL_TEXTURE0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec), pts);
        glDrawArrays(GL_POLYGON, 0, n);
        glDisableVertexAttribArray(0);
    };

    void light_init () {
        light_program = hacc::File("vis/res/light.prog").data().attr("prog");
        hacc::manage(&light_program);
        hacc::manage(&materials);
        diagnose_opengl("after loading light.prog");
        light_program->use();
    }

} using namespace vis;

HACCABLE(RGBf) {
    name("vis::RGBf");
    elem(member(&RGBf::r));
    elem(member(&RGBf::g));
    elem(member(&RGBf::b));
}

HACCABLE(Material) {
    name("vis::Material");
    elem(member(&Material::ambient));
    elem(member(&Material::diffuse));
    elem(member(&Material::specular));
    elem(member(&Material::radiant));
}

HACCABLE(Materials) {
    name("vis::Materials");
    delegate(member(&Materials::items));
    finish([](Materials& v){ v.update(); });
}

HACCABLE(Light_Program) {
    name("vis::Light_Program");
    delegate(base<Program>());
    finish(&Light_Program::finish);
}

using namespace core;

static void _light_debug () {
    light_debug_type++;
    if (light_debug_type > 3)
        light_debug_type = 0;
    print_to_console("Light debug type: " + std::to_string(light_debug_type) + "\n");
}

New_Command _light_debug_cmd ("light_debug", "Toggle between light debug modes", 0, _light_debug);

New_Command _set_materials_cmd ("set_materials", "Set the materials palette", 1, set_materials);
