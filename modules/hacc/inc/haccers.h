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
    virtual void as_pointer (Pointer&) = 0;

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
    template <class C> void as_pointer (C*& p) {
        Pointer pp (p);
        as_pointer(pp);
        p = (C*)pp.addr;
    }

};

struct Haccer::Writer : Haccer {
    uint mode () { return WRITING; }
    Writer* writer () { return this; }

    bool picked = false;
    Hacc hacc;
    Writer (HaccTable* t) :
        hacc(Value(), t->get_hacctype(), "", t->get_flags())
    { }

    void as_null () { if (picked) return; hacc.value.set(null); picked = true; }
    void as_bool (Bool& b) { if (picked) return; hacc.value.set(b); picked = true; }
    void as_integer_s (uint size, void* p) {
        if (picked) return;
        switch (size) {
            case 1: hacc.value.set(*(int8*)p); break;
            case 2: hacc.value.set(*(int16*)p); break;
            case 3: hacc.value.set(*(int32*)p); break;
            case 4: hacc.value.set(*(int64*)p); break;
            default: break;
        }
    }
    void as_float (Float& f) { if (picked) return; hacc.value.set(f); picked = true; }
    void as_double (Double& d) { if (picked) return; hacc.value.set(d); picked = true; }
    void as_string (String& s) { if (picked) return; hacc.value.set(s); picked = true; }
    void as_pointer (Pointer& p) { if (picked) return; hacc.value.set(p); picked = true; }

    bool elems_left () { return false; }

    template <class C> void elem (C& v) {
        if (!picked) { hacc.value.set(Array()); picked = true; }
        if (hacc.value.form == ARRAY) {
            HaccTable* t = Haccable<C>::get_table();
            if (!t) throw Error("No Haccable was defined for type <mangled: " + String(typeid(C).name()) + ">.");
            Writer w (Haccable<C>::get_table());
            t->describe(w, (void*)&v);
            hacc.add_elem(std::move(w.hacc));
        }
    }
    template <class C> void elem (C& v, C def) { elem(v); }

    template <class C> void attr (String name, C& v) {
        if (!picked) { hacc.value.set(Object()); picked = true; }
        if (hacc.value.form == OBJECT) {
            HaccTable* t = Haccable<C>::get_table();
            if (!t) throw Error("No Haccable was defined for type <mangled: " + String(typeid(C).name()) + ">.");
            Writer w (Haccable<C>::get_table());
            t->describe(w, (void*)&v);
            hacc.add_attr(name, std::move(w.hacc));
        }
    }
    template <class C> void attr (String name, C& v, C def) { attr(name, v); }
    
};

struct Haccer::Validator : Haccer {
    uint mode () { return VALIDATING; }
    Validator* validator () { return this; }

    HaccTable* table;
    const Hacc& hacc;
    ID_Map& id_situation;
    uint accepts = 0;
    uint n_elems = 0;
    Validator (HaccTable* table, const Hacc& hacc, ID_Map& id_sitch) :
        table(table), hacc(hacc), id_situation(id_sitch)
    {
        if (!hacc.id.empty()) {
            String type = hacc.type.empty() ? table->get_hacctype() : hacc.type;
            if (type.empty()) throw Error("Sorry, types without hacctypes cannot currently have haccids either.");
            auto res = id_situation.emplace(table->get_hacctype() + String(1, '\0') + hacc.id, (void*)-1);
            if (!res.second) {
                if (res.first->second) {
                    throw Error("Multiple declarations of id '" + hacc.id + "' for type '" + table->get_hacctype());
                }
                res.first->second = (void*)-1;
            }
        }
    }

    void finish () {
        if (!(accepts & 1<<hacc.value.form)) {
            throw Error("This type does not accept " + String(form_name(hacc.value.form)) + ".");
        }
        if (elems_left()) {
            throw Error("Array has too many elements for this type.");
        }
    }

