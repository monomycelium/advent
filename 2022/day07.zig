// TODO: fix segfault in Part 2.

export const year: u16 = 2022;
export const day: u8 = 7;

const std = @import("std");
const Allocator = std.mem.Allocator;
const Invalid = std.fmt.ParseIntError.InvalidCharacter;

const MAX_LINE_LENGTH = 21;
const SPACE_TOTAL = 70000000;
const SPACE_NEEDED = 30000000;
const SPACE_MAX = 100000;

const common = @cImport(@cInclude("common.h"));

const FsTree = struct {
    const Children = std.ArrayList(Item);

    const Dir = struct {
        parent: ?*Dir,
        items: Children,
        size: ?usize,

        pub fn init(parent: ?*Dir, allocator: *Allocator) !*Dir {
            const dir = try allocator.create(Dir);
            const items = Children.init(allocator.*);

            dir.*.parent = parent;
            dir.*.items = items;
            dir.*.size = null;

            return dir;
        }

        pub fn deinit(self: *Dir, allocator: *Allocator) void {
            for (self.items.items) |child|
                child.deinit(allocator);

            self.items.deinit();
            allocator.destroy(self); // i am allocated!
            self.* = undefined;
        }

        // returns cached or calculated size of directory recursively
        pub fn get_size(self: *Dir) usize {
            if (self.size) |s| return s;

            self.size = 0;
            for (0..self.items.items.len) |i| // needs mut
                self.size.? += self.items.items[i].get_size();

            return self.size.?;
        }

        pub fn solve(self: *const Dir, condition: *const fn (usize) bool, allocator: *Allocator) !std.ArrayList(*Dir) {
            var directories = std.ArrayList(*Dir).init(allocator.*);

            for (0..self.items.items.len) |i| {
                var child = self.items.items[i];

                // skip if not dir
                if (child.item != .Dir) continue;

                // read grandchildren
                const grandchildren = try child.more.children.?.solve(condition, allocator);
                try directories.appendSlice(grandchildren.items);
                grandchildren.deinit();

                // add current dir if condition is met
                if (condition(child.more.children.?.get_size()))
                    try directories.append(child.more.children.?); // segfaults?
            }

            return directories;
        }
    };

    const ItemType = enum { File, Dir };

    const ItemMore = union {
        size: usize, // file size
        children: ?*Dir, // directory children
    };

    const Item = struct {
        item: ItemType,
        name: []const u8,
        more: ItemMore,

        pub fn deinit(self: *const Item, allocator: *Allocator) void {
            allocator.free(self.name);

            if (self.item == .Dir)
                if (self.more.children) |children|
                    children.deinit(allocator);
        }

        fn parse_dir(line: []const u8, allocator: *Allocator) !Item {
            if (line.len < 4) return Invalid;
            const name = try allocator.dupe(u8, line[4..]);

            return Item{
                .item = ItemType.Dir,
                .name = name,
                .more = ItemMore{ .children = null },
            };
        }

        fn parse_file(line: []const u8, allocator: *Allocator) !Item {
            const space = std.mem.indexOfScalar(u8, line, ' ');
            if (space == null) return Invalid;

            const size = try std.fmt.parseUnsigned(
                usize,
                line[0..space.?],
                10,
            );
            const name = try allocator.dupe(u8, line[space.? + 1 ..]);

            return Item{
                .item = ItemType.File,
                .name = name,
                .more = ItemMore{ .size = size },
            };
        }

        fn parse_item(line: []const u8, allocator: *Allocator, item: ItemType) !Item {
            return switch (item) {
                ItemType.Dir => parse_dir(line, allocator),
                ItemType.File => parse_file(line, allocator),
            };
        }

        pub fn get_size(self: *Item) usize {
            switch (self.item) {
                ItemType.Dir => return self.more.children.?.get_size(),
                ItemType.File => return self.more.size,
            }
        }

        pub fn parse(line: []const u8, allocator: *Allocator) !?Item {
            if (line.len < 3) return null; // invalid
            var item: ?ItemType = null;

            if (std.mem.eql(u8, line[0..3], "dir")) {
                item = ItemType.Dir;
            } else if (std.ascii.isDigit(line[0])) item = ItemType.File;

            if (item) |i| {
                return try parse_item(line, allocator, i);
            } else return null; // invalid item
        }
    };

    root: *Dir,
    current: *Dir,
    allocator: *Allocator,

    pub fn init(allocator: *Allocator) !FsTree {
        const dir = try Dir.init(null, allocator);
        return FsTree{ .root = dir, .current = dir, .allocator = allocator };
    }

    pub fn deinit(self: *const FsTree) void {
        self.root.deinit(self.allocator);
    }

    pub fn get_size(self: *FsTree) usize {
        return self.root.get_size();
    }

    // cd ..
    pub fn go_up(self: *FsTree) void {
        if (self.current.parent) |p| {
            self.current = p;
        }
    }

    // cd child
    pub fn go_to_child(self: *FsTree, child: []const u8) void {
        for (self.current.items.items) |i| {
            if (i.item == .Dir and std.mem.eql(u8, i.name, child)) {
                self.current = i.more.children.?;
                return;
            }
        }
    }

    // cd /
    pub fn go_to_root(self: *FsTree) void {
        self.current = self.root;
    }

    // touch / mkdir child
    pub fn add_child(self: *FsTree, child: Item) !void {
        var item = child;
        if (item.item == ItemType.Dir and item.more.children == null) {
            item.more.children = try Dir.init(
                self.current,
                self.allocator,
            );
        }

        try self.current.items.append(item);
    }

    // return total sizes of all directories that meet condition
    pub fn solve(self: *const FsTree, condition: *const fn (usize) bool) !std.ArrayList(*Dir) {
        return try self.root.solve(condition, self.allocator);
    }

    // parser (sadly, the command-parsing is hard-coded)
    pub fn parse(reader: anytype, allocator: *Allocator) !FsTree {
        var tree = try FsTree.init(allocator);
        var buf: [MAX_LINE_LENGTH]u8 = undefined;

        var line: []u8 = try reader.readUntilDelimiter(&buf, '\n');
        outer: while (true) { // loop that reads a command every iteration
            if (line.len == 0) break;
            if (line[0] != '$' or line.len < 4) return Invalid;

            if (std.mem.eql(u8, line[2..4], "cd")) {
                if (line.len <= 5) return Invalid;
                const cd = line[5..];

                if (std.mem.eql(u8, cd, "..")) {
                    tree.go_up();
                } else if (std.mem.eql(u8, cd, "/")) {
                    tree.go_to_root();
                } else {
                    tree.go_to_child(cd);
                }

                line = try reader.readUntilDelimiterOrEof(
                    &buf,
                    '\n',
                ) orelse break;
                continue;
            } else if (!std.mem.eql(u8, line[2..4], "ls"))
                return Invalid; // early returns for sanity's sake

            while (true) {
                line = try reader.readUntilDelimiterOrEof(
                    &buf,
                    '\n',
                ) orelse break :outer;
                if (line.len == 0 or line[0] == '$') continue :outer;
                // deal with command in next iteration

                const item = try Item.parse(
                    line,
                    tree.allocator,
                ) orelse return Invalid;

                try tree.add_child(item);
            }
        }

        return tree;
    }
};

