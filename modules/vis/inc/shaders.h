#ifndef HAVE_VIS_SHADERS_H
#define HAVE_VIS_SHADERS_H

#include <stdexcept>
#include "../../core/inc/resources.h"

namespace vis {

    template <class F> F* glproc (const char* name) {
        F* r = (F*)glfwGetProcAddress(name);
        if (!r) {
            fprintf(stderr, "No GL proc was found named %s\n", name);
            throw std::logic_error("Missing GL procedure.");
        }
        return r;
    }

    struct Shader : Resource {
        uint glid = 0;

        void reload ();
        Shader (std::string name);
        ~Shader ();
    };

    struct Program : Resource {
        uint glid = 0;
        std::vector<Shader*> shaders;

        int require_uniform (const char* name);
        void use ();
        void link ();
        void reload ();
        Program (std::string name);
        ~Program ();
    };


}

#endif
