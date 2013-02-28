
namespace vis {
    
    struct Console, core::Stateful, core::Layer, core::Key_Listener, core::Char_Listener, core::Receives_Output {
        bool is_active = false;
        std::string contents = console_help();
        std::string cli = "";
        uint cli_pos = 0;

        Font* font = NULL;
        char trigger = '`';

        void receive_output (std::string message) {
            console_contents += message;
        }

        bool hear_key (int keycode, int action) {
            if (is_active)
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
                            if (cli_pos < cli_contents.size()) cli_pos++;
                            break;
                        }
                        case GLFW_KEY_HOME: {
                            cli_pos = 0;
                            break;
                        }
                        case GLFW_KEY_END: {
                            cli_pos = cli_contents.size();
                            break;
                        }
                        case GLFW_KEY_ENTER: {
                            print_to_console(cli_contents + "\n");
                            if (!cli_contents.empty()) {
                                Command* cmd;
                                bool good_command = false;
                                try {
                                    hacc::update_from_string(cmd, "[" + cli_contents + "]");
                                    good_command = true;
                                } catch (hacc::Error& e) {
                                    print_to_console("Error parsing command: " + std::string(e.what()) + "\n");
                                } catch (std::exception& e) {
                                    print_to_console("Error generating command: " + std::string(e.what()) + "\n");
                                }
                                if (good_command) {
                                    try {
                                        (*cmd)();
                                    } catch (std::exception& e) {
                                        print_to_console("Error: The command threw an exception: " + std::string(e.what()) + "\n");
                                    }
                                }
                                if (cmd) delete cmd;
                            }
                            cli_contents = "";
                            cli_pos = 0;
                            break;
                        }
                        case GLFW_KEY_BACKSPACE: {
                            if (cli_pos) {
                                cli_contents = cli_contents.substr(0, cli_pos - 1)
                                             + cli_contents.substr(cli_pos);
                                cli_pos--;
                            }
                            break;
                        }
                        case GLFW_KEY_DEL: {
                            if (cli_pos < cli_contents.size()) {
                                cli_contents = cli_contents.substr(0, cli_pos)
                                             + cli_contents.substr(cli_pos + 1);
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
                cli_contents = cli_contents.substr(0, cli_pos)
                             + std::string(1, code)
                             + cli_contents.substr(cli_pos);
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
/*        void run () {
            static auto glBindVertexArray = glproc<void (GLuint)>("glBindVertexArray");
            static auto glEnableVertexAttribArray = glproc<void (GLuint)>("glEnableVertexAttribArray");
            static auto glUniform2f = glproc<void (GLint, GLfloat, GLfloat)>("glUniform2f");
            if (console_font && core::console_is_active) {
                 // We came after the graffiti layer
                Vec pts [4];
                pts[0] = Vec(0, 0);
                pts[1] = Vec(20, 0);
                pts[2] = Vec(20, 15);
                pts[3] = Vec(0, 15);
                graffiti_pos(Vec(0, 0));
                draw_primitive(GL_QUADS, 4, pts, 0x000000cf);
            }
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            text_program->use();
            glBindVertexArray(0);
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glUniform2f(text_program_camera_pos, 10.0, 7.5);
            if (console_font && core::console_is_active) {
                glUniform2f(text_program_camera_pos, 10.0, 7.5);
                float chars_available = floor(320 / console_font->width);
                float cli_lines = 1 + floor(core::cli_contents.size() / chars_available);
                draw_text(core::console_contents, console_font, Vec(1, cli_lines * console_font->line_height)*PX, Vec(1, -1), 0x00ff00ff, 20);
                draw_text(core::cli_contents, console_font, Vec(1, 0)*PX, Vec(1, -1), 0x7fff00ff, 20);
                if (core::frame_number % 40 < 20) {
                    float cursor_pos = fmod(core::cli_pos, chars_available);
                    draw_text("_", console_font, Vec(cursor_pos * console_font->width, -1)*PX, Vec(1, -1), 0xffffffff);
                }
            }
        }*/
    };
}

using namespace vis;
HCB_BEGIN(Console)
    type_name("vis::Console");
    base<core::Stateful>("Console");
    attr("font", member(&Console::font)(required));
    attr("trigger", member(&Console::trigger)(optional));
HCB_END(Console)

