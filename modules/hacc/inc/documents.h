#ifndef HAVE_HACC_DOCUMENTS_H
#define HAVE_HACC_DOCUMENTS_H

 // This provides a container that can hold arbitrarily-typed objects
 //  with minimal overhead.  Objects can be given a string ID in the document.
 // If you have a Hacc file that contains various assorted objects, this
 //  is the recommended way to organize them.

 // There is no protection for if you do something bad like try to use one
 //  document to destroy an object that belongs to another document.

#include "types.h"
#include "dynamicism.h"

namespace hacc {

    struct DocumentData;
    struct Document {
         // Allocate an object of any type within this document.
         //  (calls malloc or new)
        template <class C, class... Args>
        C* create (Args&&... args);
         // Create an object and give it an ID.
        template <class C, class... Args>
        C* create_id (String id, Args&&... args);
         // Change the ID of an object.
        template <class C>
        void change_id (C* p, String id);
         // Destroy an object that belongs to this document.
        template <class C>
        void destroy (C* p);
         // Get a dynamically-typed pointer by ID.  Returns null if there
         //  is no object with that ID.
        Pointer get (String);
         // Return a list of the IDs of all the objects in this document.
         //  For objects without an ID, an ID is automatically generated,
         //  for the sake of storage and program-generated references.
        std::vector<String> all_ids ();

        Document ();
        ~Document ();

        DocumentData* data;
        Document (const Document&) = delete;
        Document& operator = (const Document&) = delete;

        void* alloc (Type);
        void* alloc_id (String, Type);
        void dealloc (void*);
        void change_id (void* p, String id);
    };

    namespace X {
        struct Document_Bad_ID : Logic_Error {
            size_t got;
            size_t next;
            Document_Bad_ID (size_t, size_t);
        };
    }

    template <class C, class... Args>
    C* Document::create (Args&&... args) {
        void* p = alloc(Type::CppType<C>());
        return new (p) C (std::forward<Args>(args)...);
    }
     // For performance reasons, this will not prevent you from using
     //  duplicate IDs, and if you do, things will break.  If you must
     //  prevent duplicate ids, call get(id) before create_id(id, ...)
    template <class C, class... Args>
    C* Document::create_id (String id, Args&&... args) {
        void* p = alloc_id(id, Type::CppType<C>());
        return new (p) C (std::forward<Args>(args)...);
    }
    template <class C>
    void Document::destroy (C* p) {
        p->~C();
        dealloc(p);
    }
    template <class C>
    void Document::change_id (C* p, String id) {
        change_id((void*)p, id);
    }

}

#endif
