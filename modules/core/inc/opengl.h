#ifndef HAVE_CORE_OPENGL_H
#define HAVE_CORE_OPENGL_H

#include <stdio.h>
#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <GL/glew.h>
#include <GL/glfw.h>

#ifdef CORE_OPENGL_VERY_DEBUG
#include <sstream>
#include "../../util/inc/debug.h"
#endif

#include "../../util/inc/honestly.h"

namespace core {

     // Queries glError
    GLenum diagnose_opengl (std::string when = "");

    struct Shader {
        uint glid = 0;
        GLenum type;
        std::string source;

        void compile ();
        ~Shader ();
    };

     // This class keeps track of the currently-running program and
     //  only switches if it needs to.
    struct Program {
        static Program* current;
        static size_t current_attr_count;
        std::string name = "<Anonymous Program>";
        uint glid = 0;
        std::vector<Shader*> shaders;
        std::vector<std::string> attributes;
        bool uses_vaos = false;

        int require_uniform (const char* name);
        int require_attribute (const char* name);
        void link ();
        ~Program ();
         // Supercall if you override these
        virtual void Program_begin ();
        virtual void Program_end ();
        void use ();
        static void unuse ();
    };

}

 // Perform an API trace.

#ifdef CORE_OPENGL_VERY_DEBUG
namespace core {
    extern Logger opengl_logger;  // TODO: name collisions may happen
    static std::string _concat () { return ""; }
    template <class H>
    static std::string _concat (H h) {
        std::ostringstream ss;
        ss << h;
        return ss.str();
    }
    template <class H, class... T>
    static std::string _concat (H h, T... t) {
        std::ostringstream ss;
        ss << h;
        return ss.str() + ", " + _concat(t...);
    }
}

template <class Func, class... Args>
auto opengl_debug_wrap (Func* f, const char* name, Args... args) -> decltype((*f)(args...)) {
    core::opengl_logger.log(std::string(name) + "(" + core::_concat(args...) + ")");
    return (*f)(args...);
}
template <class Func, class... Args>
auto opengl_debug_wrap_glew (Func* f, const char* name, Args... args) -> decltype((*f)(args...)) {
    core::opengl_logger.log(std::string(name) + "(" + core::_concat(args...) + ")");
    return GLEW_GET_FUN(f)(args...);
}

#ifdef glAccum
    #undef glAccum
    #define glAccum(...) opengl_debug_wrap_glew(__glewAccum, "glAccum", __VA_ARGS__)
#else
    #define glAccum(...) opengl_debug_wrap(glAccum, "glAccum", __VA_ARGS__)
#endif
#ifdef glActiveTexture
    #undef glActiveTexture
    #define glActiveTexture(...) opengl_debug_wrap_glew(__glewActiveTexture, "glActiveTexture", __VA_ARGS__)
#else
    #define glActiveTexture(...) opengl_debug_wrap(glActiveTexture, "glActiveTexture", __VA_ARGS__)
#endif
#ifdef glAreTexturesResident
    #undef glAreTexturesResident
    #define glAreTexturesResident(...) opengl_debug_wrap_glew(__glewAreTexturesResident, "glAreTexturesResident", __VA_ARGS__)
#else
    #define glAreTexturesResident(...) opengl_debug_wrap(glAreTexturesResident, "glAreTexturesResident", __VA_ARGS__)
#endif
#ifdef glArrayElement
    #undef glArrayElement
    #define glArrayElement(...) opengl_debug_wrap_glew(__glewArrayElement, "glArrayElement", __VA_ARGS__)
#else
    #define glArrayElement(...) opengl_debug_wrap(glArrayElement, "glArrayElement", __VA_ARGS__)
#endif
#ifdef glAttachShader
    #undef glAttachShader
    #define glAttachShader(...) opengl_debug_wrap_glew(__glewAttachShader, "glAttachShader", __VA_ARGS__)
#else
    #define glAttachShader(...) opengl_debug_wrap(glAttachShader, "glAttachShader", __VA_ARGS__)
#endif
#ifdef glBegin
    #undef glBegin
    #define glBegin(...) opengl_debug_wrap_glew(__glewBegin, "glBegin", __VA_ARGS__)
#else
    #define glBegin(...) opengl_debug_wrap(glBegin, "glBegin", __VA_ARGS__)
#endif
#ifdef glBindAttribLocation
    #undef glBindAttribLocation
    #define glBindAttribLocation(...) opengl_debug_wrap_glew(__glewBindAttribLocation, "glBindAttribLocation", __VA_ARGS__)
#else
    #define glBindAttribLocation(...) opengl_debug_wrap(glBindAttribLocation, "glBindAttribLocation", __VA_ARGS__)
#endif
#ifdef glBindBuffer
    #undef glBindBuffer
    #define glBindBuffer(...) opengl_debug_wrap_glew(__glewBindBuffer, "glBindBuffer", __VA_ARGS__)
#else
    #define glBindBuffer(...) opengl_debug_wrap(glBindBuffer, "glBindBuffer", __VA_ARGS__)
#endif
#ifdef glBindFramebuffer
    #undef glBindFramebuffer
    #define glBindFramebuffer(...) opengl_debug_wrap_glew(__glewBindFramebuffer, "glBindFramebuffer", __VA_ARGS__)
#else
    #define glBindFramebuffer(...) opengl_debug_wrap(glBindFramebuffer, "glBindFramebuffer", __VA_ARGS__)
#endif
#ifdef glBindRenderbuffer
    #undef glBindRenderbuffer
    #define glBindRenderbuffer(...) opengl_debug_wrap_glew(__glewBindRenderbuffer, "glBindRenderbuffer", __VA_ARGS__)
#else
    #define glBindRenderbuffer(...) opengl_debug_wrap(glBindRenderbuffer, "glBindRenderbuffer", __VA_ARGS__)
#endif
#ifdef glBindTexture
    #undef glBindTexture
    #define glBindTexture(...) opengl_debug_wrap_glew(__glewBindTexture, "glBindTexture", __VA_ARGS__)
#else
    #define glBindTexture(...) opengl_debug_wrap(glBindTexture, "glBindTexture", __VA_ARGS__)
#endif
#ifdef glBindVertexArray
    #undef glBindVertexArray
    #define glBindVertexArray(...) opengl_debug_wrap_glew(__glewBindVertexArray, "glBindVertexArray", __VA_ARGS__)
#else
    #define glBindVertexArray(...) opengl_debug_wrap(glBindVertexArray, "glBindVertexArray", __VA_ARGS__)
#endif
#ifdef glBitmap
    #undef glBitmap
    #define glBitmap(...) opengl_debug_wrap_glew(__glewBitmap, "glBitmap", __VA_ARGS__)
