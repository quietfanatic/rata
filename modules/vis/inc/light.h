#ifndef HAVE_VIS_LIGHT_H
#define HAVE_VIS_LIGHT_H

#include "common.h"

namespace vis {

     // Lights need to be able to be larger than 1.
    struct RGBf {
        float r;
        float g;
        float b;
        RGBf () : r(0), g(0), b(0) { }
        RGBf (float v) : r(v), g(v), b(v) { }
        RGBf (float r, float g, float b) : r(r), g(g), b(b) { }
        RGBf& operator += (const RGBf& o) {
            r += o.r;
            g += o.g;
            b += o.b;
            return *this;
        }
    };

    struct Material {
        uint32 ambient;
        uint32 diffuse;
        uint32 specular;
        uint32 radiant;
    };

    struct Materials {
        std::vector<Material> items;
        GLuint tex = 0;
        void update ();
        ~Materials ();
    };

    void light_init ();

    void set_materials (Materials*);


    void set_ambient (const RGBf&);
    void set_diffuse (const RGBf&);
    void set_radiant (const RGBf&);
    void light_offset (Vec);
    void draw_light (size_t, Vec*);

     // This only needs to be called once, by common.cpp
    void light_texture (GLuint);

    extern int light_debug_type;

}

#endif
