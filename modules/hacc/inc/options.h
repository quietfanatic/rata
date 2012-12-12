
 // Yeah, all the options code is in the header, because absolutely everything here is constexpr.
 // If this turns out to make compile times too slow, I guess we could hc-separate it some.
namespace hacc {
namespace {  // anonymous namespace weirdly equivalent to C-like 'static'

 // Are you ready for higher-order macros?
 // macro( name                    , value, def  , conflicts                                    )
#define HACC_OPTIONS_FORALL_WFLAGS(macro, then) \
    macro( require_type            , 1<<0 , false, f::dont_require_type                               ) then \
    macro( dont_require_type       , 1<<1 , true , f::require_type                                    ) then \
    macro( always_show_types       , 1<<2 , false, f::never_show_types  | f::show_types_when_required ) then \
    macro( never_show_types        , 1<<3 , false, f::always_show_types | f::show_types_when_required ) then \
    macro( show_types_when_required, 1<<4 , true , f::always_show_types | f::never_show_types         ) then \
    macro( require_id              , 1<<5 , false, f::dont_require_id                                 ) then \
    macro( dont_require_id         , 1<<6 , true , f::require_id                                      ) then \
    macro( always_show_ids         , 1<<7 , false, f::never_show_ids  | f::show_ids_when_required     ) then \
    macro( never_show_ids          , 1<<8 , false, f::always_show_ids | f::show_ids_when_required     ) then \
    macro( show_ids_when_required  , 1<<9 , true , f::always_show_ids | f::never_show_ids             ) then \
    macro( json_format             , 1<<10, false, f::no_json_format                                  ) then \
    macro( no_json_format          , 1<<11, true , f::json_format                                     ) then \
    macro( dos_newlines            , 1<<12, false, f::unix_newlines                                   ) then \
    macro( unix_newlines           , 1<<13, true , f::dos_newlines                                    ) then \
    macro( set_indent_string       , 1<<14, false, 0                                                  ) then \
    macro( overwrite_file          , 1<<16, true , f::dont_overwrite_file | f::append_file            ) then \
    macro( dont_overwrite_file     , 1<<17, false, f::overwrite_file | f::append_file                 ) then \
    macro( append_file             , 1<<18, false, f::overwrite_file | f::dont_overwrite_file         )
struct write_options {

#define HACC_OPTIONS_COMMA ,
#define HACC_OPTIONS_ENUM(name, value, def, conflicts) name = value
    struct f {
        enum {
            HACC_OPTIONS_FORALL_WFLAGS(HACC_OPTIONS_ENUM, HACC_OPTIONS_COMMA)
        };
    };
#define HACC_OPTIONS_DEFAULT(name, value, def, conflicts) (def ? f::name : 0)
    static const uint32 default_flags =
        HACC_OPTIONS_FORALL_WFLAGS(HACC_OPTIONS_DEFAULT, |);

    uint32 flags;
    int8 _indent_levels;
    const char* _indent_string;

#define HACC_OPTIONS_GETTER(name, value, def, conflicts) \
    constexpr bool name () { return flags & f::name; }
    HACC_OPTIONS_FORALL_WFLAGS(HACC_OPTIONS_GETTER, )

     // set_indent_string is 1<<9
    constexpr int8 indent_levels () { return _indent_levels; }
    constexpr const char* indent_string () { return _indent_string; }


    constexpr write_options (uint32 flags = default_flags, uint il = -2, const char* is = "\t") :
        flags(flags), _indent_levels(il), _indent_string(is)
    { }

     // Combining
    static constexpr uint32 combine_flags (uint32 a, uint32 b) {
#define HACC_OPTIONS_COMBINER(name, value, def, conflicts) \
    ((a & f::name || b & f::name) && !(b & (conflicts)) ? f::name : 0)
        return
            HACC_OPTIONS_FORALL_WFLAGS(HACC_OPTIONS_COMBINER, |);
    }
    constexpr write_options operator | (write_options opt) {
        return write_options(
            combine_flags(flags, opt.flags),
            opt._indent_levels == -2 ? _indent_levels : opt._indent_levels,
            opt.set_indent_string() ? opt._indent_string : _indent_string
        );
    }

};
}  // anon namespace

namespace hf {
    static constexpr write_options default_options = write_options();
#define HACC_OPTIONS_WHF(name, value, def, conflicts) \
    static constexpr write_options name = write_options(write_options::combine_flags(write_options::default_flags, write_options::f::name));
    HACC_OPTIONS_FORALL_WFLAGS(HACC_OPTIONS_WHF, ;);
    static constexpr write_options indent_levels (int8 levels) {
        return write_options(write_options::default_flags, levels); 
    }
    static constexpr write_options indent_everything = indent_levels(-1);
    static constexpr write_options indent_string (const char* s) {
        return write_options(write_options::default_flags | write_options::f::set_indent_string, -2, s);
    }
}

}


