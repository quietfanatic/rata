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
     // A Palette can do color substitution before a Texture is loaded.
    struct Palette;

    struct Texture {
        std::string name = "ALL";
        Vec offset = Vec(0, 0);
        Vec size;  // defaults to entire Image
        bool smooth = false;
        bool use_palettes = true;

        uint id = 0;

        void load (Image*);
        void unload ();
        ~Texture ();
    };

    struct Palette {
        Vec offset = Vec(0, 0);
        bool vertical = false;
        uint length = 0;  // 0 means go until it ends
        uint32 mask = 0xffffffff;  // Bits the palette affects
    };

    struct Image {
        std::string filename;
        std::vector<Texture> textures {Texture()};
        std::vector<Palette> palettes;

        Vec size;  // Set on load
         // These uint32s may be LE or BE.
        uint32* data;  // NULL when data is not in memory
        uint32* processed_data;  // After palettes are applied

        void load ();
        void unload ();
        Texture* texture_named (std::string);
    };

    struct Layout {
        Vec size;
        std::vector<Frame> frames;

        uint vbo_id;

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

    void images_init ();

    void draw_frame (Frame*, Texture*, Vec pos, Vec scale = Vec(1, 1), float z = 0.5);
     // Just draw a whole texture over the given area.
    void draw_texture (Texture*, Rect area, float z = 0.5);

}

#endif
