#ifndef HAVE_CORE_OPENGL_H
#define HAVE_CORE_OPENGL_H

#include <stdio.h>
#include <stdexcept>
#include <vector>
#include <GL/glfw.h>

namespace core {

    void* g_glproc (const char* name);
    template <class F>
    F* glproc (const char* name) { return (F*)g_glproc(name); }
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

        virtual ~Renderer ();
    };

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
