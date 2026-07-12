const std = @import("std");

const parsing_state = enum {
    expects_name, expects_content
};

pub fn get_cleaned_string(comptime str : []const u8) []const u8 {
    comptime var result : []const u8 = "";

    comptime var index = 0;
    while (index < str.len) : (index += 1) {
        if (str[index] != '\n' and str[index] != ':' and str[index] != ' ')
            result = result ++ str[index..index + 1];
    }
    return result;
}

pub fn create_string_dict(comptime prefix : []const u8, comptime filename : []const u8) std.StaticStringMap([]const u8) {

    @setEvalBranchQuota(2048);

    const content = @embedFile(prefix ++ "/" ++ filename);
    comptime var data = std.mem.splitAny(u8, content, "{}");

    comptime var amount = 0;
    while (data.next() != null)
        amount += 1;
    data.reset();

    comptime var array : [amount / 2]struct{ []const u8, []const u8 } = undefined;

    comptime var state : parsing_state = parsing_state.expects_name;
    comptime var index = 0;
    while (data.next()) |text| {
        if (text.len < 2)
            continue;

        switch (state) {
            .expects_name => {
                array[index].@"0" = get_cleaned_string(text);
                state = .expects_content;
            },
            .expects_content => {
                array[index].@"1" = text;
                state = .expects_name;
                index += 1;
            }
        }
    }

    return std.StaticStringMap([]const u8).initComptime(array);
}