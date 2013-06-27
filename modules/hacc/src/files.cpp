#include "../inc/documents.h"

namespace hacc {

    std::unordered_map<std::string, File*> files_by_filename;

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
    struct File {
        FileState state;
        std::string filename;
        Dynamic data;  // null if not loaded
        Dynamic new_data;  // For when reloading
        Func<Pointer (File*)> magic;
        std::vector<File*> dependencies;
        bool requested = false;

        File (std::string n) : filename(n) { }
    };
    std::string filename (File* f) { return f->filename; }
    FileState state (File* f) { return f->state; }
    bool loaded (File* f) { return f->state == LOADED; }
    bool magic (File* f) { return f->magic; }
    bool requested (File* f) { return f->requested; }

     // CREATING AND DECLARING

    File* new_file (std::string filename, Pointer data) {
        File* f = file(filename);
        if (f->loaded) throw Error("Tried to create \"" + filename + "\" but that file is already loaded");
        f->data = Dynamic(data);
        f->state = LOADED;
        f->requested = true;
        return f;
    }

    void magic_file (std::string name, const Func<Pointer (File*)>& hook) {
        file(filename)->magic = hook;
    }

    void dependency (File* er, File* ee) {
        er->dependencies.push_back(ee);
    }

     // SEARCHING

    File* file (std::string filename) {
        File*& f = files_by_filename[filename];
        if (!f) f = new File (filename);
        return f;
    }

    std::vector<File*> loaded_files () {
        std::vector<File*> fs;
        for (auto& p : files_by_filename)
            if (p.second->state == LOADED)
                fs.push_back(p.second);
        return fs;
    }

     // LOADING AND UNLOADING AND STUFF

    struct Transaction {
        static Transaction* current;
        Transaction () {
            if (current) throw Error("Internal error: tried to have two Transactions at once");
            current = this;
        }

         // A transaction consists of a priority queue of actions.
        enum Priority {
            PREPARE,  // Actually, this should be done ASAP and not be queued
            SET,
            FINISH,
            VERIFY,
            COMMIT  // These actions are irreversible and hopefully won't fail
        };

        struct Action {
            Priority pri;
            Func<void ()> run;

            Action* next;
            static Action* first;
            Action (Priority pri, Func<void ()>&& run) :
                pri(pri), run(run)
            {
                for(
                    Action** pp = &first;
                    *pp && pri() >= (*pp)->pri();
                    pp = &(*pp)->next;
                ) { }
                next = *pp;
                *pp = this;
            }
        };
        Action* Action::first = null;

         // LOADING
        void request_load (File* f) {
            if (f->state != UNLOADED) return;
            f->state = LOAD_PREPARING;
            for (auto d : f->dependencies) {
                request_load(d);
            }
            load_prepare(f);
        }
        void load_prepare (File* f) {
            if (f->magic) {
                f->data = Dynamic(f->magic(f));
                f->state = LOADED;
            }
            else {
                Hacc* h = hacc_from_file(f->filename);
                prepare_from_hacc(f->data, h);
                f->state = LOAD_FILLING;
                new Action(SET, [=](){ load_set(f, h); });
            }
        }
        void load_set (File* f, Hacc* h) {
            set_from_hacc(f->data, h);
            f->state = LOAD_FINISHING;
            new Action(FINISH, [=](){ load_finish(f, h); });
        }
        void load_finish (File* f, Hacc* h) {
            finish_from_hacc(f->data, h);
            f->state = LOAD_COMMITTING;
            new Action(COMMIT, [=](){ load_commit(f); });
        }
        void load_commit (File* f) {
            f->state = LOADED;
        }

         // SAVING
        void request_save (File* f) {
             // Saving is ideally a read-only operation, so no state changes
             //  are necessary.
            new Action(VERIFY, [=](){ save_prepare(f); });
        }
        void save_prepare (File* f) {
            Hacc* h = to_hacc(f->data);
            new Action(COMMIT, [=](){ save_commit(f, h); });
        }
        void save_commit (File* f, Hacc* h) {
             // TODO catch exceptions
            hacc_to_file(h, f->filename);
        }

