
namespace hacc {

struct write_options {

 // Are you ready for higher-order macros?
 // macro( name                    , value, def  , conflicts                                    )
#define HACC_OPTIONS_FORALL_FLAGS(macro, then) \
    macro( require_type            , 1<<0 , false, dont_require_type                            ) then \
    macro( dont_require_type       , 1<<1 , true , require_type                                 ) then \
    macro( always_show_types       , 1<<2 , false, never_show_types  | show_types_when_required ) then \
    macro( never_show_types        , 1<<3 , false, always_show_types | show_types_when_required ) then \
    macro( show_types_when_required, 1<<4 , true , always_show_types | never_show_types         ) then \
    macro( require_id              , 1<<5 , false, dont_require_id                              ) then \
    macro( dont_require_id         , 1<<6 , true , require_id                                   ) then \
    macro( always_show_ids         , 1<<7 , false, never_show_ids  | show_ids_when_required     ) then \
    macro( never_show_ids          , 1<<8 , false, always_show_ids | show_ids_when_required     ) then \
    macro( show_ids_when_required  , 1<<9 , true , always_show_ids | never_show_ids             ) then \
    macro( json_format             , 1<<10, false, no_json_format                               ) then \
    macro( no_json_format          , 1<<11, true , json_format                                  ) then \
    macro( dos_newlines            , 1<<12, false, unix_newlines                                ) then \
    macro( unix_newlines           , 1<<13, true , dos_newlines                                 ) then \
    macro( set_indent_string       , 1<<14, false, 0                                            ) then \
    macro( overwrite_file          , 1<<16, true , dont_overwrite_file | append_file            ) then \
    macro( dont_overwrite_file     , 1<<17, false, overwrite_file | append_file                 ) then \
    macro( append_file             , 1<<18, false, overwrite_file | dont_overwrite_file         )

#define HACC_OPTIONS_COMMA ,
#define HACC_OPTIONS_ENUM(name, value, def, conflicts) name = value
    namespace flag_values {
        enum {
            HACC_OPTIONS_FORALL(HACC_OPTIONS_ENUM, HACC_OPTIONS_COMMA)
        }
    }
#define HACC_OPTIONS_DEFAULT(name, value, def, conflicts) (def && flag_values::name)
    static const uint32 default_flags =
        HACC_OPTIONS_FORALL_FLAGS(HACC_OPTIONS_DEFAULT, |);

    uint32 flags;
    int8 _indent_levels;
    std::string _indent_string;

#define HACC_OPTIONS_GETTER(name, value, def, conflicts) \
    constexpr bool name () { return flags & flag_values::name; }
    HACC_OPTIONS_FORALL_FLAGS(HACC_OPTIONS_GETTER, )

     // set_indent_string is 1<<9
    constexpr int8 indent_levels () { return _indent_levels; }
    constexpr std::string indent_string () { return _indent_string; }


    constexpr write_options (uint32 flags = default_flags, uint il = -2, std::string is = "\t") :
        flags(flags), _indent_levels(il), _indent_string(is)
    { }

     // Combining
    static constexpr uint32 combine_flags (uint32 other_flags) {
        using namespace flag_values;
#define HACC_OPTIONS_COMBINER(name, value, def, conflicts) \
    ((flags & name || other_flags & name) && !(other_flags & (conflicts)) && name)
        return
            HACC_OPTIONS_FORALL_FLAGS(HACC_OPTIONS_COMBINER, |);
    }
    constexpr write_options operator | (write_options opt) {
        return write_options(
            combine_flags(opt.flags),
            opt._indent_levels == -2 ? _indent_levels : opt._indent_levels,
            opt.set_indent_string() ? opt._indent_string : _indent_string
        );
    }

};

namespace hf {
    static constexpr write_options default_options = write_options();
#define HACC_OPTIONS_HF(name, value, conflicts) \
    constexpr write_options name = write_options(write_options::combine_flags(default_flags, write_options::flag_values::name));
    HACC_OPTIONS_FORALL_FLAGS(HACC_OPTIONS_HF, ;);
    static constexpr write_options indent_levels (int8 levels) {
        return write_options(default_flags, levels); 
    }
    static constexpr write_options indent_everything = indent_levels(-1);
    static constexpr write_options indent_string (std::string s) {
        return write_options(default_flags | flag_values::set_indent_string, -2, s);
    }
}

}


