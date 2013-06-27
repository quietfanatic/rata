#ifndef HAVE_HACC_HACC_H
#define HAVE_HACC_HACC_H

#include <stdint.h>
#include <string>
#include <sstream>
#include <vector>
#include <typeinfo>
#include <gc/gc_cpp.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef decltype(nullptr) Null;
constexpr Null null = nullptr;

namespace hacc {

    struct Hacc;

    enum Form {
        UNDEFINED,
        NULLFORM,
        BOOL,
        INTEGER,
        FLOAT,
        DOUBLE,
        STRING,
        ARRAY,
        OBJECT,
        PATH,
        ERROR
    };
    std::string form_name (Form);
    
    typedef std::string String;
    typedef std::vector<Hacc*> Array;
    typedef std::pair<std::string, Hacc*> Pair;
    typedef std::vector<Pair> Object;

    struct Error : std::exception {
        String mess;
        String filename;
        uint line;
        uint col;
        mutable std::string longmess;
        Error (String mess = "", String filename = "", uint line = 0, uint col = 0) :
            mess(mess), filename(filename), line(line), col(col)
        { }

         // std::exception
        const char* what () const noexcept {
            if (longmess.empty()) {
                std::stringstream ss;
                ss << mess;
                if (!filename.empty()) {
                    if (line) {
                        ss << " at " << filename << " " << line << ":" << col;
                    }
                    else {
                        ss << " while processing " << filename;
                    }
                }
                else if (line) {
                    ss << " at " << line << ":" << col;
                }
                longmess = ss.str();
            }
            return longmess.c_str();
        }

    };

    enum PathType {
        TOP,
        FILE,
        ATTR,
        ELEM
    };

    struct Path : gc {
        PathType type;
        Path* target;
        std::string s;
        size_t i;

        Path() : type(TOP) { }
        Path(std::string doc) : type(FILE), s(doc) { };
        Path(Path* target, std::string key) : type(ATTR), target(target), s(key) { };
        Path(Path* target, size_t index) : type(ELEM), target(target), i(index) { };
        std::string root ();

        bool operator == (const Path& o) const {
            if (type != o.type) return false;
            switch (type) {
                case TOP: return true;
                case FILE: return s == o.s;
                case ATTR: return s == o.s && *target == *o.target;
                case ELEM: return i == o.i && *target == *o.target;
                default: throw Error("Corrupted Path");
            }
        }
    };

     // The type itself
    struct Hacc : gc {
        Form form;
        union {
            bool b;
            int64 i;
            float f;
            double d;
            String s;
            Array* a;
            Object* o;
            Path* p;
            Error* error;
        };
        Hacc (Null n = null) : form(NULLFORM) { }
        Hacc (bool b) : form(BOOL), b(b) { }
        Hacc (int8 i) : form(INTEGER), i(i) { }
        Hacc (int16 i) : form(INTEGER), i(i) { }
        Hacc (int32 i) : form(INTEGER), i(i) { }
        Hacc (int64 i) : form(INTEGER), i(i) { }
        Hacc (uint8 i) : form(INTEGER), i(i) { }
        Hacc (uint16 i) : form(INTEGER), i(i) { }
        Hacc (uint32 i) : form(INTEGER), i(i) { }
        Hacc (uint64 i) : form(INTEGER), i(i) { }
        Hacc (float f) : form(FLOAT), f(f) { }
        Hacc (double d) : form(DOUBLE), d(d) { }
        Hacc (std::string s) : form(STRING), s(s) { }
        Hacc (const Array& a) : form(ARRAY), a(new Array (a)) { }
        Hacc (Array&& a) : form(ARRAY), a(new Array (a)) { }
        Hacc (const Object& o) : form(OBJECT), o(new Object (o)) { }
        Hacc (Object&& o) : form(OBJECT), o(new Object (o)) { }
        Hacc (Path* p) : form(PATH), p(p) { }
        Hacc (const Error& e) : form(ERROR), error(new Error (e)) { }
        Hacc (Error&& e) : form(ERROR), error(new Error (e)) { }
        ~Hacc ();
        float get_float () const;
        double get_double () const;
    };

}

#endif
