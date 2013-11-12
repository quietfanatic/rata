#include <sstream>
#include <unordered_map>
#include "../inc/files.h"
#include "../inc/strings.h"
#include "types_internal.h"
#include "paths_internal.h"

namespace hacc {

    std::unordered_map<std::string, FileData*> files_by_filename;
    Func<void (String)> logger;
    std::vector<Reference> managed_objects;

     // File structure's innards.  -ing states are named after
     //  the action that is next scheduled to happen to the file
    enum FileState {
        UNLOADED,
        LOAD_PREPARING,
        LOAD_FILLING,
        LOAD_FINISHING,
        LOAD_COMMITTING,
        LOADED,
        RELOAD_PREPARING,
        RELOAD_FILLING,
        RELOAD_FINISHING,
        RELOAD_VERIFYING,
        RELOAD_COMMITTING,
        UNLOAD_VERIFYING,
        UNLOAD_COMMITTING
    };
    struct FileData {
        FileState state = UNLOADED;
        String filename;
        Dynamic data = null;  // null if not loaded
        Dynamic old_data = null;  // For when reloading
        bool requested = false;
        bool addresses_scanned = false;

        FileData (String n) : filename(n) { }
    };
    String File::filename () { return p->filename; }
    bool File::loaded () { return p->state == LOADED; }
    bool File::requested () { return p->requested; }
    File::File (String filename) {
        FileData*& fd = files_by_filename[filename];
        if (!fd) fd = new FileData (filename);
        p = fd;
    };
    File::File (std::string filename, Dynamic&& data) :
        File(filename)
    {
        if (p->state != UNLOADED) throw X::File_Already_Loaded(filename);
        p->data = data;
        p->state = LOADED;
        p->requested = true;
    }
    Reference File::data () {
        if (p->state == UNLOADED)
            load(*this);
        if (!p->data.address()) {
            throw X::Internal_Error("Something went wrong when autoloading \"" + filename() + "\"; No exception happened but it wasn't loaded.  It's state was " + std::to_string(p->state));
        }
        return p->data.address();
    }
    void File::rename (std::string new_filename) {
        if (File(new_filename).loaded()) throw X::File_Already_Loaded(new_filename);
        files_by_filename.erase(p->filename);
        p->filename = new_filename;
        files_by_filename[new_filename] = p;
    }

     // SEARCHING

    std::vector<File> loaded_files () {
        std::vector<File> fs;
        for (auto& p : files_by_filename)
            if (p.second->state == LOADED)
                fs.push_back(p.second);
        return fs;
    }

     // File extension magic

    static std::unordered_map<std::string, Special_Filetype*>& extensions () {
        static std::unordered_map<std::string, Special_Filetype*> r;
        return r;
    }

    Special_Filetype::Special_Filetype (
        std::string ext,
        const Func<Dynamic (std::string)>& load,
        const Func<void (std::string, const Dynamic&)>& save
    ) : extension(ext), load(load), save(save) {
        extensions().emplace(ext, this);
    }

    Special_Filetype* get_ext (std::string filename) {
        size_t dot = filename.rfind('.');
        if (dot == std::string::npos) return null;
        std::string ext = filename.substr(dot + 1);
        auto iter = extensions().find(ext);
        if (iter != extensions().end())
            return iter->second;
        else
            return null;
    }

     // LOADING AND UNLOADING AND STUFF

    struct Transaction {
        static Transaction* current;
        Transaction () {
            if (current) throw X::Double_Transaction();
            current = this;
        }

        std::vector<std::exception_ptr> delayed_errors;
        void throw_delayed_errors () {
            switch (delayed_errors.size()) {
                case 0: return;
                case 1: std::rethrow_exception(delayed_errors[0]);
                default: throw X::Combo_Error(std::move(delayed_errors));
            }
        }

         // A transaction consists of a priority queue of actions.
        enum Priority {
            PREPARE,
            FILL,
            FINISH,
            VERIFY,
            SAVE_COMMIT,
            COMMIT  // These actions are irreversible and hopefully won't fail
        };

        struct Action {
            Priority pri;
            Func<void ()> run;

