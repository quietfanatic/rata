#include "hacc/inc/files.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hacc/inc/haccable_standard.h"
#include "hacc/inc/strings.h"
#include "tap/inc/tap.h"
using namespace hacc;
using namespace tap;

HCB_INSTANCE(int32*)
HCB_INSTANCE(float*)

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


Tester files_tester ("hacc/files", [](){
    plan(26);

    remove("../test/eight.hacc");
    remove("../test/pointer2.hacc");

    doesnt_throw([](){ set_file_logger([](String s){ diag(s.c_str()); }); }, "Can set a custom logger");
    ok(!File("../test/seven.hacc").loaded(), "File is not loaded before load() is called on it");

    doesnt_throw([](){ load(File("../test/seven.hacc")); }, "We can call load()");
    ok(File("../test/seven.hacc").loaded(), "File is loaded when load() is called on it");

    if (is(File("../test/seven.hacc").data().type(), Type::CppType<int32>(), "Loaded file preserves type")) {
        is(*(int32*)File("../test/seven.hacc").data().address(), 7, "Loaded file preserves value");
    }
    else {
        diag(File("../test/seven.hacc").data().type().name().c_str());
        fail("Loaded file preserves value - failed because the type part failed");
    }

    ok(File("../test/eight.hacc", Dynamic::New<float>(8.0)).loaded(), "Creating new file works");

    doesnt_throw([](){ save(File("../test/eight.hacc")); }, "We can call save()");
    is(slurp("../test/eight.hacc"), String("{ float:8~41000000 }\n"), "File was saved with the correct contents");

    doesnt_throw([](){ load(File("../test/pointer.hacc")); }, "We can load a file with a pointer");
    is(File("../test/pointer.hacc").data().type(), Type::CppType<int32*>(), "Pointer is loaded with right type");
    is( *(void**)File("../test/pointer.hacc").data().address(),
        File("../test/seven.hacc").data().address(),
        "Pointer is loaded with right address"
    );

    doesnt_throw([](){
        File("../test/pointer2.hacc", Dynamic::New<float*>(
            (float*)File("../test/eight.hacc").data().address()
        ));
    }, "Creating file with pointer");
    is(
        *(void**)File("../test/pointer2.hacc").data().address(),
        File("../test/eight.hacc").data().address(),
        "Pointer address was not changed during save"
    );
    doesnt_throw([](){ save(File("../test/pointer2.hacc")); }, "Can save a file with a pointer");
    is(slurp("../test/pointer2.hacc"), "{ \"float*\":$(\"../test/eight.hacc\") }\n", "File was saved with the correct contents");

    throws<X::Unload_Would_Break>([](){
        unload(File("../test/seven.hacc"));
    }, "Can't unload a file if there are references to it");
    doesnt_throw([](){
        unload(std::vector<File>({File("../test/seven.hacc"), File("../test/pointer.hacc")}));
    }, "Can unload a file and the file that references it simultaneously");
    ok(!File("../test/seven.hacc").loaded(), "File is marked as unloaded");
    doesnt_throw([](){
        load(File("../test/pointer.hacc"));
    }, "Can load a file that was unloaded");
    ok(File("../test/seven.hacc").loaded(), "Depended-upon files are automatically loaded");
    string_to_file("{ float:9.0 }", "../test/eight.hacc");
    doesnt_throw([](){
        reload(File("../test/eight.hacc"));
    }, "Can reload a file that is loaded");
    is(*(float*)File("../test/eight.hacc").data().address(), 9.0f, "Data was updated during reload");
    is(
        *(void**)File("../test/pointer2.hacc").data().address(),
        File("../test/eight.hacc").data().address(),
        "External pointer to reloaded file was updated"
    );
    string_to_file("{ uint32:9 }", "../test/eight.hacc");
    throws<X::Reload_Would_Break_Type>([](){
        reload(File("../test/eight.hacc"));
    }, "Refuse to reload file if a path whose type has changed has an external reference");
    doesnt_throw([](){
        file_transaction([](){
            reload(File("../test/eight.hacc"));
            unload(File("../test/pointer2.hacc"));
        });
    }, "Can reload if the external pointer's file is being unloaded");


    remove("../test/eight.hacc");
    remove("../test/pointer2.hacc");

});
