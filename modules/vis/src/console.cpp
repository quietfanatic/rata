
#include "../../core/inc/phases.h"
#include "../../core/inc/input.h"
#include "../../core/inc/commands.h"
#include "../../core/inc/game.h"
#include "../../core/inc/opengl.h"
#include "../inc/graffiti.h"
#include "../inc/text.h"

namespace vis {

    using namespace core;

    struct Console : Layer, Key_Listener, Char_Listener, core::Console {
        bool is_active = false;
        std::string contents = console_help();
        std::string cli = "";
        uint cli_pos = 0;

        uint history_index = core::command_history.size();
        std::string stash_cli;

        bool ignore_a_trigger = false;

        Font* font = NULL;
        char trigger = '`';

        Console () : Layer("Z.M"), Key_Listener("A"), Char_Listener("A") { }

        void Console_print (std::string message) {
            contents += message;
        }

        bool hear_key (int keycode, int action) {
            if (is_active) {
                if (action == GLFW_PRESS) {
                    switch (keycode) {
                        case GLFW_KEY_ESC: {
                            exit_console();
                            break;
                        }
                        case GLFW_KEY_LEFT: {
                            if (cli_pos) cli_pos--;
                            break;
                        }
                        case GLFW_KEY_RIGHT: {
                            if (cli_pos < contents.size()) cli_pos++;
                            break;
                        }
                        case GLFW_KEY_UP: {
                            if (history_index > 0) {
                                if (history_index == core::command_history.size()) {
                                    stash_cli = cli;
                                }
                                history_index--;
                                cli = core::command_history[history_index];
                                cli_pos = cli.size();
                            }
                            break;
                        }
                        case GLFW_KEY_DOWN: {
                            if (history_index < core::command_history.size()) {
                                history_index++;
                                if (history_index == core::command_history.size())
                                    cli = stash_cli;
                                else
                                    cli = core::command_history[history_index];
                                cli_pos = cli.size();
                            }
                            break;
                        }
                        case GLFW_KEY_HOME: {
                            cli_pos = 0;
                            break;
                        }
                        case GLFW_KEY_END: {
                            cli_pos = contents.size();
                            break;
                        }
                        case GLFW_KEY_ENTER: {
                            print_to_console(cli + "\n");
                            command_from_string(cli);
                            cli = "";
                            cli_pos = 0;
                            stash_cli = "";
                            history_index = core::command_history.size();
                            break;
                        }
                        case GLFW_KEY_BACKSPACE: {
                            if (cli_pos) {
                                cli = cli.substr(0, cli_pos - 1)
                                    + cli.substr(cli_pos);
                                cli_pos--;
                            }
                            break;
                        }
                        case GLFW_KEY_DEL: {
                            if (cli_pos < cli.size()) {
                                cli = cli.substr(0, cli_pos)
                                    + cli.substr(cli_pos + 1);
                            }
                            break;
                        }
                        default: break;
                    }
                    return true;
                }
            }
            else {
                if (action == GLFW_PRESS) {
                    if (keycode == trigger) {
                        if (font) {
                            ignore_a_trigger = true;
                            enter_console();
                            return true;
                        }
                    }
                }
            }
            return false;
        }
        bool hear_char (int code, int action) {
            if (!is_active) return false;
            if (ignore_a_trigger && code == trigger) {
                ignore_a_trigger = false;
                return false;
            }
            if (glfwGetKey(GLFW_KEY_LCTRL) || glfwGetKey(GLFW_KEY_RCTRL)) {
                switch (code) {
                    case 'd': {
                        exit_console();
                        break;
                    }
                    case 'c': {
                        cli = "";
                        cli_pos = 0;
                        break;
                    }
                    case 'u': {
                        cli = cli.substr(cli_pos);
                        cli_pos = 0;
                        break;
                    }
                    default: break;
                }
            }
            else if (code < 256) {
                cli = cli.substr(0, cli_pos)
                    + std::string(1, code)
                    + cli.substr(cli_pos);
                cli_pos++;
            }
            return true;
        }
        void enter_console () {
            is_active = true;
        }
        void exit_console () {
            is_active = false;
        }
        void Layer_start () { }
        void Layer_run () {
            if (!font || !is_active) return;
             // Darken background
            Vec pts [4];
            pts[0] = Vec(0, 0);
            pts[1] = Vec(20, 0);
            pts[2] = Vec(20, 15);
            pts[3] = Vec(0, 15);
            graffiti_pos(Vec(0, 0));
            draw_primitive(GL_QUADS, 4, pts, 0x000000cf);
             // Draw console
            Vec cli_size = draw_text(cli + " ", font, Vec(1, 0)*PX, Vec(1, -1), 0x7fff00ff, 20);
            Vec cursor_pos = get_glyph_pos(cli, font, cli_pos, Vec(1, -1), 20);
            if (core::frames_drawn % 40 < 20) {
                draw_text("_", font, Vec(1*PX + cursor_pos.x, cli_size.y - cursor_pos.y - font->line_height*PX - 2*PX), Vec(1, -1), 0xffffffff);
            }
            draw_text(contents, font, Vec(1*PX, cli_size.y), Vec(1, -1), 0x00ff00ff, 20);
        }
    };
} using namespace vis;

HCB_BEGIN(vis::Console)
    name("vis::Console");
    attr("font", member(&vis::Console::font));
    attr("trigger", member(&vis::Console::trigger).optional());
HCB_END(vis::Console)