            Action* next;
            static Action* first;
            Action (Priority pri_, Func<void ()>&& run) :
                pri(pri_), run(run)
            {
                Action** pp = &first;
                while (*pp && pri >= (*pp)->pri)
                    pp = &(*pp)->next;
                next = *pp;
                *pp = this;
            }
        };

         // LOADING
        void request_load (File f) {
            if (f.p->state != UNLOADED) return;
            if (auto ext = get_ext(f.p->filename)) {
                f.p->state = LOAD_FILLING;
                new Action(FILL, [=](){ load_ext(f, ext); });
            }
            else {
                f.p->state = LOAD_PREPARING;
                load_prepare(f);
            }
        }
        void load_prepare (File f) {
            Tree t;
            try {
                t = tree_from_file(f.p->filename);
                Reference(&f.p->data).prepare(t);
            }
            catch (X::Error& e) {
                if (e.filename.empty()) e.filename = f.filename();
                throw;
            }
            f.p->state = LOAD_FILLING;
            new Action(FILL, [=](){ load_fill(f, t); });
        }
        void load_fill (File f, Tree t) {
            try {
                Reference(&f.p->data).fill(t);
            }
            catch (X::Error& e) {
                if (e.filename.empty()) e.filename = f.filename();
                throw;
            }
            f.p->state = LOAD_FINISHING;
            new Action(FINISH, [=](){ load_finish(f); });
        }
        void load_finish (File f) {
            try {
                Reference(&f.p->data).finish();
            }
            catch (X::Error& e) {
                if (e.filename.empty()) e.filename = f.filename();
                throw;
            }
            f.p->state = LOAD_COMMITTING;
            new Action(COMMIT, [=](){ load_commit(f); });
        }
        void load_ext (File f, Special_Filetype* ext) {
            try {
                f.p->data = ext->load(f.p->filename);
            }
            catch (X::Error& e) {
                if (e.filename.empty()) e.filename = f.filename();
                throw;
            }
            f.p->state = LOAD_COMMITTING;
            new Action(COMMIT, [=](){ load_commit(f); });
        }
        void load_commit (File f) {
            f.p->state = LOADED;
            if (logger) logger("Loaded \"" + f.filename() + "\"");
        }

         // SAVING
        void request_save (File f) {
             // Saving is ideally a read-only operation, so no state changes
             //  are necessary.
            if (auto ext = get_ext(f.p->filename)) {
                new Action(SAVE_COMMIT, [=](){ save_ext(f, ext); });
            }
            else {
                new Action(VERIFY, [=](){ save_prepare(f); });
            }
        }
        void save_prepare (File f) {
            Tree t;
            try {
                t = Reference(&f.p->data).to_tree();
            }
            catch (X::Error& e) {
                if (e.filename.empty()) e.filename = f.filename();
                throw;
            }
            new Action(SAVE_COMMIT, [=](){ save_commit(f, t); });
        }
        void save_commit (File f, Tree t) {
            try {
                tree_to_file(t, f.p->filename);
                if (logger) logger("Saved \"" + f.filename() + "\"");
            }
            catch (X::Error& e) {
                if (e.filename.empty()) e.filename = f.filename();
                delayed_errors.push_back(std::current_exception());
            }
            catch (...) {
                delayed_errors.push_back(std::current_exception());
            }
        }
        void save_ext (File f, Special_Filetype* ext) {
            try {
                ext->save(f.p->filename, f.p->data);
                if (logger) logger("Saved \"" + f.filename() + "\"");
            }
            catch (X::Error& e) {
                if (e.filename.empty()) e.filename = f.filename();
                delayed_errors.push_back(std::current_exception());
            }
            catch (...) {
                delayed_errors.push_back(std::current_exception());
            }
        }

