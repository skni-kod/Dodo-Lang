const std = @import("std");
const messages = @import("console/messages.zig");
const arg_parser = @import("misc/arguments.zig");
const screens = @import("console/screens.zig");
const parsing = @import("c_interop/parsing.zig");
const bison = @cImport({
    @cInclude("bison.h");
    @cInclude("stdio.h");
});

pub fn main(init: std.process.Init) !void {

    var buffer : [256]u8 = undefined;
    var writer = std.Io.File.stdout().writerStreaming(init.io, &buffer);
    const stdout : *std.Io.Writer = &writer.interface;

    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    const args = try init.minimal.args.toSlice(allocator);
    defer allocator.free(args);

    var sett = try arg_parser.parse(args, allocator, stdout);
    defer sett.deinit(allocator);

    try messages.message_info(stdout, "Starting compilation...");
    try messages.message_info(stdout, "Lexing and parsing...");

    //const file = lexer.fopen("stdlib/base/types.dodo", "r");
    //lexer.yyset_in(file);
    //_ = lexer.yylex();
    //_ = lexer.fclose(file);

    parsing.setup(&sett);

    _ = bison.freopen("test/test.dodo", "r", bison.stdin);
    _ = bison.yyparse();


    try messages.message_info(stdout, "Analysing...");
    try messages.message_info(stdout, "Generating output...");
    try messages.message_success(stdout, "Done!");
}