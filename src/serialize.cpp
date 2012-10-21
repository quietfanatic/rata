
 // This file is about serialization and deserialization.  The format is simplistic
 // and not very failproof.  Each class decides how it's serialized.  The file
 // consists of a version number followed by a list of elements, each of which is a
 // hash of the class's name followed by the class-specific data.  A name-hash of 0
 // terminates the list and the file.  It's the responsibility of the class to detect
 // older file versions and deserialize accordingly.
 // Yes, if a single byte is missing or extra, it'll corrupt the entire file.

#ifdef HEADER

 // Bump this any time you change the serialization code of any class.
const uint32 SERIALIZATION_VERSION = 1;
 // For backwards compatibility, use:
 // if (s->version < <new_version>) { ...old code...; return; }
 // You can remove old serializations once you know there are no old-version
 // files in existance.


 // Interface for all methods of (de)serialization.
struct Serializer {
    uint version;
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
	virtual void nl () { };
};

 // All serializable objects go in a linked list
struct Serializable_Base : Linked<Serializable_Base> {
	virtual CStr name () = 0;
	virtual void serialize (Serializer* s, uint version) = 0;  // input or output
};

 // When we deserialize, we decide what it is based on a hash of its name
 // Despite the fact that we're hashing the name, we're not using a hash table.
 // Just a dumb list of hash values. :)
uint64 x31_hash (const char*);

 // Maps hashes to classes
struct Type_Decider : Linked<Type_Decider> {
    uint64 name_hash;
    typedef Serializable_Base* (* creator )();
    creator create;
    Type_Decider (uint64 nh, creator create) : name_hash(nh), create(create) {
        for (auto p = next; p; p = p->next) {
            if (p->name_hash == name_hash) {
                printf("Gerk!  Two names hashed the same!  %s = %016lx; %s = %016lx\n",
                    p->create()->name(), p->name_hash, create()->name(), name_hash
                );
                exit(1);
            }
        }
    }
};

 // Parameterize serializability by name and subclass, which is enough
 // to make a type decider for it.
template <CStr _name, class T>
struct Serializable {
    CStr name () { return _name; }
    static Type_Decider decider;
};

void serialize_game (Serializer* s);
void save_game_to_text (CStr filename);
void load_game_from_text (CStr filename);
void save_game_to_binary (CStr filename);
void load_game_from_binary (CStr filename);


#else

uint64 x31_hash (CStr s) {
    uint64* r = 0;
    for(; *s; s++) r = (r << 5) - r + *s;
    return r;
}

template <CStr name, class T>
Serializer<name, T>::decider = Type_Decider(x31_hash(name), [](){ return new T; });

void serialize_game (Serializer* s) {
    if (s->writing()) {
        s->ser(SERIALIZATION_VERSION);
        for (auto p = Linked<Serializer_Base>::first; p; p = p->next) {
            uint64 name_hash = x31_hash(p->name());
            s->ser(name_hash);
            p->serialize(s);
            s->nl();
        }
        uint32 zero = 0;
        s->ser(zero);
        
    }
    else {
        uint32 file_version;
        s->ser(file_version);
        uint64 name_hash;
        s->ser(name_hash);
        while (name_hash) {
            for (auto d = Linked<Type_Decider>::first; d; d = d->next)
            if (name_hash == d->name_hash) {
                d->create()->serialize(s, file_version);
                s->ser(name_hash);
                break;
            }
            printf("Error: No class has name_hash %016lx.\n", name_hash);
            exit(1);
        }
    }
}
void save_game_to_text (CStr filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Error: Cannot open %s for writing: %s\n", strerror(errno));
        return;
    }
    Text_Writer tw (f);
    serialize_game(&bw);
    if (0!=fclose(f))
        printf("Error: Failed to close %s: %s\n", strerror(errno));
}
void load_game_from_text (CStr filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("Error: Cannot open %s for reading: %s\n", strerror(errno));
        return;
    }
    Text_Reader tr (f);
    serialize_game(&tr);
    if (0!=fclose(f))
        printf("Error: Failed to close %s: %s\n", strerror(errno));
}
void save_game_to_binary (CStr filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Error: Cannot open %s for writing: %s\n", strerror(errno));
        return;
    }
    Binary_Writer bw (f);
    serialize_game(&bw);
    if (0!=fclose(f))
        printf("Error: Failed to close %s: %s\n", strerror(errno));
}
void load_game_from_binary (CStr filename) {
    FILE* if = fopen(filename, "r");
    if (!f) {
        printf("Error: Cannot open %s for reading: %s\n", strerror(errno));
        return;
    }
    Binary_Reader br (f);
    serialize_game(&br);
    if (0!=fclose(f))
        printf("Error: Failed to close %s: %s\n", strerror(errno));
}

struct Binary_Writer : Serializer {
	FILE* file;
	Binary_Writer (FILE* file) :file(file) { }
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

struct Binary_Reader : Serializer {
	FILE* file;
	Binary_Reader (FILE* file) :file(file) { }
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

struct Text_Writer : Serializer {
	FILE* file;
	bool nled = true;
	Text_Writer (FILE* file) :file(file) { }
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
	void nl () { fputc('\n', file); }
};

struct Text_Reader : Serializer {
	FILE* file;
	Text_Reader (FILE* file) :file(file) { }
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

