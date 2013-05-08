import time
import random

from main import *
from plots import random_board


if __name__ == '__main__':
    random.seed(42)

    cnt = 0
    total_score = 0
    total_time = 0
    for n in range(50, 101, 5):
        board = random_board(n)
        start = time.clock()
        score = 1.0 * n / len(list(greedy(n, board)))
        t = time.clock() - start
        total_score += score
        total_time += t
        cnt += 1

        print '{:3} {:5.3} {:.3}s'.format(n, score, t)

    print 'average score: {:.3}'.format(total_score/cnt)
    print 'average time: {:.3}s'.format(total_time/cnt)