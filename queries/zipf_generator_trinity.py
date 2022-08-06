import bisect
import random
import math
from functools import reduce

import numpy
N = 30000000 # 30M

def gen_zipf(alpha, max_val, n):
    tmp = numpy.power(numpy.arange(1, max_val + 1), float(-alpha))
    zeta = numpy.r_[0.0, numpy.cumsum(tmp)]
    zeta_dist = [x / zeta[-1] for x in zeta]
    v = numpy.searchsorted(zeta_dist, numpy.random.random(n))
    samples = [t - 1 for t in v]
    return samples


picked_primary_keys = gen_zipf(1, int(N / 10), N)
picked_primary_keys

print(picked_primary_keys[:20], len(picked_primary_keys))
f = open("/proj/trinity-PG0/Trinity/queries/zipf_keys_30m", "w")
f.write("\n".join([str(x) for x in picked_primary_keys]))
f.close()