#else
    #define glBitmap(...) opengl_debug_wrap(glBitmap, "glBitmap", __VA_ARGS__)
#endif
#ifdef glBlendColor
    #undef glBlendColor
    #define glBlendColor(...) opengl_debug_wrap_glew(__glewBlendColor, "glBlendColor", __VA_ARGS__)
#else
    #define glBlendColor(...) opengl_debug_wrap(glBlendColor, "glBlendColor", __VA_ARGS__)
#endif
#ifdef glBlendEquation
    #undef glBlendEquation
    #define glBlendEquation(...) opengl_debug_wrap_glew(__glewBlendEquation, "glBlendEquation", __VA_ARGS__)
#else
    #define glBlendEquation(...) opengl_debug_wrap(glBlendEquation, "glBlendEquation", __VA_ARGS__)
#endif
#ifdef glBlendFunc
    #undef glBlendFunc
    #define glBlendFunc(...) opengl_debug_wrap_glew(__glewBlendFunc, "glBlendFunc", __VA_ARGS__)
#else
    #define glBlendFunc(...) opengl_debug_wrap(glBlendFunc, "glBlendFunc", __VA_ARGS__)
#endif
#ifdef glBufferData
    #undef glBufferData
    #define glBufferData(...) opengl_debug_wrap_glew(__glewBufferData, "glBufferData", __VA_ARGS__)
#else
    #define glBufferData(...) opengl_debug_wrap(glBufferData, "glBufferData", __VA_ARGS__)
#endif
#ifdef glCheckFramebufferStatus
    #undef glCheckFramebufferStatus
    #define glCheckFramebufferStatus(...) opengl_debug_wrap_glew(__glewCheckFramebufferStatus, "glCheckFramebufferStatus", __VA_ARGS__)
#else
    #define glCheckFramebufferStatus(...) opengl_debug_wrap(glCheckFramebufferStatus, "glCheckFramebufferStatus", __VA_ARGS__)
#endif
#ifdef glClear
    #undef glClear
    #define glClear(...) opengl_debug_wrap_glew(__glewClear, "glClear", __VA_ARGS__)
#else
    #define glClear(...) opengl_debug_wrap(glClear, "glClear", __VA_ARGS__)
#endif
#ifdef glClearAccum
    #undef glClearAccum
    #define glClearAccum(...) opengl_debug_wrap_glew(__glewClearAccum, "glClearAccum", __VA_ARGS__)
#else
    #define glClearAccum(...) opengl_debug_wrap(glClearAccum, "glClearAccum", __VA_ARGS__)
#endif
#ifdef glClearColor
    #undef glClearColor
    #define glClearColor(...) opengl_debug_wrap_glew(__glewClearColor, "glClearColor", __VA_ARGS__)
#else
    #define glClearColor(...) opengl_debug_wrap(glClearColor, "glClearColor", __VA_ARGS__)
#endif
#ifdef glClearDepth
    #undef glClearDepth
    #define glClearDepth(...) opengl_debug_wrap_glew(__glewClearDepth, "glClearDepth", __VA_ARGS__)
#else
    #define glClearDepth(...) opengl_debug_wrap(glClearDepth, "glClearDepth", __VA_ARGS__)
#endif
#ifdef glClearStencil
    #undef glClearStencil
    #define glClearStencil(...) opengl_debug_wrap_glew(__glewClearStencil, "glClearStencil", __VA_ARGS__)
#else
    #define glClearStencil(...) opengl_debug_wrap(glClearStencil, "glClearStencil", __VA_ARGS__)
#endif
#ifdef glCompileShader
    #undef glCompileShader
    #define glCompileShader(...) opengl_debug_wrap_glew(__glewCompileShader, "glCompileShader", __VA_ARGS__)
#else
    #define glCompileShader(...) opengl_debug_wrap(glCompileShader, "glCompileShader", __VA_ARGS__)
#endif
#ifdef glCopyPixels
    #undef glCopyPixels
    #define glCopyPixels(...) opengl_debug_wrap_glew(__glewCopyPixels, "glCopyPixels", __VA_ARGS__)
#else
    #define glCopyPixels(...) opengl_debug_wrap(glCopyPixels, "glCopyPixels", __VA_ARGS__)
#endif
#ifdef glCopyTexImage1D
    #undef glCopyTexImage1D
    #define glCopyTexImage1D(...) opengl_debug_wrap_glew(__glewCopyTexImage1D, "glCopyTexImage1D", __VA_ARGS__)
#else
    #define glCopyTexImage1D(...) opengl_debug_wrap(glCopyTexImage1D, "glCopyTexImage1D", __VA_ARGS__)
#endif
#ifdef glCopyTexImage2D
    #undef glCopyTexImage2D
    #define glCopyTexImage2D(...) opengl_debug_wrap_glew(__glewCopyTexImage2D, "glCopyTexImage2D", __VA_ARGS__)
#else
    #define glCopyTexImage2D(...) opengl_debug_wrap(glCopyTexImage2D, "glCopyTexImage2D", __VA_ARGS__)
#endif
#ifdef glCopyTexSubImage1D
    #undef glCopyTexSubImage1D
    #define glCopyTexSubImage1D(...) opengl_debug_wrap_glew(__glewCopyTexSubImage1D, "glCopyTexSubImage1D", __VA_ARGS__)
#else
    #define glCopyTexSubImage1D(...) opengl_debug_wrap(glCopyTexSubImage1D, "glCopyTexSubImage1D", __VA_ARGS__)
#endif
#ifdef glCopyTexSubImage2D
    #undef glCopyTexSubImage2D
    #define glCopyTexSubImage2D(...) opengl_debug_wrap_glew(__glewCopyTexSubImage2D, "glCopyTexSubImage2D", __VA_ARGS__)
#else
    #define glCopyTexSubImage2D(...) opengl_debug_wrap(glCopyTexSubImage2D, "glCopyTexSubImage2D", __VA_ARGS__)
#endif
#ifdef glCopyTexSubImage3D
    #undef glCopyTexSubImage3D
    #define glCopyTexSubImage3D(...) opengl_debug_wrap_glew(__glewCopyTexSubImage3D, "glCopyTexSubImage3D", __VA_ARGS__)
#else
    #define glCopyTexSubImage3D(...) opengl_debug_wrap(glCopyTexSubImage3D, "glCopyTexSubImage3D", __VA_ARGS__)
#endif
#ifdef glCreateProgram
    #undef glCreateProgram
    #define glCreateProgram() opengl_debug_wrap_glew(__glewCreateProgram, "glCreateProgram")