    void as_null () { accepts |= 1 << NULLFORM; }
    void as_bool (Bool& b) { accepts |= 1 << BOOL; }
    void as_integer_s (uint s, void* p) { accepts |= 1 << INTEGER; }
    void as_float (Float& f) { accepts |= 1 << FLOAT | 1 << DOUBLE | 1 << INTEGER; }
    void as_double (Double& d) { accepts |= 1 << DOUBLE | 1 << FLOAT | 1 << INTEGER; }
    void as_string (String& d) { accepts |= 1 << STRING; }
    void as_pointer (Pointer& p) {
        accepts |= 1 << POINTER;
        if (p.type.empty()) throw Error("Cannot write a Pointer that contains no #type.");
        HaccTable* ptdtable = HaccTable::by_hacctype(p.type);
        if (!ptdtable) throw Error("Referenced type '" + p.type + "' has not been registered.");
        id_situation.emplace(p.type + String(1, '\0') + p.id, null);
         // If this id already exists, that's ok.
    }

    template <class C> void elem (C& v) {
        accepts |= 1 << ARRAY;
        if (hacc.value.form == ARRAY) {
            if (hacc.value.a.size() > n_elems) {
                HaccTable* t = Haccable<C>::get_table();
                if (!t) throw Error("No Haccable was defined for type <mangled: " + String(typeid(C).name()) + ">.");
                Validator va (t, hacc.get_elem(n_elems), id_situation);
                t->describe(va, (void*)&v);
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
        if (hacc.value.form == ARRAY) {
            if (hacc.value.a.size() > n_elems) {
                HaccTable* t = Haccable<C>::get_table();
                if (!t) throw Error("No Haccable was defined for type <mangled: " + String(typeid(C).name()) + ">.");
                Validator va (t, hacc.get_elem(n_elems), id_situation);
                t->describe(va, (void*)&v);
                va.finish();
            }
            n_elems++;
        }
    }

    bool elems_left () { return hacc.value.form == ARRAY && hacc.value.a.size() > n_elems; }

    template <class C> void attr (String name, const C& v) {
        accepts |= 1 << OBJECT;
        if (hacc.value.form == OBJECT) {
            HaccTable* t = Haccable<C>::get_table();
            if (!t) throw Error("No Haccable was defined for type <mangled: " + String(typeid(C).name()) + ">.");
            if (auto att = hacc.get_attr(name)) {
                Validator va (t, hacc.get_attr(name), id_situation);
                t->describe(va, (void*)&v);
                va.finish();
            }
            else {
                throw Error("Object does not have required attribute '" + name + "' for type " + t->get_hacctype());
            }
        }
    }
    template <class C> void attr (String name, C& v, C def) {
        accepts |= 1 << OBJECT;
        if (hacc.value.form == OBJECT) {
            const Hacc& att = hacc.get_attr(name);
            if (att.defined()) {
                HaccTable* t = Haccable<C>::get_table();
                if (!t) throw Error("No Haccable was defined for type <mangled: " + String(typeid(C).name()) + ">.");
                Validator va (t, hacc.get_attr(name), id_situation);
                t->describe(va, (void*)&v);
                va.finish();
            }
        }
    }
};

struct Haccer::Reader : Haccer {
    uint mode () { return READING; }
    Reader* reader () { return this; }

    HaccTable* table;
    const Hacc& hacc;
    ID_Map& id_situation;
    void* p;
    uint elem_i = 0;
    Reader (HaccTable* table, const Hacc& hacc, ID_Map& id_sitch, void* p) :
        table(table), hacc(hacc), id_situation(id_sitch), p(p)
    { }

    void finish () {
//        table->register(p);
  //      table->register_type(p, hacc.type);
    //    table->register_id(p, hacc.id);
         // Using this form because we want to either create or overwrite.
        if (!hacc.id.empty())
            id_situation[hacc.id] = p;
    }

    void as_null () { }
    void as_bool (Bool& b) { if (hacc.value.form == BOOL) b = hacc.value.b; }
    void as_integer_s (uint size, void* p) {
        if (hacc.value.form != INTEGER) return;
        switch (size) {
            case 1: { *(int8*)p = hacc.value.i; break; }
            case 2: { *(int16*)p = hacc.value.i; break; }
            case 3: { *(int32*)p = hacc.value.i; break; }
            case 4: { *(int64*)p = hacc.value.i; break; }
            default: break;
        }
    }
    void as_float (Float& f) {
        if (hacc.value.form & (FLOAT|DOUBLE|INTEGER)) f = hacc.get_float();
    }
    void as_double (Double& d) {
        if (hacc.value.form & (FLOAT|DOUBLE|INTEGER)) d = hacc.get_double();
    }
    void as_string (String& s) { if (hacc.value.form == STRING) s = hacc.value.s; }
    void as_pointer (Pointer& p) {
         // Actually...we aren't doing anything at all right here.
    }

    template <class C> void elem (C& v) {
        if (hacc.value.form != ARRAY) return;
        if (elem_i < hacc.value.a.size()) {
            HaccTable* t = Haccable<C>::get_table();
            if (!t) throw Error("No Haccable was defined for type <mangled: " + String(typeid(C).name()) + ">.");
            Reader reader (t, hacc.get_elem(elem_i), id_situation, (void*)&v);
            t->describe(reader, (void*)&v);
            reader.finish();
        }
         // else something's fishy, but let's try not to leave the program
         // in an inconsistent state by throwing an exception.
        elem_i++;
    }
    template <class C> void elem (C& v, C def) {
        if (hacc.value.form != ARRAY) return;
        if (elem_i < hacc.value.a.size()) {
            HaccTable* t = Haccable<C>::get_table();
            if (!t) throw Error("No Haccable was defined for type <mangled: " + String(typeid(C).name()) + ">.");
            Reader reader (t, hacc.get_elem(elem_i), id_situation, (void*)&v);
            t->describe(reader, (void*)&v);
            reader.finish();
        }
        else {
            v = def;
        }
        elem_i++;
    }

    bool elems_left () { return hacc.value.form == ARRAY && elem_i < hacc.value.a.size(); }

    template <class C> void attr (String name, C& v) {
        if (hacc.value.form != OBJECT) return;
        HaccTable* t = Haccable<C>::get_table();
        if (!t) throw Error("No Haccable was defined for type <mangled: " + String(typeid(C).name()) + ">.");
        Reader reader (t, hacc.get_attr(name), id_situation, (void*)&v);
        t->describe(reader, (void*)&v);
        reader.finish();
    }
    template <class C> void attr (String name, C& v, C def) {
        if (hacc.value.form != OBJECT) return;
        if (hacc.has_attr(name)) {
            HaccTable* t = Haccable<C>::get_table();
            if (!t) throw Error("No Haccable was defined for type <mangled: " + String(typeid(C).name()) + ">.");
            Reader reader (t, hacc.get_attr(name), id_situation, (void*)&v);
            t->describe(reader, (void*)&v);
            reader.finish();
        }
        else {
            v = def;
        }
    }
};

struct Haccer::Finisher : Haccer {
    uint mode () { return FINISHING; }
    Finisher* finisher () { return this; }

    HaccTable* table;
    const Hacc& hacc;
    ID_Map& id_situation;
    uint elem_i = 0;
    Finisher (HaccTable* table, const Hacc& hacc, ID_Map& id_sitch) :
        table(table), hacc(hacc), id_situation(id_sitch)
    { }
    void finish () { }

    void as_null () { }
    void as_bool (Bool& b) { }
    void as_integer_s (uint size, void* p) { }
    void as_float (Float& f) { }
    void as_double (Double& d) { }
    void as_string (String& s) { }
    void as_pointer (Pointer& p) {
        if (hacc.value.form != POINTER) return;
        auto iter = id_situation.find(hacc.value.p.type + String(1, '\0') + hacc.value.p.id);
        if (iter != id_situation.end() && (long)iter->second != (long)-1) {
            p.addr = iter->second;
        }
        else {
            throw Error("Hmm, it seems that " + table->get_hacctype() + "'s description didn't provide the same Pointer during both validate and finish phases.");
        }
    }

    template <class C> void elem (C& v) {
        if (hacc.value.form != ARRAY) return;
        HaccTable* t = Haccable<C>::get_table();
        if (!t) throw Error("No Haccable was defined for type <mangled: " + String(typeid(C).name()) + ">.");
        Finisher finisher (t, hacc.get_elem(elem_i), id_situation);
        t->describe(finisher, (void*)&v);
        finisher.finish();
        elem_i++;
    }
    template <class C> void elem (C& v, C def) { elem(v); }

    bool elems_left () { return hacc.value.form == ARRAY && elem_i < hacc.value.a.size(); }

    template <class C> void attr (String name, C& v) {
        if (hacc.value.form != OBJECT) return;
        HaccTable* t = Haccable<C>::get_table();
        if (!t) throw Error("No Haccable was defined for type <mangled: " + String(typeid(C).name()) + ">.");
        Finisher finisher (t, hacc.get_attr(name), id_situation);
        t->describe(finisher, (void*)&v);
        finisher.finish();
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
