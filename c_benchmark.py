from math import *
import itertools
import multiprocessing

from matplotlib import pylab

import c_runner


if __name__ == '__main__':
    seed = 2

    N = 48

    tasks = [(50 + i*7%51, i+seed) for i in range(N)]

    pool = multiprocessing.Pool()
    results = pool.imap(c_runner.RunResults, tasks)
    #results = itertools.imap(c_runner.RunResults, tasks)

    sum_scores = 0
    sum_scores2 = 0

    xs = []
    ys = []
    for i, result in enumerate(results):
        print '{:3}% {}'.format(100*(i+1)/N, result)
        sum_scores += result.score
        sum_scores2 += result.score**2
        xs.append(result.n)
        ys.append(result.time)

    mean_score = sum_scores/N
    sigma_score = sqrt(1.0 / (N-1) / N * (sum_scores2 - N*mean_score**2))
    print
    print 'Average score: {:.6f} +- {:.6f}'.format(mean_score, sigma_score)

    pylab.plot(xs, ys, '.')
    pylab.show()
