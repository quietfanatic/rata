#ifdef HEADER

struct Imgset {
	GLuint tex = 0;
	CStr filename;
	Vec size;
	uint n_variants;
	VArray<VArray<Vec>> pts;  // Not the same two dimensions
	CE Imgset (CStr filename, Vec size, uint n_variants, VArray<VArray<Vec>> pts) :
		filename(filename),
		size(size),
		n_variants(n_variants),
		pts(pts)
	{ }
	void load () {
		tex = SOIL_load_OGL_texture(filename, 4, tex, 0);
		if (!tex) {
			printf("Error: failed to load image %s: %s\n", filename, SOIL_last_result());
		}
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	void draw (Vec pos, int pose = 0, int variant = 0, bool fliph = 0, bool flipv = 0);
};

namespace draw {
bool check_error (CStr when = "");
void set_video ();
void init ();
void load_images ();
void start ();
void mode ();
void finish ();
}

#include "imgs.cpp"

#else

namespace draw {
bool check_error (CStr when) {
	GLenum err;
	bool whoops = false;
	while ((err = glGetError()) != GL_NO_ERROR) {
		whoops = true;
		printf("Error: GL error 0x%04x %s\n", err, when);
	};
	return whoops;
}

CStr vs_tex =
	"#version 120\n"
	"void main () {\n"
	"	gl_Position = ftransform();\n"
	"	gl_TexCoord[0].xy = gl_MultiTexCoord0.xy / gl_MultiTexCoord0.zw;\n"
//	"	gl_TexCoord[0].zw = vec2(0, 0);\n"
	"}\n";
CStr fs_tex =
	"#version 120\n"
	"uniform sampler2D tex;\n"
	"void main () {\n"
	"	gl_FragColor = texture2D(tex,gl_TexCoord[0].xy);\n"
	"	gl_FragDepth = gl_FragColor.a;\n"
	"}\n";
CStr vs_color =
	"#version 120\n"
	"void main () {\n"
	"	gl_Position = ftransform();\n"
	"}\n";
CStr fs_color =
	"#version 120\n"
	"void main () {\n"
	"	gl_FragColor = gl_Color;\n"
	"}\n";
GLuint program_tex;
GLint uniform_tex;
GLuint program_color;

void mode (GLuint program) {
	static GLuint cur = 0xffffffff;
	if (program != cur) {
		glUseProgram(program);
		cur = program;
	}
}

GLuint make_shader (GLenum type, CStr src) {
	GLuint shader = glCreateShader(type);
	const GLint len = strlen(src);
	glShaderSource(shader, 1, &src, &len);
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!status) {
		printf("Error: a shader failed to compile...\n");
		GLint loglen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &loglen);
		GLchar log [loglen];
		glGetShaderInfoLog(shader, loglen, NULL, log);
		puts(log);
	}
	return shader;
}
GLuint make_program (CStr vs, CStr fs) {
	GLuint program = glCreateProgram();
	glAttachShader(program, make_shader(GL_VERTEX_SHADER, vs));
	glAttachShader(program, make_shader(GL_FRAGMENT_SHADER, fs));
	glLinkProgram(program);
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status) {
		printf("Error: a program failed to link...\n");
		GLint loglen;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &loglen);
		GLchar log [loglen];
		glGetProgramInfoLog(program, loglen, NULL, log);
		puts(log);
	}
	return program;
}

void set_video () {
	glfwOpenWindow(
		320*window_scale, 240*window_scale,
		8, 8, 8, 0,  // r g b a
		8, 0,  // depth stencil
		GLFW_WINDOW
	);
	glfwSwapInterval(1);
}

void init () {
	glfwInit();
	set_video();
	program_tex = make_program(vs_tex, fs_tex);
	program_color = make_program(vs_color, fs_color);
	if (check_error("after compiling program")) exit(1);
	glUseProgram(program_tex);
	uniform_tex = glGetUniformLocation(program_tex, "tex");
	glUniform1i(uniform_tex, 0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GREATER);
}

void load_images () {
	for (uint i = 0; i < imgsets.n; i++) {
		imgsets[i].load();
	}
}

void start () {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glTranslatef(-1, -1, 0);
	glScalef(1/10.0/window_scale, 1/7.5/window_scale, 1);
	glTranslatef(0.45*PX/window_scale, 0.45*PX/window_scale, 0);
}


void finish () {
	check_error();
}

}

void Imgset::draw (Vec pos, int pose, int variant, bool fliph, bool flipv) {
	draw::mode(draw::program_tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	Vec ctr = pts[pose][0];
	Rect r {
		fliph ? pos.x + ctr.x - size.x : pos.x - ctr.x,
		flipv ? pos.y + ctr.y - size.y : pos.y - ctr.y,
		fliph ? pos.x + ctr.x : pos.x - ctr.x + size.x,
		flipv ? pos.y + ctr.y : pos.x - ctr.x + size.y,
	};
	glBegin(GL_QUADS);
		glTexCoord4f(variant+ fliph, pose+!flipv, n_variants, pts.n); glVertex2f(r.l, r.b);
		glTexCoord4f(variant+!fliph, pose+!flipv, n_variants, pts.n); glVertex2f(r.r, r.b);
		glTexCoord4f(variant+!fliph, pose+ flipv, n_variants, pts.n); glVertex2f(r.r, r.t);
		glTexCoord4f(variant+ fliph, pose+ flipv, n_variants, pts.n); glVertex2f(r.l, r.t);
	glEnd();
}

#endif

