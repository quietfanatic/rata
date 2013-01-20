
#include "../inc/models.h"
#include "../inc/sprites.h"
#include "../../hacc/inc/everything.h"
#include "../../core/inc/game.h"
#include "../../core/inc/commands.h"

using namespace vis;

HCB_BEGIN(Skel)
    type_name("vis::Skel");
    attr("segs", member(&Skel::segs));
    attr("root", member(&Skel::segs));
    attr("root_offset", member(&Skel::segs));
    attr("poses", member(&Skel::poses));
    finish([](Skel& skel) { skel.finish(); }
HCB_END(Skel)

HCB_BEGIN(Skel::Seg)
    type_name("vis::Skel::Seg");
    attr("name", member(&Skel::Seg::name));
    attr("branches", member(&Skel::Seg::branches, optional<decltype(((Skel::Seg*)NULL)->branches)>()));
    attr("layout", member(&Skel::Seg::layout));
    attr("z_offset", member(&Skel::Seg::z_offset, def(0.f)));
HCB_END(Skel::Seg)

HCB_BEGIN(Pose)
    type_name("vis::Pose");
    elem(member(&Pose::name));
    elem(member(&Pose::apps));
HCB_END(Pose)

HCB_BEGIN(Pose::App)
    type_name("vis::Pose");
    elem(member(&Pose::App::target));
    elem(member(&Pose::App::frame));
    elem(member(&Pose::App::fliph, def(false)));
    elem(member(&Pose::App::flipv, def(false)));
HCB_END(Pose::App)

HCB_BEGIN(Skin)
    type_name("vis::Skin");
    attr("apps", member(&Skin::apps));
HCB_END(Skin)

HCB_BEGIN(Skin::App)
    type_name("vis::Skin::App");
    elem(member(&Skin::App::target));
    elem(member(&Skin::App::textures));
HCB_END(Skin::app)

namespace vis {

    void Skel::finish () {
        for (auto& seg : segs) {
            auto n_branches = seg.branches.size();
            for (auto& frame : *seg.layout->frames) {
                if (frame.points.size() < n_subs) {
                    fprintf(stderr, "Skeleton error: Frame %s of Seg %s doesn't have enough points (%lu < %lu).\n",
                        frame.name.c_str(), seg.name.c_str(), frame.points.size(), n_branches
                    );
                    throw std::logic_error("Skeleton contained errors.");
                }
            }
            for (Skel::Seg* branch : branches) {
                if (branch->parent) {
                    fprintf(stderr, "Skeleton error: Seg %s was claimed by multiple parents.\n", branch->name.c_str())
                    throw std::logic_error("Skeleton contained errors.");
                }
                branch->parent = branch;
            }
        }
    }

    Skel::Seg* Skel::seg_named (std::string name) {
        for (auto& p : segs)
            if (p.name == name) return &p;
        throw std::logic_error("Skel Segment not found.");
    }

    Pose* Skel::pose_named (std::string name) {
        for (auto& pose : *poses)
            if (pose.name == name) return &pose;
        throw std::logic_error("Skel Pose not found.");
    }

    uint Skel::seg_index (Seg* p) {
        uint r = p - segs.data();
        if (r < 0 || r >= segs.size()) {
            throw std::logic_error("A Segment was used with a Skeleton it doesn't belong to.");
        }
        return r;
    }

    void Model::Seg::draw (Skel::Seg* seg, Vec mpos, bool fh, bool fv, float z) {
        if (!skin) return;
        if (!pose) return;
        for (Texture* tex : skin->textures) {
            draw_sprite(
                pose->frame, tex, mpos + pos,
                pose->fliph?!fh:fh, pose->flipv?!fv:fv,
                z + seg->z_offset + pose->z_offset
            );
        }
    }
    void Model::draw (Vec pos, bool fliph, bool flipv, float z) {
        if (!skel) return;
        uint nsegs = segs.size();
        for (uint i = 0; i < nsegs; i++)
            segss[i].draw(&skel->segs[i], pos, fliph, flipv, z);
    }

    Model::Model () : skeleton(NULL), model_segments() { }
    Model::Model (Skeleton* skel) : skeleton(skel), model_segments(skel ? skel->segments.size() : 0) { }

    struct Model_Test : core::Layer {
        Model_Test () : core::Layer("E.M", "model_test", false) { }
        Model model;
        void run () {
            model.draw(Vec(10, 4));
        }
        void init () {
            model = Model(hacc::reference_file<Skel>("modules/rata/res/rata.skel"));
            model.apply_skin(hacc::reference_file<Skin>("modules/rata/res/rata.skin"));
            model.apply_pose(model.skel->pose_named("stand"));
        }
    } model_tester;

}

struct MT_Load_Command : Command {
    Skel* skel;
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
    command_description<MT_Skin_Command>("Apply a skin to the model_test layer");
    elem(member(&MT_Skin_Command::skin));
HCB_END(MT_Skin_Command)

struct MT_Pose_Command : Command {
    std::string name;
    void operator() () {
        model_tester.model.apply_pose(model_tester.model.skel->pose_named(name));
    }
};

HCB_BEGIN(MT_Pose_Command)
    base<Command>("mt_pose");
    command_description<MT_Preset_Command>("Apply a pose to the model_test layer by name");
    elem(member(&MT_Preset_Command::name));
HCB_END(MT_Preset_Command)