#else
    #define glCreateProgram() opengl_debug_wrap(glCreateProgram, "glCreateProgram")
#endif
#ifdef glCreateShader
    #undef glCreateShader
    #define glCreateShader(...) opengl_debug_wrap_glew(__glewCreateShader, "glCreateShader", __VA_ARGS__)
#else
    #define glCreateShader(...) opengl_debug_wrap(glCreateShader, "glCreateShader", __VA_ARGS__)
#endif
#ifdef glDeleteBuffers
    #undef glDeleteBuffers
    #define glDeleteBuffers(...) opengl_debug_wrap_glew(__glewDeleteBuffers, "glDeleteBuffers", __VA_ARGS__)
#else
    #define glDeleteBuffers(...) opengl_debug_wrap(glDeleteBuffers, "glDeleteBuffers", __VA_ARGS__)
#endif
#ifdef glDeleteFramebuffers
    #undef glDeleteFramebuffers
    #define glDeleteFramebuffers(...) opengl_debug_wrap_glew(__glewDeleteFramebuffers, "glDeleteFramebuffers", __VA_ARGS__)
#else
    #define glDeleteFramebuffers(...) opengl_debug_wrap(glDeleteFramebuffers, "glDeleteFramebuffers", __VA_ARGS__)
#endif
#ifdef glDeleteProgram
    #undef glDeleteProgram
    #define glDeleteProgram(...) opengl_debug_wrap_glew(__glewDeleteProgram, "glDeleteProgram", __VA_ARGS__)
#else
    #define glDeleteProgram(...) opengl_debug_wrap(glDeleteProgram, "glDeleteProgram", __VA_ARGS__)
#endif
#ifdef glDeleteRenderbuffers
    #undef glDeleteRenderbuffers
    #define glDeleteRenderbuffers(...) opengl_debug_wrap_glew(__glewDeleteRenderbuffers, "glDeleteRenderbuffers", __VA_ARGS__)
#else
    #define glDeleteRenderbuffers(...) opengl_debug_wrap(glDeleteRenderbuffers, "glDeleteRenderbuffers", __VA_ARGS__)
#endif
#ifdef glDeleteShader
    #undef glDeleteShader
    #define glDeleteShader(...) opengl_debug_wrap_glew(__glewDeleteShader, "glDeleteShader", __VA_ARGS__)
#else
    #define glDeleteShader(...) opengl_debug_wrap(glDeleteShader, "glDeleteShader", __VA_ARGS__)
#endif
#ifdef glDeleteTextures
    #undef glDeleteTextures
    #define glDeleteTextures(...) opengl_debug_wrap_glew(__glewDeleteTextures, "glDeleteTextures", __VA_ARGS__)
#else
    #define glDeleteTextures(...) opengl_debug_wrap(glDeleteTextures, "glDeleteTextures", __VA_ARGS__)
#endif
#ifdef glDeleteVertexArrays
    #undef glDeleteVertexArrays
    #define glDeleteVertexArrays(...) opengl_debug_wrap_glew(__glewDeleteVertexArrays, "glDeleteVertexArrays", __VA_ARGS__)
#else
    #define glDeleteVertexArrays(...) opengl_debug_wrap(glDeleteVertexArrays, "glDeleteVertexArrays", __VA_ARGS__)
#endif
#ifdef glDepthFunc
    #undef glDepthFunc
    #define glDepthFunc(...) opengl_debug_wrap_glew(__glewDepthFunc, "glDepthFunc", __VA_ARGS__)
#else
    #define glDepthFunc(...) opengl_debug_wrap(glDepthFunc, "glDepthFunc", __VA_ARGS__)
#endif
#ifdef glDepthMask
    #undef glDepthMask
    #define glDepthMask(...) opengl_debug_wrap_glew(__glewDepthMask, "glDepthMask", __VA_ARGS__)
#else
    #define glDepthMask(...) opengl_debug_wrap(glDepthMask, "glDepthMask", __VA_ARGS__)
#endif
#ifdef glDepthRange
    #undef glDepthRange
    #define glDepthRange(...) opengl_debug_wrap_glew(__glewDepthRange, "glDepthRange", __VA_ARGS__)
#else
    #define glDepthRange(...) opengl_debug_wrap(glDepthRange, "glDepthRange", __VA_ARGS__)
#endif
#ifdef glDisable
    #undef glDisable
    #define glDisable(...) opengl_debug_wrap_glew(__glewDisable, "glDisable", __VA_ARGS__)
#else
    #define glDisable(...) opengl_debug_wrap(glDisable, "glDisable", __VA_ARGS__)
#endif
#ifdef glDisableVertexAttribArray
    #undef glDisableVertexAttribArray
    #define glDisableVertexAttribArray(...) opengl_debug_wrap_glew(__glewDisableVertexAttribArray, "glDisableVertexAttribArray", __VA_ARGS__)
#else
    #define glDisableVertexAttribArray(...) opengl_debug_wrap(glDisableVertexAttribArray, "glDisableVertexAttribArray", __VA_ARGS__)
#endif
#ifdef glDrawArrays
    #undef glDrawArrays
    #define glDrawArrays(...) opengl_debug_wrap_glew(__glewDrawArrays, "glDrawArrays", __VA_ARGS__)
#else
    #define glDrawArrays(...) opengl_debug_wrap(glDrawArrays, "glDrawArrays", __VA_ARGS__)
#endif
#ifdef glDrawBuffer
    #undef glDrawBuffer
    #define glDrawBuffer(...) opengl_debug_wrap_glew(__glewDrawBuffer, "glDrawBuffer", __VA_ARGS__)
#else
    #define glDrawBuffer(...) opengl_debug_wrap(glDrawBuffer, "glDrawBuffer", __VA_ARGS__)
#endif
#ifdef glDrawBuffers
    #undef glDrawBuffers
    #define glDrawBuffers(...) opengl_debug_wrap_glew(__glewDrawBuffers, "glDrawBuffers", __VA_ARGS__)
#else
    #define glDrawBuffers(...) opengl_debug_wrap(glDrawBuffers, "glDrawBuffers", __VA_ARGS__)
#endif
#ifdef glDrawElements
    #undef glDrawElements
    #define glDrawElements(...) opengl_debug_wrap_glew(__glewDrawElements, "glDrawElements", __VA_ARGS__)
#else
    #define glDrawElements(...) opengl_debug_wrap(glDrawElements, "glDrawElements", __VA_ARGS__)
#endif
#ifdef glDrawPixels
    #undef glDrawPixels
    #define glDrawPixels(...) opengl_debug_wrap_glew(__glewDrawPixels, "glDrawPixels", __VA_ARGS__)