         // RELOADING
        void request_reload (File f) {
            if (f.p->state != LOADED) return;
            f.p->old_data = std::move(f.p->data);
            if (auto ext = get_ext(f.p->filename)) {
                f.p->state = RELOAD_FILLING;
                new Action(FILL, [=](){ reload_ext(f, ext); });
            }
            else {
                f.p->state = RELOAD_PREPARING;
                reload_prepare(f);
            }
        }
        void reload_prepare (File f) {
            Tree t;
            try {
                t = tree_from_file(f.p->filename);
                Reference(&f.p->data).prepare(t);
            }
            catch (X::Error& e) {
                if (e.filename.empty()) e.filename = f.filename();
                throw;
            }
            f.p->state = RELOAD_FILLING;
            new Action(FILL, [=](){ reload_fill(f, t); });
        }
        void reload_fill (File f, Tree t) {
            try {
                Reference(&f.p->data).fill(t);
            }
            catch (X::Error& e) {
                if (e.filename.empty()) e.filename = f.filename();
                throw;
            }
            f.p->state = RELOAD_FINISHING;
            new Action(FINISH, [=](){ reload_finish(f); });
        }
        bool reload_verify_scheduled = false;
        void reload_finish (File f) {
            try {
                Reference(&f.p->data).finish();
            }
            catch (X::Error& e) {
                if (e.filename.empty()) e.filename = f.filename();
                throw;
            }
            f.p->state = RELOAD_VERIFYING;
            if (!reload_verify_scheduled) {
                reload_verify_scheduled = true;
                new Action(VERIFY, [=](){ reload_verify(); });
            }
        }
        void reload_ext (File f, Special_Filetype* ext) {
            try {
                f.p->data = ext->load(f.p->filename);
                Reference(&f.p->data).finish();
            }
            catch (X::Error& e) {
                if (e.filename.empty()) e.filename = f.filename();
                throw;
            }
            f.p->state = RELOAD_VERIFYING;
            if (!reload_verify_scheduled) {
                reload_verify_scheduled = true;
                new Action(VERIFY, [=](){ reload_verify(); });
            }
        }
        typedef std::pair<Reference, void*> Update;
         // Working around lack of rvalue lambda captures.
        std::vector<Update> updates;
        void reload_verify_scan (Reference r, Path path) {
            r.foreach_pointer([&](Reference rp, Path path){
                rp.read([&](void* pp){
                    Path target = address_to_path(
                        Pointer(rp.type().data->pointee_type, *(void**)pp),
                        Path(null), true
                    );
                    if (target && File(target->root()).p->state == RELOAD_VERIFYING) {
                        Pointer new_addr = path_to_reference(target);
                        if (!new_addr) {
                            try { throw X::Reload_Would_Break(path, target); }
                            catch (...) { delayed_errors.push_back(std::current_exception()); }
                        }
                        else if (new_addr.type != rp.type().data->pointee_type) {
                            try { throw X::Reload_Would_Break_Type(path, target, new_addr.type, rp.type().data->pointee_type); }
                            catch (...) { delayed_errors.push_back(std::current_exception()); }
                        }
                        else {
                            updates.push_back(Update(rp, new_addr.address));
                        }
                    }
                });
                return false;
            }, path);
        }
        void reload_verify () {
            reload_verify_scheduled = false;
             // Check for any pointers we'll need to update
            for (auto& p : files_by_filename) {
                File f = p.second;
                try {
                    switch (f.p->state) {
                         // Files with these states either already point to new data
                         //  or will be unloaded.
                        case RELOAD_VERIFYING:
                        case LOAD_COMMITTING:
                        case UNLOAD_VERIFYING:
                        case UNLOAD_COMMITTING:
                        case UNLOADED:
                            break;
                        default: {
                            reload_verify_scan(f.p->data.address(), Path(f.p->filename));
                            break;
                        }
                    }
                }
                 // Only catch exceptions so we can attach a filename to them.
                catch (X::Error& e) {
                    if (e.filename.empty()) e.filename = f.filename();
                    throw;
                }
            }
            for (auto r : managed_objects) reload_verify_scan(r, Path(null));
            for (auto& p : files_by_filename) {
                File f = p.second;
                if (f.p->state == RELOAD_VERIFYING) {
                    f.p->state = RELOAD_COMMITTING;
                    if (logger) logger("Reloaded \"" + f.filename() + "\"");
                }
            }
            new Action(COMMIT, [=](){ reload_commit(); });
        }
        void reload_commit () {
             // Update all references
            for (auto& p : updates) {
                if (logger) {
                    std::ostringstream ss;
                    ss << "Updating " << p.first.show() << " to " << p.second;
                    logger(ss.str());
                }
                p.first.write([&](void* pp){
                    *(void**)pp = p.second;
                });
            }
            updates.clear();
             // delete old_data for all reloaded files
            for (auto& p : files_by_filename) {
                File f = p.second;
                if (f.p->state == RELOAD_COMMITTING) {
                    f.p->old_data = null;
                    f.p->state = LOADED;
                }
            }
        }

