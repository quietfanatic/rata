


const char* myshader =
	"uniform int type;\n"
	"uniform sampler2D tex;\n"
	"void main () {\n"
	"	if (type == 0) {\n"
	"		gl_FragColor = gl_Color;\n"
	"	} else {\n"
	"		gl_FragColor = texture2D(tex,gl_TexCoord[0].st);\n"
	"	}\n"
	"}\n"
;
GLint uniform_tex;
GLint uniform_type;

int draw_type = 2;
void set_draw_type (int type) {
	if (type != draw_type) {
		draw_type = type;
		glUniform1i(uniform_type, type);
	}
}

void init_shaders () {
	GLuint program = glCreateProgram();
	GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	const GLint len = strlen(myshader);
	glShaderSource(
		frag_shader, 1,
		&myshader, &len
	);
	glCompileShader(frag_shader);
	GLint compile_status;
	glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &compile_status);
	if (!compile_status) {
		printf("Error: GL shader failed to compile...\n");
	}
	printf("GL shader info:\n");
	GLint loglen;
	glGetShaderiv(frag_shader, GL_INFO_LOG_LENGTH, &loglen);
	GLchar log [loglen];
	glGetShaderInfoLog(frag_shader, loglen, NULL, log);
	puts(log);
	
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		printf("Error: GL error 0x%04x after compile\n", err);
	}
	glAttachShader(program, frag_shader);
	glLinkProgram(program);
	glUseProgram(program);
	while ((err = glGetError()) != GL_NO_ERROR) {
		printf("Error: GL error 0x%04x after use program\n", err);
	}
	uniform_type = glGetUniformLocation(program, "type");
	uniform_tex = glGetUniformLocation(program, "tex");
	while ((err = glGetError()) != GL_NO_ERROR) {
		printf("Error: GL error 0x%04x after get uniforms\n", err);
	}
	
	printf("GL program info:\n");
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &loglen);
	GLchar log2 [loglen];
	glGetProgramInfoLog(program, loglen, NULL, log2);
	puts(log);
}

void init_graphics () {
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	reset_screen();
	glfwSetTime(0);
	draw_latency = 0;
}

void reset_screen () {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glTranslatef(-1, -1, 0);
	glScalef(1/10.0/window_scale, 1/7.5/window_scale, 1);
	glTranslatef(0.45*PX/2, 0.45*PX/2, 0);
}

void camera_to_screen () {
	glTranslatef(
		- round(camera.x*UNPX)*PX + screen.x/2,
		- round(camera.y*UNPX)*PX + screen.y/2,
		0
	);
}

