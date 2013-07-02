#include "../inc/haccable.h"
#include "types_internal.h"

namespace hacc {

    void _name (Type t, String name) {
        t.data->name = name;
        types_by_name().emplace(name, t.data);
    }
    void _keys (Type t, GetSet0* gs) {
        t.data->keys = gs;
    }
    void _attrs (Type t, const Func<Reference (void*, String)>& f) {
        t.data->attrs_f = f;
    }
    void _attr (Type t, String name, GetSet0* gs) {
        t.data->attr_list.emplace_back(name, gs);
    }
    void _length (Type t, GetSet0* gs) {
        t.data->length = gs;
    }
    void _elems (Type t, const Func<Reference (void*, size_t)>& f) {
        t.data->elems_f = f;
    }
    void _elem (Type t, GetSet0* gs) {
        t.data->elem_list.push_back(gs);
    }
    void _to_tree (Type t, const Func<Tree* (void*)>& f) {
        t.data->to_tree = f;
    }
    void _prepare (Type t, const Func<void (void*, Tree*)>& f) {
        t.data->prepare = f;
    }
    void _fill (Type t, const Func<void (void*, Tree*)>& f) {
        t.data->fill = f;
    }
    void _finish (Type t, const Func<void (void*, Tree*)>& f) {
        t.data->finish = f;
    }
    void _delegate (Type t, GetSet0* gs) {
        t.data->delegate = gs;
    }
    void _is_raw_pointer (Type t, Type pt) {
        t.data->pointee_type = pt.data;
    }

}

