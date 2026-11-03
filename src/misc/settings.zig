const std = @import("std");

pub const settings = struct {
    input_files : std.ArrayList(*const u8),
    writer : *std.Io.Writer,

    // Flags
    display_help    : bool = false,
    display_version : bool = false,

    pub fn init(writer : *std.Io.Writer) settings {
        return settings{
            .input_files = .empty,
            .writer = writer
        };
    }

    pub fn deinit(this : *settings, allocator: std.mem.Allocator) void {
        this.input_files.deinit(allocator);
    }
};