var tree_g: ?FsTree = null;

const Part1 = struct {
    pub fn solve(tree: *FsTree) !usize {
        const a = try tree.solve(Part1.condition);
        defer a.deinit();

        var a_sum: usize = 0;
        for (a.items) |i|
            a_sum += i.size.?;

        return a_sum;
    }

    pub fn condition(size: usize) bool {
        return size <= SPACE_MAX;
    }
};

const Part2 = struct {
    var tree_size: usize = undefined;

    pub fn solve(tree: *FsTree) !usize {
        Part2.tree_size = tree.get_size();
        const b = try tree.solve(Part2.condition);
        defer b.deinit();

        var b_sum: usize = std.math.maxInt(usize);
        for (b.items) |item|
            b_sum = @min(b_sum, item.size.?);

        return b_sum;
    }

    pub fn condition(size: usize) bool {
        return size >= SPACE_NEEDED - (SPACE_TOTAL - tree_size);
    }
};

fn solver(
    buf: common.buf_t,
    solve: *const fn (tree: *FsTree) @typeInfo(@typeInfo(@TypeOf(Part2.solve)).Fn.return_type.?).ErrorUnion.error_set!usize,
) common.buf_t {
    var buffer: []u8 = undefined;
    buffer.len = @intCast(buf.len);
    buffer.ptr = buf.ptr;

    var stream = std.io.fixedBufferStream(buffer);
    var reader = stream.reader();
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    var alloc = gpa.allocator();

    var out = common.buf_t{ .ptr = null, .len = -1 };

    if (tree_g == null) tree_g = FsTree.parse(reader, &alloc) catch |e| {
        std.log.err("{}", .{e});
        return out;
    };

    const result: usize = solve(&tree_g.?) catch return out;
    out.ptr = @ptrFromInt(result);
    out.len = 0;

    return out;
}

export fn solve1(buf: common.buf_t) callconv(.C) common.buf_t {
    return solver(buf, Part1.solve);
}

export fn solve2(buf: common.buf_t) callconv(.C) common.buf_t {
    var res = solver(buf, Part2.solve);
    tree_g.?.deinit();
    tree_g = null;
    return res;
}
