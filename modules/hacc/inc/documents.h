#ifndef HAVE_HACC_DOCUMENTS_H
#define HAVE_HACC_DOCUMENTS_H
#include "types.h"
#include "dynamicism.h"

namespace hacc {

    struct DocumentData;
    struct Document {
        DocumentData* data;
        Document ();
        ~Document ();
        Document (const Document&) = delete;
        Document& operator = (const Document&) = delete;

        void* alloc (Type);
        void* alloc_id (String, Type);
        void dealloc (void*);

        template <class C, class... Args>
        C* create (Args&&... args) {
            void* p = alloc(Type::CppType<C>());
            return new (p) C (std::forward<Args>(args)...);
        }
         // For performance reasons, this will not prevent you from using
         //  duplicate IDs, and if you do, things will break.  If you must
         //  prevent duplicate ids, call get(id) before create_id(id, ...)
        template <class C, class... Args>
        C* create_id (String id, Args&&... args) {
            void* p = alloc_id(id, Type::CppType<C>());
            return new (p) C (std::forward<Args>(args)...);
        }
        template <class C>
        void destroy (C* p) {
            p->~C();
            dealloc(p);
        }
        void change_id (void* p, String id);
        template <class C>
        void change_id (C* p, String id) {
            change_id((void*)p, id);
        }

        Pointer get (String);
        std::vector<String> all_ids ();
    };

    namespace X {
        struct Document_Bad_ID : Logic_Error {
            size_t got;
            size_t next;
            Document_Bad_ID (size_t, size_t);
        };
    }

}

#endif
