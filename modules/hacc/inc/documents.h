#ifndef HAVE_HACC_DOCUMENTS_H
#define HAVE_HACC_DOCUMENTS_H

#include "hacc.h"

 // Most if not all of the important objects in this program should be
 //  loaded via documents through this module.

 // An object in a document must not explicitly refer to a global object.
 //  It can refer to one, but that reference must not be a canonical part
 //  of its data as stored in the document.
 // An global object may only refer to an object in a document through a
 //  weak reference (i.e. one that is removed by the object's destructor,
 //  RAII style).
 // Objects may be added to document either from the file or after the fact
 //  programmatically.  When the document is unloaded, they will be destroyed
 //  in the reverse of the order that they were added.

namespace hacc {

    struct Document_Object_G;

    struct Document {
        std::string filename;
        Pointer data;
        Document_Object_G* last;
        Hacc* hacc;  // This will not always be resident.
        bool requested;

        template <class C, class... Args> C* create (Args...);
         // The given data must belong to this document or you will get
         // nasty crashiness.
        template <class C> void destroy (C*);
    };

     // Creates a new document according to the given filename.  It won't have
     // any associated data yet.  You need to add at least one object before
     // saving it to a file.  The first object added will be the document's main object.
     // TODO: allow more than one object to be actually stored in a file, haha
    Document* new_document (std::string filename);

     // Gets a document by its filename.  Use ->data to get its main object.
     // It and its dependencies will be loaded if necessary.
     // The loaded documents will not be marked as requested.
    Document* doc (std::string);
     // Explicitly load a series of documents.
     // The loaded documents will be marked as requested.
     //  This won't return anything.  Follow up with a call
     //  to doc() if necessary
    void load (const std::vector<std::string>&);
     // Scan all loaded document objects and unload any non-requested
     // documents that have no more references to them.
    void trim ();
     // Unload these documents, and then call trim.  If there are references
     //  to the unloaded documents, they will not be fixed!
    void unload (const std::vector<std::string>&);
     // Unload but do not call trim.
    void just_unload (const std::vector<std::string>&);
     // Like unload but throws if something references it.  If there are
     //  referential cycles you may not be able to use this.
    void safe_unload (const std::vector<std::string>&);
     // 名前通り
    void just_safe_unload (const std::vector<std::string>&);
     // Reload a document.  This will scan all other documents for references
     //  to this one and update them accordingly.  This will throw if there
     //  are any references that are valid for the old version but invalid for
     //  the new version of this document.
    void reload (const std::vector<std::string>&);

     // Show all loaded documents.
    std::vector<Document*> list ();

     // INTERNAL
    struct Document_Object_G {
        virtual ~Document_Object_G () { }
        Document_Object_G* next = NULL;
        Document_Object_G* prev = NULL;
    };
    template <class C>
    struct Document_Object : Document_Object_G {
        C data;
        template <class... Args>
        Document_Object (Args... args) : data(args...) { }
    };
    template <class C, class Args...>
    C* Document::create<C> (Args... args) {
        auto obj = new Document_Object<C>(args...);
        if (last == NULL) {
            data = obj;
            last = obj;
        }
        else {
            obj->prev = last;
            last->next = obj;
        }
        return &obj->data;
    }
    template <class C>
    void Document::destroy (C* p) {
        auto obj = static_cast<Document_Object<C>*>((char*)p - offsetof(Document_Object<C>, data));
        if (obj->next) obj->next->prev = obj->prev;
        else last = obj->prev;
        if (obj->prev) obj->prev->next = obj->next;
        else data.p = NULL;
        delete obj;
    }

}

#endif
