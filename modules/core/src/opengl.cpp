#include "../inc/opengl.h"
#include "../inc/window.h"
#include "../../hacc/inc/everything.h"

namespace core {

    GLenum diagnose_opengl (std::string when) {
        GLenum err = glGetError();
        if (err)
            fprintf(stderr, "OpenGL error %04x %s\n", err, when.c_str());
        return err;
    }

    Program* Program::current = NULL;
    size_t Program::current_attr_count = 0;

    void Program::use () {
        if (current != this) {
            if (current) current->Program_end();
            if (!uses_vaos) {
                glBindVertexArray(0);
                for (uint i = attributes.size(); i < current_attr_count; i++)
                    glDisableVertexAttribArray(i);
                for (uint i = current_attr_count; i < attributes.size(); i++)
                    glEnableVertexAttribArray(i);
                current_attr_count = attributes.size();
            }
            glUseProgram(glid);
            Program_begin();
            current = this;
        }
    }
    void Program::unuse () {
        if (current) {
            current->Program_end();
            if (!current->uses_vaos) {
                for (uint i = 0; i < current_attr_count; i++)
                    glDisableVertexAttribArray(i);
                current_attr_count = 0;
            }
        }
        glUseProgram(0);
        current = NULL;
    }
    void Program::Program_begin () { }  // No default behavior
    void Program::Program_end () { }

    void Shader::compile () {
        GLuint newid = glCreateShader(type);
        if (!newid) {
            diagnose_opengl("after glCreateShader");
            throw std::logic_error("Look up the above GL error code.");
        }
        try {
            const char* srcp = source.c_str();
            int srclen = source.length();
            glShaderSource(newid, 1, &srcp, &srclen);
            diagnose_opengl("after glShaderSource");
            glCompileShader(newid);
            diagnose_opengl("after glCompileShader");
            GLint status; glGetShaderiv(newid, GL_COMPILE_STATUS, &status);
            GLint loglen; glGetShaderiv(newid, GL_INFO_LOG_LENGTH, &loglen);
            if (!status || loglen > 1) {
                char log [loglen];
                glGetShaderInfoLog(newid, loglen, NULL, log);
                fprintf(stderr, "Shader info log:\n");
                fputs(log, stderr);
                if (!status) throw std::logic_error("Failed to compile GL shader.");
            }
            if (diagnose_opengl())
                throw std::logic_error("Look up the above GL error code.");
             // Everything is successful.
            if (glid) glDeleteShader(glid);
            glid = newid;
            source = "";
        } catch (...) {
            glDeleteShader(newid);
            throw;
        }
    }

    Shader::~Shader () {
        if (glid) glDeleteShader(glid);
    }

    void Program::link () {
        for (Shader* s : shaders) {
            if (s->glid == 0)
                s->compile();
        }

        GLuint newid = glCreateProgram();
        if (!newid) {
            diagnose_opengl("after glCreateProgram()");
            throw std::logic_error("Look up the above GL error code.");
        }
        try {
            for (Shader* s : shaders) {
                glAttachShader(newid, s->glid);
                if (diagnose_opengl("after attaching a shader")) {
                    throw std::logic_error("Look up the above GL error code.");
                }
            }
            for (uint i = 0; i < attributes.size(); i++) {
                glBindAttribLocation(newid, i, attributes[i].c_str());
            }
            glLinkProgram(newid);
            GLint status; glGetProgramiv(newid, GL_LINK_STATUS, &status);
            GLint loglen; glGetProgramiv(newid, GL_INFO_LOG_LENGTH, &loglen);
            diagnose_opengl("after linking program");
            if (!status || loglen > 1) {
                char log [loglen];
                glGetProgramInfoLog(newid, loglen, NULL, log);
                fprintf(stderr, "Program info log for %s:\n", name.c_str());
                fputs(log, stderr);
                if (!status) throw std::logic_error("Failed to link GL program");
            }
            if (diagnose_opengl())
                throw std::logic_error("Look up the above GL error code.");
             // Everything is successful.
            if (glid) glDeleteProgram(glid);
            glid = newid;
        } catch (...) {
            glDeleteProgram(newid);
            throw;
        }
    }

    Program::~Program () {
        if (current == this) {
            Program_end();
            current = NULL;
        }
        if (glid) glDeleteProgram(glid);
    }

    int Program::require_uniform (const char* uni) {
        int r = glGetUniformLocation(glid, uni);
        if (r == -1) {
            throw hacc::X::Logic_Error("Program " + name + " has no uniform named " + uni);
        }
        return r;
    }

    int Program::require_attribute (const char* attr) {
        for (uint i = 0; i < attributes.size(); i++) {
            if (attributes[i] == attr)
                return i;
        }
        throw hacc::X::Logic_Error("Program " + name + " has no attribute name " + attr);
    }

} using namespace core;

