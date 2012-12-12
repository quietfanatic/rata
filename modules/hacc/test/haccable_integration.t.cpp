
#include "stdio.h"
#include "../inc/haccable_integration.h"

int k = 0;
int* kp = NULL;
int* kp2 = NULL;

#include "../../tap/inc/tap.h"
tap::Tester haccable_integration_tester ("haccable_integration", [](){
    using namespace hacc;
    using namespace tap;
    plan(22);

    is(to_string<int>(12), String("12"), "to_string");
    is(from_string<int>("90"), (int)90, "from_string");
    is(string_to<int>("445"), (int)445, "string_to");
    is(string_from<int>(-34), String("-34"), "string_from");
    update_from_string(&k, String("992"));
    is(k, 992, "update_from_string");
    kp = new_from_string<int>(String("104"));
    is(kp ? *kp : -99, 104, "new_from_string");
    free(kp);
    kp = NULL;
    kp = string_to_new<int>(String("-12341234"));
    is(kp ? *kp : -99, -12341234, "string_to_new");
    free(kp);

    remove("integration_test_file");
    doesnt_throw([](){hacc_to_file(Hacc(100), "integration_test_file");}, "hacc_to_file and...");
    is_catch([](){return (int)hacc_from_file("integration_test_file").get_integer();}, (int)100, "... hacc_from_file");
    doesnt_throw([](){file_from_hacc("integration_test_file", Hacc(999));}, "file_from_hacc and...");
    is_catch([](){return (int)file_to_hacc("integration_test_file").get_integer();}, 999, "... file_to_hacc");

    doesnt_throw([](){to_file<int>(13, "integration_test_file");}, "to_file and...");
    is_catch([](){return (int)from_file<int>("integration_test_file");}, (int)13, "... from_file");
    doesnt_throw([](){file_from<int>("integration_test_file", -15);}, "file_from and...");
    is_catch([](){return (int)file_to<int>("integration_test_file");}, -15, "... file_to");
    is_catch([](){
        update_from_file(&k, "integration_test_file");
        return k;
    }, -15, "update_from_file");
    is_catch([](){
        kp = new_from_file<int>("integration_test_file");
        return kp ? *kp : -99;
    }, -15, "new_from_file");
    free(kp);
    is_catch([](){
        kp2 = file_to_new<int>("integration_test_file");
        return kp2 ? *kp2 : -99;
    }, -15, "file_to_new");
    free(kp2);
    
});
