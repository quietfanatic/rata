
#include <stdio.h>

#include "../inc/strings.h"
#include "../../tap/inc/tap.h"

void hacc_string_test (hacc::String from, hacc::String to) {
    using namespace hacc;
    using namespace tap;
    Hacc* tree = hacc_from_string(from);
    const char* name = (escape_string(from) + " -> " + escape_string(to)).c_str();
    if (tree->form == ERROR) {
        fail(name);
        printf(" # Parse failed: %s\n", tree->error->what());
    }
    else is(hacc_to_string(tree), to, name);
}

tap::Tester hacc_strings_tester ("hacc-strings", [](){
    using namespace tap;
    plan(46);
     printf(" # Bools\n");  // 2
    hacc_string_test("true", "true");
    hacc_string_test("false", "false");
     printf(" # Null\n");  // 1
    hacc_string_test("null", "null");
     printf(" # Integers\n");  // 8
    hacc_string_test("1", "1");
    hacc_string_test("5425432", "5425432");
    hacc_string_test("\t 5425432 \n", "5425432");
    hacc_string_test("-532", "-532");
    hacc_string_test("+54", "54");
    hacc_string_test("0x7f", "127");
    hacc_string_test("-0x80", "-128");
    hacc_string_test("+0x100", "256");
     printf(" # Floats\n");  // 6
    hacc_string_test("1~3f800000", "1~3f800000");
    hacc_string_test("1.0~3f800000", "1~3f800000");
    hacc_string_test("1.0", "1~3ff0000000000000");
    hacc_string_test("~3f800000", "1~3f800000");
    hacc_string_test("~3ff0000000000000", "1~3ff0000000000000");
    hacc_string_test("2.0", "2~4000000000000000");
    hacc_string_test("0.5", "0.5~3fe0000000000000");
     printf(" # Strings\n");  // 5
    is(hacc::escape_string("\"\\\b\f\n\r\t"), "\\\"\\\\\\b\\f\\n\\r\\t", "hacc::escape_string does its job");
    hacc_string_test("\"asdfasdf\"", "\"asdfasdf\"");
    hacc_string_test("\"\"", "\"\"");
    hacc_string_test("\"\\\"\\\\\\b\\f\\n\\r\\t\"", "\"\\\"\\\\\\b\\f\\n\\r\\t\"");
    hacc_string_test("asdf", "\"asdf\"");
     printf(" # Arrays\n");  // 8
    hacc_string_test("[]", "[]");
    hacc_string_test("[1]", "[1]");
    hacc_string_test("[1, 2, 3]", "[1 2 3]");
    hacc_string_test("[ 1, 2, 3 ]", "[1 2 3]");
    hacc_string_test("[, 1 2,,,, 3,]", "[1 2 3]");
    hacc_string_test("[~3f800000, -45, \"asdf]\", null]", "[1~3f800000 -45 \"asdf]\" null]");
    hacc_string_test("[[[][]][[]][][][][[[[[[]]]]]]]", "[[[] []] [[]] [] [] [] [[[[[[]]]]]]]");
    hacc_string_test("[1, 2, [3, 4, 5], 6, 7]", "[1 2 [3 4 5] 6 7]");
     printf(" # Objects\n");  // 7
    hacc_string_test("{}", "{}");
    hacc_string_test("{\"a\": 1}", "{ a:1 }");
    hacc_string_test("{a: 1}", "{ a:1 }");
    hacc_string_test("{a: 1, b: 2, ccc: 3}", "{ a:1 b:2 ccc:3 }");
    hacc_string_test("{ , a: -32 b:\"sadf\" ,,,,,,,c:null,}", "{ a:-32 b:\"sadf\" c:null }");
    hacc_string_test("{\"\\\"\\\\\\b\\f\\n\\r\\t\": null}", "{ \"\\\"\\\\\\b\\f\\n\\r\\t\":null }");
    hacc_string_test("{a: {b: {c: {} d: {}} e: {}}}", "{ a:{ b:{ c:{} d:{} } e:{} } }");
     printf(" # Arrays and Objects\n");  // 2
    hacc_string_test("[{a: 1, b: []} [4, {c: {d: []}}]]", "[{ a:1 b:[] } [4 { c:{ d:[] } }]]");
    hacc_string_test("{a: []}", "{ a:[] }");
     printf(" # Vars\n");  // 2
    hacc_string_test("$thing_3432", "$thing_3432");
    hacc_string_test("$\"stringish\\nid\"", "$\"stringish\\nid\"");
     printf(" # Assignments\n");  // 3
    hacc_string_test("$one=1", "$one=1");
    hacc_string_test("$two = 2", "$two=2");
    hacc_string_test("{ak: $ai = \"as\" bk: $\"bi\" = \"bs\"}", "{ ak:$ai=\"as\" bk:$bi=\"bs\" }");
     printf(" # Misc\n");  // 1
    hacc_string_test("{ things: [ {test_actor: {}} ] }", "{ things:[{ test_actor:{} }] }");
    
});



