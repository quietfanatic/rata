#ifndef HAVE_HACC_HACCABLE_STANDARD_H
#define HAVE_HACC_HACCABLE_STANDARD_H

#include <tuple>
#include <memory>
#include "hacc/inc/haccable.h"

 // This contains some predefined Haccables for builtin types:
 //  - C pointers
 //  - C arrays
 //  - std::vector
 //  - hacc::named_vector (thin wrapper around std::vector where each object has a .name)
 //  - std::unordered_map
 //  - std::pair
 // You can override individual types if you want

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

HACCABLE_TEMPLATE(<class C>, std::unique_ptr<C>) {
    using namespace hacc;
    name([](){
        return "std::unique_ptr<" + Type::CppType<C>().name() + ">";
    });
    keys(hcb::template mixed_funcs<std::vector<String>>(
        [](const std::unique_ptr<C>& v){
            if (!v) return std::vector<String>();
            return std::vector<String>(1, Type(typeid(*v)).name());
        },
        [](std::unique_ptr<C>& v, const std::vector<String>& keys){
            if (keys.size() == 0) {
                v.reset(null);
            }
            else if (keys.size() == 1) {
                Type type (keys[0]);
                Dynamic dyn = Dynamic(type);
                v.reset(dyn.address());
                dyn.addr = null;
            }
            else {
                throw X::Logic_Error("A std::unique_ptr must have one key representing its type or be empty");
            }
        }
    ));
    attrs([](std::unique_ptr<C>& v, String name)->Reference{
        if (!v) throw X::No_Attrs(&v, name);
        if (name == Type(typeid(*v)).name())
            return Pointer(Type(typeid(*v)), &*v);
        else return Reference(Type(typeid(*v)), &*v).attr(name);
    });
    elems([](std::unique_ptr<C>& v, size_t index)->Reference{
        if (!v) throw X::No_Elems(&v, index);
        return Reference(Type(typeid(*v)), &*v).elem(index);
    });
}

 // C++ variadic templates are powerful but very clunky.
template <class... Es>
struct Hacc_TypeDecl<std::tuple<Es...>> : hacc::Haccability<std::tuple<Es...>> {
    using hcb = hacc::Haccability<std::tuple<Es...>>;
    static void describe ();
};

namespace {
    std::string _hcb_concat () { return ""; }
    template <class F, class... Args>
    std::string _hcb_concat (F f, Args... args) {
        return f + ", " + _hcb_concat(args...);
    }
    template <size_t i, class... Es>
    struct _HCB_Elemizer {
        using hcb = hacc::Haccability<std::tuple<Es...>>;
        using tup = std::tuple<Es...>;
        template <size_t ei> using elem = typename std::tuple_element<ei, tup>::type;
        template <size_t ei> using func = elem<i-1>& (tup&);
        static void elemize () {
            _HCB_Elemizer<i-1, Es...>::elemize();
            hcb::elem(hcb::template ref_func<elem<i-1>>((func<i-1>*)&std::get<i-1, Es...>).optional());
        }
    };
    template <class... Es>
    struct _HCB_Elemizer<0, Es...> {
        static void elemize () { }
    };
}
template <class... Es> void Hacc_TypeDecl<std::tuple<Es...>>::describe () {
    hcb::name([](){ return "std::tuple<" + _hcb_concat(hacc::Type::CppType<Es>().name()...) + ">"; });
    hcb::array();
    _HCB_Elemizer<sizeof...(Es), Es...>::elemize();
}
 // BUT IT WORKS!

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