         // UNLOADING
        bool unload_verify_scheduled = false;
        void request_unload (File f) {
            if (f.p->state != LOADED) return;
            f.p->state = UNLOAD_VERIFYING;
            if (!unload_verify_scheduled) {
                unload_verify_scheduled = true;
                new Action(VERIFY, [=](){ unload_verify(); });
            }
        }
        void unload_verify_scan (Reference r, Path path) {
            r.foreach_pointer([&](Reference rp, Path path){
                rp.read([&](void* pp){
                    Path target = address_to_path(
                        Pointer(rp.type().data->pointee_type, *(void**)pp)
                    );
                    if (target && File(target->root()).p->state == UNLOAD_VERIFYING) {
                        try { throw X::Unload_Would_Break(path, target); }
                        catch (...) { delayed_errors.push_back(std::current_exception()); }
                    }
                });
                return false;
            }, path);
        }
        void unload_verify () {
            unload_verify_scheduled = false;
             // Fail if there are any references in any other file to this file.
            for (auto& p : files_by_filename) {
                File f = p.second;
                try {
                     // But it's okay if the referencing file is also being unloaded.
                    if (f.p->state != UNLOAD_VERIFYING && f.p->state != UNLOADED) {
                        reload_verify_scan(f.p->data.address(), Path(f.p->filename));
                    }
                }
                 // Only catch exceptions so we can attach a filename to them.
                catch (X::Error& e) {
                    if (e.filename.empty()) e.filename = f.filename();
                    throw;
                }
            }
            for (auto r : managed_objects) unload_verify_scan(r, Path(null));
            for (auto& p : files_by_filename) {
                File f = p.second;
                if (f.p->state == UNLOAD_VERIFYING) {
                    f.p->state = UNLOAD_COMMITTING;
                }
            }
            new Action(COMMIT, [=](){ unload_commit(); });
        }
        void unload_commit () {
            for (auto& p : files_by_filename) {
                File f = p.second;
                if (f.p->state == UNLOAD_COMMITTING) {
                    f.p->data = null;
                    f.p->state = UNLOADED;
                    if (logger) logger("Unloaded \"" + f.filename() + "\"");
                }
            }
        }

         // EXECUTION
        bool success = false;
        void run () {
            Priority last_pri = PREPARE;
            while (Action* now = Action::first) {
                Action::first = now->next;
                if (now->pri == COMMIT && last_pri != COMMIT) {
                    throw_delayed_errors();
                }
                now->run();
                last_pri = now->pri;
                delete now;
            }
            throw_delayed_errors();
            success = true;
        }

        ~Transaction () {
             // If we left a job undone, we need to clean up.
            for (auto& p : files_by_filename) {
                File f = p.second;
                switch (f.p->state) {
                    case LOAD_PREPARING:
                    case LOAD_FILLING:
                    case LOAD_FINISHING:
                    case LOAD_COMMITTING: {
                        f.p->data = null;
                        f.p->state = UNLOADED;
                        break;
                    }
                    case RELOAD_PREPARING:
                    case RELOAD_FILLING:
                    case RELOAD_FINISHING:
                    case RELOAD_VERIFYING:
                    case RELOAD_COMMITTING: {
                        f.p->data = std::move(f.p->old_data);
                        f.p->state = LOADED;
                        break;
                    }
                    case UNLOAD_VERIFYING:
                    case UNLOAD_COMMITTING: {
                        f.p->state = LOADED;
                        break;
                    }
                    default: break;
                }
                if (f.p->addresses_scanned)
                    f.p->addresses_scanned = false;
            }
            Action* next;
            for (Action* a = Action::first; a; a = next) {
                next = a->next;
                delete a;
            }
            Action::first = null;
            current = null;
        }

         // Transaction provides some caches for path operations
        std::unordered_map<Pointer, Path> address_cache;
    };
    Transaction* Transaction::current = null;
    Transaction::Action* Transaction::Action::first = null;

