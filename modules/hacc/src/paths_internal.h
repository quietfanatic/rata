#ifndef HAVE_HACC_PATHS_INTERNAL_H
#define HAVE_HACC_PATHS_INTERNAL_H
#include "hacc/inc/files.h"

#include "hacc/inc/tree.h"

namespace hacc {

    enum PathType {
        ROOT,
        ATTR,
        ELEM
    };

    struct PathData : DPtee {
        virtual PathType type () const = 0;
        virtual String root () const = 0;
        virtual Reference to_reference (Pointer) const = 0;
    };
    struct PathRoot : PathData {
        String filename;
        PathRoot (String f) : filename(f) { }
        PathType type () const { return ROOT; }
        String root () const { return filename; }
        Reference to_reference (Pointer root) const {
            if (root) return root;
            else return File(filename).data();
        }
    };
    struct PathAttr : PathData {
        Path left;
        String name;
        PathAttr (Path l, String n) : left(l), name(n) { }
        PathType type () const { return ATTR; }
        String root () const { return left.root(); }
        Reference to_reference (Pointer root) const {
            return left->to_reference(root).attr(name);
        }
    };
    struct PathElem : PathData {
        Path left;
        size_t index;
        PathElem (Path l, size_t i) : left(l), index(i) { }
        PathType type () const { return ELEM; }
        String root () const { return left.root(); }
        Reference to_reference (Pointer root) const {
            return left->to_reference(root).elem(index);
        }
    };

}

#endif
