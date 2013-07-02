#include <sstream>
#include "../inc/common.h"

namespace hacc {

     // On OS X libgc will crash if not initted.
    struct GC_Initter {
        GC_Initter () { GC_INIT(); }
    } gc_initter;

    namespace X {
        const char* Error::what () const noexcept {
            if (longmess.empty()) {
                std::stringstream ss;
                ss << mess;
                if (!filename.empty()) {
                    if (line) {
                        ss << " at " << filename << " " << line << ":" << col;
                    }
                    else {
                        ss << " while processing " << filename;
                    }
                }
                else if (line) {
                    ss << " at " << line << ":" << col;
                }
                longmess = ss.str();
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
