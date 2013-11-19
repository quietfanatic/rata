#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "../inc/integration.h"
#include "../../hacc/inc/common.h"
 // TODO: Windows support

static inline int builtin_chdir (const char* s) { return chdir(s); }

namespace util {

    std::string cwd () {
        char* wd = get_current_dir_name();
        if (wd == NULL) {
            throw hacc::X::Logic_Error(
                "Could not get working directory: "
                + std::to_string(errno) + " " + strerror(errno)
            );
        }
        std::string r = wd;
        free(wd);
        return r;
    }

    std::string my_dir (int argc, char** argv) {
        if (argc < 1) {
            throw hacc::X::Logic_Error("Program called with no argv[0]!?\n");
        }
        std::string me = (const char*)argv[0];
         // TODO: scan $PATH
        if (me.find('/') == std::string::npos
         && me.find('\\') == std::string::npos)
            return "";
        else return dirname(me);
    }

    bool is_absolute (std::string path) {
        if (path.empty()) return false;
        if (path[0] == '/') return true;
        if (path[0] == '\\') return true;
        return false;
    }

    std::string join_path (const std::vector<std::string>& segs) {
        if (segs.empty()) return "";
        std::string path;
        if (segs[0] != "/") path += segs[0];
        for (size_t i = 1; i < segs.size(); i++) {
            path += "/" + segs[i];
        }
        if (path.empty()) return "/";
        return path;
    }

    std::vector<std::string> split_path (std::string path) {
        std::vector<std::string> segs (1);
        if (is_absolute(path)) {
            segs[0] = "/";
        }
        for (auto c : path) {
            if (c == '/' || c == '\\') {
                if (!segs.back().empty()) {
                    segs.push_back("");
                }
            }
            else {
                segs.back() += c;
            }
        }
        if (segs.back().empty())
            segs.pop_back();
        return segs;
    }

    std::string basename (std::string p) {
        if (p.empty() || p == "/" || p == ".") return p;
        std::string ret;
        bool had_slash = false;
        for (auto c : p) {
            if (c == '/') {
                had_slash = true;
            }
            else if (had_slash) {
                ret = std::string(1, c);
                had_slash = false;
            }
            else {
                ret += c;
            }
        }
        return ret;
    }

    std::string dirname (std::string p) {
        if (p.empty() || p == "/" || p == ".") return p;
        std::string ret;
        std::string seg;
        bool had_slash = false;
        for (auto c : p) {
            if (c == '/') {
                had_slash = true;
            }
            else if (had_slash) {
                if (!ret.empty())
                    ret += "/";
                ret += seg;
                seg = std::string(1, c);
                had_slash = false;
            }
            else {
                seg += c;
            }
        }
        if (p[0] == '/') return "/" + ret;
        else if (ret.empty()) return ".";
        else return ret;
    }

    std::string canonicalize (std::string p) {
        return join_path(canonicalize(split_path(p)));
    }

    std::vector<std::string> canonicalize (const std::vector<std::string>& segs) {
        std::vector<std::string> ret;
        for (auto s : segs) {
            if (!s.empty() && s != ".") {
                if (s == ".." && !ret.empty()) {
                    ret.pop_back();
                }
                else {
                    ret.push_back(s);
                }
            }
        }
        if (ret.empty())
            ret.push_back(".");
        return ret;
    }

    std::string rel2abs (std::string path, std::string base) {
        if (is_absolute(path)) {
            return canonicalize(path);
        }
        else if (base.empty()) {
            return canonicalize(cwd() + '/' + path);
        }
        else {
            return canonicalize(base + '/' + path);
        }
    }

    std::string abs2rel (std::string path, std::string base) {
         // base was already defaulted to cwd()
        const std::vector<std::string>& psegs = canonicalize(split_path(path));
        const std::vector<std::string>& bsegs = canonicalize(split_path(base));
        std::vector<std::string> ret;
        bool branched = false;
        for (size_t i = 0; i < psegs.size(); i++) {
            if (i < bsegs.size()) {
                if (branched || psegs[i] != bsegs[i]) {
                    ret.insert(ret.begin(), "..");
                    ret.push_back(psegs[i]);
                    branched = true;
                }
            }
            else {
                ret.push_back(psegs[i]);
            }
        }
        return join_path(ret);
    }

    void chdir (std::string newdir) {
        if (builtin_chdir(newdir.c_str()) == -1) {
            throw hacc::X::Logic_Error(
                "Could not chdir(\"" + newdir +
                + "\"): " + std::to_string(errno)
                + " " + strerror(errno)
            );
        }
    }

}
