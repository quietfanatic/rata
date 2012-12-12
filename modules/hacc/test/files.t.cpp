
#include <stdio.h>
#include "../inc/files.h"

hacc::String test = "My totally awesome data goes here.\n";
hacc::String test2 = "This data is even awesomer.\n";
hacc::String test3 = "There's no limit to the awesomeness.\n";
#include "../../tap/inc/tap.h"
tap::Tester hacc_files_tester ("hacc-files", [](){
    using namespace hacc;
    using namespace tap;
    plan(8);
     // This should be in modules/hacc/tmp
    remove("test_file");
    string_to_file(test, "test_file");
    is(file_to_string("test_file"), test, "Round-tripping a string through a file.");
    doesnt_throw([](){file_from_string("test_file", test2);}, "Overwrite by default.");
    is(string_from_file("test_file"), test2, "Indeed, after overwriting, the file contains the new string.");
    doesnt_throw([](){string_to_file(test3, "test_file", hf::append_file);}, "Trying to append doesn't cause an error.");
    is(file_to_string("test_file"), test2 + test3, "Appending to a file works.");
    throws<hacc::Error>([](){string_to_file(test3, "test_file", hf::dont_overwrite_file);}, "Throw an error when dont_overwrite_file is set and the file exists.");
    is(string_from_file("test_file"), test2 + test3, "The file still has its old data.");
    throws<hacc::Error>([](){file_to_string("this file is not there");}, "Throw an error when reading from a non-existing file.");
});
