#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include "../inc/documents.h"
#include "../inc/haccable.h"

namespace hacc {

    struct DocObj;

    std::unordered_set<Document*> all_docs;

    struct DocLink {
        DocLink* prev;
        DocLink* next;
        DocLink () : prev(this), next(this) { }
        DocLink (DocLink* list) : prev(list), next(list->next) {
            list->next->prev = this;
            list->next = this;
        }
        ~DocLink () {
            prev->next = next;
            next->prev = prev;
        }
    };

    struct DocObj : DocLink {
        String id;
        Type type;
        DocObj (DocLink* list, String id, Type type) :
            DocLink(list), id(id), type(type)
        { }
    };

    struct DocumentData : DocLink {
        size_t next_id = 1;
         // Will usually be non-resident
        std::unordered_map<String, DocObj*> by_id;

        DocObj* alloc (String id, Type type) {
            void* p = operator new(type.size() + sizeof(DocObj));
            return new (p) DocObj{this, id, type};
        }
        void dealloc (DocObj* obj) {
             // Doubly-linked list magic
            delete obj;
        }
        void destroy (DocObj* obj) {
            obj->type.destruct(obj + 1);
            dealloc(obj);
        }
        ~DocumentData () {
            while (prev != this) {
                destroy(static_cast<DocObj*>(prev));
            }
        }
    };

    Document::Document () : data(new DocumentData) {
        all_docs.insert(this);
    }
    Document::~Document () {
        all_docs.erase(this);
        delete data;
    }
    void* Document::alloc (Type type) {
        return data->alloc("", type) + 1;
    }
    void* Document::alloc_id (String id, Type type) {
        if (id[0] == '_') throw X::Logic_Error("Cannot create an object in a hacc::Document with an ID starting with _: " + id);
        return data->alloc(id, type) + 1;
    }
    void Document::dealloc (void* p) {
        data->dealloc((DocObj*)p - 1);
    }
    void Document::change_id (void* p, String id) {
        ((DocObj*)p - 1)->id = id;
    }

    static Pointer _get (DocumentData* data, String s) {
        if (!data->by_id.empty()) {
            auto iter = data->by_id.find(s);
            if (iter != data->by_id.end()) {
                if (!iter->second) return null;
                return Pointer(iter->second->type, iter->second + 1);
            }
            else return null;
        }
        else {
            for (DocLink* link = data->next; link != data; link = link->next) {
                auto obj = static_cast<DocObj*>(link);
                if (obj->id.empty()) {
                    obj->id = "_" + std::to_string(data->next_id++);
                }
                if (obj->id == s)
                    return Pointer(obj->type, obj + 1);
            }
            return null;
        }
    }
    Pointer Document::get (String s) { return _get(data, s); }

    static std::vector<String> _all_ids (DocumentData* data) {
        std::vector<String> r;
        for (DocLink* link = data->next; link != data; link = link->next) {
            auto obj = static_cast<DocObj*>(link);
            if (obj->id.empty()) {
                obj->id = "_" + std::to_string(data->next_id++);
            }
            r.push_back(obj->id);
        }
        return std::move(r);
    }
    std::vector<String> Document::all_ids () {
        return _all_ids(data);
    }

    Document* get_document_containing (void* p) {
        for (auto d : all_docs) {
            for (DocLink* link = d->data->next; link != d->data; link = link->next) {
                if ((char*)link + sizeof(DocObj) == (char*)p)
                    return d;
            }
        }
        return null;
    }

    namespace X {
        Document_Bad_ID::Document_Bad_ID (size_t got, size_t next) :
            Logic_Error(
                "Invalid hacc::Document special ID: " + std::to_string(got)
              + " >= _next_id " + std::to_string(next)
            ), got(got), next(next)
        { }
    }

} using namespace hacc;

HACCABLE(Document) {
    name("hacc::Document");
    delegate(ref_func<DocumentData>([](Document& d)->DocumentData&{ return *d.data; }));
}

