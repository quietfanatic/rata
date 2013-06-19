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
        VAR,
        ADDRESS,
        ASSIGNMENT,
        CONSTRAINT,
        POINTER,
        ATTR,
        ELEM,
        MACRO,
        ERROR
    };
    std::string form_name (Form);
    
    typedef std::string String;
    typedef std::vector<Hacc*> Array;
    typedef std::pair<std::string, Hacc*> Pair;
    typedef std::vector<Pair> Object;

    struct Var {
        String name;
        bool operator == (const Var& o) const { return name == o.name; }
    };
    struct Address {
        Hacc* subject;
    };
    struct Assignment {
        Hacc* left;
        Hacc* right;
    };
    struct Constraint {
        String hacctype;
        Hacc* value;
    };
    struct Pointer {
        std::type_info* cpptype;
        void* p;
    };
    struct Attr {
        Hacc* subject;
        String name;
    };
    struct Elem {
        Hacc* subject;
        size_t index;
    };
    struct Macro {
        String name;
        Array args;
    };

    struct Error : std::exception {
        String mess;
        String filename;
        uint line;
        uint col;
        const char* buf;
        Error (String mess = "", String filename = "", uint line = 0, uint col = 0) :
            mess(mess), filename(filename), line(line), col(col), buf(NULL)
        { }

         // std::exception
        const char* what () {
            if (!buf) {
                std::string s;
                std::stringstream ss (s);
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
                buf = s.c_str();
            }
            return buf;
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
            Var v;
            Address address;
            Assignment assignment;
            Constraint constraint;
            Pointer p;
            Attr attr;
            Elem elem;
            Macro* macro;
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
        Hacc (Var v) : form(VAR), v(v) { }
        Hacc (Address a) : form(ADDRESS), address(a) { }
        Hacc (Assignment a) : form(ASSIGNMENT), assignment(a) { }
        Hacc (Constraint c) : form(CONSTRAINT), constraint(c) { }
        Hacc (Pointer p) : form(POINTER), p(p) { }
        Hacc (Attr a) : form(ATTR), attr(a) { }
        Hacc (Elem e) : form(ELEM), elem(e) { }
        Hacc (const Macro& m) : form(MACRO), macro(new Macro (m)) { }
        Hacc (Macro&& m) : form(MACRO), macro(new Macro (m)) { }
        Hacc (const Error& e) : form(ERROR), error(new Error (e)) { }
        Hacc (Error&& e) : form(ERROR), error(new Error (e)) { }
        ~Hacc ();
        float get_float () const;
        double get_double () const;
    };
}

#endif
