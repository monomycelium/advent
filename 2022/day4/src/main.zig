// Advent of Code 2022, Day 4

// TODO: modify get_sum to be more flexible by solving both parts at once
//
// - take in an array of callback functions and return an array of usize?
//      each element in the returned array would represent the sum of lines
//      that return true when callback i is called?

const std = @import("std");
const MAX_LINE_LENGTH = 13;

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
    sum: usize,

    // return whether an elf's assignment contains the other elf's in the pair
    pub fn contains(pair: [4]usize) bool {
        return (pair[0] <= pair[2] and pair[1] >= pair[3]) or (pair[2] <= pair[0] and pair[3] >= pair[1]);
    }

    // wrapper to solve for part a
    pub fn solve(reader: anytype) !part_a {
        return part_a{ .sum = try get_sum(reader, contains) };
    }
};

const part_b = struct {
    sum: usize,

    // return whether the pair overlaps
    pub fn overlaps(pair: [4]usize) bool {
        return pair[0] <= pair[3] and pair[2] <= pair[1];
    }

    // wrapper to solve for part a
    pub fn solve(reader: anytype) !part_b {
        return part_b{ .sum = try get_sum(reader, overlaps) };
    }
};

pub fn main() !void {
    if (std.os.argv.len != 2) {
        std.log.err("usage: {s} <INPUT_FILE>", .{std.os.argv[0]});
        std.process.exit(1);
    }

    // open file for buffered reading
    const path = std.mem.span(std.os.argv[1]);
    var file = try std.fs.cwd().openFile(path, .{ .mode = .read_only });
    defer file.close();

    // prepare buffered reader
    var buf_reader = std.io.bufferedReader(file.reader());
    var reader = buf_reader.reader();

    const a: part_a = try part_a.solve(reader);
    try file.seekTo(0); // rewind to the start of the file
    const b: part_b = try part_b.solve(reader);

    const stdout = std.io.getStdOut().writer();
    try stdout.print("part a: {}\npart b: {}\n", .{ a.sum, b.sum });
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
