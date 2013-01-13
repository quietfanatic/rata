#ifndef HAVE_VIS_VIS_H
#define HAVE_VIS_VIS_H

#include <unordered_map>

#include "../../core/inc/resources.h"

namespace vis {

    struct Image : Resource {
        GLuint tex = 0;
        uint w;
        uint h;

        bool load ();
        bool unload ();
        bool reload ();

        static ResourceGroup all;

        Image (std::string name) : Resource(name) { }
        ~Image () { unload(); }
    };

    void start_draw ();
    void finish_draw ();
}

#endif
