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


/*
 // These are autovivifying OpenGL wrappers.
extern void (* glGenBuffers )(GLsizei, GLuint*);
extern void (* glBindBuffer )(GLenum, GLuint);
extern void (* glBufferData )(GLenum, GLsizeiptr, const GLvoid*, GLenum);
extern void (* glDeleteBuffers )(GLsizei, const GLuint*);

extern void (* glGenVertexArrays )(GLsizei, GLuint*);
extern void (* glBindVertexArray )(GLuint);
extern void (* glEnableVertexAttribArray )(GLuint);
extern void (* glDisableVertexAttribArray )(GLuint);
extern void (* glVertexAttribPointer )(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*);
extern void (* glDeleteVertexArrays )(GLsizei, const GLuint*);

extern GLuint (* glCreateShader )(GLenum);
extern void (* glShaderSource )(GLuint, GLsizei, const GLchar**, const GLint*);
extern void (* glCompileShader )(GLuint);
extern void (* glGetShaderiv )(GLuint, GLenum, GLint*);
extern void (* glGetShaderInfoLog )(GLuint, GLsizei, GLsizei*, GLchar*);
extern void (* glDeleteShader )(GLuint);

extern GLuint (* glCreateProgram )();
extern GLuint (* glAttachShader )(GLuint, GLuint);
extern void (* glBindAttribLocation )(GLuint, GLuint, const GLchar*);
extern void (* glLinkProgram )(GLuint);
extern void (* glGetProgramiv )(GLuint, GLenum, GLint*);
extern void (* glGetProgramInfoLog )(GLuint, GLsizei, GLsizei*, GLchar*);
extern void (* glDeleteProgram )(GLuint);
extern GLint (* glGetUniformLocation )(GLuint, const GLchar*);

extern void (* glUseProgram )(GLuint);
extern void (* glUniform1i )(GLint, GLint);
extern void (* glUniform2i )(GLint, GLint, GLint);
extern void (* glUniform3i )(GLint, GLint, GLint, GLint);
extern void (* glUniform4i )(GLint, GLint, GLint, GLint, GLint);
extern void (* glUniform1f )(GLint, GLfloat);
extern void (* glUniform2f )(GLint, GLfloat, GLfloat);
extern void (* glUniform3f )(GLint, GLfloat, GLfloat, GLfloat);
extern void (* glUniform4f )(GLint, GLfloat, GLfloat, GLfloat, GLfloat);

extern void (* glGenFramebuffers )(GLsizei, GLuint*);
extern void (* glBindFramebuffer )(GLenum, GLuint);
extern void (* glDeleteFramebuffers )(GLsizei, GLuint*);
extern void (* glGenRenderbuffers )(GLsizei, GLuint*);
extern void (* glBindRenderbuffer )(GLenum, GLuint);
extern void (* glDeleteRenderbuffers )(GLsizei, GLuint*);
extern void (* glRenderbufferStorage )(GLenum, GLenum, GLsizei, GLsizei);
extern void (* glFramebufferRenderbuffer )(GLenum, GLenum, GLenum, GLuint);
extern void (* glFramebufferTexture2D )(GLenum, GLenum, GLenum, GLuint, GLint);
extern void (* glDrawBuffers )(GLsizei, const GLenum*);
extern GLenum (* glCheckFramebufferStatus )(GLenum);
*/

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

