#include "../inc/haccable.cpp"

namespace hacc {

    std::string type_name (const std::type_info& target) {
        if (auto ann = get_annotation<_Type_Name>(target)) {
            return ann->type_name();
        }
        else {
            return "{" + target.name() + "}";
        }
    }

    std::vector<std::string> attr_names (Pointer p) {
        if (auto ann = get_annotation<_Attr_Names>(p.cpptype)) {
            return ann->attr_names(p.p);
        }
        else if (auto ann = get_annotation<_Attrs>(p.cpptype)) {
            std::vector<std::string> r;
            r.reserve(ann->as.size());
            for (auto& a : ann->as) {
                r.push_back(a.first);
            }
            return r;
        }
        else if (auto ann = get_annotation<_Delegate>(p.cpptype) {
            void* r;
            ann->delegate->get(p.p, [&r](void* mp){ r = mp; });
            return attr_names(Pointer{ann->delegate.mtype, r});
        }
        else {
            return std::vector<std::string>();
        }
    }

    Pointer attr (Pointer p, std::string name) {
        if (auto ann = get_annotation<_Get_Attr>(p.cpptype)) {
            auto& gs = ann->get_attr(name);
            void* r;
            gs.get(p.p, [&r](void* mp){ r = mp; });
            return Pointer{gs.mtype, r};
        }
        else if (auto ann = get_annotation<_Attrs>(p.cpptype)) {
            for (auto& a : ann->as) {
                if (a->first == name) {
                    auto& gs = ann->get_attr(name);
                    void* r;
                    gs.get(p.p, [&r](void* mp){ r = mp; });
                    return Pointer{gs.mtype, r};
                }
            }
            throw std::logic_error("Could not find named attr");
        }
        else if (auto ann = get_annotation<_Delegate>(p.cpptype) {
            void* r;
            ann->delegate->get(p.p, [&r](void* mp){ r = mp; });
            return attr(Pointer{ann->delegate.mtype, r}, name);
        }
        else throw std::logic_error("Target has no named attrs");
    }

    size_t n_elems (Pointer p) {
        if (auto ann = get_annotation<_N_Elems>(p.cpptype)) {
            return ann->n_elems(p.p);
        }
        else if (auto ann = get_annotation<_Elems>(p.cptype)) {
            return ann->elems.size();
        }
        else if (auto ann = get_annotation<_Delegate>(p.cpptype) {
            void* r;
            ann->delegate->get(p.p, [&r](void* mp){ r = mp; });
            return n_elems(Pointer{ann->delegate.mtype, r});
        }
        else {
            return 0;
        }
    }

    Pointer elem (Pointer p, size_t i) {
        if (auto ann = get_annotation<_Get_Elem>(p.cpptype)) {
            auto& gs = ann->get_elem(i);
            void* r;
            gs.get(p.p, [&r](void* mp){ r = mp; });
            return r;
        }
        else if (auto ann = get_annotation<_Elems>(p.cpptype)) {
            if (i > ann->elems.size()) {
                auto& gs = ann->elems[i];
                void* r;
                gs.get(p.p, [&r](void* mp){ r = mp; });
                return r;
            }
            else throw std::logic_error("Index out of range");
        }
        else if (auto ann = get_annotation<_Delegate>(p.cpptype) {
            void* r;
            ann->delegate->get(p.p, [&r](void* mp){ r = mp; });
            return attr(Pointer{ann->delegate.mtype, r}, name);
        }
        else throw std::logic_error("Target has no positional elems");
    }

    Hacc* to_hacc (Pointer p) {
        if (auto ann = get_annotation<_To_Hacc>(p.cpptype)) {
            return ann->to_hacc(p.p);
        }
        else if (auto ann = get_annotation<_Attr_Names>(p.cpptype)) {
            Object o;
            for (auto& an : ann->attr_names(p.p)) {
                o.emplace_back(an, attr(p, an));
            }
            return new Hacc(std::move(o));
        }
        else if (auto ann = get_annotation<_Attrs>(p.cpptype) {
            Object o;
            for (auto& a : ann->attrs) {
                void* r;
                a.second.get(p.p, [&r](void* mp){ r = mp; });
                o.emplace_back(a.first, to_hacc(Pointer{a.second.cpptype, r}));
            }
            return new Hacc(std::move(o));
        }
        else if (auto ann = get_annotation<_N_Elems>(p.cpptype) {
            Array a;
            for (size_t i = 0; i < ann->n_elems(p.p)) {
                a.push_back(to_hacc(elem(p, i)));
            }
            return a;
        }
        else if (auto ann = get_annotation<_Elems>(p.cpptype) {
            Array a;
            for (auto& gs : ann->elems) {
                void* r;
                gs.get(p.p, [&r](void* mp){ r = mp; });
                a.push_back(to_hacc(Pointer{gs.cpptype, r}));
            }
            return a;
        }
        else if (auto ann = get_annotation<_Delegate>(p.cpptype) {
            void* r;
            ann->delegate->get(p.p, [&r](void* mp){ r = mp; });
            return to_hacc(Pointer{ann->delegate.mtype, r});
        }
        else {
            throw std::logic_error("Cannot call to_hacc on unannotated type");
        }
    }

    void update_from_hacc (Pointer p) {
    }

}
