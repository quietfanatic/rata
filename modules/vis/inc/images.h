#ifndef HAVE_VIS_IMAGES_H
#define HAVE_VIS_IMAGES_H

#include <string>

#include "../../util/inc/Vec.h"
#include "../../util/inc/Rect.h"
#include "../../core/inc/resources.h"

#define PX (1/16.0)

namespace vis {

     // All positions of these structures are in PX
    struct Texture;
    struct Image;
    struct Frame;
    struct Layout;

    struct Texture {
        std::string name;
        Vec offset;
        Vec size;
        Layout* layout = NULL;  // Not required
        bool smooth = false;

        uint tex = 0;

        void load (Image*);
        void unload ();
        ~Texture ();
    };

    struct Image {
        std::string filename;
        std::vector<Texture> textures;

        Vec size;  // Set on load
        uint8* data;  // NULL when data is not in memory

        void load ();
        void unload ();
        Texture* texture_named (std::string);
    };

    struct Frame {
        std::string name;
        Vec offset;
        Rect box;
        std::vector<Vec> points;
    };

     // This may store a VBO in the future.
    struct Layout {
        Vec size;
        std::vector<Frame> frames;

        Frame* frame_named (std::string);
    };

}

#endif
