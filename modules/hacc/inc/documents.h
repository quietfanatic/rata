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

namespace hacc {
     // Simply returns a reference to a document's main object.
     // It and its dependencies will be loaded if necessary.
     // The loaded documents will not be marked as requested.
    Pointer doc (std::string);
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

    struct Document {
        std::string filename;
        Pointer data;
        Hacc* hacc;  // This will not always be resident.
        bool requested;
    };
     // Show all loaded documents.
    std::vector<Document*> list ();
}

#endif
