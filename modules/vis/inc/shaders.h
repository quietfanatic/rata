#ifndef HAVE_VIS_SHADERS_H
#define HAVE_VIS_SHADERS_H

#include "../../core/inc/resources.h"

namespace vis {
    struct Shader : Resource {
        uint glid = 0;

        void reload ();
        Shader (std::string name);
        ~Shader ();
    };

    struct Program : Resource {
        uint glid = 0;
        std::vector<Shader*> shaders;

        void use ();
        void link ();
        void reload ();
        Program (std::string name);
        ~Program ();
    };

}

#endif
