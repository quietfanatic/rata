#include "../inc/documents.h"
#include "../inc/files.h"

#include "../../tap/inc/tap.h"

tap::Tester documents_tester ("hacc/documents", [](){
    using namespace hacc;
    using namespace tap;
    plan(3);
    doesnt_throw([](){ load(File("../test/document.hacc")); }, "Can load a document");
    is(File("../test/document.hacc").data().attr("asdf").type(), Type::CppType<int32>(), "Document object has right type");
    is(*(int32*)File("../test/document.hacc").data().attr("asdf").address(), (int32)50, "Document object has right value");
});
