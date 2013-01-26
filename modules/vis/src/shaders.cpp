#include "GL/gl.h"
#include "GL/glfw.h"
#include "../../hacc/inc/everything.h"
#include "../inc/shaders.h"

namespace vis {

    static GLenum diagnose_opengl (std::string s = "") {
        GLenum err = glGetError();
        if (err)
            fprintf(stderr, "OpenGL error %04x %s\n", err, s.c_str());
        return err;
    }
    
    void Shader::reload () {

        static auto glCreateShader = glproc<GLuint (GLenum)>("glCreateShader");
        static auto glShaderSource = glproc<void (GLuint, GLsizei, const GLchar**, const GLint*)>("glShaderSource");
        static auto glCompileShader = glproc<void (GLuint)>("glCompileShader");
        static auto glGetShaderiv = glproc<void (GLuint, GLenum, GLint*)>("glGetShaderiv");
        static auto glGetShaderInfoLog = glproc<void (GLuint, GLsizei, GLsizei*, GLchar*)>("glGetShaderInfoLog");
        static auto glDeleteShader = glproc<void (GLuint)>("glDeleteShader");

        std::string ext = name.substr(name.length() - 5);
        GLuint newid;
        if (ext == ".vert")
            newid = glCreateShader(GL_VERTEX_SHADER);
        else if (ext == ".frag")
            newid = glCreateShader(GL_FRAGMENT_SHADER);
        else throw std::logic_error("Filename for shader doesn't end in .vert or .frag");
        std::string src = hacc::string_from_file(name);
        if (!newid) {
            diagnose_opengl("after glCreateShader");
            throw std::logic_error("Look up the above GL error code.");
        }
        try {
            const char* srcp = src.c_str();
            int srclen = src.length();
            glShaderSource(newid, 1, &srcp, &srclen);
            glCompileShader(newid);
            GLint status; glGetShaderiv(newid, GL_COMPILE_STATUS, &status);
            GLint loglen; glGetShaderiv(newid, GL_INFO_LOG_LENGTH, &loglen);
            diagnose_opengl("after glCompileShader");
            if (!status || loglen > 1) {
                char log [loglen];
                glGetShaderInfoLog(newid, loglen, NULL, log);
                fprintf(stderr, "Shader info log for %s:\n", name.c_str());
                fputs(log, stderr);
                if (!status) throw std::logic_error("Failed to compile GL shader.");
            }
            if (diagnose_opengl())
                throw std::logic_error("Look up the above GL error code.");
             // Everything is successful.
            if (glid) glDeleteShader(glid);
            glid = newid;
        } catch (std::exception& e) {
            glDeleteShader(newid);
        }
    }

    Shader::Shader (std::string name) : Resource(name) { reload(); }
    Shader::~Shader () {
        static auto glDeleteShader = glproc<void (GLuint)>("glDeleteShader");
        if (glid) glDeleteShader(glid);
    }
    static Program* currently_using = NULL;

    void Program::use () {
        static auto glUseProgram = glproc<void (GLuint)>("glUseProgram");
        if (currently_using != this) {
            glUseProgram(glid);
            currently_using = this;
        }
    }

    void Program::link () {

        static auto glCreateProgram = glproc<GLuint ()>("glCreateProgram");
        static auto glAttachShader = glproc<void (GLuint, GLuint)>("glAttachShader");
        static auto glLinkProgram = glproc<void (GLuint)>("glLinkProgram");
        static auto glGetProgramiv = glproc<void (GLuint, GLenum, GLint*)>("glGetProgramiv");
        static auto glGetProgramInfoLog = glproc<void (GLuint, GLsizei, GLsizei*, GLchar*)>("glGetProgramInfoLog");
        //static auto glUseProgram = glproc<void (GLuint)>("glUseProgram");
        static auto glDeleteProgram = glproc<void (GLuint)>("glDeleteProgram");

        GLuint newid = glCreateProgram();
        if (!newid) {
            diagnose_opengl("after glCreateProgram()");
            throw std::logic_error("Look up the above GL error code.");
        }
        try {
            for (Shader* s : shaders) {
                glAttachShader(newid, s->glid);
                if (diagnose_opengl("after attaching shader " + s->name)) {
                    throw std::logic_error("Look up the above GL error code.");
                }
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
        } catch (std::exception& e) {
            glDeleteProgram(newid);
        }
    }

    Program::~Program () {
        static auto glDeleteProgram = glproc<void (GLuint)>("glDeleteProgram");
        if (glid) glDeleteProgram(glid);
    }

    int Program::require_uniform (const char* uni) {
        static auto glGetUniformLocation = glproc<GLint (GLuint, const GLchar*)>("glGetUniformLocation");
        int r = glGetUniformLocation(glid, uni);
        if (r == -1) {
            fprintf(stderr, "Program %s has no uniform named %s\n", name.c_str(), uni);
            throw std::logic_error("Uniform not found.");
        }
        return r;
    }

}

using namespace vis;

static ResourceGroup shaders;
HCB_BEGIN(Shader)
    type_name("vis::Shader");
    resource_haccability<Shader, &shaders>();
HCB_END(Shader)

HCB_BEGIN(Program)
    type_name("vis::Program");
    attr("name", member(&Program::name, def(std::string("anonymous program"))));
    attr("shaders", member(&Program::shaders));
    finish([](Program& p){ p.link(); });
HCB_END(Program)