void debug_draw () {
	set_draw_type(0);
	if (debug_mode)
	for (Actor* a=active_actors; a; a = a->next_active) {
		if (a->has_body()) {
			Object* o = (Object*) a;
			for (b2Fixture* f = o->body->GetFixtureList(); f; f = f->GetNext())
			if (f->GetFilterData().categoryBits)
			switch (f->GetType()) {
				case (b2Shape::e_edge): {
					b2EdgeShape* e = (b2EdgeShape*)f->GetShape();
					draw_line(
						o->pos + Vec(e->m_vertex1),
						o->pos + Vec(e->m_vertex2),
						0x00ff007f
					);
					if (mag2(rata->cursor_pos() - o->pos + Vec(e->m_vertex1)) < 1)
						draw_line(
							o->pos + Vec(e->m_vertex1),
							o->pos + Vec(e->m_vertex0) + Vec(3, 3)*PX,
							0xffff007f
						);
					if (mag2(rata->cursor_pos() - o->pos + Vec(e->m_vertex2)) < 1)
						draw_line(
							o->pos + Vec(e->m_vertex2),
							o->pos + Vec(e->m_vertex3) - Vec(3, 3)*PX,
							0x0000ff7f
						);
					break;
				}
				case (b2Shape::e_polygon): {
					b2PolygonShape* p = (b2PolygonShape*)f->GetShape();					
					Color color = f->GetFilterData().categoryBits == 256 ? 0x0000ff4f : 0x00ff007f;
					color.setGL();
					glBegin(GL_LINE_LOOP);
					for (int i=0; i < p->m_vertexCount; i++) {
						vertex(o->pos + Vec(p->m_vertices[i]));
					}
					glEnd();
					break;
				}
				case (b2Shape::e_circle): {
					b2CircleShape* c = (b2CircleShape*)f->GetShape();
					Color color = f->GetFilterData().categoryBits == 256 ? 0x0000ff4f : 0x00ff007f;
					draw_circle(Vec(c->m_p)+o->pos, c->m_radius, color);
					break;
				}
				default: { }
			}
		}
		 // Debug draw rata path.
		if (mag2(rata->pos - oldratapos) > 0.2) {
			debug_path[debug_path_pos % debug_path_size] = rata->pos;
			uint8 whiteshift = rata->float_frames * 255.0 / (rata->stats.float_time*FPS);
			debug_path_color[debug_path_pos % debug_path_size] =
				whiteshift ? Color(255, whiteshift, whiteshift, 127) : 0x0000ff7f;
			oldratapos = rata->pos;
			debug_path_pos++;
		}

		uint i = debug_path_pos>=debug_path_size ? debug_path_pos-debug_path_size+1 : 0;
		glBegin(GL_LINE_STRIP);
		for (; i < debug_path_pos; i++) {
			debug_path_color[i % debug_path_size].setGL();
			vertex(debug_path[i % debug_path_size]);
		}
		glEnd();
		 // Debug draw Camera walls.
		Color(0x7f007f7f).setGL();
		glBegin(GL_LINES);
		for (uint i=0; i < current_room->n_walls; i++) {
			vertex(current_room->sides[i].a);
			vertex(current_room->sides[i].b);
			//vertex(bound_a(current_room->sides[i]).a);
			//vertex(bound_a(current_room->sides[i]).b);
			//vertex(bound_b(current_room->sides[i]).a);
			//vertex(bound_b(current_room->sides[i]).b);
		}
		glEnd();
		for (uint i=0; i < current_room->n_walls; i++)
		if (current_room->walls[i].r > 0) {
			draw_circle(
				current_room->walls[i].c,
				current_room->walls[i].r,
				0x7f007f7f
			);
		}
		 // Draw camera and focus
		Color(0x007f7f7f).setGL();
		glBegin(GL_LINE_LOOP);
			vertex(rata->cursor_pos() + Vec(-9, -6.5));
			vertex(rata->cursor_pos() + Vec(9, -6.5));
			vertex(rata->cursor_pos() + Vec(9, 6.5));
			vertex(rata->cursor_pos() + Vec(-9, 6.5));
		glEnd();
		for (uint i=0; i < MAX_ATTENTIONS; i++) {
			if (attention[i].priority == -1/0.0) break;
			glBegin(GL_LINE_LOOP);
				vertex(Vec(attention[i].range.l, attention[i].range.b));
				vertex(Vec(attention[i].range.r, attention[i].range.b));
				vertex(Vec(attention[i].range.r, attention[i].range.t));
				vertex(Vec(attention[i].range.l, attention[i].range.t));
			glEnd();
		}
		 // Draw debug points
		for (uint i=0; i < n_debug_points; i++) {
			draw_rect(Rect(debug_points[i] - Vec(1, 1)*PX, debug_points[i] + Vec(1, 1)*PX), 0xffff007f);
		}
		if (defined(debug_line.a))
			draw_line(debug_line.a, debug_line.b, 0x007f7f7f);
		draw_rect(Rect(oldfocus - Vec(1, 1)*PX, oldfocus + Vec(1, 1)*PX), 0x007f7f7f);
		//draw_rect(Rect(focus - Vec(1, 1)*PX, focus + Vec(1, 1)*PX), 0x0000ff7f);
		draw_rect(Rect(camera - Vec(1, 1)*PX, camera + Vec(1, 1)*PX), 0xff00007f);
	}
	else { debug_path_pos = 0; }
}

void finish_drawing () {
	reset_screen();
	 // Draw hud
	if (draw_hud) {
		(*draw_hud)();
	}
	 // Scale view
	if (window_scale > 1.0) {
		glDisable(GL_BLEND);
		glPixelZoom(window_scale, window_scale);
		glCopyPixels(0, 0, 320, 240, GL_COLOR);
		glPixelZoom(1.0, 1.0);
		glEnable(GL_BLEND);
	}
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		printf("Error: GL error 0x%04x\n", err);
	}
}




void vertex (Vec v) {
	glVertex2f(
		round(v.x*UNPX)*PX,
		round(v.y*UNPX)*PX
	);
}


