
#include <stdio.h>

#include "../inc/strings.h"
#include "../../tap/inc/tap.h"

void tst (hacc::String from, hacc::String to) {
    using namespace hacc;
    using namespace tap;
    String name = escape_string(from) + " -> " + escape_string(to);
    try {
        Tree tree = tree_from_string(from, "current file");
        is(tree_to_string(tree, "current file"), to, name.c_str());
    }
    catch (std::exception& e) {
        fail(name.c_str());
        printf(" # Parse failed: %s\n", e.what());
    }
}

tap::Tester tree_strings_tester ("hacc/strings", [](){
    using namespace tap;
    plan(47);
     printf(" # Bools\n");  // 2
    tst("true", "true");
    tst("false", "false");
     printf(" # Null\n");  // 1
    tst("null", "null");
     printf(" # Integers\n");  // 8
    tst("1", "1");
    tst("5425432", "5425432");
    tst("\t 5425432 \n", "5425432");
    tst("-532", "-532");
    tst("+54", "54");
    tst("0x7f", "127");
    tst("-0x80", "-128");
    tst("+0x100", "256");
     printf(" # Floats\n");  // 7
    tst("1~3f800000", "1~3f800000");
    tst("1.0~3f800000", "1~3f800000");
    tst("1.0", "1~3ff0000000000000");
    tst("~3f800000", "1~3f800000");
    tst("~3ff0000000000000", "1~3ff0000000000000");
    tst("2.0", "2~4000000000000000");
    tst("0.5", "0.5~3fe0000000000000");
     printf(" # Strings\n");  // 6
    is(hacc::escape_string("\"\\\b\f\n\r\t"), "\\\"\\\\\\b\\f\\n\\r\\t", "hacc::escape_string does its job");
    tst("asdfasdf", "asdfasdf");
    tst("\"\"", "\"\"");
    tst("\"\\\"\\\\\\b\\f\\n\\r\\t\"", "\"\\\"\\\\\\b\\f\\n\\r\\t\"");
    tst("asdf", "asdf");
    tst("std::string", "std::string");
     printf(" # Arrays\n");  // 8
    tst("[]", "[]");
    tst("[1]", "[1]");
    tst("[1, 2, 3]", "[1 2 3]");
    tst("[ 1, 2, 3 ]", "[1 2 3]");
    tst("[, 1 2,,,, 3,]", "[1 2 3]");
    tst("[~3f800000, -45, \"asdf]\", null]", "[1~3f800000 -45 \"asdf]\" null]");
    tst("[[[][]][[]][][][][[[[[[]]]]]]]", "[[[] []] [[]] [] [] [] [[[[[[]]]]]]]");
    tst("[1, 2, [3, 4, 5], 6, 7]", "[1 2 [3 4 5] 6 7]");
     printf(" # Objects\n");  // 7
    tst("{}", "{}");
    tst("{\"a\": 1}", "{ a:1 }");
    tst("{a: 1}", "{ a:1 }");
    tst("{a: 1, b: 2, ccc: 3}", "{ a:1 b:2 ccc:3 }");
    tst("{ , a: -32 b:\"sadf\" ,,,,,,,c:null,}", "{ a:-32 b:sadf c:null }");
    tst("{\"\\\"\\\\\\b\\f\\n\\r\\t\": null}", "{ \"\\\"\\\\\\b\\f\\n\\r\\t\":null }");
    tst("{a: {b: {c: {} d: {}} e: {}}}", "{ a:{ b:{ c:{} d:{} } e:{} } }");
     printf(" # Arrays and Objects\n");  // 2
    tst("[{a: 1, b: []} [4, {c: {d: []}}]]", "[{ a:1 b:[] } [4 { c:{ d:[] } }]]");
    tst("{a: []}", "{ a:[] }");
     printf(" # Paths\n");  // 4
    tst("$(\"asdf\")", "$(\"asdf\")");
    tst("$.attr", "$.attr");
    tst("$[4]", "$[4]");
    tst("$(\"asdf\").attr[7].a2", "$(\"asdf\").attr[7].a2");
     printf(" # Misc\n");  // 1
    tst("{ things: [ {test_actor: {}} ] }", "{ things:[{ test_actor:{} }] }");
     printf(" # Refs\n");  // 1
    tst("[$ref=4 5 $ref]", "[4 5 4]");
    
});



