#ifndef HAVE_HACC_HACCABLE_H
#define HAVE_HACC_HACCABLE_H

#include <unordered_map>
#include "hacc.h"
#include "haccable_getset.h"
#include "../../util/inc/annotations.h"

namespace hacc {

     // USAGE API
    std::string type_name (const std::type_info&);
    template <class C>
    std::string type_name () { return type_name(typeid(C)); }

    std::vector<std::string> attr_names (Pointer);
    Reference attr (Pointer, std::string);
    size_t n_elems (Pointer);
    Reference elem (Pointer, size_t);

     // Probably best to keep these to internal usage.
    Hacc* to_hacc (Pointer);
    void update_from_hacc (Pointer, Hacc*);


     // DECLARATION API

    struct _Attrs {
        std::vector<std::pair<std::string, GetSet0>> attrs;
    };
    template <class C>
    struct Attrs {
        Attrs (std::initializer_list<std::pair<std::string, GetSet1<C>>> as) {
            for (auto& a : as) {
                util::annotation<C, _Attrs>().attrs.emplace(a.first, a.second);
            }
        }
    };

    struct _Elems {
        std::vector<GetSet0> elems;
    };
    template <class C>
    struct Elems {
        Elems (std::initializer_list<GetSet1>.elems.emplace(a.first, a.second) {
            for (auto& e : es) {
                util::annotation<C, _Attrs>().elems.emplace_back(e);
            }
        }
    };

    struct _Get_Attr {
        std::function<GetSet0 (std::string)> get_attr;
    };
    template <class C>
    struct Get_Attr {
        Get_Attr (const std::function<GetSet1<C> (std::string)>& ga) {
            util::annotation<C, _Get_Attr>().get_attr = [ga](std::string n)->GetSet0{
                return ga(n);
            };
        }
    };
    
    struct _Attr_Names {
        std::function<std::vector<std::string> (void*)> attr_names;
    };
    template <class C>
    struct Attr_Names {
        Attr_Names (const std::function<std::vector<std::string> (const C&)>& an) {
            util::annotation<C, _Attr_Names>().attr_names =
                *(std::function<std::vector<std::string> (void*)>*)(an);
        }
    };

    struct _N_Elems {
        std::function<size_t (void*)> n_elems;
    };
    template <class C>
    struct N_Elems {
        N_Elems (const std::function<size_t (const C&)>& ne) {
            util::annotation<C, _Attr_Names>().n_elems =
                *(std::function<size_t (void*)>*)ne;
        }
    };

    struct _Type_Name {
        std::function<std::string ()> type_name;
    };
    template <class C>
    struct Type_Name {
        Type_Name (const std::function<std::string ()>& tn) {
            util::annotation<C, _Type_Name>().type_name = tn;
        }
        Type_Name (std::string tn) {
            util::annotation<C, _Type_Name>().type_name =
                [tn](){ return tn; };
        }
    };

    struct _Delegate {
        GetSet0 delegate;
    };
    template <class C>
    struct Delegate {
        Delegate (GetSet1<C> d) {
            util::annotation<C, _Delegate>().delegate = d;
        }
    };

     // You probably should only use this and From_Hacc with atomic forms.
    struct _To_Hacc {
        std::function<Hacc* (void*)> to_hacc;
    };
    template <class C>
    struct To_Hacc {
        To_Hacc (std::function<Hacc* (const C& c)>) {
            util::annotation<C, _To_Hacc>().to_hacc =
                *(std::function<Hacc* (void*)>*)c;
        }
    };

    struct _Update_From_Hacc {
        std::function<void (void*, Hacc*)> update_from_hacc;
    };
    template <class C>
    struct Update_From_Hacc {
        Update_From_Hacc (std::function<Hacc*

}

#define HACC_APPEND(a, b) a##b
#define HACC_APPEND_COUNTER(a) HACC_APPEND(a, __COUNTER__)
#define __ HACC_APPEND_COUNTER(_anon_)

#endif