#else
    #define glDrawPixels(...) opengl_debug_wrap(glDrawPixels, "glDrawPixels", __VA_ARGS__)
#endif
#ifdef glDrawRangeElements
    #undef glDrawRangeElements
    #define glDrawRangeElements(...) opengl_debug_wrap_glew(__glewDrawRangeElements, "glDrawRangeElements", __VA_ARGS__)
#else
    #define glDrawRangeElements(...) opengl_debug_wrap(glDrawRangeElements, "glDrawRangeElements", __VA_ARGS__)
#endif
#ifdef glEnable
    #undef glEnable
    #define glEnable(...) opengl_debug_wrap_glew(__glewEnable, "glEnable", __VA_ARGS__)
#else
    #define glEnable(...) opengl_debug_wrap(glEnable, "glEnable", __VA_ARGS__)
#endif
#ifdef glEnableVertexAttribArray
    #undef glEnableVertexAttribArray
    #define glEnableVertexAttribArray(...) opengl_debug_wrap_glew(__glewEnableVertexAttribArray, "glEnableVertexAttribArray", __VA_ARGS__)
#else
    #define glEnableVertexAttribArray(...) opengl_debug_wrap(glEnableVertexAttribArray, "glEnableVertexAttribArray", __VA_ARGS__)
#endif
#ifdef glEnd
    #undef glEnd
    #define glEnd(...) opengl_debug_wrap_glew(__glewEnd, "glEnd", __VA_ARGS__)
#else
    #define glEnd(...) opengl_debug_wrap(glEnd, "glEnd", __VA_ARGS__)
#endif
#ifdef glFinish
    #undef glFinish
    #define glFinish(...) opengl_debug_wrap_glew(__glewFinish, "glFinish", __VA_ARGS__)
#else
    #define glFinish(...) opengl_debug_wrap(glFinish, "glFinish", __VA_ARGS__)
#endif
#ifdef glFlush
    #undef glFlush
    #define glFlush(...) opengl_debug_wrap_glew(__glewFlush, "glFlush", __VA_ARGS__)
#else
    #define glFlush(...) opengl_debug_wrap(glFlush, "glFlush", __VA_ARGS__)
#endif
#ifdef glFramebufferRenderbuffer
    #undef glFramebufferRenderbuffer
    #define glFramebufferRenderbuffer(...) opengl_debug_wrap_glew(__glewFramebufferRenderbuffer, "glFramebufferRenderbuffer", __VA_ARGS__)
#else
    #define glFramebufferRenderbuffer(...) opengl_debug_wrap(glFramebufferRenderbuffer, "glFramebufferRenderbuffer", __VA_ARGS__)
#endif
#ifdef glFramebufferTexture2D
    #undef glFramebufferTexture2D
    #define glFramebufferTexture2D(...) opengl_debug_wrap_glew(__glewFramebufferTexture2D, "glFramebufferTexture2D", __VA_ARGS__)
#else
    #define glFramebufferTexture2D(...) opengl_debug_wrap(glFramebufferTexture2D, "glFramebufferTexture2D", __VA_ARGS__)
#endif
#ifdef glGenBuffers
    #undef glGenBuffers
    #define glGenBuffers(...) opengl_debug_wrap_glew(__glewGenBuffers, "glGenBuffers", __VA_ARGS__)
#else
    #define glGenBuffers(...) opengl_debug_wrap(glGenBuffers, "glGenBuffers", __VA_ARGS__)
#endif
#ifdef glGenFramebuffers
    #undef glGenFramebuffers
    #define glGenFramebuffers(...) opengl_debug_wrap_glew(__glewGenFramebuffers, "glGenFramebuffers", __VA_ARGS__)
#else
    #define glGenFramebuffers(...) opengl_debug_wrap(glGenFramebuffers, "glGenFramebuffers", __VA_ARGS__)
#endif
#ifdef glGenRenderbuffers
    #undef glGenRenderbuffers
    #define glGenRenderbuffers(...) opengl_debug_wrap_glew(__glewGenRenderbuffers, "glGenRenderbuffers", __VA_ARGS__)
#else
    #define glGenRenderbuffers(...) opengl_debug_wrap(glGenRenderbuffers, "glGenRenderbuffers", __VA_ARGS__)
#endif
#ifdef glGenTextures
    #undef glGenTextures
    #define glGenTextures(...) opengl_debug_wrap_glew(__glewGenTextures, "glGenTextures", __VA_ARGS__)
#else
    #define glGenTextures(...) opengl_debug_wrap(glGenTextures, "glGenTextures", __VA_ARGS__)
#endif
#ifdef glGenVertexArrays
    #undef glGenVertexArrays
    #define glGenVertexArrays(...) opengl_debug_wrap_glew(__glewGenVertexArrays, "glGenVertexArrays", __VA_ARGS__)
#else
    #define glGenVertexArrays(...) opengl_debug_wrap(glGenVertexArrays, "glGenVertexArrays", __VA_ARGS__)
#endif
#ifdef glGetPixelMapfv
    #undef glGetPixelMapfv
    #define glGetPixelMapfv(...) opengl_debug_wrap_glew(__glewGetPixelMapfv, "glGetPixelMapfv", __VA_ARGS__)
#else
    #define glGetPixelMapfv(...) opengl_debug_wrap(glGetPixelMapfv, "glGetPixelMapfv", __VA_ARGS__)
#endif
#ifdef glGetPixelMapuiv
    #undef glGetPixelMapuiv
    #define glGetPixelMapuiv(...) opengl_debug_wrap_glew(__glewGetPixelMapuiv, "glGetPixelMapuiv", __VA_ARGS__)
#else
    #define glGetPixelMapuiv(...) opengl_debug_wrap(glGetPixelMapuiv, "glGetPixelMapuiv", __VA_ARGS__)
#endif
#ifdef glGetPixelMapusv
    #undef glGetPixelMapusv
    #define glGetPixelMapusv(...) opengl_debug_wrap_glew(__glewGetPixelMapusv, "glGetPixelMapusv", __VA_ARGS__)
#else
    #define glGetPixelMapusv(...) opengl_debug_wrap(glGetPixelMapusv, "glGetPixelMapusv", __VA_ARGS__)
#endif
#ifdef glGetProgramInfoLog
    #undef glGetProgramInfoLog
    #define glGetProgramInfoLog(...) opengl_debug_wrap_glew(__glewGetProgramInfoLog, "glGetProgramInfoLog", __VA_ARGS__)
#else
    #define glGetProgramInfoLog(...) opengl_debug_wrap(glGetProgramInfoLog, "glGetProgramInfoLog", __VA_ARGS__)
