const console = @import("console.zig");
const colour = console.colour;
const std = @import("std");
const strings = @import("../strings/strings.zig");
const settings = @import("../misc/settings.zig").settings;
const string_utils = @import("../strings/common/utils.zig");

pub fn message_info(writer : *std.Io.Writer, message : []const u8) !void {
    try print_header_info(writer);
    try console.print_str(writer, message, console.colour.white());
    try console.print_char(writer, '\n', console.colour.white());
    try writer.flush();
}

pub fn message_success(writer : *std.Io.Writer, message : []const u8) !void {
    try print_header_success(writer);
    try console.print_str(writer, message, console.colour.white());
    try console.print_char(writer, '\n', console.colour.white());
    try writer.flush();
}

fn print_header(writer : *std.Io.Writer, comptime header : []const u8, comptime col : colour) !void {
    try console.print_char(writer, '[', console.colour.white());
    try console.print_str(writer, header, col);
    try console.print_str(writer, "] ", console.colour.white());
}

fn print_header_error(writer : *std.Io.Writer) !void {
    try print_header(writer, strings.general.erro.lowercase, colour.erro());
}

fn print_header_warning(writer : *std.Io.Writer) !void {
    try print_header(writer, strings.general.warn.lowercase, colour.warn());
}

fn print_header_success(writer : *std.Io.Writer) !void {
    try print_header(writer, strings.general.succ.lowercase, colour.succ());
}

fn print_header_info(writer : *std.Io.Writer) !void {
    try print_header(writer, strings.general.info.lowercase, colour.info());
}

pub const err = struct {
    const _arguments = struct {
        message : []const u8,
        code    : err_code,

        fn create(comptime code : err_code, comptime message : string_utils.translation) _arguments {
            return _arguments {
                .message = message.capitalized,
                .code = code
            };
        }

        pub fn display(this : *const _arguments, sett : *settings, argument : []const u8, from : usize, to : usize) err_code {
            this._display(sett, argument, from, to)
            catch return err_code.crash;
            return this.code;
        }

        fn _display(this : *const _arguments, sett : *settings, argument : []const u8, from : usize, to : usize) !void {
            // error type
            try print_header_error(sett.writer);
            try console.print_str(sett.writer, this.message, colour.white());
            try console.print_char(sett.writer, '\n', colour.white());

            // location, first line
            try console.print_str(sett.writer, "       ", colour.white());
            try console.print_str(sett.writer, strings.general.at.capitalized, colour.white());
            try console.print_str(sett.writer, ": ", colour.white());
            if (from > 0)
                try console.print_str(sett.writer, argument[0 .. from], colour.white());
            try console.print_str(sett.writer, argument[from .. to + 1], colour.erro());
            if (to + 1 < argument.len)
                try console.print_str(sett.writer, argument[to + 1 ..], colour.white());
            try console.print_char(sett.writer, '\n', colour.white());

            // location, second line
            const offset = 2 + strings.general.at.capitalized.len + ((from + to) / 2);
            try console.print_str(sett.writer, "       ", colour.white());
            for (0 .. offset) |_|
                try console.print_char(sett.writer, '-', colour.erro());
            try console.print_str(sett.writer, "^\n", colour.erro());

            try sett.writer.flush();
        }
    };

    const _parser = struct {
        message : []const u8,
        code    : err_code,

        fn create(comptime code : err_code, comptime message : string_utils.translation) _parser {
            return _parser {
                .message = message.capitalized,
                .code = code
            };
        }

        pub fn display(this : *const _parser, sett : *settings, message : []const u8, line : c_int, column : c_int) err_code {
            this._display(sett, message, line, column)
            catch return err_code.crash;
            return this.code;
        }

        fn _display(this : *const _parser, sett : *settings, message : []const u8, line : c_int, column : c_int) !void {
            // error type
            try print_header_error(sett.writer);
            try console.print_str(sett.writer, this.message, colour.white());
            try console.print_str(sett.writer, message, colour.white());
            try console.print_char(sett.writer, '\n', colour.white());

            try console.print_str(sett.writer, "       ", colour.white());
            try console.print_str(sett.writer, strings.general.at.capitalized, colour.white());
            try console.print_str(sett.writer, ": ", colour.white());
            try console.print_cint(sett.writer, line, colour.white());
            try console.print_char(sett.writer, ':', colour.white());
            try console.print_cint(sett.writer, column, colour.white());
            try console.print_char(sett.writer, '\n', colour.white());

            // location

            try sett.writer.flush();
        }
    };

    pub const invalid_flag = _arguments.create(err_code.invalid_flag, strings.errors.invalid_flag);
    pub const parser = _parser.create(err_code.parser, strings.errors.parser);

};

pub const err_code = error {
    crash,
    invalid_flag,
    invalid_arg_syntax,
    parser,
};
