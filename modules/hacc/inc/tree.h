#ifndef HAVE_HACC_TREE_H
#define HAVE_HACC_TREE_H

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

    struct Tree;

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
    typedef std::vector<Tree*> Array;
    typedef std::pair<std::string, Tree*> Pair;
    typedef std::vector<Pair> Object;
    
    namespace X { struct Error; }
    struct Path;

    struct Tree : gc {
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
            X::Error* error;
        };
        Tree (Null n = null) : form(NULLFORM) { }
        Tree (bool b) : form(BOOL), b(b) { }
        Tree (int8 i) : form(INTEGER), i(i) { }
        Tree (int16 i) : form(INTEGER), i(i) { }
        Tree (int32 i) : form(INTEGER), i(i) { }
        Tree (int64 i) : form(INTEGER), i(i) { }
        Tree (uint8 i) : form(INTEGER), i(i) { }
        Tree (uint16 i) : form(INTEGER), i(i) { }
        Tree (uint32 i) : form(INTEGER), i(i) { }
        Tree (uint64 i) : form(INTEGER), i(i) { }
        Tree (float f) : form(FLOAT), f(f) { }
        Tree (double d) : form(DOUBLE), d(d) { }
        Tree (std::string s) : form(STRING), s(s) { }
        Tree (const Array& a) : form(ARRAY), a(new Array (a)) { }
        Tree (Array&& a) : form(ARRAY), a(new Array (a)) { }
        Tree (const Object& o) : form(OBJECT), o(new Object (o)) { }
        Tree (Object&& o) : form(OBJECT), o(new Object (o)) { }
        Tree (Path* p) : form(PATH), p(p) { }
        Tree (X::Error* e) : form(ERROR), error(e) { }
        ~Tree ();
        float get_float () const;
        double get_double () const;
    };

    namespace X {
        struct Error : std::exception, gc {
            String mess;
            String filename;
            uint line;
            uint col;
            mutable std::string longmess;
            Error (String mess = "", String filename = "", uint line = 0, uint col = 0) :
                mess(mess), filename(filename), line(line), col(col)
            { }

             // std::exception
            const char* what () const noexcept;
        };
         // A particularly bad kind of error indicating memory corruption
        struct Corrupted : Error {
            Corrupted (String mess) : Error(mess) { }
        };
         // Things like incorrect attribute names, type mismatches
        struct Logic_Error : Error {
            Logic_Error (String mess) : Error(mess) { }
        };

         // Tree-related errors
        struct Corrupted_Tree : Corrupted {
            Tree* tree;
            Corrupted_Tree (Tree*);
        };
    }

}

#endif
