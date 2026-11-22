const c = @cImport({
    @cInclude("parsing.h");
});

const std = @import("std");
const msg = @import("../console/messages.zig");
const settings = @import("../misc/settings.zig").settings;

var _sett    : ?(*settings)     = null;

pub fn setup(sett: *settings) void {
    _sett = sett;
}

pub export var yyline_no : c_int = 1;
pub export var yycol_no : c_int = 1;
pub export var yycol_no_prev : c_int = 1;

pub export fn yyerror(err : [*c]u8) void {
    msg.err.parser.display(_sett orelse unreachable, std.mem.span(err), yyline_no, yycol_no_prev)
    catch return;
}