import random
import subprocess
import time


class RunResults(object):
    def __init__(self, (n, seed)):
        self.n = n
        rnd = random.Random(seed)

        start = time.clock()
        p = subprocess.Popen(
            ['FragileMirrors/Release/FragileMirrors.exe'],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        input = str(n) + '\n'
        for i in range(n):
            for j in range(n):
                input += rnd.choice('RL')
            input += '\n'

        out, err = p.communicate(input)
        ret_code = p.wait()
        assert ret_code == 0

        self.time = time.clock() - start

        self.data_points = []
        self.subtasks = []
        for line in err.split('\n'):
            line = line.strip()
            prefix = 'data point: '
            if line.startswith(prefix):
                self.data_points.append(eval(line[len(prefix):]))
            prefix = 'subtask: '
            if line.startswith(prefix):
                self.subtasks.append(eval(line[len(prefix):]))

        self.num_steps = int(out.partition('\n')[0]) // 2
        self.score = 1.0 * n / self.num_steps

    def __str__(self):
        return 'Run(n={}, score={:.3f}, t={:.2f}s)'.format(self.n, self.score, self.time)
