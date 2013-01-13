#include "../inc/resources.h"

bool ResourceGroup::unload () {
    bool success = true;
    for (auto& p : loaded)
        success *= p.second->unload();
    if (success) loaded.clear();
    return success;
}
bool ResourceGroup::reload () {
    bool success = true;
    for (auto& p : loaded)
        success *= p.second->reload();
    return success;
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
    void operator() () { if (grp) grp->reload(); }
};
HCB_BEGIN(ReloadAllCommand)
    base<Command>("reload_all");
    elem(member(&ReloadAllCommand::grp));
HCB_END(ReloadAllCommand)