         // RELOADING
        void request_reload (File* f) {
            if (f->state != LOADED) return;
            f->state = RELOAD_PREPARING;
            reload_prepare(f);
        }
        void reload_prepare (File* f) {
            if (f->magic) {
                f->new_data = Dynamic(f->magic(f));
                f->state = RELOAD_VERIFYING;
                new Action(VERIFY, [=](){ reload_verify(f); });
            }
            else {
                Hacc* h = hacc_from_file(f->filename);
                prepare_from_hacc(f->new_data, h);
                f->state = RELOAD_FILLING;
                new Action(SET, [=](){ reload_set(f, h); });
            }
        }
        void reload_set (File* f, Hacc* h) {
            set_from_hacc(f->new_data, h);
            f->state = RELOAD_FINISHING;
            new Action(FINISH, [=](){ reload_finish(f, h); });
        }
        void reload_finish (File* f, Hacc* h) {
            finish_from_hacc(f->new_data, h);
            f->state = RELOAD_VERIFYING;
            if (!reload_verify_scheduled) {
                reload_verify_scheduled = true;
                new Action(VERIFY, [=](){ reload_verify(); });
            }
        }
        bool reload_verify_scheduled = false;
        void reload_verify () {
            reload_verify_scheduled = false;
            struct Pointer_Info {
                Reference r;
                Pointer new_address;
            };
            std::vector<Pointer_Info> pointers;
            foreach_pointer([&](Reference pr){
                pr.get([&](void* pp){
                    Path* path = pointer_to_path(*(Pointer*)pp);
                    if (path) {
                        File* f = file(path->root());
                        if (f->state == RELOAD_VERIFYING) {
                            Pointer new_address = path_to_reference(path, f->new_data);
                            pointers.emplace_back(pr, new_address);
                        }
                    }
                });
            });
            std::vector<Pointer_Info>&& pmove = std::move(pointers);
            new Action(COMMIT, [pmove](){ reload_commit(pmove); });
        }
        void reload_commit (std::vector<Pointer_Info>& pointers) {
            for (auto& pi : pointers) {
                pi.r.set([&](void* pp){ *(Pointer*)pp = pi.new_address; });
            }
        }

         // UNLOADING
        void request_unload (File* f) {
             // TODO: dependency analysis
            if (f->state != LOADED) return;
            f->state = UNLOAD_VERIFYING;
            new Action(VERIFY, [=](){ unload_verify(f); });
        }
        void unload_verify () {
            for (auto& p : files_by_filename) {
                File* f = p.second;
                if (f->state != UNLOAD_VERIFYING && f->state != UNLOADED) {
                    foreach_pointer([&](Reference pr){
                        pr.get([&](void* pp){
                            Path* path = pointer_to_path(*(Pointer*)pp);
                            if (path) {
                                std::string filename = path->root();
                                if (state(file(filename)) == UNLOAD_VERIFYING) {
                                    throw Error("Cannot unload file \"" + filename + "\" because it is referenced by file \"" + f->filename + "\"");
                                }
                            }
                        });
                    }, f->data);
                }
            }
            for (auto& p : files_by_filename) {
                if (p.second->state == UNLOAD_VERIFYING)
                    p.second->state = UNLOAD_COMMITTING;
            }
            new Action(COMMIT, [=](){ unload_commit(); });
        }
        void unload_commit () {
            for (auto& p : files_by_filename) {
                File* f = p.second;
                if (f->state == UNLOAD_COMMITTING) {
                    f->data = null;
                    f->state = UNLOADED;
                }
            }
        }

         // EXECUTION
        bool success = false;
        void run () {
            while (Action* now = Action::current) {
                Action::current = now->next;
                now->run();
            }
            success = true;
        }

        ~Transaction () {
             // If we left a job undone, we need to clean up.
            for (auto& p : files_by_filename) {
                File* f = p.second;
                switch (f->state) {
                    case LOAD_PREPARING:
                    case LOAD_FILLING:
                    case LOAD_FINISHING:
                    case LOAD_COMMITTING: {
                        f->data = null;
                        f->state = UNLOADED;
                        break;
                    }
                    case RELOAD_PREPARING:
                    case RELOAD_FILLING:
                    case RELOAD_FINISHING:
                    case RELOAD_VERIFYING:
                    case RELOAD_COMMITTING: {
                        f->new_data = null;
                        f->state = LOADED;
                        break;
                    }
                    case UNLOAD_VERIFYING:
                    case UNLOAD_COMMITTING: {
                        f->state = LOADED;
                        break;
                    }
                    default: break;
                }
            }
            Action* next;
            for (Action* a = Action::current; a; a = next) {
                next = a->next;
                delete a;
            }
            clear_scans();
            GC_collect();
            current = null;
        }
    };
    Transaction* Transaction::current = null;

    void with_transaction (const Func<void (Transaction*)>& f) {
        if (Transaction::current) {
            f(Transaction::current);
        }
        else {
            Transaction tr;
            f(&tr);
            tr.run();
        }
    }

    void load (const std::vector<File*>& files) {
        with_transaction([&](Transaction* tr){
            for (auto f : files) tr->request_load(f);
        });
    }
    void save (const std::vector<File*>& files) {
        with_transaction([&](Transaction* tr){
            for (auto f : files) tr->request_save(f);
        });
    }
    void reload (const std::vector<File*>& files) {
        with_transaction([&](Transaction* tr){
            for (auto f : files) tr->request_reload(f);
        });
    }
    void unload (const std::vector<File*>& files) {
        with_transaction([&](Transaction* tr){
            for (auto f : files) tr->request_unload(f);
        });
    }

}