#define glClearColor(...) opengl_debug_wrap(glClearColor, "glClearColor", __VA_ARGS__)
#define glClear(...) opengl_debug_wrap(glClear, "glClear", __VA_ARGS__)
#define glBlendFunc(...) opengl_debug_wrap(glBlendFunc, "glBlendFunc", __VA_ARGS__)
#define glPointSize(...) opengl_debug_wrap(glPointSize, "glPointSize", __VA_ARGS__)
#define glLineWidth(...) opengl_debug_wrap(glLineWidth, "glLineWidth", __VA_ARGS__)
#define glDrawBuffer(...) opengl_debug_wrap(glDrawBuffer, "glDrawBuffer", __VA_ARGS__)
#define glReadBuffer(...) opengl_debug_wrap(glReadBuffer, "glReadBuffer", __VA_ARGS__)
#define glEnable(...) opengl_debug_wrap(glEnable, "glEnable", __VA_ARGS__)
#define glDisable(...) opengl_debug_wrap(glDisable, "glDisable", __VA_ARGS__)
#define glIsEnabled(...) opengl_debug_wrap(glIsEnabled, "glIsEnabled", __VA_ARGS__)
#define glFinish(...) opengl_debug_wrap(glFinish, "glFinish", __VA_ARGS__)
#define glFlush(...) opengl_debug_wrap(glFlush, "glFlush", __VA_ARGS__)
#define glClearDepth(...) opengl_debug_wrap(glClearDepth, "glClearDepth", __VA_ARGS__)
#define glDepthFunc(...) opengl_debug_wrap(glDepthFunc, "glDepthFunc", __VA_ARGS__)
#define glDepthMask(...) opengl_debug_wrap(glDepthMask, "glDepthMask", __VA_ARGS__)
#define glDepthRange(...) opengl_debug_wrap(glDepthRange, "glDepthRange", __VA_ARGS__)
#define glClearAccum(...) opengl_debug_wrap(glClearAccum, "glClearAccum", __VA_ARGS__)
#define glAccum(...) opengl_debug_wrap(glAccum, "glAccum", __VA_ARGS__)
#define glBegin(...) opengl_debug_wrap(glBegin, "glBegin", __VA_ARGS__)
#define glEnd(...) opengl_debug_wrap(glEnd, "glEnd", __VA_ARGS__)
#define glArrayElement(...) opengl_debug_wrap(glArrayElement, "glArrayElement", __VA_ARGS__)
#define glDrawArrays(...) opengl_debug_wrap(glDrawArrays, "glDrawArrays", __VA_ARGS__)
#define glDrawElements(...) opengl_debug_wrap(glDrawElements, "glDrawElements", __VA_ARGS__)
#define glPixelZoom(...) opengl_debug_wrap(glPixelZoom, "glPixelZoom", __VA_ARGS__)
#define glPixelStoref(...) opengl_debug_wrap(glPixelStoref, "glPixelStoref", __VA_ARGS__)
#define glPixelStorei(...) opengl_debug_wrap(glPixelStorei, "glPixelStorei", __VA_ARGS__)
#define glPixelTransferf(...) opengl_debug_wrap(glPixelTransferf, "glPixelTransferf", __VA_ARGS__)
#define glPixelTransferi(...) opengl_debug_wrap(glPixelTransferi, "glPixelTransferi", __VA_ARGS__)
#define glReadPixels(...) opengl_debug_wrap(glReadPixels, "glReadPixels", __VA_ARGS__)
#define glDrawPixels(...) opengl_debug_wrap(glDrawPixels, "glDrawPixels", __VA_ARGS__)
#define glCopyPixels(...) opengl_debug_wrap(glCopyPixels, "glCopyPixels", __VA_ARGS__)
#define glPixelMapfv(...) opengl_debug_wrap(glPixelMapfv, "glPixelMapfv", __VA_ARGS__)
#define glPixelMapuiv(...) opengl_debug_wrap(glPixelMapuiv, "glPixelMapuiv", __VA_ARGS__)
#define glPixelMapusv(...) opengl_debug_wrap(glPixelMapusv, "glPixelMapusv", __VA_ARGS__)
#define glGetPixelMapfv(...) opengl_debug_wrap(glGetPixelMapfv, "glGetPixelMapfv", __VA_ARGS__)
#define glGetPixelMapuiv(...) opengl_debug_wrap(glGetPixelMapuiv, "glGetPixelMapuiv", __VA_ARGS__)
#define glGetPixelMapusv(...) opengl_debug_wrap(glGetPixelMapusv, "glGetPixelMapusv", __VA_ARGS__)
#define glBitmap(...) opengl_debug_wrap(glBitmap, "glBitmap", __VA_ARGS__)
#define glStencilFunc(...) opengl_debug_wrap(glStencilFunc, "glStencilFunc", __VA_ARGS__)
#define glStencilMask(...) opengl_debug_wrap(glStencilMask, "glStencilMask", __VA_ARGS__)
#define glStencilOp(...) opengl_debug_wrap(glStencilOp, "glStencilOp", __VA_ARGS__)
#define glClearStencil(...) opengl_debug_wrap(glClearStencil, "glClearStencil", __VA_ARGS__)
#define glTexGend(...) opengl_debug_wrap(glTexGend, "glTexGend", __VA_ARGS__)
#define glTexGenf(...) opengl_debug_wrap(glTexGenf, "glTexGenf", __VA_ARGS__)
#define glTexGeni(...) opengl_debug_wrap(glTexGeni, "glTexGeni", __VA_ARGS__)
#define glTexGendv(...) opengl_debug_wrap(glTexGendv, "glTexGendv", __VA_ARGS__)
#define glTexGenfv(...) opengl_debug_wrap(glTexGenfv, "glTexGenfv", __VA_ARGS__)
#define glTexGeniv(...) opengl_debug_wrap(glTexGeniv, "glTexGeniv", __VA_ARGS__)
#define glGetTexGendv(...) opengl_debug_wrap(glGetTexGendv, "glGetTexGendv", __VA_ARGS__)
#define glGetTexGenfv(...) opengl_debug_wrap(glGetTexGenfv, "glGetTexGenfv", __VA_ARGS__)
#define glGetTexGeniv(...) opengl_debug_wrap(glGetTexGeniv, "glGetTexGeniv", __VA_ARGS__)
#define glTexEnvf(...) opengl_debug_wrap(glTexEnvf, "glTexEnvf", __VA_ARGS__)
#define glTexEnvi(...) opengl_debug_wrap(glTexEnvi, "glTexEnvi", __VA_ARGS__)
#define glTexEnvfv(...) opengl_debug_wrap(glTexEnvfv, "glTexEnvfv", __VA_ARGS__)
#define glTexEnviv(...) opengl_debug_wrap(glTexEnviv, "glTexEnviv", __VA_ARGS__)
#define glGetTexEnvfv(...) opengl_debug_wrap(glGetTexEnvfv, "glGetTexEnvfv", __VA_ARGS__)
#define glGetTexEnviv(...) opengl_debug_wrap(glGetTexEnviv, "glGetTexEnviv", __VA_ARGS__)
#define glTexParameterf(...) opengl_debug_wrap(glTexParameterf, "glTexParameterf", __VA_ARGS__)
#define glTexParameteri(...) opengl_debug_wrap(glTexParameteri, "glTexParameteri", __VA_ARGS__)
#define glTexParameterfv(...) opengl_debug_wrap(glTexParameterfv, "glTexParameterfv", __VA_ARGS__)
#define glTexParameteriv(...) opengl_debug_wrap(glTexParameteriv, "glTexParameteriv", __VA_ARGS__)
#define glGetTexParameterfv(...) opengl_debug_wrap(glGetTexParameterfv, "glGetTexParameterfv", __VA_ARGS__)
#define glGetTexParameteriv(...) opengl_debug_wrap(glGetTexParameteriv, "glGetTexParameteriv", __VA_ARGS__)
#define glGetTexLevelParameterfv(...) opengl_debug_wrap(glGetTexLevelParameterfv, "glGetTexLevelParameterfv", __VA_ARGS__)
#define glGetTexLevelParameteriv(...) opengl_debug_wrap(glGetTexLevelParameteriv, "glGetTexLevelParameteriv", __VA_ARGS__)
#define glTexImage1D(...) opengl_debug_wrap(glTexImage1D, "glTexImage1D", __VA_ARGS__)
#define glTexImage2D(...) opengl_debug_wrap(glTexImage2D, "glTexImage2D", __VA_ARGS__)
#define glGetTexImage(...) opengl_debug_wrap(glGetTexImage, "glGetTexImage", __VA_ARGS__)
#define glGenTextures(...) opengl_debug_wrap(glGenTextures, "glGenTextures", __VA_ARGS__)
#define glDeleteTextures(...) opengl_debug_wrap(glDeleteTextures, "glDeleteTextures", __VA_ARGS__)
#define glBindTexture(...) opengl_debug_wrap(glBindTexture, "glBindTexture", __VA_ARGS__)
#define glPrioritizeTextures(...) opengl_debug_wrap(glPrioritizeTextures, "glPrioritizeTextures", __VA_ARGS__)
#define glAreTexturesResident(...) opengl_debug_wrap(glAreTexturesResident, "glAreTexturesResident", __VA_ARGS__)
#define glIsTexture(...) opengl_debug_wrap(glIsTexture, "glIsTexture", __VA_ARGS__)
#define glTexSubImage1D(...) opengl_debug_wrap(glTexSubImage1D, "glTexSubImage1D", __VA_ARGS__)
#define glTexSubImage2D(...) opengl_debug_wrap(glTexSubImage2D, "glTexSubImage2D", __VA_ARGS__)
#define glCopyTexImage1D(...) opengl_debug_wrap(glCopyTexImage1D, "glCopyTexImage1D", __VA_ARGS__)
#define glCopyTexImage2D(...) opengl_debug_wrap(glCopyTexImage2D, "glCopyTexImage2D", __VA_ARGS__)
#define glCopyTexSubImage1D(...) opengl_debug_wrap(glCopyTexSubImage1D, "glCopyTexSubImage1D", __VA_ARGS__)
#define glCopyTexSubImage2D(...) opengl_debug_wrap(glCopyTexSubImage2D, "glCopyTexSubImage2D", __VA_ARGS__)
#define glDrawRangeElements(...) opengl_debug_wrap(glDrawRangeElements, "glDrawRangeElements", __VA_ARGS__)
#define glTexImage3D(...) opengl_debug_wrap(glTexImage3D, "glTexImage3D", __VA_ARGS__)
#define glTexSubImage3D(...) opengl_debug_wrap(glTexSubImage3D, "glTexSubImage3D", __VA_ARGS__)
#define glCopyTexSubImage3D(...) opengl_debug_wrap(glCopyTexSubImage3D, "glCopyTexSubImage3D", __VA_ARGS__)
#define glBlendEquation(...) opengl_debug_wrap(glBlendEquation, "glBlendEquation", __VA_ARGS__)
#define glBlendColor(...) opengl_debug_wrap(glBlendColor, "glBlendColor", __VA_ARGS__)
#define glActiveTexture(...) opengl_debug_wrap(glActiveTexture, "glActiveTexture", __VA_ARGS__)
#endif

#endif
