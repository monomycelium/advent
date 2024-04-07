export const year: u16 = 2022;
export const day: u8 = 4;

const std = @import("std");
const MAX_LINE_LENGTH = 13;
const common = @cImport(@cInclude("common.h"));

// iterate through lines of the reader
// and increment sum if callback returns true for each pair
pub fn get_sum(reader: anytype, callback: *const fn ([4]usize) bool) !usize {
    var sum: usize = 0;

    var buffer: [MAX_LINE_LENGTH]u8 = undefined;
    while (try reader.readUntilDelimiterOrEof(&buffer, '\n')) |line| {
        // if only zig had sscanf! separate line using delimeters instead
        // initialise pair "a-b,x-y" -> [4]usize{a, b, x, y}

        var sequence = std.mem.splitAny(u8, line, "-,");

        var pair: [4]usize = undefined;
        var i: usize = 0;
        while (sequence.next()) |split| {
            pair[i] = try std.fmt.parseUnsigned(usize, split, 10);

            i += 1;
            if (i == pair.len) break;
        }

        if (callback(pair)) sum += 1;
    }

    return sum;
}

const part_a = struct {
    // return whether an elf's assignment contains the other elf's in the pair
    pub fn contains(pair: [4]usize) bool {
        return (pair[0] <= pair[2] and pair[1] >= pair[3]) or (pair[2] <= pair[0] and pair[3] >= pair[1]);
    }

    // wrapper to solve for part a
    pub fn solve(reader: anytype) ?usize {
        return get_sum(reader, contains) catch |e| {
            std.log.err("failed: {}", .{e});
            return null;
        };
    }
};

const part_b = struct {
    // return whether the pair overlaps
    pub fn overlaps(pair: [4]usize) bool {
        return pair[0] <= pair[3] and pair[2] <= pair[1];
    }

    // wrapper to solve for part a
    pub fn solve(reader: anytype) ?usize {
        return get_sum(reader, overlaps) catch |e| {
            std.log.err("failed: {}", .{e});
            return null;
        };
    }
};

fn solver(buf: common.buf_t, func: *const fn (reader: anytype) ?usize) common.buf_t {
    var buffer: []u8 = undefined;
    buffer.len = @intCast(buf.len);
    buffer.ptr = buf.ptr;

    var stream = std.io.fixedBufferStream(buffer);
    var reader = stream.reader();

    const result: usize = func(reader) orelse return common.buf_t{ .ptr = null, .len = -1 };

    return common.buf_t{
        .len = 0,
        .ptr = @ptrFromInt(result),
    };
}

export fn solve1(buf: common.buf_t) callconv(.C) common.buf_t {
    return solver(buf, part_a.solve);
}

export fn solve2(buf: common.buf_t) callconv(.C) common.buf_t {
    return solver(buf, part_b.solve);
}

test "part a: contains" {
    const tests = [_][4]usize{
        [4]usize{ 2, 4, 6, 8 },
        [4]usize{ 2, 3, 4, 5 },
        [4]usize{ 5, 7, 7, 9 },
        [4]usize{ 2, 8, 3, 7 },
        [4]usize{ 6, 6, 4, 6 },
        [4]usize{ 2, 6, 4, 8 },
    };

    const results = [_]bool{ false, false, false, true, true, false };

    for (tests, results) |t, r| {
        try std.testing.expectEqual(r, part_a.contains(t));
    }
}

test "part b: overlaps" {
    const tests = [_][4]usize{
        [4]usize{ 2, 4, 6, 8 },
        [4]usize{ 2, 3, 4, 5 },
        [4]usize{ 5, 7, 7, 9 },
        [4]usize{ 2, 8, 3, 7 },
        [4]usize{ 6, 6, 4, 6 },
        [4]usize{ 2, 6, 4, 8 },
    };

    const results = [_]bool{
        false,
        false,
        true,
        true,
        true,
        true,
    };

    for (tests, results) |t, r| {
        try std.testing.expectEqual(r, part_b.overlaps(t));
    }
}
