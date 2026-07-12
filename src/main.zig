const std = @import("std");
const console = @import("console/console.zig");
const messages = @import("console/messages.zig");
const settings = @import("misc/settings.zig");
const screens = @import("console/screens.zig");

pub fn main(init: std.process.Init) !void {

    var buffer : [256]u8 = undefined;
    var writer = std.Io.File.stdout().writerStreaming(init.io, &buffer);
    const stdout : *std.Io.Writer = &writer.interface;

    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    const args = try init.minimal.args.toSlice(allocator);

    // will be redone
    for (args) |arg| {
        if (std.mem.eql(u8,arg, "-h"))
            return screens.print_help_screen(stdout);
        if (std.mem.eql(u8,arg, "-v"))
            return screens.print_version_screen(stdout);
    }
    
    try messages.message_info(stdout, "Starting compilation...");
    try messages.message_info(stdout, "Lexing...");
    try messages.message_info(stdout, "Parsing...");
    try messages.message_info(stdout, "Analysing...");
    try messages.message_info(stdout, "Generating output...");
    try messages.message_success(stdout, "Done!");
}