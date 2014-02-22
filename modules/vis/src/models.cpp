#include <sstream>
#include "../inc/models.h"
#include "../inc/common.h"
#include "../../hacc/inc/everything.h"
#include "../../util/inc/debug.h"

namespace vis {

    Logger model_logger ("models", false);

    void Skel::finish () {
        for (auto& seg : segs) {
            auto n_branches = seg.branches.size();
            for (auto& frame : seg.layout->frames) {
                if (frame.points.size() < n_branches) {
                    throw hacc::X::Logic_Error(
                        "Skeleton error: Frame " + frame.name
                      + " of Seg " + seg.name
                      + " doesn't have enough points (" + std::to_string(frame.points.size())
                      + " < " + std::to_string(n_branches) + ")"
                    );
                }
            }
            for (Skel::Seg* branch : seg.branches) {
                if (branch->parent) {
                    throw hacc::X::Logic_Error(
                        "Skeleton error: Seg " + branch->name
                      + " was claimed by multiple parents"
                    );
                }
                branch->parent = branch;
            }
        }
    }

    uint Skel::seg_index (Seg* p) {
        uint r = p - segs.data();
        if (r >= segs.size()) {
            std::ostringstream ss;
            ss << "Segment " << p << " doesn't belong to Skel " << this;
            throw std::logic_error(ss.str());
        }
        return r;
    }

    void Model::draw_seg (Skel::Seg* ss, Vec pos, bool fliph, bool flipv, float z) {
        Model::Seg* ms = &segs[skel->seg_index(ss)];
        if (!ms->pose) return;
         // Wishing for a boolean xor
        bool fh = ms->pose->fliph ? !fliph : fliph;
        bool fv = ms->pose->flipv ? !flipv : flipv;
        if (ms->skin) {
            for (Texture* tex : ms->skin->textures) {
                draw_frame(
                    ms->pose->frame, tex, pos,
                    Vec(fh?-1:1, fv?-1:1), z + ss->z_offset
                );
            }
        }
        for (Skel::Seg*& branch : ss->branches) {
            Vec pt = PX*ms->pose->frame->points[&branch - ss->branches.data()];
            if (fh) pt.x = -pt.x;
            if (fv) pt.y = -pt.y;
            draw_seg(branch, pos + pt, fliph, flipv, z);
        }
    }
    void Model::draw (Vec pos, bool fliph, bool flipv, float z) {
        if (!skel) {
            model_logger.log("Model has no associated skeleton");
            return;
        }
        model_logger.log("Drawing a model with %lu segs", skel->segs.size());
        draw_seg(skel->root, pos + skel->root_offset, fliph, flipv, z);
    }

    void Model::apply_pose (Pose* pose) {
        for (auto& app : pose->apps) {
            segs[skel->seg_index(app.target)].pose = &app;
        }
    }
    void Model::apply_skin (Skin* skin) {
        for (auto& app : skin->apps) {
            segs[skel->seg_index(app.target)].skin = &app;
        }
    }

    Model::Model (Skel* skel, Model::Seg* segs) : skel(skel), segs(segs) {
        for (size_t i = 0; i < skel->segs.size(); i++) {
            segs[i].pose = NULL;
            segs[i].skin = NULL;
        }
    }

} using namespace vis;

HACCABLE(Skel) {
    name("vis::Skel");
    attr("segs", member(&Skel::segs));
    attr("root", member(&Skel::root));
    attr("root_offset", member(&Skel::root_offset));
    finish([](Skel& skel){ skel.finish(); });
}

HACCABLE(Skel::Seg) {
    name("vis::Skel::Seg");
    attr("name", member(&Skel::Seg::name));
    attr("branches", member(&Skel::Seg::branches).optional());
    attr("layout", member(&Skel::Seg::layout));
    attr("z_offset", member(&Skel::Seg::z_offset).optional());
}

HACCABLE(Pose) {
    name("vis::Pose");
    delegate(member(&Pose::apps));
}

HACCABLE(Pose::App) {
    name("vis::Pose");
    elem(member(&Pose::App::target));
    elem(member(&Pose::App::frame));
    elem(member(&Pose::App::fliph).optional());
    elem(member(&Pose::App::flipv).optional());
}

HACCABLE(Skin) {
    name("vis::Skin");
    attr("apps", member(&Skin::apps));
}

HACCABLE(Skin::App) {
    name("vis::Skin::App");
    elem(member(&Skin::App::target));
    elem(member(&Skin::App::textures));
}

