#include <typeinfo>
#include <typeindex>
#include "../inc/annotations.h"

namespace util {

    typedef std::unordered_map<std::type_index, std::unordered_map<std::type_index, void*>> Annotations;
    Annotations& annotations () {
        static Annotations r;
        return r;
    }

    void set_annotation (const std::type_info& target, const std::type_info& a, void* dat) {
        annotations()[target].emplace(a, dat);
    }
    void* get_annotation (const std::type_info& target, const std::type_info& a) {
        auto& mid = annotations()[target];
        auto iter = mid.find(a);
        if (iter != mid.end()) {
            return iter->second;
        }
        else {
            return nullptr;
        }
    }
    void* annotation (const std::type_info& target, const std::type_info& a, void* (* gen )()) {
        auto& p = annotations()[target][a];
        if (!p) p = gen();
        return p;
    }

}
