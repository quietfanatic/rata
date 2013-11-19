#ifndef HAVE_UTIL_INTEGRATION_H
#define HAVE_UTIL_INTEGRATION_H
#include <string>
#include <vector>

 // This currently just does file path manipulation.
 // NOTE: in the interest of easy portability, this treats both '/' and '\' as
 //  path separators, and canonicalizes both to '/'.

 // These functions are not guaranteed to be maximally efficient, but at least
 //  they don't do weird things like spawn new processes.

namespace util {
     // Gets the current working directory, or throws a
     //  hacc::X::Logic_Error if there's an error.
    std::string cwd ();
     // Attempts to return the directory that contains the executable.
     //  Returns empty string if it can't.  This may be a relative path!
    std::string my_dir (int argc, char** argv);
     // Quick and simple
    bool is_absolute (std::string);
     // No canonicalization in these functions
    std::string join_path (const std::vector<std::string>&);
    std::vector<std::string> split_path (std::string);
     // These are intended to match the POSIX API.  No canonicalization.
    std::string basename (std::string);
    std::string dirname (std::string);
     // Factor out ., .., extra slashes, etc.  Does not follow symlinks
     //  or check for existence.
    std::string canonicalize (std::string);
    std::vector<std::string> canonicalize (const std::vector<std::string>&);
     // Both of these also canonicalize the path.
     // The default base is the current working directory, but
     //  cwd() is only called if necessary.
     // These do not check that the file and all containing directories
     //  actually exist.
    std::string rel2abs (std::string, std::string base = "");
    std::string abs2rel (std::string, std::string base = cwd());
     // This actually does something active.  It throws an exception if we
     //  can't chdir.
    void chdir (std::string);
}

#endif
