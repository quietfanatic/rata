#include "../inc/common.h"
#include "../inc/haccable.h"

namespace hacc {

    namespace X {
        const char* Error::what () const noexcept {
            if (longmess.empty()) {
                longmess = mess;
                if (!filename.empty()) {
                    if (line) {
                        longmess += " at " + filename
                                     + " " + std::to_string(line)
                                     + ":" + std::to_string(col);
                    }
                    else {
                        longmess += " while processing " + filename;
                    }
                }
                else if (line) {
                    longmess += " at " + std::to_string(line) + ":" + std::to_string(col);
                }
            }
            return longmess.c_str();
        }
        static String combine_messes (const std::vector<std::exception_ptr>& errs) {
            String r = "Combo Error: [\n";
             // This is kind of a dumb way to mix error messages, but it's the
             //  only way to do it without requiring boilerplate everywhere.
            for (auto e : errs) {
                try { std::rethrow_exception(e); }
                catch (std::exception& e) {
                    r += "    " + String(e.what()) + "\n";
                }
            }
            return r + "]";
        }
        Combo_Error::Combo_Error (const std::vector<std::exception_ptr>& errs) :
            Error(combine_messes(errs)), errs(errs)
        { }
        Combo_Error::Combo_Error (std::vector<std::exception_ptr>&& errs) :
            Error(combine_messes(errs)), errs(errs)
        { }
    }

}

HACCABLE(hacc::Unknown) {
    name("hacc::Unknown");
}
