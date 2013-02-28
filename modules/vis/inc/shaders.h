#ifndef HAVE_VIS_SHADERS_H
#define HAVE_VIS_SHADERS_H

#include <stdexcept>
#include <GL/glfw.h>

namespace vis {

    template <class F> F* glproc (const char* name) {
        F* r = (F*)glfwGetProcAddress(name);
        if (!r) {
            fprintf(stderr, "No GL proc was found named %s\n", name);
            throw std::logic_error("Missing GL procedure.");
        }
        return r;
    }
    GLenum diagnose_opengl (std::string when);

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

     // These should be global singletons.
    struct Renderer {
         // Called when the current renderer is switched to this one
        virtual void start_rendering () = 0;
         // Called when the current renderer is switched away from this one
        virtual void finish_rendering () { }
         // Does the aforementioned switch
        void use ();
         // Keeps track
        static Renderer* current;
    };


}

#endif
