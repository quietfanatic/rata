
#include "../inc/models.h"
#include "../../hacc/inc/everything.h"
#include "../inc/vis.h"
#include "../../core/inc/game.h"

using namespace vis;

HCB_BEGIN(Pose)
    type_name("vis::Pose");
    attr("name", member(&Pose::name));
    attr("subimg", member(&Pose::subimg));
    attr("joints", member(&Pose::joints, optional<decltype(((Pose*)NULL)->joints)>()));
HCB_END(Pose)

HCB_BEGIN(Segment)
    type_name("vis::Segment");
    attr("name", member(&Segment::name));
    attr("subs", member(&Segment::subs));
    attr("poses", member(&Segment::poses));
     // Find a segment by skeleton:segment
     //  e.g. &"modules/rata/res/rata-skeleton.hacc:head"
    find_by_id([](std::string id){
        size_t colonpos = id.find(':');
        if (colonpos == std::string::npos)
            return (Segment*)NULL;
        Skeleton* skel = hacc::find_by_id<Skeleton>(id.substr(0, colonpos));
        if (!skel) return (Segment*)NULL;
        try {
            return skel->segment_named(id.substr(colonpos + 1));
        } catch (std::logic_error& e) {
            return (Segment*)NULL;
        }
    });
HCB_END(Segment)

HCB_BEGIN(Preset)
    type_name("vis::Preset");
    attr("name", member(&Preset::name));
    attr("segment_poses", member(&Preset::segment_poses));
    find_by_id([](std::string id){
        size_t colonpos = id.find(':');
        if (colonpos == std::string::npos)
            return (Preset*)NULL;
        Skeleton* skel = hacc::find_by_id<Skeleton>(id.substr(0, colonpos));
        if (!skel) return (Preset*)NULL;
        try {
            return skel->preset_named(id.substr(colonpos + 1));
        } catch (std::logic_error& e) {
            return (Preset*)NULL;
        }
    });
HCB_END(Preset)

static ResourceGroup skeletons ("skeletons");
HCB_BEGIN(Skeleton)
    type_name("vis::Skeleton");
    attr("segments", member(&Skeleton::segments));
    attr("root", member(&Skeleton::root));
    attr("root_pos", member(&Skeleton::root_pos));
    attr("pose_lists", member(&Skeleton::pose_lists));
    attr("presets", member(&Skeleton::presets));
    resource_haccability<Skeleton, &skeletons>();
HCB_END(Skeleton)

HCB_BEGIN(SkinSegment)
    type_name("vis::SkinSegment");
    attr("image", member(&SkinSegment::image));
    attr("subimg_offset", member(&SkinSegment::subimg_offset, def(Vec(0, 0))));
    attr("z_shift", member(&SkinSegment::z_shift, def(0.f)));
    attr("covers", member(&SkinSegment::covers, def((SkinSegment*)NULL)));
HCB_END(SkinSegment)

static ResourceGroup skins ("skins");
HCB_BEGIN(Skin)
    type_name("vis::Skin");
    resource_haccability<Skin, &skins>();
    attr("segments", (member(&Skin::segments)));
HCB_END(Skin)

namespace vis {
    Skeleton::Skeleton () { }
    Skeleton::Skeleton (std::string name) : Resource(name) { hacc::update_from_file(*this, name); }
    Skeleton::~Skeleton () { /* Don't have pointer funkiness any more due to the pose_lists attr */ }
    void Skeleton::reload () {
        Skeleton&& tmp = hacc::value_from_file<Skeleton>(name);
        segments = std::move(tmp.segments);
        root = std::move(tmp.root);
        root_pos = std::move(tmp.root_pos);
        presets = std::move(tmp.presets);
        pose_lists = std::move(tmp.pose_lists);
    }

    Pose* Segment::pose_named (std::string name) {
        for (auto& pose : *poses)
            if (pose.name == name) return &pose;
        throw std::logic_error("Segment Pose not found.");
    }

    Segment* Skeleton::segment_named (std::string name) {
        for (auto& p : segments)
            if (p.name == name) return &p;
        throw std::logic_error("Skeleton Segment not found.");
    }
    Preset* Skeleton::preset_named (std::string name) {
        for (auto& p : presets)
            if (p.name == name) return &p;
        throw std::logic_error("Skeleton Preset not found.");
    }
    uint Skeleton::offset_of_segment (Segment* p) {
        uint r = p - segments.data();
        if (r < 0 || r >= segments.size()) {
            throw std::logic_error("A Segment was used with a Skeleton it doesn't belong to.");
        }
        return r;
    }

