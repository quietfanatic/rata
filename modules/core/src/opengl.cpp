#include "../inc/opengl.h"
#include "../../hacc/inc/everything.h"

namespace core {

    void* g_glproc (const char* name) {
        void* r = glfwGetProcAddress(name);
        if (!r) {
            fprintf(stderr, "No GL proc was found named %s\n", name);
            throw std::logic_error("Missing GL procedure.");
        }
        return r;
    }

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

    Renderer::~Renderer () {
         // Not good to call finish_rendering on a deleted renderer :)
        if (current == this) finish_rendering();
        current = NULL;
    }

    void Shader::finish () {

        static auto glCreateShader = glproc<GLuint (GLenum)>("glCreateShader");
        static auto glShaderSource = glproc<void (GLuint, GLsizei, const GLchar**, const GLint*)>("glShaderSource");
        static auto glCompileShader = glproc<void (GLuint)>("glCompileShader");
        static auto glGetShaderiv = glproc<void (GLuint, GLenum, GLint*)>("glGetShaderiv");
        static auto glGetShaderInfoLog = glproc<void (GLuint, GLsizei, GLsizei*, GLchar*)>("glGetShaderInfoLog");
        static auto glDeleteShader = glproc<void (GLuint)>("glDeleteShader");

        GLuint newid = glCreateShader(type);
        if (!newid) {
            diagnose_opengl("after glCreateShader");
            throw std::logic_error("Look up the above GL error code.");
        }
        try {
            const char* srcp = source.c_str();
            int srclen = source.length();
            glShaderSource(newid, 1, &srcp, &srclen);
            glCompileShader(newid);
            GLint status; glGetShaderiv(newid, GL_COMPILE_STATUS, &status);
            GLint loglen; glGetShaderiv(newid, GL_INFO_LOG_LENGTH, &loglen);
            diagnose_opengl("after glCompileShader");
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
        } catch (std::exception& e) {
            glDeleteShader(newid);
        }
    }

    Shader::~Shader () {
        static auto glDeleteShader = glproc<void (GLuint)>("glDeleteShader");
        if (glid) glDeleteShader(glid);
    }

    void Program::link () {

        static auto glCreateProgram = glproc<GLuint ()>("glCreateProgram");
        static auto glAttachShader = glproc<void (GLuint, GLuint)>("glAttachShader");
        static auto glBindAttribLocation = glproc<void (GLuint, GLuint, const GLchar*)>("glBindAttribLocation");
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
                if (diagnose_opengl("after attaching a shader")) {
                    throw std::logic_error("Look up the above GL error code.");
                }
            }
            for (auto& va : attributes) {
                glBindAttribLocation(newid, va.second, va.first.c_str());
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

} using namespace core;

HCB_BEGIN(Shader)
    name("core::Shader");
    attr("type", value_functions<std::string>(
        [](const Shader& s)->std::string{ return s.type == GL_FRAGMENT_SHADER ? "fragment" : "vertex"; },
        [](Shader& s, std::string t){
            if (t == "fragment") s.type = GL_FRAGMENT_SHADER;
            else if (t == "vertex") s.type = GL_VERTEX_SHADER;
            else throw std::logic_error("Unknown shader type\n");
        }
    ));
    attr("source", member(&Shader::source));
    finish([](Shader& s){ s.finish(); });
HCB_END(Shader)

HCB_BEGIN(Program)
    name("core::Program");
    attr("name", member(&Program::name).optional());
    attr("shaders", member(&Program::shaders));
    attr("attributes", member(&Program::attributes).optional());
    finish([](Program& p){ p.link(); });
HCB_END(Program)

