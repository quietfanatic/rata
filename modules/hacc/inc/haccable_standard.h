#ifndef HAVE_HACC_HACCABLE_STANDARD_H
#define HAVE_HACC_HACCABLE_STANDARD_H

 // This contains some predefined Haccables for builtin types:
 //  - C pointers
 //  - C arrays
 //  - std::vector
 //  - hacc::named_vector (thin wrapper around std::vector where each object has a .name)
 //  - std::unordered_map
 //  - std::pair
 // You can override individual types if you want

#include "haccable.h"

namespace hacc {
    template <class C>
    struct named_vector : std::vector<C> {
        C* named (std::string name) {
            for (auto& e : *this)
                if (e.name == name)
                    return &e;
            return NULL;
        }
    };
}

template <class C, size_t n> using CArray = C[n];
 // Heh, this actually works.
HACCABLE_TEMPLATE(<class C HCB_COMMA size_t n>, C[n]) {
    using namespace hacc;
    name([](){
        return Type::CppType<C>().name() + "[" + std::to_string(n) + "]";
    });
    array();
    length(hcb::template value_funcs<size_t>(
        [](const CArray<C, n>&){ return n; },
        [](CArray<C, n>& v, size_t size){
            if (size != n)
                throw hacc::X::Wrong_Size(Pointer(&v), size, n);
        }
    ));
    elems([](CArray<C, n>& v, size_t index){
        if (index <= n)
            return Reference(v + index);
        else
            throw hacc::X::Out_Of_Range(Pointer(&v), index, n);
    });
}

HACCABLE_TEMPLATE(<class C>, std::vector<C>) {
    using namespace hacc;
    name([](){
        return "std::vector<" + Type::CppType<C>().name() + ">";
    });
    array();
    length(hcb::template value_funcs<size_t>(
        [](const std::vector<C>& v){
            return v.size();
        },
        [](std::vector<C>& v, size_t size){
            v.resize(size);
        }
    ));
    elems([](std::vector<C>& v, size_t index){
        return Reference(&v.at(index));
    });
}

HACCABLE_TEMPLATE(<class C>, hacc::named_vector<C>) {
    using namespace hacc;
    name([](){
        return "hacc::named_vector<" + Type::CppType<C>().name() + ">";
    });
    delegate(hcb::template base<std::vector<C>>());
    attrs([](named_vector<C>& v, std::string name){
        C* r = v.named(name);
        if (r) return Reference(r);
        else throw X::No_Attr(Pointer(&v), name);
    });
}

HACCABLE_TEMPLATE(<class C>, std::unordered_map<std::string HCB_COMMA C>) {
    using namespace hacc;
    name([](){
        return "std::unordered_map<std::string, " + Type::CppType<C>().name() + ">";
    });
    keys(hcb::template mixed_funcs<std::vector<std::string>>(
        [](const std::unordered_map<std::string, C>& m){
            std::vector<std::string> r;
            for (auto& p : m)
                r.push_back(p.first);
            return r;
        },
        [](std::unordered_map<std::string, C>& m, const std::vector<std::string>& ks){
            m.clear();
            for (auto k : ks)
                m[k];  // autovivifies
        }
    ));
    attrs([](std::unordered_map<std::string, C>& m, std::string k){
        return Reference(&m.at(k));
    });
}

HACCABLE_TEMPLATE(<class A HCB_COMMA class B>, std::pair<A HCB_COMMA B>) {
    using namespace hacc;
    name([](){
        return "std::pair<" + Type::CppType<A>().name() + ", " + Type::CppType<B>().name() + ">";
    });
    elem(member(&std::pair<A, B>::first));
    elem(member(&std::pair<A, B>::second));
}

 // This is the default haccability for pointers.
HACCABLE_TEMPLATE(<class C>, C*) {
    using namespace hacc;
    name([](){
        return Type::CppType<C>().name() + "*";
    });
    is_raw_pointer(Type::CppType<C>());
}

 // Alternatively, you can override that and call this.

namespace hacc {
    template <class C, class M>
    void hacc_pointer_by_member (M C::* p, std::vector<C*>& all, bool required = false) {
        Haccability<C*>::to_tree([p](C* const& x){
            return x ? Tree(x->*p) : Tree(null);
        });
        Haccability<C*>::fill([p, &all, required](C*& x, Tree t){
            M m;
            Reference(&m).from_tree(t);
            for (auto c : all) {
                if (c->*p == m) {
                    x = c;
                    return;
                }
            }
            x = NULL;
            if (required) throw X::Logic_Error("No " + Type::CppType<C*>().name() + " with the given id was found.");
        });
    }
    template <class C, class M>
    void hacc_pointer_by_method (M (C::* f) () const, std::vector<C*>& all, bool required = false) {
        Haccability<C*>::to_tree([f](C* const& x){
            return x ? Tree((x->*f)()) : Tree(null);
        });
        Haccability<C*>::fill([f, &all, required](C*& x, Tree t){
            M m;
            Reference(&m).from_tree(t);
            for (auto c : all) {
                if ((c->*f)() == m) {
                    x = c;
                    return;
                }
            }
            x = NULL;
            if (required) throw X::Logic_Error("No " + Type::CppType<C*>().name() + " with the given id was found.");
        });
    }
}

#endif
