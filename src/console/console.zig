const std = @import("std");

pub const colour = struct {
    r : u8 = 255,
    g : u8 = 255,
    b : u8 = 255,

    pub fn new(r : u8, g : u8, b : u8) colour {
        return colour {
            .r = r,
            .g = g,
            .b = b
        };
    }

    pub fn white() colour {
        return colour {};
    }
    pub fn info() colour {
        return colour {.r = 64, .g = 64, .b = 255};
    }
    pub fn warn() colour {
        return colour {.r = 255, .g = 255, .b = 32};
    }
    pub fn erro() colour {
        return colour {.r = 255, .g = 32, .b = 32};
    }
    pub fn succ() colour {
        return colour {.r = 32, .g = 255, .b = 32};
    }
};

pub fn reset_colour(writer : *std.Io.Writer) !void {
    return writer.print("\x1b[38;2;255;255;255m", .{});
}

pub fn print_str(writer : *std.Io.Writer, value : []const u8, col : colour) !void {
    return writer.print("\x1b[38;2;{d};{d};{d}m{s}", .{col.r, col.g, col.b, value});
}

pub fn print_char(writer : *std.Io.Writer, value : u21, col : colour) !void {
    return writer.print("\x1b[38;2;{d};{d};{d}m{u}", .{col.r, col.g, col.b, value});
}

pub fn print_frame(writer : *std.Io.Writer, comptime file_path : []const u8, col_frame : colour, col_inside : colour) !void {
    var frame = std.mem.splitAny(u8, @embedFile("../strings/" ++ file_path), "\n");

    try print_str(writer, frame.first(), col_frame);
    try writer.print("\n", .{});
    while (frame.next()) |line| {
        if (frame.peek() != null) {
            const first_len : usize = try std.unicode.utf8ByteSequenceLength(line[0]);
            try print_str(writer, line[0..first_len], col_frame);
            try print_str(writer, line[first_len .. line.len - first_len], col_inside);
            try print_str(writer, line[line.len - first_len..], col_frame);
        }
        else
            try print_str(writer, line, col_frame);

        try writer.print("\n", .{});
    }
}