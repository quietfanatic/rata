#ifndef HAVE_HACC_FILES_H
#define HAVE_HACC_FILES_H

#include "hacc.h"

 // Most if not all of the important objects in this program should be
 //  loaded from files through this module.

 // When in a file, objects refer to other objects through a path,
 //  consisting of a filename and a series of attribute or element lookups. 
 //  When in memory, objects refer to other objects through plain pointers.

 // Pointers from document objects to outside object cannot be saved
 //  in a file, so if any such references exist, they must be invisible to
 //  the hacc declaration, or document saving will fail.
 // Outside objects may point to document objects, but if the document is
 //  unloaded, the pointers will be broken.  One way to prevent this is to
 //  have the object's destructor remove such pointers.

 // The functions for loading and unloading and stuff take multiple documents
 //  as arguments, because it is often more efficient to load and unload them
 //  all at once than to do it individually.

namespace hacc {

    struct File;

    bool is_loaded (File*);  // If it's currently loaded
    bool is_magic (File*);  // Loaded through code instead of a file
    bool is_requested (File*);  // Was loaded manually, won't be autounloaded

     // This doesn't create a file; it looks one up.  It doesn't load it.
    File* file (std::string filename);

     // Get the underlying object, loading it from disk if necessary.  If it's
     //  loaded, it won't be marked as requested.
    Pointer file_data (File*);

     // Create a new file associated with the given filename.
    File* create_file (std::string filename, Pointer data);
     // Create a pseudo-file which will be loaded with code instead of a filename.
    File* magic_file (std::string name, const Func<Pointer (File*)>& hook);
     // Create an explicit dependency between two (likely magical) documents.
    void dependency (File* er, File* ee);

     // Show all loaded files.
    std::vector<File*> loaded_files ();

     // Explicitly load a list of files.
     // The loaded documents will be marked as requested.
    void load (const std::vector<File*>&);
     // Unload these files, and then call trim.  If there are references
     //  to the unloaded objects, they will not be fixed!
    void unload (const std::vector<File*>&);
     // Reload some files.  This will scan all other file-objects for references
     //  to this one and update them accordingly.  It will throw if there
     //  are any references that are valid for the old version but invalid for
     //  the new version of this file.
    void reload (const std::vector<File*>&);
     // Saves the files to disk.  This will throw if any of them were loaded
     //  through magic.
    void save (const std::vector<File*>&);

     // Scan all loaded file-objects and unload any non-requested files
     //  that have no more references to them.
    void trim ();

}

#endif