#endif
#ifdef glGetProgramiv
    #undef glGetProgramiv
    #define glGetProgramiv(...) opengl_debug_wrap_glew(__glewGetProgramiv, "glGetProgramiv", __VA_ARGS__)
#else
    #define glGetProgramiv(...) opengl_debug_wrap(glGetProgramiv, "glGetProgramiv", __VA_ARGS__)
#endif
#ifdef glGetShaderInfoLog
    #undef glGetShaderInfoLog
    #define glGetShaderInfoLog(...) opengl_debug_wrap_glew(__glewGetShaderInfoLog, "glGetShaderInfoLog", __VA_ARGS__)
#else
    #define glGetShaderInfoLog(...) opengl_debug_wrap(glGetShaderInfoLog, "glGetShaderInfoLog", __VA_ARGS__)
#endif
#ifdef glGetShaderiv
    #undef glGetShaderiv
    #define glGetShaderiv(...) opengl_debug_wrap_glew(__glewGetShaderiv, "glGetShaderiv", __VA_ARGS__)
#else
    #define glGetShaderiv(...) opengl_debug_wrap(glGetShaderiv, "glGetShaderiv", __VA_ARGS__)
#endif
#ifdef glGetTexEnvfv
    #undef glGetTexEnvfv
    #define glGetTexEnvfv(...) opengl_debug_wrap_glew(__glewGetTexEnvfv, "glGetTexEnvfv", __VA_ARGS__)
#else
    #define glGetTexEnvfv(...) opengl_debug_wrap(glGetTexEnvfv, "glGetTexEnvfv", __VA_ARGS__)
#endif
#ifdef glGetTexEnviv
    #undef glGetTexEnviv
    #define glGetTexEnviv(...) opengl_debug_wrap_glew(__glewGetTexEnviv, "glGetTexEnviv", __VA_ARGS__)
#else
    #define glGetTexEnviv(...) opengl_debug_wrap(glGetTexEnviv, "glGetTexEnviv", __VA_ARGS__)
#endif
#ifdef glGetTexGendv
    #undef glGetTexGendv
    #define glGetTexGendv(...) opengl_debug_wrap_glew(__glewGetTexGendv, "glGetTexGendv", __VA_ARGS__)
#else
    #define glGetTexGendv(...) opengl_debug_wrap(glGetTexGendv, "glGetTexGendv", __VA_ARGS__)
#endif
#ifdef glGetTexGenfv
    #undef glGetTexGenfv
    #define glGetTexGenfv(...) opengl_debug_wrap_glew(__glewGetTexGenfv, "glGetTexGenfv", __VA_ARGS__)
#else
    #define glGetTexGenfv(...) opengl_debug_wrap(glGetTexGenfv, "glGetTexGenfv", __VA_ARGS__)
#endif
#ifdef glGetTexGeniv
    #undef glGetTexGeniv
    #define glGetTexGeniv(...) opengl_debug_wrap_glew(__glewGetTexGeniv, "glGetTexGeniv", __VA_ARGS__)
#else
    #define glGetTexGeniv(...) opengl_debug_wrap(glGetTexGeniv, "glGetTexGeniv", __VA_ARGS__)
#endif
#ifdef glGetTexImage
    #undef glGetTexImage
    #define glGetTexImage(...) opengl_debug_wrap_glew(__glewGetTexImage, "glGetTexImage", __VA_ARGS__)
#else
    #define glGetTexImage(...) opengl_debug_wrap(glGetTexImage, "glGetTexImage", __VA_ARGS__)
#endif
#ifdef glGetTexLevelParameterfv
    #undef glGetTexLevelParameterfv
    #define glGetTexLevelParameterfv(...) opengl_debug_wrap_glew(__glewGetTexLevelParameterfv, "glGetTexLevelParameterfv", __VA_ARGS__)
#else
    #define glGetTexLevelParameterfv(...) opengl_debug_wrap(glGetTexLevelParameterfv, "glGetTexLevelParameterfv", __VA_ARGS__)
#endif
#ifdef glGetTexLevelParameteriv
    #undef glGetTexLevelParameteriv
    #define glGetTexLevelParameteriv(...) opengl_debug_wrap_glew(__glewGetTexLevelParameteriv, "glGetTexLevelParameteriv", __VA_ARGS__)
#else
    #define glGetTexLevelParameteriv(...) opengl_debug_wrap(glGetTexLevelParameteriv, "glGetTexLevelParameteriv", __VA_ARGS__)
#endif
#ifdef glGetTexParameterfv
    #undef glGetTexParameterfv
    #define glGetTexParameterfv(...) opengl_debug_wrap_glew(__glewGetTexParameterfv, "glGetTexParameterfv", __VA_ARGS__)
#else
    #define glGetTexParameterfv(...) opengl_debug_wrap(glGetTexParameterfv, "glGetTexParameterfv", __VA_ARGS__)
#endif
#ifdef glGetTexParameteriv
    #undef glGetTexParameteriv
    #define glGetTexParameteriv(...) opengl_debug_wrap_glew(__glewGetTexParameteriv, "glGetTexParameteriv", __VA_ARGS__)
#else
    #define glGetTexParameteriv(...) opengl_debug_wrap(glGetTexParameteriv, "glGetTexParameteriv", __VA_ARGS__)
#endif
#ifdef glGetUniformLocation
    #undef glGetUniformLocation
    #define glGetUniformLocation(...) opengl_debug_wrap_glew(__glewGetUniformLocation, "glGetUniformLocation", __VA_ARGS__)
#else
    #define glGetUniformLocation(...) opengl_debug_wrap(glGetUniformLocation, "glGetUniformLocation", __VA_ARGS__)
#endif
#ifdef glIsEnabled
    #undef glIsEnabled
    #define glIsEnabled(...) opengl_debug_wrap_glew(__glewIsEnabled, "glIsEnabled", __VA_ARGS__)
#else
    #define glIsEnabled(...) opengl_debug_wrap(glIsEnabled, "glIsEnabled", __VA_ARGS__)
#endif
#ifdef glIsTexture
    #undef glIsTexture
    #define glIsTexture(...) opengl_debug_wrap_glew(__glewIsTexture, "glIsTexture", __VA_ARGS__)
#else
    #define glIsTexture(...) opengl_debug_wrap(glIsTexture, "glIsTexture", __VA_ARGS__)
#endif
#ifdef glLineWidth
    #undef glLineWidth
    #define glLineWidth(...) opengl_debug_wrap_glew(__glewLineWidth, "glLineWidth", __VA_ARGS__)
#else
    #define glLineWidth(...) opengl_debug_wrap(glLineWidth, "glLineWidth", __VA_ARGS__)
