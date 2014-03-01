#include "core/inc/opengl.h"

#ifdef CORE_OPENGL_VERY_DEBUG
    #include <sstream>
#endif
#include <stdexcept>
#include "core/inc/window.h"
#include "hacc/inc/everything.h"
#include "util/inc/debug.h"

namespace core {

    GLenum diagnose_opengl (std::string when) {
        GLenum err = glGetError();
        if (err)
            fprintf(stderr, "OpenGL error %04x %s\n", err, when.c_str());
        return err;
    }

    Program* Program::current = NULL;

    void Program::use () {
        if (current != this) {
            if (current) current->Program_end();
            glUseProgram(glid);
            Program_begin();
            current = this;
        }
    }
    void Program::unuse () {
        if (current) {
            current->Program_end();
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
                glGetShaderInfoLog(newid, loglen, (int*)NULL, log);
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
                glGetProgramInfoLog(newid, loglen, (int*)NULL, log);
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
            unuse();
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
    finish([](Program& p){ p.link(); });
}

