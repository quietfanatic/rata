#ifndef HAVE_VIS_IMAGES_H
#define HAVE_VIS_IMAGES_H

#include <string>

#include "../../util/inc/Vec.h"
#include "../../util/inc/Rect.h"
#include "../../core/inc/resources.h"

#define PX (1/16.0)

namespace vis {

    struct Image : Resource {
        uint tex = 0;
        Vec size;

        void unload ();
        void reload ();

        Image (std::string name);
        ~Image ();
    };

     // Part of another image
    struct SubImg {
        std::string name;
        Vec pos;
        Rect box;
        std::vector<Vec> points;
    };

     // A set of subimgs.  Worthy of requiring a file.
    struct Layout : Resource {
        std::vector<SubImg> subimgs;

        SubImg* sub_named (std::string);

        void reload ();
        Layout (std::string name);
        Layout ();
    };

}

#endif
