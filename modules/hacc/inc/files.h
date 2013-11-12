#ifndef HAVE_HACC_FILES_H
#define HAVE_HACC_FILES_H

#include "common.h"
#include "dynamicism.h"

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

     // FILES
    struct FileData;
    struct File {
        FileData* p;

         // Get the underlying object, loading it from disk if necessary.
         //  If so, it won't be marked as requested.
        Reference data ();
        String filename ();
        bool loaded ();  // if it's fully and completely loaded
        bool requested ();  // if it was manually loaded
         // Rename:
         //   Fails if new filename is already loaded.
         //   Does not affect disk.
         //   Does not duplicate data.
         //   Old filename will be marked as unloaded.
         //   Other File objects referring to this data will stay referring
         //    to the same data under the new name.
        void rename (String);

        File (FileData* p) : p(p) { }
         // Get a file by name.  If it doesn't exist, you won't get
         //  an error until you try to use it.
        File (std::string filename);
         // Create a new file with the given data.
        File (std::string filename, Dynamic&& data);
    };

     // Show all loaded files.
    std::vector<File> loaded_files ();

     // Explicitly load a list of files.
     // The loaded documents will be marked as requested.
    void load (File);
    void load (const std::vector<File>&);
     // Unload these files, and then call trim.  If there are references
     //  elsewhere to the unloaded objects, this transaction wlil fail.
    void unload (File);
    void unload (const std::vector<File>&);
     // (NYI) Reload some files.  This will scan all other file-objects for references
     //  to this one and update them accordingly.  It will throw if there
     //  are any references that are valid for the old version but invalid for
     //  the new version of this file.
    void reload (File);
    void reload (const std::vector<File>&);
     // Saves the files to disk according to their filename.
    void save (File);
    void save (const std::vector<File>&);

     // If you're doing a lot of file API calls, wrap them in this and the
     //  module will do its best to provide transactional semantics; that is,
     //  either all of the operations succeed or none of them will.
    void file_transaction (const Func<void ()>&);

    void set_file_logger (const Func<void (String)>&);

     // PATHS

     // path_to_reference does not require any scans.
     // If a root is provided, the path loookup will start
     //  from there instead of the file indicated by the path.
     // Throws if the path doesn't resolve to a location.
    Reference path_to_reference (Path, Pointer root = null);

     // address_to_path may require all or some file-objects to be scanned.
     // If a prefix is provided, only paths starting with it will be
     //  considered.  This can save time since it won't have to scan every
     //  single file-object.  Returns null if the address isn't found.
     // If you run address_to_path inside a file_transaction, the address
     //  scan results will be cached between calls.
     // The reload_verify flag is for internal optimization use.
    Path address_to_path (Pointer, Path prefix = Path(null), bool reload_verify = false);

     // Performs an operation for each pointer found in the given root, or in
     //  every file-object if root is null.  The callback will be always be
     //  provided with a Reference to Pointer.
    void foreach_pointer (const Func<void (Reference)>&, Pointer root = null);

     // MISC

     // Let this object be scanned and managed by the hacc system.
     //  This can reference file objects, but file objects cannot reference it.
     //  Note: if you manage a pointer, make sure to call this on the address of
     //  the pointer, not the pointer itself.  For example,
     //      Program* prog = hacc::File("vis/res/color.prog").data().attr("prog");
     //      hacc::manage(&prog);  // NOT hacc::manage(prog)
    void manage (Reference);

    namespace X {
        struct File_Already_Loaded : Logic_Error {
            String filename;
            File_Already_Loaded(String);
        };
        struct Double_Transaction : Internal_Error {
            Double_Transaction ();
        };
        struct Reload_NYI : Internal_Error {
            Reload_NYI ();
        };
        struct Unload_Would_Break : Logic_Error {
            Path ref;
            Path target;
            Unload_Would_Break (Path, Path);
        };
        struct Reload_Would_Break : Logic_Error {
            Path ref;
            Path target;
            Reload_Would_Break (Path, Path);
        };
        struct Reload_Would_Break_Type : Logic_Error {
            Path ref;
            Path target;
            Type ot;
            Type nt;
            Reload_Would_Break_Type (Path, Path, Type, Type);
        };
    }

}

#endif
