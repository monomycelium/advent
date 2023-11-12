export const year: u16 = 2022;
export const day: u8 = 6;

const std = @import("std");

const MAX_FILE_SIZE = 4096;
const START_PACKET_MARKER_LEN = 4;
const START_MESSAGE_MARKER_LEN = 14;
const MAX_POSSIBLE_BYTES = 'z' - 'a' + 1; // alphabet size
const BITSET = std.bit_set.IntegerBitSet(MAX_POSSIBLE_BYTES);
const common = @cImport(@cInclude("common.h"));

// returns whether slice is a start marker
// by checking that the bytes do not repeat
// can be used for both start-of-packet and start-of-message markers
pub fn is_start_marker(slice: []const u8) bool {
    var alphabet = BITSET.initEmpty();

    for (slice) |c| {
        // adjust for the kind of bytes

        const byte = (c & 0x1F) - 1;
        if (alphabet.isSet(byte)) return false;
        alphabet.set(byte);
    }

    return true;
}

// returns indexes of byte after start markers
pub fn packet_start(stream: []const u8, length: usize) ?usize {
    for (0..(stream.len - length + 1)) |i| {
        const index = length + i;
        const slice = stream[i..index];

        if (is_start_marker(slice)) return index;
    }

    return null;
}

fn solver(buf: common.buf_t, length: usize) common.buf_t {
    var stream: []u8 = undefined;
    var result = common.buf_t{ .ptr = null, .len = 0 };

    stream.len = buf.len;
    stream.ptr = buf.ptr;

    const r: usize = packet_start(stream, length) orelse return result;
    const x: []u8 = strFromInt(r, std.heap.raw_c_allocator) catch return result;

    result.ptr = x.ptr;
    result.len = x.len;
    return result;
}

export fn solve1(buf: common.buf_t) callconv(.C) common.buf_t {
    return solver(buf, START_PACKET_MARKER_LEN);
}

export fn solve2(buf: common.buf_t) callconv(.C) common.buf_t {
    return solver(buf, START_MESSAGE_MARKER_LEN);
}

fn strFromInt(value: anytype, allocator: std.mem.Allocator) ![]u8 {
    var buf: []u8 = try allocator.alloc(u8, std.math.log10(std.math.maxInt(@TypeOf(value))) + 1);
    const u: usize = std.fmt.formatIntBuf(buf, value, 10, .lower, .{ .alignment = .left });

    buf[u] = 0;
    return try allocator.realloc(buf, u + 1);
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