#endif
#ifdef glLinkProgram
    #undef glLinkProgram
    #define glLinkProgram(...) opengl_debug_wrap_glew(__glewLinkProgram, "glLinkProgram", __VA_ARGS__)
#else
    #define glLinkProgram(...) opengl_debug_wrap(glLinkProgram, "glLinkProgram", __VA_ARGS__)
#endif
#ifdef glPixelMapfv
    #undef glPixelMapfv
    #define glPixelMapfv(...) opengl_debug_wrap_glew(__glewPixelMapfv, "glPixelMapfv", __VA_ARGS__)
#else
    #define glPixelMapfv(...) opengl_debug_wrap(glPixelMapfv, "glPixelMapfv", __VA_ARGS__)
#endif
#ifdef glPixelMapuiv
    #undef glPixelMapuiv
    #define glPixelMapuiv(...) opengl_debug_wrap_glew(__glewPixelMapuiv, "glPixelMapuiv", __VA_ARGS__)
#else
    #define glPixelMapuiv(...) opengl_debug_wrap(glPixelMapuiv, "glPixelMapuiv", __VA_ARGS__)
#endif
#ifdef glPixelMapusv
    #undef glPixelMapusv
    #define glPixelMapusv(...) opengl_debug_wrap_glew(__glewPixelMapusv, "glPixelMapusv", __VA_ARGS__)
#else
    #define glPixelMapusv(...) opengl_debug_wrap(glPixelMapusv, "glPixelMapusv", __VA_ARGS__)
#endif
#ifdef glPixelStoref
    #undef glPixelStoref
    #define glPixelStoref(...) opengl_debug_wrap_glew(__glewPixelStoref, "glPixelStoref", __VA_ARGS__)
#else
    #define glPixelStoref(...) opengl_debug_wrap(glPixelStoref, "glPixelStoref", __VA_ARGS__)
#endif
#ifdef glPixelStorei
    #undef glPixelStorei
    #define glPixelStorei(...) opengl_debug_wrap_glew(__glewPixelStorei, "glPixelStorei", __VA_ARGS__)
#else
    #define glPixelStorei(...) opengl_debug_wrap(glPixelStorei, "glPixelStorei", __VA_ARGS__)
#endif
#ifdef glPixelTransferf
    #undef glPixelTransferf
    #define glPixelTransferf(...) opengl_debug_wrap_glew(__glewPixelTransferf, "glPixelTransferf", __VA_ARGS__)
#else
    #define glPixelTransferf(...) opengl_debug_wrap(glPixelTransferf, "glPixelTransferf", __VA_ARGS__)
#endif
#ifdef glPixelTransferi
    #undef glPixelTransferi
    #define glPixelTransferi(...) opengl_debug_wrap_glew(__glewPixelTransferi, "glPixelTransferi", __VA_ARGS__)
#else
    #define glPixelTransferi(...) opengl_debug_wrap(glPixelTransferi, "glPixelTransferi", __VA_ARGS__)
#endif
#ifdef glPixelZoom
    #undef glPixelZoom
    #define glPixelZoom(...) opengl_debug_wrap_glew(__glewPixelZoom, "glPixelZoom", __VA_ARGS__)
#else
    #define glPixelZoom(...) opengl_debug_wrap(glPixelZoom, "glPixelZoom", __VA_ARGS__)
#endif
#ifdef glPointSize
    #undef glPointSize
    #define glPointSize(...) opengl_debug_wrap_glew(__glewPointSize, "glPointSize", __VA_ARGS__)
#else
    #define glPointSize(...) opengl_debug_wrap(glPointSize, "glPointSize", __VA_ARGS__)
#endif
#ifdef glPrioritizeTextures
    #undef glPrioritizeTextures
    #define glPrioritizeTextures(...) opengl_debug_wrap_glew(__glewPrioritizeTextures, "glPrioritizeTextures", __VA_ARGS__)
#else
    #define glPrioritizeTextures(...) opengl_debug_wrap(glPrioritizeTextures, "glPrioritizeTextures", __VA_ARGS__)
#endif
#ifdef glReadBuffer
    #undef glReadBuffer
    #define glReadBuffer(...) opengl_debug_wrap_glew(__glewReadBuffer, "glReadBuffer", __VA_ARGS__)
#else
    #define glReadBuffer(...) opengl_debug_wrap(glReadBuffer, "glReadBuffer", __VA_ARGS__)
#endif
#ifdef glReadPixels
    #undef glReadPixels
    #define glReadPixels(...) opengl_debug_wrap_glew(__glewReadPixels, "glReadPixels", __VA_ARGS__)
#else
    #define glReadPixels(...) opengl_debug_wrap(glReadPixels, "glReadPixels", __VA_ARGS__)
#endif
#ifdef glRenderbufferStorage
    #undef glRenderbufferStorage
    #define glRenderbufferStorage(...) opengl_debug_wrap_glew(__glewRenderbufferStorage, "glRenderbufferStorage", __VA_ARGS__)
#else
    #define glRenderbufferStorage(...) opengl_debug_wrap(glRenderbufferStorage, "glRenderbufferStorage", __VA_ARGS__)
#endif
#ifdef glShaderSource
    #undef glShaderSource
    #define glShaderSource(...) opengl_debug_wrap_glew(__glewShaderSource, "glShaderSource", __VA_ARGS__)
#else
    #define glShaderSource(...) opengl_debug_wrap(glShaderSource, "glShaderSource", __VA_ARGS__)
#endif
#ifdef glStencilFunc
    #undef glStencilFunc
    #define glStencilFunc(...) opengl_debug_wrap_glew(__glewStencilFunc, "glStencilFunc", __VA_ARGS__)
#else
    #define glStencilFunc(...) opengl_debug_wrap(glStencilFunc, "glStencilFunc", __VA_ARGS__)
#endif
#ifdef glStencilMask
    #undef glStencilMask
    #define glStencilMask(...) opengl_debug_wrap_glew(__glewStencilMask, "glStencilMask", __VA_ARGS__)
#else
    #define glStencilMask(...) opengl_debug_wrap(glStencilMask, "glStencilMask", __VA_ARGS__)
#endif
#ifdef glStencilOp
    #undef glStencilOp
    #define glStencilOp(...) opengl_debug_wrap_glew(__glewStencilOp, "glStencilOp", __VA_ARGS__)
#else
    #define glStencilOp(...) opengl_debug_wrap(glStencilOp, "glStencilOp", __VA_ARGS__)
#endif
#ifdef glTexEnvf
    #undef glTexEnvf
    #define glTexEnvf(...) opengl_debug_wrap_glew(__glewTexEnvf, "glTexEnvf", __VA_ARGS__)
