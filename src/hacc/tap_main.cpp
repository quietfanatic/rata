
 // Link this with an object that has defined a Tester.
 // Don't link with anything else.

namespace tap {
    extern void run_all_testers ();
}

int main () {
    tap::run_all_testers();
    return 0;
}



