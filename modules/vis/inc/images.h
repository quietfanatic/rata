#ifndef HAVE_VIS_IMAGES_H
#define HAVE_VIS_IMAGES_H

#include <string>

#include "../../util/inc/Vec.h"
#include "../../util/inc/Rect.h"
#include "../../core/inc/resources.h"

#define PX (1/16.0)

namespace vis {

     // Only the barebones image file
    struct Image : Resource {
        uint tex = 0;
        Vec size;

        void unload ();
        void reload ();

        Image (std::string name);
        ~Image () { unload(); }
    };

     // Part of another image
    struct SubImg {
        Vec pos;
        Rect box;
        std::vector<Vec> misc;
    };

}

#endif