HACCABLE(Shader) {
    name("core::Shader");
    attr("type", value_funcs<std::string>(
        [](const Shader& s)->std::string{ return s.type == GL_FRAGMENT_SHADER ? "fragment" : "vertex"; },
        [](Shader& s, std::string t){
            if (t == "fragment") s.type = GL_FRAGMENT_SHADER;
            else if (t == "vertex") s.type = GL_VERTEX_SHADER;
            else throw std::logic_error("Unknown shader type\n");
        }
    ));
    attr("source", member(&Shader::source));
}

HACCABLE(Program) {
    name("core::Program");
    attr("name", member(&Program::name).optional());
    attr("shaders", member(&Program::shaders));
    attr("attributes", member(&Program::attributes).optional());
    attr("uses_vaos", member(&Program::uses_vaos).optional());
    finish([](Program& p){ p.link(); });
}

template <class C> using P = C*;
 // This is a lazy-loaded thunk that replaces itself with an OpenGL function
template <class Ret, class... Args>
struct glthunk1 {
    template <Ret(** fp )(Args...), const char** name>
    static Ret glthunk2 (Args... args) {
        if (!window || !window->is_open)
            throw hacc::X::Error("Cannot call OpenGL procedure before window is open: " + std::string(*name));
        void* func = glfwGetProcAddress(*name);
        if (!func)
            throw hacc::X::Error("OpenGL procedure not found: " + std::string(*name));
        *(void**)fp = func;
         // Can't use std::forward, but these are all C types anyway
        return (**fp)(args...);
    }
};
#define OPENGL_THUNK(name, Ret, ...) \
    static const char* name##_name = #name; \
    Ret (* name )(__VA_ARGS__) = &glthunk1<Ret, __VA_ARGS__>::glthunk2<&name, &name##_name>;
#define OPENGL_THUNK0(name, Ret) \
    static const char* name##_name = #name; \
    Ret (* name )() = &glthunk1<Ret>::glthunk2<&name, &name##_name>;

OPENGL_THUNK(glGenBuffers, void, GLsizei, GLuint*)
OPENGL_THUNK(glBindBuffer, void, GLenum, GLuint)
OPENGL_THUNK(glBufferData, void, GLenum, GLsizeiptr, const GLvoid*, GLenum)
OPENGL_THUNK(glDeleteBuffers, void, GLsizei, const GLuint*)

OPENGL_THUNK(glGenVertexArrays, void, GLsizei, GLuint*)
OPENGL_THUNK(glBindVertexArray, void, GLuint)
OPENGL_THUNK(glEnableVertexAttribArray, void, GLuint)
OPENGL_THUNK(glDisableVertexAttribArray, void, GLuint)
OPENGL_THUNK(glVertexAttribPointer, void, GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*)
OPENGL_THUNK(glDeleteVertexArrays, void, GLsizei, const GLuint*)

OPENGL_THUNK(glCreateShader, GLuint, GLenum)
OPENGL_THUNK(glShaderSource, void, GLuint, GLsizei, const GLchar**, const GLint*)
OPENGL_THUNK(glCompileShader, void, GLuint)
OPENGL_THUNK(glGetShaderiv, void, GLuint, GLenum, GLint*)
OPENGL_THUNK(glGetShaderInfoLog, void, GLuint, GLsizei, GLsizei*, GLchar*)
OPENGL_THUNK(glDeleteShader, void, GLuint)

OPENGL_THUNK0(glCreateProgram, GLuint)
OPENGL_THUNK(glAttachShader, GLuint, GLuint, GLuint)
OPENGL_THUNK(glBindAttribLocation, void, GLuint, GLuint, const GLchar*)
OPENGL_THUNK(glLinkProgram, void, GLuint)
OPENGL_THUNK(glGetProgramiv, void, GLuint, GLenum, GLint*)
OPENGL_THUNK(glGetProgramInfoLog, void, GLuint, GLsizei, GLsizei*, GLchar*)
OPENGL_THUNK(glDeleteProgram, void, GLuint)
OPENGL_THUNK(glGetUniformLocation, GLint, GLuint, const GLchar*)

OPENGL_THUNK(glUseProgram, void, GLuint)
OPENGL_THUNK(glUniform1i, void, GLint, GLint)
OPENGL_THUNK(glUniform2i, void, GLint, GLint, GLint)
OPENGL_THUNK(glUniform3i, void, GLint, GLint, GLint, GLint)
OPENGL_THUNK(glUniform4i, void, GLint, GLint, GLint, GLint, GLint)
OPENGL_THUNK(glUniform1f, void, GLint, GLfloat)
OPENGL_THUNK(glUniform2f, void, GLint, GLfloat, GLfloat)
OPENGL_THUNK(glUniform3f, void, GLint, GLfloat, GLfloat, GLfloat)
OPENGL_THUNK(glUniform4f, void, GLint, GLfloat, GLfloat, GLfloat, GLfloat)
