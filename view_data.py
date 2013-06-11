import collections
import pprint

import numpy
import numpy.linalg
from numpy import dot


if __name__ == '__main__':

    qq = collections.defaultdict(collections.Counter)

    AB = None

    class C(object): pass
    with open('training_data.txt') as fin:
        for line in fin:
            features = C()
            steps, features.__dict__ = eval(line)
            if features.n**2/10 <= features.mc <= 200000:
                s, x, y = features.mc, features.rows, features.cols
                x, y = min(x, y), max(x, y)
                #if not x == y == features.n:
                #    continue
                num_ones = (features.col_pops + features.row_pops).count(1)
                qq[s].update([steps])
                n = 1.0*features.n
                s /= n**2
                moment = sum((k/n)**2 for k in features.col_pops + features.row_pops)/n
                ab = numpy.array([1/n, s, x/n+y/n, moment, num_ones/n, steps/n])
                if AB is None:
                    AB = numpy.array([ab])
                else:
                    AB = numpy.vstack([AB, ab])

    #print AB
    #A = numpy.arra
    A = AB[:,:-1]
    B = AB[:,-1]

    #print .shape
    A = numpy.hstack([numpy.ones((A.shape[0], 1)), A])
    print A
    #print B
    theta = dot(numpy.linalg.pinv(dot(A.T, A)), dot(A.T, B))
    print theta
    error = dot(A, theta) - B
    #print error**2
    print 'average error', numpy.average(error**2)**0.5

    for q in range(3, 10) + range(10, 201, 5):
        xs = numpy.argwhere(A[:,1] == q)[:,0]
        if len(xs) == 0:
            continue
        e = error[xs]
        print q, numpy.average(e**2)**0.5, len(xs)

    #pprint.pprint(dict(qq))