#include "hacc/inc/tree.h"

#include "hacc/inc/haccable.h"
#include "hacc/src/paths_internal.h"

namespace hacc {

    std::string form_name (Form f) {
        switch (f) {
            case UNDEFINED: return "undefined";
            case NULLFORM: return "null";
            case BOOL: return "bool";
            case INTEGER: return "integer";
            case FLOAT: return "float";
            case DOUBLE: return "double";
            case STRING: return "string";
            case ARRAY: return "array";
            case OBJECT: return "object";
            case PATH: return "path";
            default: return "corrupted";
        }
    }
    struct TreeData : DPtee {
        virtual Form form () const = 0;
        template <class C>
        const C& as () const;
    };

    template <class C>
    Form form_of_type () { throw X::Internal_Error("Tried to call form_of_type on a non-TreeData type"); }
    template <> Form form_of_type<Null>() { return NULLFORM; }
    template <> Form form_of_type<bool>() { return BOOL; }
    template <> Form form_of_type<int64>() { return INTEGER; }
    template <> Form form_of_type<float>() { return FLOAT; }
    template <> Form form_of_type<double>() { return DOUBLE; }
    template <> Form form_of_type<String>() { return STRING; }
    template <> Form form_of_type<Array>() { return ARRAY; }
    template <> Form form_of_type<Object>() { return OBJECT; }
    template <> Form form_of_type<Path>() { return PATH; }

    template <class C>
    struct TreeDataT : TreeData {
        Form form () const { return form_of_type<C>(); }
        C it;
        TreeDataT (const C& it) : it(it) { }
        TreeDataT (C&& it) : it(it) { }
        operator const C& () const { return it; }
    };

    template <class C>
    const C& TreeData::as () const {
        if (form_of_type<C>() == form()) return *static_cast<const TreeDataT<C>*>(this);
        else throw X::Wrong_Form(form_of_type<C>(), Tree(this));
    }

    Form Tree::form () const { return (*this)->form(); }
    Tree::Tree (Null n) : DPtr(new TreeDataT<Null>(n)) { }
    Tree::Tree (bool b) : DPtr(new TreeDataT<bool>(b)) { }
    Tree::Tree (int64 i) : DPtr(new TreeDataT<int64>(i)) { }
    Tree::Tree (float f) : DPtr(new TreeDataT<float>(f)) { }
    Tree::Tree (double d) : DPtr(new TreeDataT<double>(d)) { }
    Tree::Tree (String s) : DPtr(new TreeDataT<String>(s)) { }
    Tree::Tree (const Array& a) : DPtr(new TreeDataT<Array>(a)) { }
    Tree::Tree (Array&& a) : DPtr(new TreeDataT<Array>(a)) { }
    Tree::Tree (const Object& o) : DPtr(new TreeDataT<Object>(o)) { }
    Tree::Tree (Object&& o) : DPtr(new TreeDataT<Object>(o)) { }
    Tree::Tree (Path p) : DPtr(new TreeDataT<Path>(p)) { }
    Tree::operator Null () const { return (**this).as<Null>(); }
    Tree::operator bool () const { return (**this).as<bool>(); }
    Tree::operator int64 () const { return (**this).as<int64>(); }
    Tree::operator float () const {
        switch (form()) {
            case INTEGER: return static_cast<const TreeDataT<int64>&>(**this).it;
            case FLOAT: return static_cast<const TreeDataT<float>&>(**this).it;
            case DOUBLE: return static_cast<const TreeDataT<double>&>(**this).it;
            default: throw X::Wrong_Form(FLOAT, *this);
        }
    }
    Tree::operator double () const {
        switch (form()) {
            case INTEGER: return static_cast<const TreeDataT<int64>&>(**this).it;
            case FLOAT: return static_cast<const TreeDataT<float>&>(**this).it;
            case DOUBLE: return static_cast<const TreeDataT<double>&>(**this).it;
            default: throw X::Wrong_Form(DOUBLE, *this);
        }
    }
    Tree::operator String () const { return (**this).as<String>(); }
    Tree::operator const Array& () const { return (**this).as<Array>(); }
    Tree::operator const Object& () const { return (**this).as<Object>(); }
    Tree::operator Path () const { return (**this).as<Path>(); }

    Tree Tree::elem (size_t index) const {
        const Array& a = this->as<const Array&>();
        return a.at(index);
    }
    Tree Tree::attr (String name) const {
        const Object& o = this->as<const Object&>();
        for (auto& p : o)
            if (p.first == name)
                return p.second;
        throw X::Logic_Error("This tree has no attr with key \"" + name + "\"");
    }

    bool operator == (const Tree& a, const Tree& b) {
        if (a.form() != b.form()) return false;
        switch (a.form()) {
            case NULLFORM: return true;
            case BOOL: return bool(a) == bool(b);
            case INTEGER: return int64(a) == int64(b);
            case FLOAT: return float(a) == float(b);
            case DOUBLE: return double(a) == double(b);
            case STRING: return String(a) == String(b);
            case ARRAY: return a.as<const Array&>() == b.as<const Array&>();
            case OBJECT: return a.as<const Object&>() == b.as<const Object&>();
            case PATH: return a.as<Path>() == b.as<Path>();
            default: throw X::Corrupted_Tree(a);
        }
    }

    String Path::root () const { return (*this)->root(); }
    bool operator == (const PathRoot& a, const PathRoot& b) {
        return a.filename == b.filename;
    }
    bool operator == (const PathAttr& a, const PathAttr& b) {
        return a.left == b.left && a.name == b.name;
    }
    bool operator == (const PathElem& a, const PathElem& b) {
        return a.left == b.left && a.index == b.index;
    }
    bool operator == (const Path& a, const Path& b) {
        if (&*a == &*b) return true;
        if (!a || !b) return false;
        if (a->type() != b->type()) return false;
        switch (a->type()) {
            case ROOT: return static_cast<const PathRoot&>(*a)
                           == static_cast<const PathRoot&>(*b);
            case ATTR: return static_cast<const PathAttr&>(*a)
                           == static_cast<const PathAttr&>(*b);
            case ELEM: return static_cast<const PathElem&>(*a)
                           == static_cast<const PathElem&>(*b);
            default: throw X::Corrupted_Path(a);
        }
    }

    Path::Path (String filename) : DPtr(new PathRoot(filename)) { }
    Path::Path (Path left, String name) : DPtr(new PathAttr(left, name)) { }
    Path::Path (Path left, size_t index) : DPtr(new PathElem(left, index)) { }


    namespace X {
        Corrupted_Tree::Corrupted_Tree (Tree tree) :
            Corrupted("Corrupted tree: nonsensical form number " + std::to_string(tree.form())),
            tree(tree)
        { }
        Wrong_Form::Wrong_Form (Form form, Tree tree) :
            Logic_Error(
                "Form mismatch: Tried to use tree of form " + form_name(tree.form())
              + " as " + form_name(form)
            ), form(form), tree(tree)
        { }
        Corrupted_Path::Corrupted_Path (Path path) :
            Corrupted("Corrupted path: nonsensical path type number " + std::to_string(path->type())),
            path(path)
        { }
    }

} using namespace hacc;

HACCABLE(Tree) {
    name("hacc::Tree");
    to_tree([](const Tree& t){ return t; });
    prepare([](Tree& t, Tree tt){ t = tt; });
    fill([](Tree& t, Tree tt){ });
    finish ([](Tree& t){ });
}
