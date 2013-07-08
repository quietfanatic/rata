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

    struct TreeData;
    struct Tree : DPtr<const TreeData> {
        Form form () const;
        explicit Tree (const TreeData* d) : DPtr(d) { }
        explicit Tree (Null n = null);
        explicit Tree (bool b);
        explicit Tree (int64 i);
        explicit Tree (int32 i) : Tree((int64)i) { }
        explicit Tree (int16 i) : Tree((int64)i) { }
        explicit Tree (int8 i) : Tree((int64)i) { }
        explicit Tree (uint64 i) : Tree((int64)i) { }
        explicit Tree (uint32 i) : Tree((int64)i) { }
        explicit Tree (uint16 i) : Tree((int64)i) { }
        explicit Tree (uint8 i) : Tree((int64)i) { }
        explicit Tree (char i) : Tree((int64)i) { }
        explicit Tree (float f);
        explicit Tree (double d);
        explicit Tree (String s);
        explicit Tree (const char* s) : Tree(String(s)) { }
        explicit Tree (const Array& a);
        explicit Tree (Array&& a);
        explicit Tree (const Object& o);
        explicit Tree (Object&& o);
        explicit Tree (Path* p);
        explicit operator Null () const;
        explicit operator bool () const;
        explicit operator int64 () const;
        explicit operator int32 () const { return int64(*this); }
        explicit operator int16 () const { return int64(*this); }
        explicit operator int8 () const { return int64(*this); }
        explicit operator uint64 () const { return int64(*this); }
        explicit operator uint32 () const { return int64(*this); }
        explicit operator uint16 () const { return int64(*this); }
        explicit operator uint8 () const { return int64(*this); }
        explicit operator char () const { return int64(*this); }
        explicit operator float () const;
        explicit operator double () const;
        explicit operator String () const;
        explicit operator const Array& () const;
        explicit operator const Object& () const;
        explicit operator Path* () const;
        Tree elem (size_t index) const;
        Tree attr (String name) const;
         // Implicit conversions can be troublesome, so use this
        template <class C>
        C as () const { return C(*this); }
    };

    bool operator == (const Tree& a, const Tree& b);

    namespace X {
        struct Corrupted_Tree : Corrupted {
            Tree tree;
            Corrupted_Tree (Tree);
        };
        struct Wrong_Form : Logic_Error {
            Form form;
            Tree tree;
            Wrong_Form (Form, Tree);
        };
    }

}

#endif
