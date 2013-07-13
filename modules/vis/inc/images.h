#ifndef HAVE_VIS_IMAGES_H
#define HAVE_VIS_IMAGES_H

#include <string>
#include "../../util/inc/geometry.h"

namespace vis {
    using namespace util;

     // All positions of these structures are in PX

     // Represents one OpenGL texture
    struct Texture;
     // Represents one file-loaded image, which may contain multiple textures
    struct Image;
     // Represents an intepretation of a texture, containing multiple frames
    struct Layout;
     // Represents one drawable region of a texture
    struct Frame;

    struct Texture {
        std::string name = "ALL";
        Vec offset = Vec(0, 0);
        Vec size;  // defaults to entire Image
        bool smooth = false;

        uint id = 0;

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

    struct Layout {
        Vec size;
        std::vector<Frame> frames;

        uint vbo_id;
        uint vao_id;

        void finish ();
        ~Layout ();
         // Hacc attribute lookup goes here
        Frame* frame_named (std::string);
    };

    struct Frame {
        Layout* parent;
        std::string name;
        Vec offset;
        Rect box;
        std::vector<Vec> points;
    };
}

#endif
