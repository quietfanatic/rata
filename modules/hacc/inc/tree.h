#ifndef HAVE_HACC_TREE_H
#define HAVE_HACC_TREE_H

#include "common.h"

namespace hacc {

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
        PATH
    };
    std::string form_name (Form);
    
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
        ~Tree ();
        bool get_bool () const;
        int64 get_integer () const;
        float get_float () const;
        double get_double () const;
        String get_string () const;
    };

    namespace X {
        struct Corrupted_Tree : Corrupted {
            Tree* tree;
            Corrupted_Tree (Tree*);
        };
    }

}

#endif
