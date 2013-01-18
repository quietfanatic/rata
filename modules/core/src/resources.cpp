#include "../inc/resources.h"

void ResourceGroup::unload_all () {
    for (auto& p : loaded) {
        try {
            delete p.second;
        } catch (std::exception& e) {
            fprintf(stderr, "Problem unloading \"%s\": %s\n", p.first.c_str(), e.what());
        }
    }
    loaded.clear();
}
void ResourceGroup::reload_all () {
    for (auto& p : loaded) {
        try {
            p.second->reload();
        } catch (std::exception& e) {
            fprintf(stderr, "Failed to reload \"%s\": %s\n", p.first.c_str(), e.what());
        }
    }
}

HCB_BEGIN(ResourceGroup)
    get_id([](const ResourceGroup& v){ return v.name; });
    find_by_id([](std::string id){
        auto iter = ResourceGroup::all_groups().find(id);
        if (iter == ResourceGroup::all_groups().end())
            return (ResourceGroup*)NULL;
        else return iter->second;
    });
HCB_END(ResourceGroup)

struct ReloadAllCommand : Command {
    ResourceGroup* grp;
    void operator() () { if (grp) grp->reload_all(); }
};
HCB_BEGIN(ReloadAllCommand)
    base<Command>("reload_all");
    elem(member(&ReloadAllCommand::grp));
HCB_END(ReloadAllCommand)



