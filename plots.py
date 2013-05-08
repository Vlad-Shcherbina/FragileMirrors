import random
import collections
import itertools

import matplotlib
from matplotlib import pylab

from main import *


def random_board(n):
    result = {}
    for i in range(n):
        for j in range(n):
            result[i, j] = random.randrange(2)
    return result


def interacting_pairs(n, board):
    enters = list(iter_enters(n))

    passing = collections.defaultdict(list)
    for enter in enters:
        path = trace_path(n, board, enter)
        for pt in path:
            passing[pt].append(enter)

    pairs = set()
    for ps in passing.values():
        for p1 in ps:
            for p2 in ps:
                pairs.add((p1, p2))

    return pairs


def smooth(xs, ys, k):
    buckets = [[] for _ in range(k)]
    for x, y in zip(xs, ys):
        assert 0 <= x <= 1
        b = int(x*k)
        if b == k:
            b = k - 1
        buckets[b].append(y)
    axs = []
    ays = []
    for i, ys in enumerate(buckets):
        if ys:
            axs.append((i+0.5) / k)
            ays.append(1.0 * sum(ys) / len(ys))
    return axs, ays


if __name__ == '__main__':
    fig = pylab.figure()

    ax1 = fig.add_subplot(221)
    ax2 = fig.add_subplot(222)
    ax3 = fig.add_subplot(223)
    ax4 = fig.add_subplot(224)

    for algo in greedy, greedy_depth_two:
        random.seed(432)

        n = 80

        xs = []
        ys = []
        zs = []
        ts = []
        rs = []
        qs = []
        for i in range(15):
            board = random_board(n)
            for step, (_, board) in enumerate(algo(n, board)):
                paths = [trace_path(n, board, enter) for enter in iter_enters(n)]

                x = 1 - 1.0 * len(board) / n**2
                y = 1.0 * max(map(len, paths)) / n
                z = (1.0 + step) / n
                xs.append(x)
                ys.append(y)
                zs.append(z)

                rows = set()
                columns = set()
                for i, j in board:
                    rows.add(i)
                    columns.add(j)
                ts.append(len(interacting_pairs(n, board)))
                rs.append(len(rows) + len(columns))
            qs.append((step + 1.0) / n)


        SMOOTH = 20

        axs, ays = smooth(xs, ys, SMOOTH)
        ys = [1.0 / y for y in ys]
        ays = [1.0 / y for y in ays]

        ax1.set_ylim([0, 10])

        ax1.plot(xs, ys, '.')
        ax1.plot(axs, ays, '-')

        axs, azs = smooth(xs, zs, SMOOTH)

        ax2.plot(xs, zs, '.')
        ax2.plot(axs, azs, '-')

        ax2.plot([1.0] * len(qs), qs, 'rd')

        axs, ats = smooth(xs, ts, SMOOTH)
        ax3.plot(xs, ts, '.')
        ax3.plot(axs, ats, '-')

        ax4.plot(xs, rs, '.')

    pylab.show()
