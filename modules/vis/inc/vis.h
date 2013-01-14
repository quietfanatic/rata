#ifndef HAVE_VIS_VIS_H
#define HAVE_VIS_VIS_H

#include <unordered_map>

#include "../../util/inc/Vec.h"
#include "../../util/inc/Rect.h"
#include "../../core/inc/resources.h"

#define PX (1/16.0)

namespace vis {

     // Only the barebones image file
    struct Image : Resource {
        GLuint tex = 0;
        Vec size;

        bool load ();
        bool unload ();
        bool reload ();

        static ResourceGroup all;

        Image (std::string name) : Resource(name) { }
        ~Image () { unload(); }
    };

     // Part of another image
    struct SubImg {
        Vec pos;
        Rect box;
        std::vector<Vec> misc;
    };


    void test ();

    void start_draw ();
    void draw_img (Image* set, SubImg* sub, Vec p, bool fliph, bool flipv);
    void finish_draw ();
}

#endif
