#ifndef HAVE_CORE_IMAGES_H
#define HAVE_CORE_IMAGES_H

#include <string>
#include "../../util/inc/geometry.h"

namespace core {
    using namespace util;

     // All positions of these structures are in PX
    struct Texture;
    struct Image;

    struct Texture {
        std::string name = "ALL";
        Vec offset = Vec(0, 0);
        Vec size;
        bool smooth = false;

        uint tex = 0;

        void load (Image*);
        void unload ();
        ~Texture ();
    };

    struct Image {
        std::string filename;
        std::vector<Texture> textures {Texture()};

        Vec size;  // Set on load
        uint8* data;  // NULL when data is not in memory

        void load ();
        void unload ();
        Texture* texture_named (std::string);
    };
}

#endif
