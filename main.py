import sys


def print_board(n, board):
    for y in range(n):
        for x in range(n):
            print {None: ' ', 0: '/', 1: '\\'}[board.get((x, y))],
        print


def iter_enters(n):
    for i in range(n):
        yield -1, i
        yield i, -1
        yield n, i
        yield i, n


def trace_path(n, board, enter):
    x, y = enter
    dx = dy = 0
    if x == -1:
        dx = 1
    elif y == -1:
        dy = 1
    elif x == n:
        dx = -1
    elif y == n:
        dy = -1
    else:
        assert False

    x += dx
    y += dy

    visited = set()
    result = []
    while 0 <= x < n and 0 <= y < n:
        q = board.get((x, y))
        if (x, y) in visited:
            q = None
        if q is not None:
            if q:
                dy, dx = dx, dy
            else:
                dy, dx = -dx, -dy
            visited.add((x, y))
            result.append((x, y))

        x += dx
        y += dy

    return result


class SkipCache(object):
    def __init__(self, n, board):
        assert len(board) == n*n
        self.up = {}
        self.down = {}
        self.left = {}
        self.right = {}
        for x in range(n):
            for y in range(n):
                xy = x, y
                self.up[xy] = (x, y-1)
                self.down[xy] = (x, y+1)
                self.left[xy] = (x-1, y)
                self.right[xy] = (x+1, y)
        for i in range(n):
            self.down[i, -1] = (i, 0)
            self.up[i, n] = (i, n-1)
            self.right[-1, i] = (0, i)
            self.left[n, i] = (n-1, i)

    def break_mirrors(self, path):
        for xy in path:
            x, y = xy
            self.down[self.up[xy]] = self.down[xy]
            self.up[self.down[xy]] = self.up[xy]
            self.left[self.right[xy]] = self.left[xy]
            self.right[self.left[xy]] = self.right[xy]

            del self.up[xy]
            del self.down[xy]
            del self.left[xy]
            del self.right[xy]

    def trace_path(self, n, board, enter):
        x, y = enter
        if x == -1:
            dir = self.right
        elif y == -1:
            dir = self.down
        elif x == n:
            dir = self.left
        elif y == n:
            dir = self.up
        else:
            assert False

        pt = dir[enter]

        visited = set()
        result = []
        while 0 <= pt[0] < n and 0 <= pt[1] < n:
            q = board.get(pt)
            if pt in visited:
                q = None
            if q is None:
                pt = dir[pt]
                continue

            if q:
                if dir is self.right:
                    dir = self.down
                elif dir is self.down:
                    dir = self.right
                elif dir is self.left:
                    dir = self.up
                elif dir is self.up:
                    dir = self.left
                else:
                    assert False
            else:
                if dir is self.right:
                    dir = self.up
                elif dir is self.up:
                    dir = self.right
                elif dir is self.left:
                    dir = self.down
                elif dir is self.down:
                    dir = self.left
                else:
                    assert False

            visited.add(pt)
            result.append(pt)

        #assert result == trace_path(n, board, enter)
        return result


class ComponentsCache(object):
    def __init__(self, n, board):
        assert len(board) == n*n
        self.row_pop = [n] * n
        self.col_pop = [n] * n

    def break_mirrors(self, path):
        for x, y in path:
            self.row_pop[y] -= 1
            self.col_pop[x] -= 1
            assert self.row_pop[y] >= 0
            assert self.col_pop[x] >= 0

    def new_isolated(self, path, skip_cache):
        path = set(path)

        n = len(self.row_pop)

        row_counts = {}
        col_counts = {}
        for x, y in path:
            if y not in row_counts:
                row_counts[y] = 1
            else:
                row_counts[y] += 1
            if x not in col_counts:
                col_counts[x] = 1
            else:
                col_counts[x] += 1

        isolated = set()
        for y, cnt in row_counts.items():
            if self.row_pop[y] == cnt + 1:
                pt = skip_cache.right[-1, y]
                while pt in path:
                    pt = skip_cache.right[pt]
                assert 0 <= pt[0] < n, pt
                if self.col_pop[pt[0]] == col_counts.get(pt[0], 0) + 1:
                    isolated.add(pt)

        for x, cnt in col_counts.items():
            if self.col_pop[x] == cnt + 1:
                pt = skip_cache.down[x, -1]
                while pt in path:
                    pt = skip_cache.down[pt]
                assert 0 <= pt[1] < n, pt
                if self.row_pop[pt[1]] == row_counts.get(pt[1], 0) + 1:
                    isolated.add(pt)

        return len(isolated)


