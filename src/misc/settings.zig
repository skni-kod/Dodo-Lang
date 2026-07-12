const translations = @import("../strings/translation_parser.zig");

const prefix : []const u8 = "EN";

pub const help_strings = translations.create_string_dict(prefix, "help.txt");
pub const general_strings = translations.create_string_dict(prefix, "general.txt");

const string_types = enum {
    help, general
};

const getter = struct {
    name : []const u8 = "",
    string_type : string_types = string_types.help,

    fn help(comptime name_str : []const u8) getter {
        return getter {
            .name = name_str,
            .string_type = string_types.help
        };
    }

    fn general(comptime name_str : []const u8) getter {
        return getter {
            .name = name_str,
            .string_type = string_types.general
        };
    }

    pub fn get(comptime this : *const getter) []const u8 {
        return switch (this.string_type) {
            string_types.help => help_strings.get(this.name) orelse unreachable,
            string_types.general => general_strings.get(this.name) orelse unreachable,
        };
    }
};

pub const help = struct {
    pub const usage   = getter.help("USAGE");
    pub const help    = getter.help("HELP");
    pub const options = getter.help("OPTIONS");
    pub const file    = getter.help("FILE");
};

pub const general = struct {
    pub const dodo_lang_compiler   = getter.general("DODO_LANG_COMPILER");
    pub const version              = getter.general("VERSION");
    pub const written_by           = getter.general("WRITTEN_BY");
    pub const version_information  = getter.general("VERSION_INFORMATION");
    pub const build                = getter.general("BUILD");
    pub const branch               = getter.general("BRANCH");
    pub const summed               = getter.general("SUMMED");
    pub const compilation_time     = getter.general("COMPILATION_TIME");
};