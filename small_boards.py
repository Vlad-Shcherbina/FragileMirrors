import pprint
import time

from main import *


def enum_boards(size, callback):

    def rec(start):
        if len(board) == size:
            if start <= width * (height-1):
                return
            if len(connected_components(board)) == 1:
                callback(board)
            return
        for i in range(start, width*height):
            y, x = divmod(i, width)
            for mirror in [0, 1]:
                board[x, y] = mirror
                rec(i+1)
            del board[x, y]

    board = {}

    for width in range(size//2 + 1, size + 1):
        height = size + 1 - width
        print width, height
        rec(0)
        assert board == {}


def canonical_four(a, b, c, d):
    # It is guaranteed that
    # h(a, b, c, d) == h(b, a, d, c) == h(c, d, a, b)
    m = min(a, b, c, d)
    if a == m:
        pass
    elif b == m:
        a, b, c, d = b, a, d, c
    elif c == m:
        a, b, c, d = c, d, a, b
    elif d == m:
        a, b, c, d = d, c, b, a
    assert a == m
    if b == m and c > d:
        c, d = d, c
    if c == m and b > d:
        a, b, c, d = c, d, a, b
    if d == m and b > c:
        a, b, c, d = d, c, b, a

    return a, b, c, d

for (a, b, c, d) in [
    ((1, 2, 3, 4)),
    ((1, 1, 1, 2)),
    ((1, 1, 2, 3)),
    ((1, 2, 1, 3)),
    ((1, 2, 3, 1)),
    ]:
    assert canonical_four(a, b, c, d) == (a, b, c, d)
    assert canonical_four(b, a, d, c) == (a, b, c, d)
    assert canonical_four(c, d, a, b) == (a, b, c, d)
    assert canonical_four(d, c, b, a) == (a, b, c, d)
#assert canonical_four(1, 2, 3, 4) == (1, 2, 3, 4)
#exit()

def board_hash(board, depth):
    n = 1 + max(sum(board, (0,)))
    values = dict.fromkeys(board, 1)

    def trace(x, y, dx, dy):
        while True:
            x += dx
            y += dy
            if (x, y) in values:
                return values[x, y]
            if not 0 <= x < n or not 0 <= y < n:
                return 0

    for _ in range(depth):
        new_values = {}
        for (x, y), v in values.items():
            a = trace(x, y, 0, -1)
            b = trace(x, y, 1, 0)
            c = trace(x, y, -1, 0)
            d = trace(x, y, 0, 1)
            if board[x, y]:
                a, b, c, d = a, c, b, d
            new_values[x, y] = hash(canonical_four(a, b, c, d))
        values = new_values

    return tuple(sorted(values.values()))

#print board_hash({(1, 1): 0, (0, 1): 0, (0, 0): 1})
#exit()

if __name__ == '__main__':
    start = time.clock()
    size = 4

    boards_by_hash = {}

    cnt = [0]
    def callback(board):
        #print_board(size, board)
        h = board_hash(board, 4)
        if h not in boards_by_hash:
            boards_by_hash[h] = []
        #boards_by_hash[h].append(dict(board))
        #boards_by_hash[h].append(board)
        boards_by_hash[h] = 1
        cnt[0] += 1

    enum_boards(size, callback)
    print cnt[0], 'total boards'

    print len(boards_by_hash), 'hashes'
    #pprint.pprint(boards_by_hash)

    print 'it took', time.clock() - start