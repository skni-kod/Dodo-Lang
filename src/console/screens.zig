const console = @import("console.zig");
const std = @import("std");
const settings = @import("../misc/settings.zig");
const version = @import("../misc/version.zig");

pub fn print_help_screen(writer : *std.Io.Writer) !void {

    // top banner
    try console.print_frame(writer,
        "common/banner.txt",
        console.colour.new(255, 255, 255),
        console.colour.new(128, 255, 128));

    // a bit of space
    try writer.print("\n", .{});

    try writer.print("{s}: dodoc [{s}] {s}\n\n", .{
        settings.help.usage.get(),
        settings.help.options.get(),
        settings.help.file.get(),
    });


    try writer.print("{s}: Szymon Jabłoński\n", .{
        settings.general.written_by.get(),
    });

    try writer.flush();
}

pub fn print_version_screen(writer : *std.Io.Writer) !void {

    try writer.print("{s}, {s}:\n", .{
        settings.general.dodo_lang_compiler.get(),
        settings.general.version_information.get(),
    });

    try writer.print("\t{s}: {s}\n", .{
        settings.general.version.get(),
        version.number,
    });

    try writer.print("\t{s}: {s}, {s}: {s}\n", .{
        settings.general.branch.get(),
        version.branch,
        settings.general.build.get(),
        version.branch_build,
    });

    try writer.print("\t{s} {s}: {s}\n", .{
        settings.general.summed.get(),
        settings.general.build.get(),
        version.total_build,
    });

    try writer.print("\t{s}: {s}\n", .{
        settings.general.compilation_time.get(),
        version.date_time,
    });

    try writer.flush();
}