#include <stdio.h>
#include <stdlib.h>
#include "../inc/files.h"
#include "../inc/haccable_standard.h"

using namespace hacc;

HCB_INSTANCE(int32*)
HCB_INSTANCE(float*)

#include "../../tap/inc/tap.h"
tap::Tester files_tester ("hacc/files", [](){
    using namespace tap;
    plan(21);
    doesnt_throw([](){ set_file_logger([](String s){ diag(s.c_str()); }); }, "Can set a custom logger");
    FILE* f = fopen("../test/eight.hacc", "w");
    if (fclose(f) != 0) {
        BAIL_OUT("Failed to clobber ../test/eight.hacc");
        exit(1);
    }
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
    f = fopen("../test/eight.hacc", "r");
    if (!f) {
        BAIL_OUT("Failed to open a file we just ostensibly wrote to");
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);
    char* cs = (char*)malloc(size + 1);
    fread(cs, 1, size, f);
    cs[size] = 0;
    is((const char*)cs, "{ float:8~41000000 }\n", "File was saved with the correct contents");
    free(cs);
    fclose(f);
    doesnt_throw([](){ load(File("../test/pointer.hacc")); }, "We can load a file with a pointer");
    is(File("../test/pointer.hacc").data().type(), Type::CppType<int32*>(), "Pointer is loaded with right type");
    is( *(void**)File("../test/pointer.hacc").data().address(),
        File("../test/seven.hacc").data().address(),
        "Pointer is loaded with right address"
    );
    diag("The float is at %lu", (unsigned long)File("../test/eight.hacc").data().address());
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
    diag("The float is at %lu", (unsigned long)File("../test/eight.hacc").data().address());
    doesnt_throw([](){ save(File("../test/pointer2.hacc")); }, "Can save a file with a pointer");
    diag("The float is at %lu", (unsigned long)File("../test/eight.hacc").data().address());
    f = fopen("../test/pointer2.hacc", "r");
    if (!f) {
        BAIL_OUT("Failed to open a file we just ostensibly wrote to");
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    rewind(f);
    cs = (char*)malloc(size + 1);
    fread(cs, 1, size, f);
    cs[size] = 0;
    is((const char*)cs, "{ \"float*\":$(\"../test/eight.hacc\") }\n", "File was saved with the correct contents");
    free(cs);
    fclose(f);
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

});
