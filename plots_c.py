import random
import subprocess

from matplotlib import pylab


if __name__ == '__main__':
    random.seed(42)
    n = 50

    for _ in range(10):
        xs = []
        ys = []
        print _
        p = subprocess.Popen(
            ['FragileMirrors/Release/FragileMirrors.exe'],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        input = str(n) + '\n'
        for i in range(n):
            for j in range(n):
                input += random.choice('RL')
            input += '\n'

        out, err = p.communicate(input)
        ret_code = p.wait()
        assert ret_code == 0

        for line in err.split('\n'):
            line = line.strip()
            prefix = 'data point: '
            if line.startswith(prefix):
                y, x = eval(line[len(prefix):])
                x *= 1.0 / n**2
                x = x ** 0.5
                x = 1 - x
                y *= 1.0 / n

                xs.append(x)
                ys.append(y)
                #print x, y
    #print out
        pylab.plot(xs, ys, '.')
    pylab.plot([0, 1], [0, 1], '-')
    pylab.show()
