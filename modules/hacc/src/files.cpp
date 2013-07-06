#include <unordered_map>
#include "../inc/files.h"
#include "../inc/strings.h"
#include "types_internal.h"

namespace hacc {

    std::unordered_map<std::string, FileData*> files_by_filename;

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
        Dynamic new_data = null;  // For when reloading
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
        return p->data.address();
    }

     // SEARCHING

    std::vector<File> loaded_files () {
        std::vector<File> fs;
        for (auto& p : files_by_filename)
            if (p.second->state == LOADED)
                fs.push_back(p.second);
        return fs;
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
            PREPARE,  // Actually, this should be done ASAP and not be queued
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
            f.p->state = LOAD_PREPARING;
            load_prepare(f);
        }
        void load_prepare (File f) {
            Tree* t;
            try {
                t = tree_from_file(f.p->filename);
                Reference(&f.p->data).prepare(t);
            }
            catch (X::Error& e) {
                if (e.filename.empty()) e.filename = f.filename();
                throw e;
            }
            f.p->state = LOAD_FILLING;
            new Action(FILL, [=](){ load_fill(f, t); });
        }
        void load_fill (File f, Tree* t) {
            try {
                Reference(&f.p->data).fill(t);
            }
            catch (X::Error& e) {
                if (e.filename.empty()) e.filename = f.filename();
                throw e;
            }
            f.p->state = LOAD_FINISHING;
            new Action(FINISH, [=](){ load_finish(f, t); });
        }
        void load_finish (File f, Tree* t) {
            try {
                Reference(&f.p->data).finish(t);
            }
            catch (X::Error& e) {
                if (e.filename.empty()) e.filename = f.filename();
                throw e;
            }
            f.p->state = LOAD_COMMITTING;
            new Action(COMMIT, [=](){ load_commit(f); });
        }
        void load_commit (File f) {
            f.p->state = LOADED;
        }

         // SAVING
        void request_save (File f) {
             // Saving is ideally a read-only operation, so no state changes
             //  are necessary.
            new Action(VERIFY, [=](){ save_prepare(f); });
        }
        void save_prepare (File f) {
            Tree* t;
            try {
                t = Reference(&f.p->data).to_tree();
            }
            catch (X::Error& e) {
                if (e.filename.empty()) e.filename = f.filename();
                throw e;
            }
            new Action(SAVE_COMMIT, [=](){ save_commit(f, t); });
        }
        void save_commit (File f, Tree* t) {
            try {
                tree_to_file(t, f.p->filename);
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
            f.p->state = RELOAD_PREPARING;
            reload_prepare(f);
        }
        void reload_prepare (File f) {
            Tree* t;
            try {
                t = tree_from_file(f.p->filename);
                Reference(&f.p->new_data).prepare(t);
            }
            catch (X::Error& e) {
                if (e.filename.empty()) e.filename = f.filename();
                throw e;
            }
            f.p->state = RELOAD_FILLING;
            new Action(FILL, [=](){ reload_fill(f, t); });
        }
        void reload_fill (File f, Tree* t) {
            try {
                Reference(&f.p->new_data).fill(t);
            }
            catch (X::Error& e) {
                if (e.filename.empty()) e.filename = f.filename();
                throw e;
            }
            f.p->state = RELOAD_FINISHING;
            new Action(FINISH, [=](){ reload_finish(f, t); });
        }
        bool reload_verify_scheduled = false;
        void reload_finish (File f, Tree* t) {
            try {
                Reference(&f.p->new_data).finish(t);
            }
            catch (X::Error& e) {
                if (e.filename.empty()) e.filename = f.filename();
                throw e;
            }
            f.p->state = RELOAD_VERIFYING;
            if (!reload_verify_scheduled) {
                reload_verify_scheduled = true;
                new Action(VERIFY, [=](){ reload_verify(); });
            }
        }
        void reload_verify () {
            reload_verify_scheduled = false;
            throw X::Reload_NYI();
            new Action(COMMIT, [=](){ reload_commit(); });
        }
        void reload_commit () {
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
        void unload_verify () {
            unload_verify_scheduled = false;
            for (auto& p : files_by_filename) {
                File f = p.second;
                try {
                    if (f.p->state != UNLOAD_VERIFYING) {
                        Reference(f.data()).foreach_pointer([&](Reference rp, Path* path){
                            rp.read([&](void* pp){
                                Path* target = address_to_path(
                                    Pointer(rp.type().data->pointee_type, *(void**)pp)
                                );
                                if (target && File(target->root()).p->state == UNLOAD_VERIFYING) {
                                    try { throw X::Unload_Would_Break(f.p->filename, path, target); }
                                    catch (...) { delayed_errors.push_back(std::current_exception()); }
                                }
                            });
                            return false;
                        }, new Path(f.p->filename));
                    }
                }
                catch (X::Error& e) {
                    if (e.filename.empty()) e.filename = f.filename();
                    throw e;
                }
            }
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
                        f.p->new_data = null;
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
        std::unordered_map<Pointer, Path*> address_cache;
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
            GC_gcollect();
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

     // PATHS STUFF

    String Path::root () const {
        switch (type) {
            case ROOT: return s;
            case ATTR: return target->root();
            case ELEM: return target->root();
            default: throw X::Corrupted_Path(const_cast<Path*>(this));
        }
    }

    Reference path_to_reference (Path* path, Pointer root) {
        switch (path->type) {
            case ROOT: {
                if (root) return root;
                else return File(path->s).data();
            }
            case ATTR: {
                Reference l = path_to_reference(path->target, root);
                return l.attr(path->s);
            }
            case ELEM: {
                Reference l = path_to_reference(path->target, root);
                return l.elem(path->i);
            }
            default: throw X::Corrupted_Path(path);
        }
    }
    Path* address_to_path (Pointer ptr, Path* prefix) {
        if (prefix != null) {
            Path* found = null;
            Reference start = path_to_reference(prefix);
            start.foreach_address([&](Pointer p, Path* path){
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
                if (p.second->state != UNLOADED
                 && p.second->state != UNLOAD_COMMITTING) {
                    scannable_files.push_back(p.second);
                }
            }
            Path* found = null;
            if (Transaction::current) {
                for (auto f : scannable_files) {
                    Reference(f.data()).foreach_address(
                        [&](Pointer p, Path* path){
                            Transaction::current->address_cache.emplace(p, path);
                            if (p == ptr) found = path;
                            return false;
                        },
                        new Path (f.filename())
                    );
                    f.p->addresses_scanned = true;
                    if (found) return found;
                }
                return null;
            }
            else {
                 // With no transaction and no prefix, we just gotta scan
                 //  the whole haystack every time.
                for (auto f : scannable_files) {
                    Reference(f.data()).foreach_address(
                        [&](Pointer p, Path* path){
                            if (p == ptr) found = path;
                            return true;
                        },
                        new Path (f.filename())
                    );
                    if (found) return found;
                }
                return null;
            }
        }
    }
    void foreach_pointer (const Func<void (Reference)>&, Pointer root) {
        throw X::Internal_Error("Paths NYI, sorry");
    }

    bool operator == (const Path& a, const Path& b) {
        if (a.type != b.type) return false;
        switch (a.type) {
            case ROOT: return a.s == b.s;
            case ATTR: return a.s == b.s && *a.target == *b.target;
            case ELEM: return a.i == b.i && *a.target == *b.target;
            default: throw X::Corrupted_Path(const_cast<Path*>(&a));
        }
    }

    namespace X {
        Double_Transaction::Double_Transaction () :
            Internal_Error("Internal error: Tried two start a second transaction")
        { }
        Corrupted_Path::Corrupted_Path (Path* path) :
            Corrupted("Corrupted path: nonsensical path type number " + path->type),
            path(path)
        { }
        File_Already_Loaded::File_Already_Loaded (String filename) :
            Logic_Error("Cannot create file \"" + filename + "\" because that filename is already loaded."),
            filename(filename)
        { }
        Unload_Would_Break::Unload_Would_Break (String filename, Path* ref, Path* target) :
            Logic_Error(
                "Cannot unload file \"" + filename
              + "\" because it would break the pointer at " + path_to_string(ref)
              + " pointing to " + path_to_string(target)
            ), filename(filename), ref(ref), target(target)
        { }
        Reload_NYI::Reload_NYI () : Internal_Error("Reload NYI, sorry") { }
    }

}
