const std = @import("std");
const settings = @import("settings.zig").settings;
const msg = @import("../console/messages.zig");

pub const argument_error = error {
    screen_displayed, unknown_argument, unknown_flag, non_letter_flag, invalid_argument_format
};

pub fn parse(arguments : []const [:0]const u8, allocator: std.mem.Allocator, writer : *std.Io.Writer) msg.err_code!settings {
    var sett = settings.init(writer);
    errdefer sett.deinit(allocator);

    for (arguments) |argument| {
        if (argument.len >= 2 and argument[0] == '-' and argument[1] != '-') {
            // flags
            for (argument[1 ..], 1 ..) |flag, index|{
                process_flag(&sett, flag)
                catch
                    return msg.err.invalid_flag.display(&sett, argument, index, index);
            }
        }
        else if (argument.len >= 3 and argument[0] == '-' and argument[1] == '-') {
            // verbose arguments
            // const content = argument[2 ..];
        }
        else {
            // input files
        }
    }

    return sett;
}

fn process_flag(sett : *settings, flag: u8) argument_error!void {
    if (!(flag >= 'a' and flag <= 'z') and !(flag >= 'A' and flag <= 'Z'))
        return argument_error.non_letter_flag;

    const _flag =
        if (flag >= 'A' and flag <= 'Z')
            flag
        else
            flag - ('a' - 'A');

    switch (_flag) {
        'H' => sett.display_help    = true,
        'V' => sett.display_version = true,
        else => return argument_error.unknown_flag
    }
}

// fn process_varbose_argument(sett : *settings, flag: [:0]const u8) argument_error!void {
//
// }