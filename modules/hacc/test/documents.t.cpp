#include <string.h>
#include "../../tap/inc/tap.h"
#include "../inc/documents.h"
#include "../inc/files.h"

using namespace hacc;
using namespace tap;

static void with_file (const char* filename, const char* mode, const Func<void (FILE*)>& f) {
    FILE* file = fopen(filename, mode);
    if (!file) BAIL_OUT((
        "Failed to open \"" + String(filename)
      + "\" with mode \"" + String(mode)
      + "\": " + strerror(errno)
    ).c_str());
    f(file);
    if (fclose(file) != 0) BAIL_OUT((
        "Failed to close \"" + String(filename)
      + "\": " + strerror(errno)
    ).c_str());
}
static String slurp (const char* filename) {
    String r;
    with_file(filename, "r", [&](FILE* f){
        fseek(f, 0, SEEK_END);
        size_t size = ftell(f);
        rewind(f);
        char* cs = (char*)malloc(size);
        fread(cs, 1, size, f);
        r = String(cs, size);
        free(cs);
    });
    return r;
}
static void clobber (const char* filename) {
    with_file(filename, "w", [](FILE*){});
}

tap::Tester documents_tester ("hacc/documents", [](){
    plan(3);
    doesnt_throw([](){ load(File("../test/document.hacc")); }, "Can load a document");
    is(File("../test/document.hacc").data().attr("asdf").type(), Type::CppType<int32>(), "Document object has right type");
    is(*(int32*)File("../test/document.hacc").data().attr("asdf").address(), (int32)50, "Document object has right value");
});
