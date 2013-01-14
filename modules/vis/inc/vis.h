#ifndef HAVE_VIS_VIS_H
#define HAVE_VIS_VIS_H

#include <unordered_map>

#include "../../util/inc/Vec.h"
#include "../../core/inc/resources.h"

#define PX (1/16.0)

namespace vis {

    struct Image : Resource {
        GLuint tex = 0;
        uint w = 0;
        uint h = 0;

        bool load ();
        bool unload ();
        bool reload ();

        static ResourceGroup all;

        Image (std::string name) : Resource(name) { }
        ~Image () { unload(); }
    };

    void start_draw ();
    void draw_image (Image* img, Vec p, bool fliph, bool flipv);
    void finish_draw ();
}

#endif
