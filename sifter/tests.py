
import time
from collections import deque
from injector import InjectorResults


class Tests:
    r = InjectorResults() # current result
    IL=20 # instruction log len
    UL=10 # artifact log len
    il = deque(maxlen=IL) # instruction log
    al = deque(maxlen=UL) # artifact log
    ad = dict() # artifact dict
    ic = 0 # instruction count
    ac = 0 # artifact count
    start_time = time.time()

    def elapsed(self):
        m, s = divmod(time.time() - self.start_time, 60)
        h, m = divmod(m, 60)
        return "%02d:%02d:%02d.%02d" % (h, m, int(s), int(100*(s-int(s))) )

