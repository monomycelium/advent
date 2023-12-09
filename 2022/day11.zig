export const year: u16 = 2022;
export const day: u8 = 11;

const std = @import("std");
const mem = std.mem;
const c = @cImport(@cInclude("common.h"));

const List = std.ArrayListUnmanaged;
const Callback: type = *const fn (*State, *Item) void;
const Item: type = usize;
const Items: type = List(Item);

const Monkey: type = struct {
    items: Items,
    oprtn: Operation,
    testd: Test,
    count: usize,

    /// Inspect an item and evaluate worry level.
    fn inspect(self: *Monkey, item: *Item) void {
        self.count += 1;
        item.* = self.oprtn.eval(item.*);
    }

    fn lessThanFn(_: void, a: Monkey, b: Monkey) bool {
        return a.count > b.count;
    }
};

const State: type = struct {
    monkeys: []Monkey,
    allocator: mem.Allocator,
    divisor: usize, // common divisor

    fn round(state: *State, callback: Callback) !void {
        for (state.monkeys, 0..) |*m, x| {
            _ = x;

            for (m.items.items) |*i| {
                m.inspect(i);
                callback(state, i);
                const dest: usize = m.testd.eval(i.*);
                std.debug.assert(dest < state.monkeys.len);
                try state.monkeys[dest].items.append(state.allocator, i.*);
            }

            m.items.clearRetainingCapacity();
        }
    }

    fn parse(input: []const u8, alloc: mem.Allocator) !State {
        const Array: type = List(Monkey);
        var array: Array = try Array.initCapacity(alloc, 0);
        errdefer array.deinit(alloc);

        var count: u4 = 0;
        var index: usize = 0;
        var monkey: Monkey = undefined;
        var divisor: usize = 1;
        var iter: mem.SplitIterator(u8, .scalar) = mem.splitScalar(
            u8,
            input,
            '\n',
        );

        while (iter.next()) |line| {
            switch (count) {
                0 => { // Monkey %zu:
                    const str: []const u8 = "Monkey ";
                    const start: usize = str.len;

                    if (line.len < start + 2 or
                        line[line.len - 1] != ':' or
                        !mem.eql(u8, line[0..start], str))
                        return error.ParseError;

                    index = std.fmt.parseUnsigned(
                        usize,
                        line[start .. line.len - 1],
                        10,
                    ) catch return error.ParseError;
                },
                1 => {
                    const str: []const u8 = "  Starting items:";
                    const start: usize = str.len + 1;

                    if (line.len < str.len or
                        line.len == start or
                        !mem.eql(u8, line[0..str.len], str))
                        return error.ParseError;
                    if (line.len == str.len) break; // empty items

                    monkey.items = try Items.initCapacity(alloc, 0);
                    var items: mem.SplitIterator(u8, .sequence) = mem.splitSequence(
                        u8,
                        line[str.len + 1 ..],
                        ", ",
                    );

                    while (items.next()) |num|
                        try monkey.items.append(alloc, std.fmt.parseUnsigned(
                            u8,
                            num,
                            10,
                        ) catch return error.ParseError);
                },
                2 => {
                    const str: []const u8 = "  Operation: new = ";
                    const start: usize = str.len;

                    if (line.len < start + 5 or
                        !mem.eql(u8, line[0..str.len], str))
                        return error.ParseError;

                    var cnt: u3 = 0;
                    var items: mem.SplitIterator(u8, .scalar) = mem.splitScalar(
                        u8,
                        line[start..],
                        ' ',
                    );

                    while (items.next()) |it| {
                        switch (cnt) {
                            0 => monkey.oprtn.lhs = try Operation.parseTerm(it),
                            2 => monkey.oprtn.rhs = try Operation.parseTerm(it),
                            1 => {
                                if (it.len != 1) return error.ParseError;
                                monkey.oprtn.opr = Operation.Operator.parse(it[0]) orelse return error.ParseError;
                            },
                            else => return error.ParseError,
                        }

                        cnt += 1;
                    }
                },
                3, 4, 5 => {
                    const i: u2 = @intCast(count - 3);
                    const str: []const u8 = switch (i) {
                        0 => "  Test: divisible by ",
                        1 => "    If true: throw to monkey ",
                        2 => "    If false: throw to monkey ",
                        else => unreachable,
                    };
                    const start: usize = str.len;

                    if (line.len < start + 1 or
                        !mem.eql(u8, line[0..str.len], str))
                        return error.ParseError;

                    const ptr: *usize = switch (i) {
                        0 => &monkey.testd.divisor,
                        1 => &monkey.testd.receive[1],
                        2 => &monkey.testd.receive[0],
                        else => unreachable,
                    };

                    ptr.* = std.fmt.parseUnsigned(
                        u8,
                        line[start..],
                        10,
                    ) catch return error.ParseError;

                    if (i == 0) divisor *= monkey.testd.divisor;
                },
                else => {
                    monkey.count = 0;

                    if (index != array.items.len) return error.ParseError;
                    try array.append(alloc, monkey); // ignores given index

                    monkey = undefined;
                    count = 0;
                    continue;
                },
            }

            count += 1;
        }

        return .{
            .allocator = alloc,
            .monkeys = try array.toOwnedSlice(alloc),
            .divisor = divisor,
        };
    }

    fn deinit(state: *State) void {
        for (state.monkeys) |*m|
            m.items.deinit(state.allocator);

        state.allocator.free(state.monkeys);
        state.* = undefined;
    }
};

