
namespace draw {
	const char* fshader_src =
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
	GLint uniform_type;
	GLint uniform_tex;
	int draw_type = -1;
	void set_draw_type (int type) {
		if (draw_type != type)
			glUniform1i(uniform_type, draw_type = type);
	}

	void init () {
		GLint status;
		GLint len;
		 // Compile fragment shader
		GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
		len = strlen(fshader_src);
		glShaderSource(
			fshader, 1,
			&fshader_src, &len
		);
		glCompileShader(fshader);
		glGetShaderiv(fshader, GL_COMPILE_STATUS, &status);
		if (!status) {
			printf("Error: GL shader failed to compile...\n");
			GLint loglen;
			glGetShaderiv(fshader, GL_INFO_LOG_LENGTH, &loglen);
			GLchar log [loglen];
			glGetShaderInfoLog(fshader, loglen, NULL, log);
			puts(log);
		}
		 // Create GPU program
		GLuint program = glCreateProgram();
		glAttachShader(program, fshader);
		glLinkProgram(program);
		glValidateProgram(program);
		glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
		if (!status) {
			printf("Error: GL program failed to validate...\n");
			GLint loglen;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &loglen);
			GLchar log [loglen];
			glGetProgramInfoLog(program, loglen, NULL, log);
			puts(log);
		}
		 // Set up to use program
		glUseProgram(program);
		uniform_type = glGetUniformLocation(program, "type");
		uniform_tex = glGetUniformLocation(program, "tex");
		 // Initialize other gl things
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		unset_camera();
	}

	void start () {
		//glClear(GL_COLOR_BUFFER_BIT);
	}

	void finish () {
		 // Scale view
		if (window_scale > 1.0) {
			glDisable(GL_BLEND);
			glPixelZoom(window_scale, window_scale);
			glCopyPixels(0, 0, 320, 240, GL_COLOR);
			glPixelZoom(1.0, 1.0);
			glEnable(GL_BLEND);
		}
		 // Check for errors
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR) {
			printf("Error: GL error 0x%04x\n", err);
		}
	}

	void unset_camera () {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glTranslatef(-1, -1, 0);
		glScalef(1/10.0/window_scale, 1/7.5/window_scale, 1);
		glTranslatef(0.45*PX/2, 0.45*PX/2, 0);
	}

	void set_camera () {
		glTranslatef(
			- round(camera.x*UNPX)*PX + screen.x/2,
			- round(camera.y*UNPX)*PX + screen.y/2,
			0
		);
	}

	void vertex (Vec v) {
		glVertex2f(
			round(v.x*UNPX)*PX,
			round(v.y*UNPX)*PX
		);
	}

	void image_2 (img::Def* image, float tl, float tr, float tt, float tb, float x, float y, float iw, float ih) {
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
	void image (img::Def* image, Vec p, int sub, bool fliph, bool flipv) {
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
		image_2(image, tl, tr, tt, tb, x, y, iw, ih);
	}

	void rect (const Rect& r, Color color) {
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
	void line (Vec a, Vec b, Color color) {
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

	void circle (Vec p, float r, Color color) {
		set_draw_type(0);
		color.setGL();
		glBegin(GL_LINE_LOOP);
		for (uint i=0; i < 32; i++) {
			vertex(p + r * circle_points[i]);
		}
		glEnd();
	};


	void debug () {
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
						draw::line(
							o->pos + Vec(e->m_vertex1),
							o->pos + Vec(e->m_vertex2),
							0x00ff007f
						);
						if (mag2(rata->cursor_pos() - o->pos + Vec(e->m_vertex1)) < 1)
							draw::line(
								o->pos + Vec(e->m_vertex1),
								o->pos + Vec(e->m_vertex0) + Vec(3, 3)*PX,
								0xffff007f
							);
						if (mag2(rata->cursor_pos() - o->pos + Vec(e->m_vertex2)) < 1)
							draw::line(
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
						draw::circle(Vec(c->m_p)+o->pos, c->m_radius, color);
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
		}
		else { debug_path_pos = 0; }
	}
}


