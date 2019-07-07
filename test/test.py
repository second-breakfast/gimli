import time
from gimli import Gimli

g = Gimli()

start_time = time.time()
g.stat()
print("stat took {} seconds".format(time.time() - start_time))

start_time = time.time()
g.cpu()
print("cpu took {} seconds".format(time.time() - start_time))

start_time = time.time()
g.meminfo()
print("meminfo took {} seconds".format(time.time() - start_time))

start_time = time.time()
g.mem()
print("mem took {} seconds".format(time.time() - start_time))
