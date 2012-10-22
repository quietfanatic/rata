
 // This file is about serialization and deserialization.  The format is simplistic
 // and not very failproof.  Each class decides how it's serialized.  The file
 // consists of a version number followed by a list of elements, each of which is a
 // hash of the class's name followed by the class-specific data.  A name-hash of 0
 // terminates the list and the file.  It's the responsibility of the class to detect
 // older file versions and deserialize accordingly.
 // Yes, if a single byte is missing or extra, it'll corrupt the entire file.

#ifdef HEADER

 // Dubious backwards compatibility measure.
 // Prefer per-class versions.
const uint32 SERIALIZATION_VERSION = 1;

 // Interface for all methods of (de)serialization.
struct Serializer {
     // For reader only, file and class versions for compatibility
    uint file_version;
    uint class_version = 0;
     // True for writers
	virtual bool writing () = 0;
     // Main serialization routines
	virtual void ser (int8&) = 0;
	virtual void ser (uint8& x) { ser((int8&)x); }
	virtual void ser (int16&) = 0;
	virtual void ser (uint16& x) { ser((int16&)x); }
	virtual void ser (int32&) = 0;
	virtual void ser (uint32& x) { ser((int32&)x); }
	virtual void ser (int64&) = 0;
	virtual void ser (uint64& x) { ser((int64&)x); }
	virtual void ser (float&) = 0;
     // We're not using doubles, but we might eventually
//	virtual void ser (double&) = 0;
	virtual void ser (Vec&) = 0;
	virtual void ser (CStr&) = 0;
	virtual void ser (bool& x) {
		int8 x2 = x;
		ser(x2);
		x = x2;
	}
     // Emit newline on write; warn on absence on read
	virtual void nl () { };
    virtual ~Serializer () { }
};

 // All serializable objects go in a linked list
struct Serializable : Linked<Serializable> {
    virtual uint serialization_version () { return 0; }
	virtual CStr serialization_name () = 0;
    virtual Serializable* create () = 0;
	virtual void serialize (Serializer* s) = 0;  // input or output
};

 // Parameterize serializability by name and subclass, which is enough
 // to make a type decider for it.
template <class T, CStr _name>
struct Serialized : Hashed<Serializable, _name> {
    CStr serialization_name () { return _name; }
    Serializable* create () { return new T; }
};

void serialize_game (Serializer* s);


#else


void serialize_game (Serializer* s) {
    if (s->writing()) {
        uint v = SERIALIZATION_VERSION;
        s->ser(v);
        FOR_LINKED(p, Serializable) {
            CStr name = p->serialization_name();
            s->ser(name);
            uint version = p->serialization_version();
            s->ser(version);
            p->serialize(s);
            s->nl();
        }
        CStr empty = "";
        s->ser(empty);
        s->nl();
    }
    else {
        uint32 file_version;
        s->ser(file_version);
        CStr name;
        for (s->ser(name); name[0]; s->ser(name)) {
            auto p = Class_Hash<Serializable>::lookup(name);
            if (!p) exit(1);
            uint32 class_version;
            s->ser(class_version);
            p->create()->serialize(s);
        }
    }
}

 // All the kinds of serializers (and shared code)
 // We may eventually have to seperate implementation out.

template <bool _writing>
struct File_Serializer : Serializer {
    const char* filename = NULL;
    FILE* file;
    bool failed = false;
	File_Serializer (FILE* file) : file(file) { }
    File_Serializer (const char* filename) : filename(filename) {
        file = fopen(filename, _writing ? "w" : "r");
        if (!file) {
            printf("Error: Cannot open %s for %s: %s\n", filename, _writing ? "writing" : "reading", strerror(errno));
        }
        failed = true;
    }
    ~File_Serializer () {
        if (filename && 0!=fclose(file))
            printf("Error: Failed to close %s: %s\n", filename, strerror(errno));
        failed = true;
    }
    bool writing () const { return _writing; }
};

struct Binary_Writer : File_Serializer<true> {
	Binary_Writer (FILE* file) : File_Serializer(file) { }
    Binary_Writer (const char* filename) : File_Serializer(filename) { }
	void put (int8 x) { if (!failed) failed = (fputc(x, file) == EOF); }
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

struct Binary_Reader : File_Serializer<false> {
	Binary_Reader (FILE* file) : File_Serializer(file) { }
    Binary_Reader (const char* filename) : File_Serializer(filename) { }
	char get () {
        if (!failed) {
    		int c = fgetc(file);
    		if (c == EOF) {
    	        printf("Error: premature EOF\n");
    			return 0;
    		}
    		return c;
        }
        return 0;
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
        if (failed) {
            x = NULL;
            return;
        }
		char buf [256];
		uint index = 0;
		char c;
		while ((c = get())) {
			buf[index++] = c;
			if (index == 255) {
		        printf("Error: Serialized string wasn't terminated before 255 bytes.\n");
				failed = true;
                x = NULL;
                return;
			}
		}
		buf[index] = 0;
		x = (CStr)malloc(index);
		memcpy((void*)x, (void*)buf, index);
	}

};

struct Text_Writer : File_Serializer<true> {
	bool nled = true;
	Text_Writer (FILE* file) : File_Serializer(file) { }
    Text_Writer (const char* filename) : File_Serializer(filename) { }
	void pad () {
		if (nled) {
			nled = false;
		}
		else {
			fputc(' ', file);
		}
	}
	void ser (int8& x) { pad(); fprintf(file, "%" PRIi8, x); }
	void ser (int16& x) { pad(); fprintf(file, "%" PRIi16, x); }
	void ser (int32& x) { pad(); fprintf(file, "%" PRIi32, x); }
	void ser (int64& x) { pad(); fprintf(file, "%" PRIi64, x); }
	void ser (float& x) { pad(); fprintf(file, "%10e", x); }
	void ser (Vec& x) { pad(); fprintf(file, "%10e,%10e", x.x, x.y); }
	void ser (CStr& x) { pad(); fputs(x, file); fputc(0, file); }
	void nl () { fputc('\n', file); }
};

struct Text_Reader : File_Serializer<false> {
	Text_Reader (FILE* file) : File_Serializer(file) { }
    Text_Reader (const char* filename) : File_Serializer(filename) { }
	void ser (int8& x) { if (fscanf(file, " %" SCNi8, &x) != 1) failed = true; }
	void ser (int16& x) { if (fscanf(file, " %" SCNi16, &x) != 1) failed = true; }
	void ser (int32& x) { if (fscanf(file, " %" SCNi32, &x) != 1) failed = true; }
	void ser (int64& x) { if (fscanf(file, " %" SCNi64, &x) != 1) failed = true; }
	void ser (float& x) { if (fscanf(file, " %e", &x) != 1) failed = true; }
	void ser (Vec& x) { if (fscanf(file, " %e,%e", &x.x, &x.y) != 2) failed = true; }
	void ser (CStr& x) {
		char buf [256];
		uint index = 0;
		char c;
		while ((c = fgetc(file))) {
			if (c == EOF) {
				x = "";
				return;
			}
			buf[index++] = c;
			if (index == 255) {
			    printf("Error: Serialized string wasn't terminated before 255 bytes.\n");
				failed = true;
                x = NULL;
                return;
			}
			break;
		}
		buf[index] = 0;
		x = (CStr)malloc(index);
		memcpy((void*)x, (void*)buf, index);
	}
};




#endif

