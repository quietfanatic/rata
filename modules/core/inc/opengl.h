#ifndef HAVE_CORE_OPENGL_H
#define HAVE_CORE_OPENGL_H

#include <stdio.h>
#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <GL/glfw.h>

namespace core {

    GLenum diagnose_opengl (std::string when = "");

    struct Shader {
        uint glid = 0;
        GLenum type;
        std::string source;
    
        void compile ();
        ~Shader ();
    };

    struct Program {
        static Program* current;
        static size_t current_attr_count;
        std::string name = "<Anonymous Program>";
        uint glid = 0;
        std::vector<Shader*> shaders;
        std::unordered_map<std::string, uint> attributes;

        int require_uniform (const char* name);
        void link ();
        ~Program ();
         // Supercall if you override these
        virtual void Program_begin ();
        virtual void Program_end ();
        void use ();
        static void unuse ();
    };

}

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

#endif
