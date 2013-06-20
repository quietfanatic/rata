#ifndef HAVE_CORE_OPENGL_H
#define HAVE_CORE_OPENGL_H

#include <stdio.h>
#include <stdexcept>
#include <GL/glfw.h>

namespace core {

    template <class F> F* glproc (const char* name) {
        F* r = (F*)glfwGetProcAddress(name);
        if (!r) {
            fprintf(stderr, "No GL proc was found named %s\n", name);
            throw std::logic_error("Missing GL procedure.");
        }
        return r;
    }
    GLenum diagnose_opengl (std::string when = "");

    struct Renderer {
         // Called when the current renderer is switched to this one
        virtual void start_rendering () { }
         // Called when the current renderer is switched away from this one
        virtual void finish_rendering () { }
         // Does the aforementioned switch
        void use ();
         // Keeps track
        static Renderer* current;

        virtual ~Renderer () { }
    };

}

#endif
