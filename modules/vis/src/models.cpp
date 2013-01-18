
#include "../inc/models.h"
#include "../../hacc/inc/everything.h"
#include "../inc/vis.h"
#include "../../core/inc/game.h"

using namespace vis;

HCB_BEGIN(Pose)
    type_name("vis::Pose");
    attr("name", member(&Pose::name));
    attr("subimg", member(&Pose::subimg));
    attr("joints", member(&Pose::joints));
HCB_END(Pose)

HCB_BEGIN(Segment)
    type_name("vis::Segment");
    attr("name", member(&Segment::name));
    attr("subs", member(&Segment::subs));
    attr("poses", member(&Segment::poses));
HCB_END(Segment)

HCB_BEGIN(Preset)
    type_name("vis::Preset");
    attr("name", member(&Preset::name));
    attr("segment_poses", member(&Preset::segment_poses));
HCB_END(Preset)

static ResourceGroup skeletons ("skeletons");
HCB_BEGIN(Skeleton)
    type_name("vis::Skeleton");
    attr("segments", member(&Skeleton::segments));
    attr("root", member(&Skeleton::root));
    attr("root_pos", member(&Skeleton::root_pos));
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

namespace vis {
    Skeleton::Skeleton () { }
    Skeleton::Skeleton (std::string name) : Resource(name) { hacc::update_from_file(*this, name); }
    Skeleton::~Skeleton () {
         // If we don't have a name, we're not canonical.
        if (name.empty()) return;
        if (segments.empty()) return;
         // There may be duplicate pose lists.
        void* freed [segments.size()];
        uint freedi = 0;
        for (Segment& seg : segments) {
            for (uint i = 0; i < freedi; i++)
                if (freed[i] == (void*)seg.poses.p) goto next_seg;
            freed[freedi++] = (void*)seg.poses.p;
            delete seg.poses;
            next_seg: { }
        }
    }
    void Skeleton::reload () {
        Skeleton&& tmp = hacc::value_from_file<Skeleton>(name);
        segments = std::move(tmp.segments);
        root = std::move(tmp.root);
        root_pos = std::move(tmp.root_pos);
        presets = std::move(tmp.presets);
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
        throw std::logic_error("Skeleton Segment not found.");
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
        Vec posesubpos;
        for (SkinSegment* ss = skin; ss; ss = ss->covers) {
            subimg.pos = posesubpos + ss->subimg_offset;
            draw_img(ss->image, &subimg, mpos + pos, pose->fliph?!fliph:fliph, pose->flipv?!flipv:flipv);
        }
    }
    void Model::reposition_segment (Segment* segment, Vec pos) {
        ModelSegment& ms = model_segments.at(skeleton->offset_of_segment(segment));
        ms.pos = pos;
        if (!ms.pose) return;
        for (uint i = 0; i < segment->subs.size(); i++) {
            reposition_segment(segment->subs[i], pos + ms.pose->joints[i]);
        }
    }
    void Model::apply_pose (Segment* segment, Pose* pose) {
        model_segments.at(skeleton->offset_of_segment(segment)).pose = pose;
        reposition_segment(segment, Vec(0, 0));
    }
    void Model::apply_preset (Preset* preset, Pose* pose) {
        for (auto& s_p : preset->segment_poses)
            model_segments.at(skeleton->offset_of_segment(s_p.first)).pose = s_p.second;
        reposition();
    }
    void Model::draw (Vec pos, bool fliph, bool flipv) {
        for (ModelSegment& ms : model_segments)
            ms.draw(pos, fliph, flipv);
    }



    struct Model_Tester : core::Layer {
        Model_Tester () : core::Layer("E.M", "model_tester", false) { }
        void run () { }
    } model_tester;


}
