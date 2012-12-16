#ifndef HAVE_HACC_HACCERS_H
#define HAVE_HACC_HACCERS_H


#include <typeinfo>

#include "hacc.h"

namespace hacc {

struct Haccer {
    enum {
        INERT,
        WRITING,
        VALIDATING,
        READING,
        FINISHING
    };
    struct Writer;
    struct Validator;
    struct Reader;
    struct Finisher;
    
    virtual uint mode () = 0;
    virtual Writer* writer () { return hacc::null; }
    virtual Reader* reader () { return hacc::null; }
    virtual Validator* validator () { return hacc::null; }
    virtual Finisher* finisher () { return hacc::null; }

    virtual void as_null () = 0;
    void as_null (Null&) { as_null(); }  // For consistency
    virtual void as_bool (Bool&) = 0;
    virtual void as_integer_s (uint, void*) = 0;
    virtual void as_float (Float&) = 0;
    virtual void as_double (Double&) = 0;
    virtual void as_string (String&) = 0;

    virtual bool elems_left () = 0;

    template <class C> void elem (C& val);
    template <class C> void elem (C& val, C def);
    template <class C> void attr (String name, C& val);
    template <class C> void attr (String name, C& val, C def);

    void as_integer (char& i) { return as_integer_s(1, &i); }
    void as_integer (int8& i) { return as_integer_s(1, &i); }
    void as_integer (uint8& i) { return as_integer_s(1, &i); }
    void as_integer (int16& i) { return as_integer_s(2, &i); }
    void as_integer (uint16& i) { return as_integer_s(2, &i); }
    void as_integer (int32& i) { return as_integer_s(3, &i); }
    void as_integer (uint32& i) { return as_integer_s(3, &i); }
    void as_integer (int64& i) { return as_integer_s(4, &i); }
    void as_integer (uint64& i) { return as_integer_s(4, &i); }

};

struct Haccer::Writer : Haccer {
    uint mode () { return WRITING; }
    Writer* writer () { return this; }

    bool picked = false;
    Hacc hacc = Hacc(hacc::null);
    Writer () { }

    void as_null () { }
    void as_bool (Bool& b) { if (picked) return; hacc = Hacc(b); picked = true; }
    void as_integer_s (uint size, void* p) {
        if (picked) return;
        switch (size) {
            case 1: hacc = Hacc(*(int8*)p); break;
            case 2: hacc = Hacc(*(int16*)p); break;
            case 3: hacc = Hacc(*(int32*)p); break;
            case 4: hacc = Hacc(*(int64*)p); break;
            default: break;
        }
    }
    void as_float (Float& f) { if (picked) return; hacc = Hacc(f); picked = true; }
    void as_double (Double& d) { if (picked) return; hacc = Hacc(d); picked = true; }
    void as_string (String& s) { if (picked) return; hacc = Hacc(s); picked = true; }

    bool elems_left () { return false; }

    template <class C> void elem (C& v) {
        if (!picked) { hacc = Hacc(Array()); picked = true; }
        if (hacc.valtype() == ARRAY) {
            Writer w;
            run_description(w, v);
            hacc.assume_array().push_back(w.hacc);
        }
    }
    template <class C> void elem (C& v, C def) { elem(v); }

    template <class C> void attr (String name, C& v) {
        if (!picked) { hacc = Hacc(Object()); picked = true; }
        if (hacc.valtype() == OBJECT) {
            Writer w;
            run_description(w, v);
            hacc.assume_object().push_back(Pair<Hacc>(name, w.hacc));
        }
    }
    template <class C> void attr (String name, C& v, C def) { attr(name, v); }
    
};

struct Haccer::Validator : Haccer {
    uint mode () { return VALIDATING; }
    Validator* validator () { return this; }

    Hacc hacc;
    uint accepts = 0;
    uint n_elems = 0;
    Validator (Hacc hacc) : hacc(hacc) { }

    void as_null () { accepts |= 1 << VALNULL; }
    void as_bool (Bool& b) { accepts |= 1 << BOOL; }
    void as_integer_s (uint s, void* p) { accepts |= 1 << INTEGER; }
    void as_float (Float& f) { accepts |= 1 << FLOAT | 1 << DOUBLE | 1 << INTEGER; }
    void as_double (Double& d) { accepts |= 1 << DOUBLE | 1 << FLOAT | 1 << INTEGER; }
    void as_string (String& d) { accepts |= 1 << STRING; }

    template <class C> void elem (C& v) {
        accepts |= 1 << ARRAY;
        if (hacc.valtype() == ARRAY) {
            if (hacc.assume_array().size() > n_elems) {
                Validator va (hacc.assume_array()[n_elems]);
                run_description(va, v);
                va.finish();
                n_elems++;
            }
            else {
                throw Error("Array does not have enough elements for <mangled: " + String(typeid(C).name()) + ">.");
            }
        }
    }
    template <class C> void elem (C& v, C def) {
        accepts |= 1 << ARRAY;
        if (hacc.valtype() == ARRAY) {
            if (hacc.assume_array().size() > n_elems) {
                Validator va (hacc.assume_array()[n_elems]);
                run_description(va, v);
                va.finish();
            }
            n_elems++;
        }
    }

    bool elems_left () { return hacc.valtype() == ARRAY && hacc.assume_array().size() > n_elems; }

