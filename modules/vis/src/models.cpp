
#include "../inc/models.h"
#include "../inc/common.h"
#include "../../hacc/inc/everything.h"
#include "../../core/inc/phases.h"
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
        if (r < 0 || r >= segs.size()) {
            throw std::logic_error("A Segment was used with a Skeleton it doesn't belong to.");
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
        model_logger.log("Drawing a model with %lu segs", segs.size());
        draw_seg(skel->root, pos + skel->root_offset, fliph, flipv, z);
    }

    void Model::apply_skel (Skel* skel_) {
        skel = skel_;
        segs.resize(skel->segs.size());
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

    Model::Model () : skel(NULL), segs() { }
    Model::Model (Skel* skel) : skel(skel), segs(skel ? skel->segs.size() : 0) { }


    struct Model_Tester : Drawn<Sprites> {
        bool flip = false;
        Model model = Model(hacc::File("modules/ent/res/small.skel").data());

        Model_Tester () { }
        void finish () {
            model.apply_skin(hacc::File("modules/rata/res/rata-base.hacc").data().attr("skin"));
            model.apply_pose(hacc::File("modules/ent/res/small.hacc").data().attr("poses").attr("stand"));
            appear();
        }
        void Drawn_draw (Sprites) override {
            model.draw(Vec(10, 4), flip, false, 0.5);
        }
    };

} using namespace vis;

HCB_BEGIN(Skel)
    name("vis::Skel");
    attr("segs", member(&Skel::segs));
    attr("root", member(&Skel::root));
    attr("root_offset", member(&Skel::root_offset));
    finish([](Skel& skel){ skel.finish(); });
HCB_END(Skel)

HCB_BEGIN(Skel::Seg)
    name("vis::Skel::Seg");
    attr("name", member(&Skel::Seg::name));
    attr("branches", member(&Skel::Seg::branches).optional());
    attr("layout", member(&Skel::Seg::layout));
    attr("z_offset", member(&Skel::Seg::z_offset).optional());
HCB_END(Skel::Seg)

HCB_BEGIN(Pose)
    name("vis::Pose");
    delegate(member(&Pose::apps));
HCB_END(Pose)

HCB_BEGIN(Pose::App)
    name("vis::Pose");
    elem(member(&Pose::App::target));
    elem(member(&Pose::App::frame));
    elem(member(&Pose::App::fliph).optional());
    elem(member(&Pose::App::flipv).optional());
HCB_END(Pose::App)

HCB_BEGIN(Skin)
    name("vis::Skin");
    attr("apps", member(&Skin::apps));
HCB_END(Skin)

HCB_BEGIN(Skin::App)
    name("vis::Skin::App");
    elem(member(&Skin::App::target));
    elem(member(&Skin::App::textures));
HCB_END(Skin::App)

HCB_BEGIN(Model_Tester)
    name("vis::Model_Tester");
    finish([](Model_Tester& mt){ mt.finish(); });
HCB_END(Model_Tester)
