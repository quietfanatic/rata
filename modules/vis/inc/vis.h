#ifndef HAVE_VIS_VIS_H
#define HAVE_VIS_VIS_H

#include <unordered_map>

namespace vis {
/*
    struct Image : Resource<Image> {
        const char* filename;
        GLuint tex = 0;
        uint w;
        uint h;

        void load ();
        void unload ();
        void reload ();

        Image (const char* filename) : filename(filename) {
            load();
        }
    };
*/
    void start_draw ();
    void finish_draw ();
}

#endif
