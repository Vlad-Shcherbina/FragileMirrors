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


def break_mirrors(board, path):
    backup_data = {}
    for x, y in path:
        backup_data[x, y] = board[x, y]
        del board[x, y]

    def undo():
        board.update(backup_data)

    return undo


def greedy(n, board):
    enters = list(iter_enters(n))
    num_steps = 0
    while board:
        best_enter = None
        best_path = None
        best_len = -1
        for enter in enters:
            path = trace_path(n, board, enter)
            if len(path) > best_len:
                best_len = len(path)
                best_enter = enter
                best_path = path

        yield best_enter, board

        u = break_mirrors(board, best_path)
        num_steps += 1


class FragileMirrors(object):
    def destroy(self, rows):
        board = {}
        n = len(rows)
        for i, row in enumerate(rows):
            assert len(row) == n
            for j, c in enumerate(row):
                board[j, i] = {'L': 0, 'R': 1}[c]

        result = []
        for enter, _ in greedy(n, board):
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
