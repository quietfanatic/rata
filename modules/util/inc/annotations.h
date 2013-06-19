#ifndef HAVE_UTIL_ANNOTATIONS_H
#define HAVE_UTIL_ANNOTATIONS_H

#include <typeinfo>
#include <unordered_map>

namespace util {

    void set_annotation (const std::type_info&, const std::type_info&, void*);
    template <class A>
    void set_annotation (const std::type_info& target, const A& a) {
        set_annotation(target, typeid(A), new A (a));
    }
    template <class A>
    void set_annotation (const std::type_info& target, A&& a) {
        set_annotation(target, typeid(A), new A (a));
    }
    template <class Target, class A>
    void set_annotation (const A& a) {
        set_annotation(typeid(Target), typeid(A), new A (a));
    }
    template <class Target, class A>
    void set_annotation (A&& a) {
        set_annotation(typeid(Target), typeid(A), new A (a));
    }

    void* get_annotation (const std::type_info&, const std::type_info&);
    template <class A>
    A* get_annotation (const std::type_info& target) {
        return (A*)get_annotation(target, typeid(A));
    }
    template <class Target, class A>
    A* get_annotation () {
        return (A*)get_annotation(typeid(Target), typeid(A));
    }

    template <class A> void* _new () { return new A; }

    void* annotation (const std::type_info&, const std::type_info&, void* (* def ) ());
    template <class A>
    A& annotation (const std::type_info& target) {
        return *(A*)annotation(target, typeid(A), _new<A>);
    }
    template <class Target, class A>
    A& annotation () {
        return *(A*)annotation(typeid(Target), typeid(A), _new<A>);
    }

    template <class Target>
    struct Annotation {
        template <class A>
        Annotation (const A& a) {
            set_annotation(typeid(Target), typeid(A), (void*)&a);
        }
        template <class A>
        Annotation (A&& a) {
            set_annotation(typeid(Target), typeid(A), (void*)&a);
        }
    };


}

#endif