HACCABLE(DocumentData) {
    name("hacc::DocumentData");
    prepare([](DocumentData& d, Tree t){
        if (t.form() != OBJECT)
            throw X::Form_Mismatch(Type::CppType<DocumentData>(), t);
        const Object& o = t.as<const Object&>();
         // Verify format
        size_t next_id = 1;
        size_t largest_id = 0;
        for (auto& pair : o) {
            if (pair.first[0] == '_') {
                if (pair.first == "_next_id") {
                    if (pair.second.form() != INTEGER)
                        throw X::Logic_Error("The _next_id attribute of a hacc::Document must be an integer");
                    next_id = pair.second.as<size_t>();
                }
                else {
                    std::istringstream ss (pair.first);
                    char _;
                    size_t id;
                    ss >> _ >> id;
                    if (!ss.eof()) throw X::Logic_Error("Invalid hacc::Document special id " + pair.first);
                    if (id > largest_id) largest_id = id;
                    if (d.by_id.find(pair.first) != d.by_id.end())
                        throw X::Logic_Error("Duplicate ID in hacc::Document: " + pair.first);
                }
            }
            else {
                if (pair.second.form() != OBJECT)
                    throw X::Logic_Error("Each object in a hacc::Document must be a {type:value} pair");
                const Object& oo = pair.second.as<const Object&>();
                if (oo.size() != 1)
                    throw X::Logic_Error("Each object in a hacc::Document must have a single type:value pair");
                Type(oo[0].first);  // Validate type name
                if (d.by_id.find(pair.first) != d.by_id.end())
                    throw X::Logic_Error("Duplicate ID in hacc::Document: " + pair.first);
            }
        }
        if (largest_id >= next_id) {
            throw X::Document_Bad_ID(largest_id, next_id);
        }
         // Allocate and prepare
        for (auto& pair : o) {
            if (pair.first == "_next_id") continue;
            const Object& oo = pair.second.as<const Object&>();
            Type type = Type(oo[0].first);
            DocObj* obj = d.alloc(pair.first, type);
            type.construct(obj + 1);
            d.by_id[pair.first] = obj;
            Reference(type, obj + 1).prepare(oo[0].second);
        }
    });
    fill([](DocumentData& d, Tree t){
        for (auto& pair : t.as<const Object&>()) {
            if (pair.first == "_next_id") {
                Reference(&d.next_id).fill(pair.second);
            }
            else Reference(_get(&d, pair.first)).fill(
                pair.second.as<const Object&>()[0].second
            );
        }
    });
    keys(mixed_funcs<std::vector<String>>(
        [](const DocumentData& d){
            std::vector<String> r = _all_ids(&const_cast<DocumentData&>(d));
            r.push_back("_next_id");
            return std::move(r);
        }, [](DocumentData&, const std::vector<String>&){
            throw X::Logic_Error("Cannot set_keys on hacc::Document");
        }
    ));
    attr("_next_id", member(&DocumentData::next_id).optional());
    attrs([](DocumentData& d, String name){
        Pointer p = _get(&d, name);
        if (p) return p;
        else throw X::No_Attr(Pointer(&d), name);
    });
    finish([](DocumentData& d){
        for (DocLink* link = d.next; link != &d; link = link->next) {
            DocObj* obj = static_cast<DocObj*>(link);
            Reference(obj->type, obj + 1).finish();
        }
        d.by_id.clear();
    });
    to_tree([](const DocumentData& d){
        Object o;
        o.emplace_back("_next_id", Tree(d.next_id));
        for (DocLink* link = d.next; link != &d; link = link->next) {
            auto obj = static_cast<DocObj*>(link);
            Tree val = Reference(obj->type, obj + 1).to_tree();
            o.emplace_back(obj->id, Tree(Object{Pair{obj->type.name(), val}}));
        }
        return Tree(std::move(o));
    });
}
