const console = @import("console.zig");
const std = @import("std");
const strings = @import("../strings/strings.zig");
const version = @import("../misc/version.zig");
const settings = @import("../misc/settings.zig").settings;

fn print_help_screen(writer : *std.Io.Writer) !void {

    // top banner
    try console.print_frame(writer,
        "common/banner.txt",
        console.colour.new(255, 255, 255),
        console.colour.new(128, 255, 128));

    // a bit of space
    try writer.print("\n", .{});

    try writer.print("{s}: dodoc [{s}] {s}\n\n", .{
        strings.help.usage,
        strings.help.options,
        strings.help.file,
    });


    try writer.print("{s}: Szymon Jabłoński\n", .{
        strings.general.written_by,
    });

    try writer.flush();
}

fn print_version_screen(writer : *std.Io.Writer) !void {

    try writer.print("{s}, {s}:\n", .{
        strings.general.dodo_lang_compiler,
        strings.general.version_information,
    });

    try writer.print("\t{s}: {s}\n", .{
        strings.general.version,
        version.number,
    });

    try writer.print("\t{s}: {s}, {s}: {s}\n", .{
        strings.general.branch,
        version.branch,
        strings.general.build,
        version.branch_build,
    });

    try writer.print("\t{s} {s}: {s}\n", .{
        strings.general.summed,
        strings.general.build,
        version.total_build,
    });

    try writer.print("\t{s}: {s}\n", .{
        strings.general.compilation_time,
        version.date_time,
    });

    try writer.flush();
}

pub fn display_screens(sett : *const settings) !bool {
    if (sett.display_help){
        try print_help_screen(sett.writer);
        return true;
    }
    if (sett.display_version){
        try print_version_screen(sett.writer);
        return true;
    }
    return false;
}