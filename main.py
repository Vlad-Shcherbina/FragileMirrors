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


def greedy_depth_two(n, board):
    skip_cache = SkipCache(n, board)

    enters = list(iter_enters(n))
    while board:
        best_pair = None
        best_len = -1

        passing = {}
        for enter in enters:
            path = skip_cache.trace_path(n, board, enter)
            if len(path) == len(board):
                yield enter, board
                return
            if 2*len(path) > best_len:
                best_len = 2*len(path)
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

        cutoff = 4 + (2-n*0.02)

        for e1, e2s in pairs.items():
            path1 = skip_cache.trace_path(n, board, e1)
            if cutoff*len(path1) < best_len:
                continue
            u1 = break_mirrors(board, path1)
            for e2 in e2s:
                path2 = skip_cache.trace_path(n, board, e2)
                if len(path1) + len(path2) > best_len:
                    best_len = len(path1) + len(path2)
                    best_pair = e1, e2
            u1()

        e1, e2 = best_pair

        yield e1, board
        path1 = skip_cache.trace_path(n, board, e1)
        u1 = break_mirrors(board, path1)
        skip_cache.break_mirrors(path1)

        if e2 is not None:
            yield e2, board
            path2 = skip_cache.trace_path(n, board, e2)
            u2 = break_mirrors(board, path2)
            skip_cache.break_mirrors(path2)


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