    void file_transaction (const Func<void ()>& f) {
        if (Transaction::current) {
            f();
        }
        else {
            {
                Transaction tr;
                f();
                tr.run();
            }
        }
    }

    void load (File f) { load(std::vector<File>(1, f)); }
    void load (const std::vector<File>& files) {
        file_transaction([&](){
            for (auto f : files)
                Transaction::current->request_load(f);
        });
    }
    void save (File f) { save(std::vector<File>(1, f)); }
    void save (const std::vector<File>& files) {
        file_transaction([&](){
            for (auto f : files)
                Transaction::current->request_save(f);
        });
    }
    void reload (File f) { reload(std::vector<File>(1, f)); }
    void reload (const std::vector<File>& files) {
        file_transaction([&](){
            for (auto f : files)
                Transaction::current->request_reload(f);
        });
    }
    void unload (File f) { unload(std::vector<File>(1, f)); }
    void unload (const std::vector<File>& files) {
        file_transaction([&](){
            for (auto f : files)
                Transaction::current->request_unload(f);
        });
    }

    void set_file_logger (const Func<void (String)>& f) {
        logger = f;
    }

     // PATHS STUFF

    Reference path_to_reference (Path path, Pointer root) {
        return path->to_reference(root);
    }
    Path address_to_path (Pointer ptr, Path prefix, bool reload_verify) {
        if (prefix != Path(null)) {
            Path found = Path(null);
            Reference start = path_to_reference(prefix);
            start.foreach_address([&](Pointer p, Path path){
                if (p == ptr) {
                    found = path;
                    return true;
                }
                return false;
            }, prefix);
            return found;
        }
        else {
             // We can't iterate over the main files hash, because
             //  if a new file is inserted it'll invalidate the iterators
            std::vector<File> scannable_files;
            for (auto& p : files_by_filename) {
                 // TODO: There is probably some more optimization that can be done here.
                if (reload_verify) {
                    if (p.second->state == RELOAD_VERIFYING)
                        scannable_files.push_back(p.second);
                }
                else {
                    if (p.second->state != UNLOADED
                     && p.second->state != UNLOAD_COMMITTING) {
                        scannable_files.push_back(p.second);
                    }
                }
            }
            Path found = Path(null);
            for (auto f : scannable_files) {
                Reference ref = reload_verify && f.p->old_data.address()
                    ? Reference(f.p->old_data.address())
                    : f.data();
                ref.foreach_address(
                    [&](Pointer p, Path path){
                        if (p == ptr) {
                            found = path;
                            return true;
                        }
                        else {
                            return false;
                        }
                    },
                    Path(f.filename())
                );
                if (found) return found;
            }
            return Path(null);
        }
    }
    void foreach_pointer (const Func<void (Reference)>&, Pointer root) {
        throw X::Internal_Error("Paths NYI, sorry");
    }

    void manage (Reference r) { managed_objects.emplace_back(r); }

    namespace X {
        Double_Transaction::Double_Transaction () :
            Internal_Error("Internal error: Tried two start a second transaction")
        { }
        File_Already_Loaded::File_Already_Loaded (String filename) :
            Logic_Error("Cannot create file \"" + filename + "\" because that filename is already loaded."),
            filename(filename)
        { }
        Unload_Would_Break::Unload_Would_Break (Path ref, Path target) :
            Logic_Error(
                "Reload would break the pointer at " + path_to_string(ref)
              + " pointing to " + path_to_string(target)
            ), ref(ref), target(target)
        { }
        Reload_Would_Break::Reload_Would_Break (Path ref, Path target) :
            Logic_Error(
                "Reload would break the pointer at " + path_to_string(ref)
              + " pointing to " + path_to_string(target)
            ), ref(ref), target(target)
        { }
        Reload_Would_Break_Type::Reload_Would_Break_Type (Path ref, Path target, Type ot, Type nt) :
            Logic_Error(
                "Reload would break the pointer at " + path_to_string(ref)
              + " pointing to " + path_to_string(target)
              + ", which had an old type of " + ot.name()
              + " and has a new type of " + nt.name()
            ), ref(ref), target(target), ot(ot), nt(nt)
        { }
        Reload_NYI::Reload_NYI () : Internal_Error("Reload NYI, sorry") { }
    }

}
