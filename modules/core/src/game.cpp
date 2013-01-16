
#include <stdlib.h>
#include "GL/glfw.h"

#include "../inc/game.h"
#include "../inc/input.h"
#include "../../hacc/inc/everything.h"

namespace core {

    template <void (Phase::* method )()>
    void all_phases () {
        for (Phase* p : game_phases())
            (p->*method)();
        for (Phase* p : draw_phases())
            (p->*method)();
    }

    void quit_game () {
        glfwTerminate();
        exit(0);
    }
    void set_video (uint scale) {
        glfwOpenWindow(320*scale, 240*scale, 8, 8, 8, 0, 0, 0, GLFW_WINDOW);
    }
    
    static bool initialized = false;

    void init () {
        if (initialized) return;
        initialized = true;
        glfwInit();
        set_video(2);
        all_phases<&Phase::init>();
    }

    void start () {
        try {
            init();
            all_phases<&Phase::start>();
            for (;;) {
                for (Phase* p : game_phases())
                    p->run();
                for (Phase* p : draw_phases())
                    p->run();
                glfwSwapBuffers();
                glfwSleep(1/60.0);
            }
        } catch (std::exception& e) {
            fprintf(stderr, "Game was aborted due to an exception: %s\n", e.what());
        }
    }

    void stop () {
        all_phases<&Phase::stop>();
    }

    Phase::Phase (std::vector<Phase*>& type, std::string order) : order(order) {
        for (auto i = type.begin(); i != type.end(); i++) {
            if (order < (*i)->order) {
                type.insert(i, this);
                return;
            }
        }
        type.push_back(this);
    }

    std::vector<Phase*>& game_phases () { static std::vector<Phase*> r; return r; }
    std::vector<Phase*>& draw_phases () { static std::vector<Phase*> r; return r; }

}

