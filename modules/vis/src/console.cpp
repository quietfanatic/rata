
#include "../../core/inc/phases.h"
#include "../../core/inc/state.h"
#include "../../core/inc/input.h"
#include "../../core/inc/commands.h"
#include "../../core/inc/game.h"
#include "../inc/shaders.h"
#include "../inc/graffiti.h"
#include "../inc/text.h"

namespace vis {

    using namespace core;
    
    struct Console : Stateful, Layer, Key_Listener, Char_Listener, Receives_Output {
        bool is_active = false;
        std::string contents = console_help();
        std::string cli = "";
        uint cli_pos = 0;

        Font* font = NULL;
        char trigger = '`';

        Console () : Layer("Z.M"), Key_Listener("A"), Char_Listener("A") { }

        void receive_output (std::string message) {
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
                }
            }
            else {
                if (action == GLFW_PRESS) {
                    if (keycode == trigger) {
                        if (font)
                            enter_console();
                    }
                }
            }
            return true;
        }
        bool hear_char (int code, int action) {
            if (!is_active) return false;
            if (code < 256) {
                cli = cli.substr(0, cli_pos)
                    + std::string(1, code)
                    + cli.substr(cli_pos);
                cli_pos++;
                return true;
            }
            return false;
        }
        void enter_console () {
            is_active = true;
        }
        void exit_console () {
            is_active = false;
        }
        void start () { }
        void run () {
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
            float chars_available = floor(320 / font->width);
            float cli_lines = 1 + floor(cli.size() / chars_available);
            draw_text(contents, font, Vec(1, cli_lines * font->line_height)*PX, Vec(1, -1), 0x00ff00ff, 20);
            draw_text(cli, font, Vec(1, 0)*PX, Vec(1, -1), 0x7fff00ff, 20);
            if (core::frame_number % 40 < 20) {
                float cursor_pos = fmod(cli_pos, chars_available);
                draw_text("_", font, Vec(cursor_pos * font->width, -1)*PX, Vec(1, -1), 0xffffffff);
            }
        }
    };
}

using namespace vis;
HCB_BEGIN(Console)
    type_name("vis::Console");
    base<core::Stateful>("Console");
    attr("font", member(&Console::font)(required));
    attr("trigger", member(&Console::trigger)(optional));
HCB_END(Console)