void draw_image_2 (img::Def* image, float tl, float tr, float tt, float tb, float x, float y, float iw, float ih) {
//	glEnable(GL_TEXTURE_2D);
	set_draw_type(1);
	glBindTexture(GL_TEXTURE_2D, image->tex);
//	glColor4f(1, 1, 1, 1);
//	glUniform1i(uniform_tex, 0);
	glBegin(GL_QUADS);
		glTexCoord2f(tl, tb); vertex(Vec(x,       y      ));
		glTexCoord2f(tr, tb); vertex(Vec(x+iw*PX, y      ));
		glTexCoord2f(tr, tt); vertex(Vec(x+iw*PX, y+ih*PX));
		glTexCoord2f(tl, tt); vertex(Vec(x,       y+ih*PX));
	glEnd();
	dbg_draw("Drawing image at <%f v%f >%f ^%f\n", x, y, x+iw*PX, y+ih*PX);
}
void draw_image (img::Def* image, Vec p, int sub, bool fliph, bool flipv) {
	if (!image) return;
	sub %= image->numsubs();
	if (sub < 0) sub += image->numsubs();
	uint tw = image->tw;
	uint th = image->th;
	uint iw = image->w;
	uint ih = image->h;
	uint subx = (sub % (tw / iw));
	uint suby = (sub / (tw / iw));
	float x = p.x - (fliph ? iw-image->x : image->x)*PX;
	float y = p.y - (flipv ? image->y : ih-image->y)*PX;

	float tl = (subx*iw + iw*fliph)/(float)tw;
	float tr = (subx*iw + iw*!fliph)/(float)tw;
	float tt = (suby*ih + ih*flipv)/(float)th;
	float tb = (suby*ih + ih*!flipv)/(float)th;
	 // separated out for profiling
	draw_image_2(image, tl, tr, tt, tb, x, y, iw, ih);
}
void draw_rect (const Rect& r, Color color) {
	//glDisable(GL_TEXTURE_2D);
	set_draw_type(0);
	color.setGL();
	glRectf(
		round(r.l*UNPX)*PX,
		round(r.b*UNPX)*PX,
		round(r.r*UNPX)*PX,
		round(r.t*UNPX)*PX
	);
	dbg_draw("Drawing rect at <%f v%f >%f ^%f\n", r.l, r.b, r.r, r.t);
};
void draw_line (Vec a, Vec b, Color color) {
//	glDisable(GL_TEXTURE_2D);
	set_draw_type(0);
	color.setGL();
	glBegin(GL_LINES);
		vertex(a);
		vertex(b);
	glEnd();
};

Vec circle_points [32] = {
	Vec(cos(pi*0/16.0), sin(pi*0/16.0)),
	Vec(cos(pi*1/16.0), sin(pi*1/16.0)),
	Vec(cos(pi*2/16.0), sin(pi*2/16.0)),
	Vec(cos(pi*3/16.0), sin(pi*3/16.0)),
	Vec(cos(pi*4/16.0), sin(pi*4/16.0)),
	Vec(cos(pi*5/16.0), sin(pi*5/16.0)),
	Vec(cos(pi*6/16.0), sin(pi*6/16.0)),
	Vec(cos(pi*7/16.0), sin(pi*7/16.0)),
	Vec(cos(pi*8/16.0), sin(pi*8/16.0)),
	Vec(cos(pi*9/16.0), sin(pi*9/16.0)),
	Vec(cos(pi*10/16.0), sin(pi*10/16.0)),
	Vec(cos(pi*11/16.0), sin(pi*11/16.0)),
	Vec(cos(pi*12/16.0), sin(pi*12/16.0)),
	Vec(cos(pi*13/16.0), sin(pi*13/16.0)),
	Vec(cos(pi*14/16.0), sin(pi*14/16.0)),
	Vec(cos(pi*15/16.0), sin(pi*15/16.0)),
	Vec(cos(pi*16/16.0), sin(pi*16/16.0)),
	Vec(cos(pi*17/16.0), sin(pi*17/16.0)),
	Vec(cos(pi*18/16.0), sin(pi*18/16.0)),
	Vec(cos(pi*19/16.0), sin(pi*19/16.0)),
	Vec(cos(pi*20/16.0), sin(pi*20/16.0)),
	Vec(cos(pi*21/16.0), sin(pi*21/16.0)),
	Vec(cos(pi*22/16.0), sin(pi*22/16.0)),
	Vec(cos(pi*23/16.0), sin(pi*23/16.0)),
	Vec(cos(pi*24/16.0), sin(pi*24/16.0)),
	Vec(cos(pi*25/16.0), sin(pi*25/16.0)),
	Vec(cos(pi*26/16.0), sin(pi*26/16.0)),
	Vec(cos(pi*27/16.0), sin(pi*27/16.0)),
	Vec(cos(pi*28/16.0), sin(pi*28/16.0)),
	Vec(cos(pi*29/16.0), sin(pi*29/16.0)),
	Vec(cos(pi*30/16.0), sin(pi*30/16.0)),
	Vec(cos(pi*31/16.0), sin(pi*31/16.0)),
};


void draw_circle (Vec p, float r, Color color) {
	set_draw_type(0);
	color.setGL();
	glBegin(GL_LINE_LOOP);
	for (uint i=0; i < 32; i++) {
		vertex(p + r * circle_points[i]);
	}
	glEnd();
};




