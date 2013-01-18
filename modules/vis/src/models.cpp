
#include "../inc/models.h"
#include "../../hacc/inc/everything.h"

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

static ResourceGroup skeletons;
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
    attr("subimg_offset", member<Vec>(&SkinSegment::subimg_offset, Vec(0, 0)));
    attr("z_shift", member<float>(&SkinSegment::z_shift, 0.0));
    attr("with", member<SkinSegment*>(&SkinSegment::with, (SkinSegment*)NULL));
HCB_END(SkinSegment)

namespace vis {
    Skeleton::Skeleton (std::string name) : Resource(name) { hacc::update_from_file(*this, name); }
    Skeleton::~Skeleton () {
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
    bool Skeleton::reload () {
        hacc::update_from_file(*this, name);
        return true;
    }
}
