const expect = @import("std").testing.expect;
const std = @import("std");

const settings = @import("misc/settings.zig");

test "help_strings_present" {
    const T = @typeInfo(@TypeOf(settings.help));
    inline for (T.@"struct".fields) |field| {
        std.debug.print("Testing for string: \"{s}\" in help file...\n", .{@field(settings.help, field.name).name});
        try expect(settings.help_strings.has(@field(settings.help, field.name).name));
        std.debug.print("Found!\n", .{});
    }
}

test "general_strings_present" {
    const T = @typeInfo(@TypeOf(settings.general));
    inline for (T.@"struct".fields) |field| {
        std.debug.print("Testing for string: \"{s}\" in general file...\n", .{@field(settings.general, field.name).name});
        try expect(settings.general_strings.has(@field(settings.general, field.name).name));
        std.debug.print("Found!\n", .{});

    }
}