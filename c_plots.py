import random

from matplotlib import pylab

import c_runner


if __name__ == '__main__':
    random.seed(42)

    for _ in range(5):
        n = random.randrange(50, 101)
        print _, n,

        r = c_runner.RunResults((n, _))
        print r.time

        xs = []
        ys = []
        for y, x in r.data_points:
            x *= 1.0 / n**2
            x = x ** 0.5
            x = 1 - x
            y *= 1.0 / n
            xs.append(x)
            ys.append(y)

        style = {5: '.r', 6: '.y', 7: '.g', 8: '.b', 9: '.k', 10: '.k'}[n//10]
        pylab.plot(xs, ys, style)

    pylab.plot([0, 1], [0, 1], '-')
    pylab.show()
