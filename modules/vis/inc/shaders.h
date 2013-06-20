#ifndef HAVE_VIS_SHADERS_H
#define HAVE_VIS_SHADERS_H

#include <stdexcept>
#include <GL/glfw.h>

namespace vis {

    struct Shader {
        uint glid = 0;
        GLenum type;
        std::string source;
    
        void finish ();
        ~Shader ();
    };

    struct Program {
        std::string name;
        uint glid = 0;
        std::vector<Shader*> shaders;
        std::vector<std::pair<std::string, uint>> attributes;

        int require_uniform (const char* name);
        void link ();
        ~Program ();
    };

}

#endif