#else
    #define glTexEnvf(...) opengl_debug_wrap(glTexEnvf, "glTexEnvf", __VA_ARGS__)
#endif
#ifdef glTexEnvfv
    #undef glTexEnvfv
    #define glTexEnvfv(...) opengl_debug_wrap_glew(__glewTexEnvfv, "glTexEnvfv", __VA_ARGS__)
#else
    #define glTexEnvfv(...) opengl_debug_wrap(glTexEnvfv, "glTexEnvfv", __VA_ARGS__)
#endif
#ifdef glTexEnvi
    #undef glTexEnvi
    #define glTexEnvi(...) opengl_debug_wrap_glew(__glewTexEnvi, "glTexEnvi", __VA_ARGS__)
#else
    #define glTexEnvi(...) opengl_debug_wrap(glTexEnvi, "glTexEnvi", __VA_ARGS__)
#endif
#ifdef glTexEnviv
    #undef glTexEnviv
    #define glTexEnviv(...) opengl_debug_wrap_glew(__glewTexEnviv, "glTexEnviv", __VA_ARGS__)
#else
    #define glTexEnviv(...) opengl_debug_wrap(glTexEnviv, "glTexEnviv", __VA_ARGS__)
#endif
#ifdef glTexGend
    #undef glTexGend
    #define glTexGend(...) opengl_debug_wrap_glew(__glewTexGend, "glTexGend", __VA_ARGS__)
#else
    #define glTexGend(...) opengl_debug_wrap(glTexGend, "glTexGend", __VA_ARGS__)
#endif
#ifdef glTexGendv
    #undef glTexGendv
    #define glTexGendv(...) opengl_debug_wrap_glew(__glewTexGendv, "glTexGendv", __VA_ARGS__)
#else
    #define glTexGendv(...) opengl_debug_wrap(glTexGendv, "glTexGendv", __VA_ARGS__)
#endif
#ifdef glTexGenf
    #undef glTexGenf
    #define glTexGenf(...) opengl_debug_wrap_glew(__glewTexGenf, "glTexGenf", __VA_ARGS__)
#else
    #define glTexGenf(...) opengl_debug_wrap(glTexGenf, "glTexGenf", __VA_ARGS__)
#endif
#ifdef glTexGenfv
    #undef glTexGenfv
    #define glTexGenfv(...) opengl_debug_wrap_glew(__glewTexGenfv, "glTexGenfv", __VA_ARGS__)
#else
    #define glTexGenfv(...) opengl_debug_wrap(glTexGenfv, "glTexGenfv", __VA_ARGS__)
#endif
#ifdef glTexGeni
    #undef glTexGeni
    #define glTexGeni(...) opengl_debug_wrap_glew(__glewTexGeni, "glTexGeni", __VA_ARGS__)
#else
    #define glTexGeni(...) opengl_debug_wrap(glTexGeni, "glTexGeni", __VA_ARGS__)
#endif
#ifdef glTexGeniv
    #undef glTexGeniv
    #define glTexGeniv(...) opengl_debug_wrap_glew(__glewTexGeniv, "glTexGeniv", __VA_ARGS__)
#else
    #define glTexGeniv(...) opengl_debug_wrap(glTexGeniv, "glTexGeniv", __VA_ARGS__)
#endif
#ifdef glTexImage1D
    #undef glTexImage1D
    #define glTexImage1D(...) opengl_debug_wrap_glew(__glewTexImage1D, "glTexImage1D", __VA_ARGS__)
#else
    #define glTexImage1D(...) opengl_debug_wrap(glTexImage1D, "glTexImage1D", __VA_ARGS__)
#endif
#ifdef glTexImage2D
    #undef glTexImage2D
    #define glTexImage2D(...) opengl_debug_wrap_glew(__glewTexImage2D, "glTexImage2D", __VA_ARGS__)
#else
    #define glTexImage2D(...) opengl_debug_wrap(glTexImage2D, "glTexImage2D", __VA_ARGS__)
#endif
#ifdef glTexImage3D
    #undef glTexImage3D
    #define glTexImage3D(...) opengl_debug_wrap_glew(__glewTexImage3D, "glTexImage3D", __VA_ARGS__)
#else
    #define glTexImage3D(...) opengl_debug_wrap(glTexImage3D, "glTexImage3D", __VA_ARGS__)
#endif
#ifdef glTexParameterf
    #undef glTexParameterf
    #define glTexParameterf(...) opengl_debug_wrap_glew(__glewTexParameterf, "glTexParameterf", __VA_ARGS__)
#else
    #define glTexParameterf(...) opengl_debug_wrap(glTexParameterf, "glTexParameterf", __VA_ARGS__)
#endif
#ifdef glTexParameterfv
    #undef glTexParameterfv
    #define glTexParameterfv(...) opengl_debug_wrap_glew(__glewTexParameterfv, "glTexParameterfv", __VA_ARGS__)
#else
    #define glTexParameterfv(...) opengl_debug_wrap(glTexParameterfv, "glTexParameterfv", __VA_ARGS__)
#endif
#ifdef glTexParameteri
    #undef glTexParameteri
    #define glTexParameteri(...) opengl_debug_wrap_glew(__glewTexParameteri, "glTexParameteri", __VA_ARGS__)
#else
    #define glTexParameteri(...) opengl_debug_wrap(glTexParameteri, "glTexParameteri", __VA_ARGS__)
#endif
#ifdef glTexParameteriv
    #undef glTexParameteriv
    #define glTexParameteriv(...) opengl_debug_wrap_glew(__glewTexParameteriv, "glTexParameteriv", __VA_ARGS__)
#else
    #define glTexParameteriv(...) opengl_debug_wrap(glTexParameteriv, "glTexParameteriv", __VA_ARGS__)
#endif
#ifdef glTexSubImage1D
    #undef glTexSubImage1D
    #define glTexSubImage1D(...) opengl_debug_wrap_glew(__glewTexSubImage1D, "glTexSubImage1D", __VA_ARGS__)
#else
    #define glTexSubImage1D(...) opengl_debug_wrap(glTexSubImage1D, "glTexSubImage1D", __VA_ARGS__)
#endif
#ifdef glTexSubImage2D
    #undef glTexSubImage2D
    #define glTexSubImage2D(...) opengl_debug_wrap_glew(__glewTexSubImage2D, "glTexSubImage2D", __VA_ARGS__)
#else
    #define glTexSubImage2D(...) opengl_debug_wrap(glTexSubImage2D, "glTexSubImage2D", __VA_ARGS__)
