const console = @import("console.zig");
const std = @import("std");

pub fn message_info(writer : *std.Io.Writer, message : []const u8) !void {
    try console.print_char(writer, '[', console.colour.white());
    try console.print_str(writer, "INF", console.colour.info());
    try console.print_str(writer, "] ", console.colour.white());
    try console.print_str(writer, message, console.colour.white());
    try console.print_char(writer, '\n', console.colour.white());
    try writer.flush();
}

pub fn message_success(writer : *std.Io.Writer, message : []const u8) !void {
    try console.print_char(writer, '[', console.colour.white());
    try console.print_str(writer, "SUC", console.colour.succ());
    try console.print_str(writer, "] ", console.colour.white());
    try console.print_str(writer, message, console.colour.white());
    try console.print_char(writer, '\n', console.colour.white());
    try writer.flush();
}