def break_mirrors(board, path):
    backup_data = {}
    for x, y in path:
        backup_data[x, y] = board[x, y]
        del board[x, y]

    def undo():
        board.update(backup_data)

    return undo


def greedy(n, board):
    skip_cache = SkipCache(n, board)

    enters = list(iter_enters(n))
    while board:
        best_enter = None
        best_path = None
        best_len = -1
        for enter in enters:
            path = skip_cache.trace_path(n, board, enter)
            if len(path) > best_len:
                best_len = len(path)
                best_enter = enter
                best_path = path

        yield best_enter, board

        u = break_mirrors(board, best_path)
        skip_cache.break_mirrors(best_path)


def connected_components(board):
    assert board

    board = dict(board)

    xs = set()
    ys = set()
    for x, y in board:
        xs.add(x)
        ys.add(y)

    components = []

    while xs:
        visited = set()
        x = next(iter(xs))
        tasks = [x]
        while tasks:
            task = tasks.pop()
            if task in visited:
                continue
            visited.add(task)
            for x, y in board:
                if task == x and y+1000 not in visited:
                    tasks.append(y+1000)
                elif task - 1000 == y and x not in visited:
                    tasks.append(x)

        new_component = {}
        for (x, y), mirror in board.items():
            if x in visited:
                new_component[x, y] = mirror
                if x in xs:
                    xs.remove(x)
        components.append(new_component)
    return components


def greedy_depth_two(n, board):
    skip_cache = SkipCache(n, board)
    components_cache = ComponentsCache(n, board)

    enters = list(iter_enters(n))
    while board:
        print>>sys.stderr, sorted(map(len, connected_components(board)))
        best_pair = None
        best_gain = -1

        if len(board)*10 < n*n:
            def new_iso(path):
                return components_cache.new_isolated(path, skip_cache)
        else:
            def new_iso(path):
                return 0

        passing = {}
        for enter in enters:
            path = skip_cache.trace_path(n, board, enter)
            if len(path) == len(board):
                yield enter, board
                return
            gain = len(path) / (1.0 + new_iso(path))
            if gain > best_gain:
                best_gain = gain
                best_pair = (enter, None)
            for pt in path:
                if pt not in passing:
                    passing[pt] = []
                passing[pt].append(enter)

        pairs = {}
        for ps in passing.values():
            for p1 in ps:
                if p1 not in pairs:
                    pairs[p1] = set()
                for p2 in ps:
                    pairs[p1].add(p2)

        cutoff = 2 + 0.3*(2-n*0.02)

        for e1, e2s in pairs.items():
            path1 = skip_cache.trace_path(n, board, e1)
            if cutoff*len(path1) < best_gain:
                continue
            u1 = break_mirrors(board, path1)
            for e2 in e2s:
                path2 = skip_cache.trace_path(n, board, e2)
                gain = (len(path1) + len(path2)) / (2.0 + new_iso(path1+path2))
                if gain > best_gain:
                    best_gain = gain
                    best_pair = e1, e2
            u1()

        e1, e2 = best_pair

        yield e1, board
        path1 = skip_cache.trace_path(n, board, e1)

        u1 = break_mirrors(board, path1)
        skip_cache.break_mirrors(path1)
        components_cache.break_mirrors(path1)

        if e2 is not None:
            yield e2, board
            path2 = skip_cache.trace_path(n, board, e2)
            u2 = break_mirrors(board, path2)
            skip_cache.break_mirrors(path2)
            components_cache.break_mirrors(path2)


class FragileMirrors(object):
    def destroy(self, rows):
        board = {}
        n = len(rows)
        for i, row in enumerate(rows):
            assert len(row) == n
            for j, c in enumerate(row):
                board[j, i] = {'L': 0, 'R': 1}[c]

        result = []
        for enter, _ in greedy_depth_two(n, board):
            result.append(enter[1])
            result.append(enter[0])
        return result


if __name__ == '__main__':
    n = int(raw_input())
    rows = [raw_input().rstrip() for _ in range(n)]

    result = FragileMirrors().destroy(rows)

    print len(result)
    for x in result:
        print x
    sys.stdout.flush()