#endif
#ifdef glTexSubImage3D
    #undef glTexSubImage3D
    #define glTexSubImage3D(...) opengl_debug_wrap_glew(__glewTexSubImage3D, "glTexSubImage3D", __VA_ARGS__)
#else
    #define glTexSubImage3D(...) opengl_debug_wrap(glTexSubImage3D, "glTexSubImage3D", __VA_ARGS__)
#endif
#ifdef glUniform1f
    #undef glUniform1f
    #define glUniform1f(...) opengl_debug_wrap_glew(__glewUniform1f, "glUniform1f", __VA_ARGS__)
#else
    #define glUniform1f(...) opengl_debug_wrap(glUniform1f, "glUniform1f", __VA_ARGS__)
#endif
#ifdef glUniform1i
    #undef glUniform1i
    #define glUniform1i(...) opengl_debug_wrap_glew(__glewUniform1i, "glUniform1i", __VA_ARGS__)
#else
    #define glUniform1i(...) opengl_debug_wrap(glUniform1i, "glUniform1i", __VA_ARGS__)
#endif
#ifdef glUniform2f
    #undef glUniform2f
    #define glUniform2f(...) opengl_debug_wrap_glew(__glewUniform2f, "glUniform2f", __VA_ARGS__)
#else
    #define glUniform2f(...) opengl_debug_wrap(glUniform2f, "glUniform2f", __VA_ARGS__)
#endif
#ifdef glUniform2i
    #undef glUniform2i
    #define glUniform2i(...) opengl_debug_wrap_glew(__glewUniform2i, "glUniform2i", __VA_ARGS__)
#else
    #define glUniform2i(...) opengl_debug_wrap(glUniform2i, "glUniform2i", __VA_ARGS__)
#endif
#ifdef glUniform3f
    #undef glUniform3f
    #define glUniform3f(...) opengl_debug_wrap_glew(__glewUniform3f, "glUniform3f", __VA_ARGS__)
#else
    #define glUniform3f(...) opengl_debug_wrap(glUniform3f, "glUniform3f", __VA_ARGS__)
#endif
#ifdef glUniform3i
    #undef glUniform3i
    #define glUniform3i(...) opengl_debug_wrap_glew(__glewUniform3i, "glUniform3i", __VA_ARGS__)
#else
    #define glUniform3i(...) opengl_debug_wrap(glUniform3i, "glUniform3i", __VA_ARGS__)
#endif
#ifdef glUniform4f
    #undef glUniform4f
    #define glUniform4f(...) opengl_debug_wrap_glew(__glewUniform4f, "glUniform4f", __VA_ARGS__)
#else
    #define glUniform4f(...) opengl_debug_wrap(glUniform4f, "glUniform4f", __VA_ARGS__)
#endif
#ifdef glUniform4i
    #undef glUniform4i
    #define glUniform4i(...) opengl_debug_wrap_glew(__glewUniform4i, "glUniform4i", __VA_ARGS__)
#else
    #define glUniform4i(...) opengl_debug_wrap(glUniform4i, "glUniform4i", __VA_ARGS__)
#endif
#ifdef glUniform1fv
    #undef glUniform1fv
    #define glUniform1fv(...) opengl_debug_wrap_glew(__glewUniform1fv, "glUniform1fv", __VA_ARGS__)
#else
    #define glUniform1fv(...) opengl_debug_wrapv(glUniform1fv, "glUniform1fv", __VA_ARGS__)
#endif
#ifdef glUniform1iv
    #undef glUniform1iv
    #define glUniform1iv(...) opengl_debug_wrap_glew(__glewUniform1iv, "glUniform1iv", __VA_ARGS__)
#else
    #define glUniform1iv(...) opengl_debug_wrapv(glUniform1iv, "glUniform1iv", __VA_ARGS__)
#endif
#ifdef glUniform2fv
    #undef glUniform2fv
    #define glUniform2fv(...) opengl_debug_wrap_glew(__glewUniform2fv, "glUniform2fv", __VA_ARGS__)
#else
    #define glUniform2fv(...) opengl_debug_wrapv(glUniform2fv, "glUniform2fv", __VA_ARGS__)
#endif
#ifdef glUniform2iv
    #undef glUniform2iv
    #define glUniform2iv(...) opengl_debug_wrap_glew(__glewUniform2iv, "glUniform2iv", __VA_ARGS__)
#else
    #define glUniform2iv(...) opengl_debug_wrapv(glUniform2iv, "glUniform2iv", __VA_ARGS__)
#endif
#ifdef glUniform3fv
    #undef glUniform3fv
    #define glUniform3fv(...) opengl_debug_wrap_glew(__glewUniform3fv, "glUniform3fv", __VA_ARGS__)
#else
    #define glUniform3fv(...) opengl_debug_wrapv(glUniform3fv, "glUniform3fv", __VA_ARGS__)
#endif
#ifdef glUniform3iv
    #undef glUniform3iv
    #define glUniform3iv(...) opengl_debug_wrap_glew(__glewUniform3iv, "glUniform3iv", __VA_ARGS__)
#else
    #define glUniform3iv(...) opengl_debug_wrapv(glUniform3iv, "glUniform3iv", __VA_ARGS__)
#endif
#ifdef glUniform4fv
    #undef glUniform4fv
    #define glUniform4fv(...) opengl_debug_wrap_glew(__glewUniform4fv, "glUniform4fv", __VA_ARGS__)
#else
    #define glUniform4fv(...) opengl_debug_wrapv(glUniform4fv, "glUniform4fv", __VA_ARGS__)
#endif
#ifdef glUniform4iv
    #undef glUniform4iv
    #define glUniform4iv(...) opengl_debug_wrap_glew(__glewUniform4iv, "glUniform4iv", __VA_ARGS__)
#else
    #define glUniform4iv(...) opengl_debug_wrapv(glUniform4iv, "glUniform4iv", __VA_ARGS__)
#endif
#ifdef glUseProgram
    #undef glUseProgram
    #define glUseProgram(...) opengl_debug_wrap_glew(__glewUseProgram, "glUseProgram", __VA_ARGS__)
#else
    #define glUseProgram(...) opengl_debug_wrap(glUseProgram, "glUseProgram", __VA_ARGS__)
#endif
#ifdef glVertexAttribPointer
    #undef glVertexAttribPointer
    #define glVertexAttribPointer(...) opengl_debug_wrap_glew(__glewVertexAttribPointer, "glVertexAttribPointer", __VA_ARGS__)
#else
    #define glVertexAttribPointer(...) opengl_debug_wrap(glVertexAttribPointer, "glVertexAttribPointer", __VA_ARGS__)
#endif
#endif

#endif
