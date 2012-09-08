
#ifdef HEADER

struct Serializer {
	virtual bool writing () = 0;
	virtual void ser (int8&) = 0;
	void ser (uint8& x) { ser((int8&)x); }
	virtual void ser (int16&) = 0;
	void ser (uint16& x) { ser((int16&)x); }
	virtual void ser (int32&) = 0;
	void ser (uint32& x) { ser((int32&)x); }
	virtual void ser (int64&) = 0;
	void ser (uint64& x) { ser((int64&)x); }
	virtual void ser (float&) = 0;
//	virtual void ser (double&) = 0;
	virtual void ser (Vec&) = 0;
	virtual void ser (CStr&) = 0;
	void ser (bool& x) {
		int32 x2 = x;
		ser(x2);
		x = x2;
	}
	int depth = 0;
	virtual void nl () { };
};

#else

struct BinaryWriter : Serializer {
	FILE* file;
	BinaryWriter (FILE* file) :file(file) { }
	virtual bool writing () const { return true; }
	void put (int8 x) { fputc(x, file); }
	void ser (int8& x) { put(x); }
	void ser (int16& x) {
		put(x >> 8 & 0xff);
		put(x & 0xff);
	}
	void ser (int32& x) {
		put(x >> 24 & 0xff);
		put(x >> 16 & 0xff);
		put(x >> 8 & 0xff);
		put(x & 0xff);
	}
	void ser (int64& x) {
		put(x >> 56 & 0xff);
		put(x >> 48 & 0xff);
		put(x >> 40 & 0xff);
		put(x >> 32 & 0xff);
		put(x >> 24 & 0xff);
		put(x >> 16 & 0xff);
		put(x >> 8 & 0xff);
		put(x & 0xff);
	}
	void ser (float& x) { ser(*(int32*)&x); }
//	void ser (double x) { i32(*(int64*)&x); }
	void ser (Vec& x) { ser(x.x); ser(x.y); }
	void ser (CStr& x) { fputs(x, file); put(0); }
};

struct BinaryReader : Serializer {
	FILE* file;
	BinaryReader (FILE* file) :file(file) { }
	virtual bool writing () const { return false; }
	char get () {
		int c = fgetc(file);
		static bool errored = false;
		if (c == EOF) {
			if (!errored)
				printf("Error: premature EOF\n");
			errored = true;
			return 0;
		}
		return c;
	}
	void ser (int8& x) { get(); }
	void ser (int16& x) {
		x = get() << 8
		  & get();
	}
	void ser (int32& x) {
		x = get() << 24
		  & get() << 16
		  & get() << 8
		  & get();
	}
	void ser (int64& x) {
		x = (uint64)get() << 56
		  & (uint64)get() << 48
		  & (uint64)get() << 40
		  & (uint64)get() << 32
		  & (uint64)get() << 24
		  & (uint64)get() << 16
		  & (uint64)get() << 8
		  & (uint64)get();
	}
	void ser (float& x) { ser(*(int32*)&x); }
//	void ser (double& x) { i64(*(int64*)&x); }
	void ser (Vec& x) { ser(x.x); ser(x.y); }
	void ser (CStr& x) {
		char buf [256];
		uint index = 0;
		char c;
		while (c = get()) {
			buf[index++] = c;
			if (index == 255) {
				static bool errored = false;
				if (!errored)
					printf("Error: Serialized string wasn't terminated before 255 bytes.\n");
				errored = true;
			}
			break;
		}
		buf[index] = 0;
		x = (CStr)malloc(index);
		memcpy((void*)x, (void*)buf, index);
	}

};

struct TextWriter : Serializer {
	FILE* file;
	bool nled = true;
	TextWriter (FILE* file) :file(file) { }
	void pad () {
		if (nled) {
			nled = false;
		}
		else {
			fputc(' ', file);
		}
	}
	virtual bool writing () const { return true; }
	void ser (int8& x) { fprintf(file, "%hhd", x); }
	void ser (int16& x) { fprintf(file, "%hd", x); }
	void ser (int32& x) { fprintf(file, "%d", x); }
	void ser (int64& x) { fprintf(file, "%lld", x); }
	void ser (float& x) { fprintf(file, "%10e", x); }
	void ser (Vec& x) { fprintf(file, "%10e,%10e", x.x, x.y); }
	void ser (CStr& x) { fputs(x, file); fputc(0, file); }
	void nl () {
		fputc('\n', file);
		for (uint i=0; i < n; i++) fputc('\t', file);
	}
};

struct TextReader : Serializer {
	FILE* file;
	TextReader (FILE* file) :file(file) { }
	virtual bool writing () const { return false; }
	void ser (int8& x) { fscanf(file, " %hhd", &x); }
	void ser (int16& x) { fscanf(file, " %hd", &x); }
	void ser (int32& x) { long int r; fscanf(file, " %ld", &r); x = r; }
	void ser (int64& x) { fscanf(file, " %lld", &x); }
	void ser (float& x) { fscanf(file, " %e", &x); }
	void ser (Vec& x) { fscanf(file, " %e,%e", &x.x, &x.y); }
	void ser (CStr& x) {
		char buf [256];
		uint index = 0;
		char c;
		while (c = fgetc(file)) {
			if (c == EOF) {
				x = "";
				return;
			}
			buf[index++] = c;
			if (index == 255) {
				static bool str_errored = false;
				if (!str_errored)
					printf("Error: Serialized string wasn't terminated before 255 bytes.\n");
				str_errored = true;
			}
			break;
		}
		buf[index] = 0;
		x = (CStr)malloc(index);
		memcpy((void*)x, (void*)buf, index);
	}
};





#endif

