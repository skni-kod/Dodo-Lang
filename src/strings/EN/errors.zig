const utils = @import("../common/utils.zig");

pub const invalid_flag       = utils.new_translation("I", "invalid flag, refer to the help screen (dodoc -h) for options");
pub const invalid_arg_syntax = utils.new_translation("I", "invalid argument syntax");
pub const parser             = utils.new_translation("F", "from parser: ");