const Operation: type = struct {
    const Term: type = ?Item; // null for old

    const Operator: type = enum {
        add,
        mul,

        fn parse(scalar: u8) ?Operator {
            return switch (scalar) {
                '+' => .add,
                '*' => .mul,
                else => null,
            };
        }
    };

    opr: Operator,
    lhs: Term,
    rhs: Term,

    fn parseTerm(term: []const u8) !Term {
        return std.fmt.parseUnsigned(
            usize,
            term,
            10,
        ) catch if (mem.eql(u8, term, "old")) null else error.ParseError;
    }

    fn eval(self: Operation, item: Item) Item {
        const lhs: Item = self.lhs orelse item;
        const rhs: Item = self.rhs orelse item;

        return switch (self.opr) {
            .add => lhs + rhs,
            .mul => lhs * rhs,
        };
    }
};

const Test: type = struct {
    divisor: usize,
    receive: [2]usize,

    fn eval(t: Test, w: usize) usize {
        return t.receive[@intFromBool(w % t.divisor == 0)];
    }
};

fn solve(input: c.buf_t, n: usize, callback: Callback) c.buf_t {
    var ret: c.buf_t = .{ .ptr = null, .len = 0 };

    var buf: []u8 = undefined;
    buf.ptr = input.ptr orelse return ret;
    buf.len = input.len;

    var state: State = State.parse(buf, std.heap.raw_c_allocator) catch |e| {
        std.log.err("failed to parse input: {any}", .{e});
        return ret;
    };
    defer state.deinit();

    for (0..n) |_| state.round(callback) catch return ret;

    // for (state.monkeys) |monkey| std.debug.print("{any}\n", .{monkey.items.items});
    // std.debug.print("\n", .{});

    mem.sort(Monkey, state.monkeys, void{}, Monkey.lessThanFn);
    const num: []u8 = strFromInt(state.monkeys[0].count * state.monkeys[1].count, std.heap.raw_c_allocator) catch return ret;
    return .{ .ptr = num.ptr, .len = num.len };
}

fn strFromInt(value: anytype, allocator: mem.Allocator) ![]u8 {
    var buf: []u8 = try allocator.alloc(u8, maxBytes(@TypeOf(value)) + 2); // +1 for NUL byte, +1 for minus sign
    const u: usize = std.fmt.formatIntBuf(buf, value, 10, .lower, .{ .alignment = .left });

    buf[u] = 0;
    return try allocator.realloc(buf, u + 1);
}

fn maxBytes(comptime T: type) usize {
    return std.fmt.comptimePrint("{}", .{std.math.maxInt(T)}).len;
}

fn call1(_: *State, item: *Item) void {
    item.* /= 3;
}

fn call2(state: *State, item: *Item) void {
    item.* %= state.divisor;
}

export fn solve1(input: c.buf_t) c.buf_t {
    return solve(input, 20, call1);
}

export fn solve2(input: c.buf_t) c.buf_t {
    return solve(input, 10000, call2);
}