    template <class C> void attr (String name, const C& v) {
        accepts |= 1 << OBJECT;
        if (hacc.valtype() == OBJECT) {
            Object& o = hacc.assume_object();
            bool found = false;
            auto it = o.begin();
            for (o.begin(); it != o.end(); it++) {
                if (it->first == name) {
                    found = true;
                    break;
                }
            }
            if (found) {
                Validator va (it->second);
                run_description(va, v);
                va.finish();
            }
            else {
                throw Error("Object does not have required attribute '" + name + "' for <mangled: " + String(typeid(C).name()) + ">.");
            }
        }
    }
    template <class C> void attr (String name, C& v, C def) {
        accepts |= 1 << OBJECT;
        if (hacc.valtype() == OBJECT) {
            if (Hacc* a = hacc.get_attr(name)) {
                Validator va (*a);
                run_description(va, v);
                va.finish();
            }
        }
    }

    void finish () {
        if (!(accepts & 1<<hacc.valtype())) {
            throw Error("This type does not accept " + String(valtype_name(hacc.valtype())) + ".");
        }
        if (elems_left()) {
            throw Error("Array has too many elements for this type.");
        }
    }
};

struct Haccer::Reader : Haccer {
    uint mode () { return READING; }
    Reader* reader () { return this; }

    Hacc hacc;
    uint elem_i = 0;
    Reader (Hacc hacc) : hacc(hacc) { }

    void as_null () { }
    void as_bool (Bool& b) { if (hacc.valtype() == BOOL) b = hacc.assume_bool(); }
    void as_integer_s (uint size, void* p) {
        if (hacc.valtype() != INTEGER) return;
        switch (size) {
            case 1: { *(int8*)p = hacc.assume_integer(); break; }
            case 2: { *(int16*)p = hacc.assume_integer(); break; }
            case 3: { *(int32*)p = hacc.assume_integer(); break; }
            case 4: { *(int64*)p = hacc.assume_integer(); break; }
            default: break;
        }
    }
    void as_float (Float& f) {
        if (hacc.valtype() & (FLOAT|DOUBLE|INTEGER)) f = hacc.get_float();
    }
    void as_double (Double& d) {
        if (hacc.valtype() & (FLOAT|DOUBLE|INTEGER)) d = hacc.get_double();
    }
    void as_string (String& s) { if (hacc.valtype() == STRING) s = hacc.assume_string(); }

    template <class C> void elem (C& v) {
        if (hacc.valtype() != ARRAY) return;
        Array& a = hacc.assume_array();
        if (elem_i < a.size()) {
            run_description(Reader(a[elem_i]), v);
        }
         // else something's fishy, but let's try not to leave the program
         // in an inconsistent state by throwing an exception.
        elem_i++;
    }
    template <class C> void elem (C& v, C def) {
        if (hacc.valtype() != ARRAY) return;
        Array& a = hacc.assume_array();
        if (elem_i < a.size()) {
            Reader reader (a[elem_i]);
            run_description(reader, v);
        }
        else {
            v = def;
        }
        elem_i++;
    }

    bool elems_left () { return hacc.valtype() == ARRAY && elem_i < hacc.assume_array().size(); }

    template <class C> void attr (String name, C& v) {
        if (hacc.valtype() != OBJECT) return;
        if (Hacc* a = hacc.get_attr(name)) {
            Reader reader (*a);
            run_description(reader, v);
        }
        else {
            // err...
        }
    }
    template <class C> void attr (String name, C& v, C def) {
        if (hacc.valtype() != OBJECT) return;
        if (Hacc* a = hacc.get_attr(name)) {
            Reader reader (*a);
            run_description(reader, v);
        }
        else {
            v = def;
        }
    }
};

struct Haccer::Finisher : Haccer {
    uint mode () { return FINISHING; }
    Finisher* finisher () { return this; }

    Hacc hacc;
    uint elem_i = 0;
    Finisher (Hacc hacc) : hacc(hacc) { }

    void as_null () { }
    void as_bool (Bool& b) { }
    void as_integer_s (uint size, void* p) { }
    void as_float (Float& f) { }
    void as_double (Double& d) { }
    void as_string (String& s) { }

    template <class C> void elem (C& v) {
        if (hacc.valtype() != ARRAY) return;
        Array& a = hacc.assume_array();
        if (elem_i < a.size()) {
            Finisher finisher (a[elem_i]);
            run_description(finisher, v);
        }
        elem_i++;
    }
    template <class C> void elem (C& v, C def) { elem(v); }

    bool elems_left () { return hacc.valtype() == ARRAY && elem_i < hacc.assume_array().size(); }

    template <class C> void attr (String name, C& v) {
        if (hacc.valtype() != OBJECT) return;
        if (Hacc* a = hacc.get_attr(name)) {
            Finisher finisher (*a);
            run_description(finisher, v);
        }
    }
    template <class C> void attr (String name, C& v, C def) { attr(name, v); }
};

template <class C> void Haccer::elem (C& val) {
    switch (mode()) {
        case WRITING: writer()->elem(val); break;
        case VALIDATING: validator()->elem(val); break;
        case READING: reader()->elem(val); break;
        case FINISHING: finisher()->elem(val); break;
    }
}
template <class C> void Haccer::elem (C& val, C def) {
    switch (mode()) {
        case WRITING: writer()->elem(val, def); break;
        case VALIDATING: validator()->elem(val, def); break;
        case READING: reader()->elem(val, def); break;
        case FINISHING: finisher()->elem(val, def); break;
    }
}
template <class C> void Haccer::attr (String name, C& val) {
    switch (mode()) {
        case WRITING: writer()->attr(name, val); break;
        case VALIDATING: validator()->attr(name, val); break;
        case READING: reader()->attr(name, val); break;
        case FINISHING: finisher()->attr(name, val); break;
    }
}
template <class C> void Haccer::attr (String name, C& val, C def) {
    switch (mode()) {
        case WRITING: writer()->attr(name, val, def); break;
        case VALIDATING: validator()->attr(name, val, def); break;
        case READING: reader()->attr(name, val, def); break;
        case FINISHING: finisher()->attr(name, val, def); break;
    }
}

}


#endif