    void ModelSegment::draw (Vec mpos, bool fliph, bool flipv) {
        if (!skin) return;
        if (!pose) return;
        SubImg subimg = pose->subimg;
        Vec posesubpos = subimg.pos;
        for (SkinSegment* ss = skin; ss; ss = ss->covers) {
            subimg.pos = posesubpos + ss->subimg_offset;
            draw_img(ss->image, &subimg, mpos + pos, pose->fliph?!fliph:fliph, pose->flipv?!flipv:flipv);
        }
    }
    void Model::reposition_segment (Segment* segment, Vec pos) {
        if (!skeleton) return;
        ModelSegment& ms = model_segments.at(skeleton->offset_of_segment(segment));
        ms.pos = pos;
        if (!ms.pose) return;
        for (uint i = 0; i < segment->subs.size(); i++) {
            reposition_segment(segment->subs[i], pos + ms.pose->joints[i]);
        }
    }
    void Model::apply_pose (Segment* segment, Pose* pose) {
        if (!skeleton) return;
        model_segments.at(skeleton->offset_of_segment(segment)).pose = pose;
        reposition_segment(segment, Vec(0, 0));
    }
    void Model::apply_preset (Preset* preset) {
        if (!skeleton) return;
        for (auto& s_p : preset->segment_poses)
            model_segments.at(skeleton->offset_of_segment(s_p.first)).pose = s_p.second;
        reposition();
    }
    void Model::apply_segment_skin (Segment* segment, SkinSegment* ss) {
        if (!skeleton) return;
        model_segments.at(skeleton->offset_of_segment(segment)).skin = ss;
    }
    void Model::apply_skin (Skin* skin) {
        if (!skeleton) return;
        for (auto& s_ss : skin->segments)
            model_segments.at(skeleton->offset_of_segment(s_ss.first)).skin = &s_ss.second;
    }
    void Model::draw (Vec pos, bool fliph, bool flipv) {
        for (ModelSegment& ms : model_segments)
            ms.draw(pos, fliph, flipv);
    }

    Model::Model () : skeleton(NULL), model_segments() { }
    Model::Model (Skeleton* skel) : skeleton(skel), model_segments(skel ? skel->segments.size() : 0) { }

    void Skin::reload () { segments = hacc::value_from_file<Skin>(name).segments; }
    Skin::Skin () { }
    Skin::Skin (std::string name) : Resource(name) { hacc::update_from_file(*this, name); }

    struct Model_Test : core::Layer {
        Model_Test () : core::Layer("E.M", "model_test", false) { }
        Model model;
        void run () {
            model.draw(Vec(10, 4));
        }
        void init () {
            model = Model(hacc::require_id<Skeleton>("modules/rata/res/rata-skeleton.hacc"));
            model.apply_skin(hacc::require_id<Skin>("modules/rata/res/rata-skin.hacc"));
            model.apply_preset(model.skeleton->preset_named("idle"));
        }
    } model_tester;

}

struct MT_Load_Command : Command {
    Skeleton* skel;
    void operator() () {
        model_tester.model = Model(skel);
    }
};

HCB_BEGIN(MT_Load_Command)
    base<Command>("mt_load");
    command_description<MT_Load_Command>("Load a skeleton into the model_test layer.\nUnload with 'mt_load null'");
    elem(member(&MT_Load_Command::skel));
HCB_END(MT_Load_Command)

struct MT_Skin_Command : Command {
    Skin* skin;
    void operator() () {
        model_tester.model.apply_skin(skin);
    }
};

HCB_BEGIN(MT_Skin_Command)
    base<Command>("mt_skin");
    command_description<MT_Skin_Command>("Load a skin into the model_test layer");
    elem(member(&MT_Skin_Command::skin));
HCB_END(MT_Skin_Command)

struct MT_Preset_Command : Command {
    Preset* preset;
    void operator() () {
        model_tester.model.apply_preset(preset);
    }
};

HCB_BEGIN(MT_Preset_Command)
    base<Command>("mt_preset");
    command_description<MT_Preset_Command>("Load a pose preset into the model_test layer");
    elem(member(&MT_Preset_Command::preset));
HCB_END(MT_Preset_Command)

struct MT_Dump_Command : Command {
    void operator() () {
        if (!model_tester.model.skeleton) return;
        fputs(hacc::string_from(*model_tester.model.skeleton).c_str(), stdout);
    }
};

HCB_BEGIN(MT_Dump_Command)
    base<Command>("mt_dump");
    empty();
HCB_END(MT_Dump_Command)

