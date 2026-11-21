const std = @import("std");

pub fn new_translation(comptime first_letter_capital : []const u8, comptime lowercase : []const u8) translation {

    // TODO: do utf8 uppercasing
    // const utf_length = std.unicode.utf8ByteSequenceLength(standard[0]);
    // const first = standard[0..utf_length];
    // const rest = standard[utf_length..];
    // const iter = std.unicode.Utf8View.initComptime(first);
    // const codepoint = try iter.iterator().nextCodepoint();

    return translation {
        .lowercase = lowercase,
        .capitalized = first_letter_capital ++ lowercase[1..]
    };
}

pub const translation = struct {
    capitalized : []const u8,
    lowercase : []const u8,
};