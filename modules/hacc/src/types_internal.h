#ifndef HAVE_HACC_TYPES_INTERNAL_H
#define HAVE_HACC_TYPES_INTERNAL_H

#include <typeindex>
#include <unordered_map>
#include "../inc/types.h"
#include "../inc/dynamicism.h"

namespace hacc {

     // These are in dynamicism.cpp
    std::vector<TypeData*>& types_to_init ();
    std::unordered_map<std::type_index, TypeData*>& types_by_cpptype ();
    std::unordered_map<String, TypeData*>& types_by_name ();

    struct TypeData {
         // General
        const std::type_info* cpptype = null;
        size_t size = -1;
        void (* construct )(void*) = null;
        void (* destruct )(void*) = null;
        void (* copy_construct )(void*, void*) = null;
        Func<String ()> name = null;
        void (* describe )() = null;
         // Hacc-specific
        GetSet0* keys = null;
        Func<Reference (void*, String)> attrs_f = null;
        std::vector<std::pair<String, GetSet0*>> attr_list;
        GetSet0* length = null;
        Func<Reference (void*, size_t)> elems_f = null;
        std::vector<GetSet0*> elem_list;
        Func<Tree* (void*)> to_tree = null;
        Func<void (void*, Tree*)> prepare = null;
        Func<void (void*, Tree*)> fill = null;
        Func<void (void*, Tree*)> finish = null;
        GetSet0* delegate = null;
        TypeData* pointee_type = null;
        bool initialized = false;

        TypeData (
            const std::type_info& cpptype,
            size_t size,
            void (* construct )(void*),
            void (* destruct )(void*),
            void (* copy_construct )(void*, void*)
        ) :
            cpptype(&cpptype), size(size),
            construct(construct), destruct(destruct),
            copy_construct(copy_construct)
        {
            types_to_init().push_back(this);
        }
    };

}

#endif
