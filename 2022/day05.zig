export const year: u16 = 2022;
export const day: u8 = 5;

const std = @import("std");
const STACK = std.ArrayList(u8);
const SHIP = std.ArrayList(STACK);
const CRATE_SIZE = 4;
const MAX_SHIP_LINE_LENGTH = 1024;
const MAX_INSTRUCTION_LINE_LENGTH = 512;
const common = @cImport(@cInclude("common.h"));

const Ship = struct {
    inner: ?SHIP,

    // return empty ship
    pub fn empty() Ship {
        return Ship{ .inner = null };
    }

    // allocate memory for new ship
    pub fn new(size: usize, allocator: *std.mem.Allocator) !Ship {
        var inner = try SHIP.initCapacity(allocator.*, size);

        for (0..size) |_|
            inner.appendAssumeCapacity(STACK.init(allocator.*));

        return Ship{ .inner = inner };
    }

    pub fn is_initialized(self: *const Ship) bool {
        return self.*.inner != null;
    }

    // free memory for ship
    pub fn deinit(self: *Ship) void {
        if (self.inner) |i| {
            for (i.items) |stack| stack.deinit();
            i.deinit();
        }
    }

    pub fn push(self: *Ship, line: []const u8, allocator: *std.mem.Allocator) !void {
        // initialise new ship if ship is null
        if (!self.is_initialized()) {
            const stacks = line.len / CRATE_SIZE + @intFromBool(line.len % CRATE_SIZE > 0);
            self.* = try new(stacks, allocator);
        }

        // parse each line and push to corresponding stacks
        for (line, 0..) |c, i| {
            if (!std.ascii.isUpper(c)) continue; // not a crate
            if (i / 4 >= self.*.inner.?.capacity) return; // out of range!

            try self.*.inner.?.items[i / 4].append(c);
        }
    }

    // execute instruction (part a - CrateMover 9000); move len crates one by one
    pub fn exec_a(self: *Ship, instruction: Instruction) !void {
        std.debug.assert(self.is_initialized());
        if (self.*.inner.?.items.len == 0) return; // should return error

        for (0..instruction.len) |_| {
            try self.*.inner.?.items[instruction.dst].append(self.*.inner.?.items[instruction.src].pop());
        }
    }

    // execute instruction (part b - CrateMover 9001); move len crates together
    pub fn exec_b(self: *Ship, instruction: Instruction) !void {
        std.debug.assert(self.is_initialized());
        if (self.*.inner.?.items.len == 0) return; // should return error

        const val = self.*.inner.?.items[instruction.src].items[self.*.inner.?.items[instruction.src].items.len - instruction.len ..];
        try self.*.inner.?.items[instruction.dst].appendSlice(val);
        self.*.inner.?.items[instruction.src].items.len -= instruction.len; // this is how pop works
    }

    // reverse
    pub fn reverse(self: *Ship) void {
        std.debug.assert(self.is_initialized());

        for (self.inner.?.items) |stack| {
            std.mem.reverse(u8, stack.items);
        }
    }

    // debug print
    pub fn print(self: *const Ship, comptime callback: fn (comptime []const u8, anytype) void) void {
        std.debug.assert(self.is_initialized());

        for (self.*.inner.?.items, 0..) |stack, i| {
            callback("{}: ", .{i});

            for (stack.items) |crate| {
                callback("{c}", .{crate});
            }

            callback("\n", .{});
        }
    }

    // get a string containing crates that end up on top of each stack
    pub fn top_crates(self: *const Ship, allocator: *std.mem.Allocator) !STACK {
        std.debug.assert(self.is_initialized());
        var string = try STACK.initCapacity(allocator.*, self.*.inner.?.capacity);

        for (self.*.inner.?.items) |stack| {
            string.appendAssumeCapacity(stack.getLastOrNull() orelse continue);
        }

        return string;
    }
};

