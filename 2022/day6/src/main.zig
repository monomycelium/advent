// DISCLAIMER:
// Usually, I would solve the two parts separately, but I combined them
// (hopefully without significantly reducing readability) as the challenge
// seemed to want both parts in the end, unlike the previous challenges.

const std = @import("std");

const MAX_FILE_SIZE = 4096;
const START_PACKET_MARKER_LEN = 4;
const START_MESSAGE_MARKER_LEN = 14;
const MAX_POSSIBLE_BYTES = 'z' - 'a' + 1; // alphabet size
const BITSET = std.bit_set.IntegerBitSet(MAX_POSSIBLE_BYTES);

// returns whether slice is a start marker
// by checking that the bytes do not repeat
// can be used for both start-of-packet and start-of-message markers
pub fn is_start_marker(slice: []const u8) bool {
    var alphabet = BITSET.initEmpty();

    for (slice) |c| {
        // adjust for the kind of bytes
        // this only works for lowercase characters

        if (!std.ascii.isLower(c)) continue;
        if (alphabet.isSet(c - 'a')) return false;

        alphabet.set(c - 'a');
    }

    return true;
}

// returns indexes of each byte after start markers
pub fn packet_start(stream: []const u8, comptime lengths: []const usize) [lengths.len]?usize {
    var results: [lengths.len]?usize = std.mem.zeroes([lengths.len]?usize);

    // iterate through range to get slices
    for (0..stream.len) |i| {
        for (lengths, 0..) |l, r| { // for each required-length of start markers
            if (results[r] != null or i >= stream.len - l + 1) continue; // skip if solved

            const index = l + i;
            const slice = stream[i..index];

            if (is_start_marker(slice)) results[r] = index;
        }
    }

    return results;
}

pub fn main() !void {
    if (std.os.argv.len != 2) {
        std.log.err("usage: {s} <INPUT_FILE>", .{std.os.argv[0]});
        std.process.exit(1);
    }

    // open file
    const path = std.mem.span(std.os.argv[1]);
    var file = try std.fs.cwd().openFile(path, .{ .mode = .read_only });
    defer file.close();

    // read file
    var buf: [MAX_FILE_SIZE]u8 = undefined;
    _ = try file.readAll(&buf);

    // solve
    const a = packet_start(&buf, &[_]usize{
        START_PACKET_MARKER_LEN,
        START_MESSAGE_MARKER_LEN,
    });

    // print results
    const stdout = std.io.getStdOut().writer();
    try stdout.print("part a: {?}\npart b: {?}\n", .{ a[0], a[1] });
}

test "packet_start" {
    const Test = struct {
        input: []const u8,
        expected: ?usize,
    };

    const tests = [_]Test{
        Test{ .input = "mjqjpqmgbljsphdztnvjfqwrcgsmlb", .expected = 7 },
        Test{ .input = "bvwbjplbgvbhsrlpgdmjqwftvncz", .expected = 5 },
        Test{ .input = "nppdvjthqldpwncqszvftbrmjlhg", .expected = 6 },
        Test{ .input = "nznrnfrfntjfmvfwmzdfjlvtqnbhcprsg", .expected = 10 },
        Test{ .input = "zcfzfwzzqfrljwzlrfnpqdbhtmscgvjw", .expected = 11 },
    };

    for (tests) |t| {
        try std.testing.expectEqual(t.expected, packet_start(t.input));
    }
}
