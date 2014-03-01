#ifndef HAVE_VIS_LIGHT_H
#define HAVE_VIS_LIGHT_H

#include <vector>
#include "util/inc/geometry.h"

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
        uint tex = 0;
        void update ();
        ~Materials ();
    };

    void light_init ();

    void set_materials (Materials*);


    void set_ambient (const RGBf&);
    void set_diffuse (const RGBf&);
    void set_radiant (const RGBf&);
    void light_offset (util::Vec);
    void draw_light (size_t, util::Vec*);

     // This only needs to be called once, by common.cpp
    void light_texture (uint);

    extern int light_debug_type;

}

#endif