const Instruction = struct {
    src: usize,
    dst: usize,
    len: usize,

    // parse line "move <len> from <src> to <dst>" into optional self
    pub fn parse(line: []const u8) ?Instruction {
        var parts = std.mem.splitScalar(u8, line, ' ');
        var self: Instruction = undefined;

        _ = parts.next();
        const len = parts.next() orelse return null;
        self.len = std.fmt.parseUnsigned(usize, len, 10) catch return null;

        _ = parts.next();
        const src = parts.next() orelse return null;
        self.src = (std.fmt.parseUnsigned(usize, src, 10) catch return null) - 1;

        _ = parts.next();
        const dst = parts.next() orelse return null;
        self.dst = (std.fmt.parseUnsigned(usize, dst, 10) catch return null) - 1;

        return self;
    }
};

pub fn read_file(reader: anytype, allocator: *std.mem.Allocator, exec_callback: *const fn (*Ship, Instruction) std.mem.Allocator.Error!void) !STACK {
    var ship: Ship = Ship.empty();
    defer ship.deinit();

    // parse ship
    while (try reader.readUntilDelimiterOrEofAlloc(allocator.*, '\n', MAX_SHIP_LINE_LENGTH)) |line| {
        defer allocator.free(line);

        if (line.len < 7) break; // too few stacks (less than two)
        if (line[1] == '1') break; // break once done parsing (`1` being from the crate index)

        try ship.push(line, allocator);
    }

    ship.reverse(); // reverse each stack after reading from top
    try reader.skipUntilDelimiterOrEof('\n');

    // parse and execute instructions
    while (try reader.readUntilDelimiterOrEofAlloc(allocator.*, '\n', MAX_INSTRUCTION_LINE_LENGTH)) |line| {
        defer allocator.free(line);

        const x = Instruction.parse(line) orelse continue;

        try exec_callback(&ship, x);
    }

    return try ship.top_crates(allocator);
}

fn solver(buf: common.buf_t, func: *const fn (*Ship, Instruction) std.mem.Allocator.Error!void) common.buf_t {
    var buffer: []u8 = undefined;
    buffer.len = @intCast(buf.len);
    buffer.ptr = buf.ptr;

    var stream = std.io.fixedBufferStream(buffer);
    var reader = stream.reader();
    var alloc = std.heap.raw_c_allocator;

    var bytes: []u8 = undefined;
    var out = common.buf_t{ .ptr = null, .len = -1 };

    var result: STACK = read_file(reader, &alloc, func) catch |e| {
        std.log.err("{}", .{e});
        return out;
    };

    result.append(0) catch return out;
    bytes = result.toOwnedSlice() catch return out;
    out.ptr = bytes.ptr;
    out.len = @intCast(bytes.len);

    return out;
}

export fn solve1(buffer: common.buf_t) callconv(.C) common.buf_t {
    return solver(buffer, Ship.exec_a);
}

export fn solve2(buffer: common.buf_t) callconv(.C) common.buf_t {
    return solver(buffer, Ship.exec_b);
}

test "part one: ship push" {
    const case = struct {
        line: []const u8,
        expected: []const u8,
    };

    const cases = [_]case{
        case{ .line = "[P] [H] [P] [Q] [P] [M] [P] [F] [D]", .expected = "PHPQPMPFD" },
        case{ .line = "[R]     [S] [G]     [P]         [H]", .expected = "R SG P  H" },
    };

    var allocator = std.testing.allocator;

    for (cases) |c| {
        var ptr: Ship = Ship.empty();
        try ptr.push(c.line, &allocator);

        for (ptr.inner.?.items, 0..) |stack, i| {
            const x = stack.getLastOrNull();
            const y = if (x == null) ' ' else x.?;
            try std.testing.expectEqual(c.expected[i], y);
        }

        defer ptr.deinit();
    }
}

test "part one: instruction" {
    const case = struct {
        line: []const u8,
        expected: ?Instruction,
    };

    const cases = [_]case{
        case{ .line = "move 3 from 8 to 9", .expected = Instruction{ .len = 3, .src = 8, .dst = 9 } },
        case{ .line = "move 169 from 214 to 2", .expected = Instruction{ .len = 169, .src = 214, .dst = 2 } },
        case{ .line = "move from to", .expected = null },
    };

    for (cases) |c| {
        try std.testing.expectEqual(c.expected, Instruction.parse(c.line));
    }
}
