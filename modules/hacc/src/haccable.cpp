#include "../inc/haccable.h"

namespace hacc {

    std::string type_name (const std::type_info& t) {
        if (auto tn = (_Type_Name*)util::get_annotation(t, typeid(_Type_Name))) {
            return tn->type_name;
        }
        else return t.name();
    }

    std::vector<std::string> attr_names (Pointer p) {
        if (auto an = (_Attr_Names*)util::get_annotation(p.cpptype, typeid(_Attr_Names))) {
            return an->attr_names(p.p);
        }
        else if (auto as = (_Attrs*)util::get_annotation(p.cpptype, typeid(_Attrs))) {
            std::vector<std::string> r;
            for (auto& a : as) {
                r.emplace_back(a.first);
            }
            return r;
        }
        else return {};
    }

    Pointer get_attr (Pointer p, std::string name) {
        if (auto ga = (_Get_Attr*)util::get_annotation(p.cpptype, typeid(_Get_Attr))) {

            return Pointer(
                ga.get_attr.xtype,
            );
        }
    }

    size_t n_elems (Pointer p) {
        if (auto ne = (_N_Elems*)util::get_annotation(p.cpptype, typeid(_N_Elems))) {
            return ne.n_elems(p.p);
        }
        else return 0;
    }

}
