#include "../inc/opengl.h"

namespace core {

    GLenum diagnose_opengl (std::string when) {
        GLenum err = glGetError();
        if (err)
            fprintf(stderr, "OpenGL error %04x %s\n", err, when.c_str());
        return err;
    }

    Renderer* Renderer::current = NULL;

    void Renderer::use () {
        if (current != this) {
            if (current) current->finish_rendering();
            start_rendering();
            current = this;
        }
    }


}
