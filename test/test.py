import time
from gimli import Gimli

g = Gimli()

start_time = time.time()
g.cpu_util()
print("cpu_util took {} seconds".format(time.time() - start_time))

start_time = time.time()
g.meminfo()
print("meminfo took {} seconds".format(time.time() - start_time))

start_time = time.time()
g.memusage()
print("memusage took {} seconds".format(time.time() - start_time))

start_time = time.time()
g.cpu_util_str()
print("cpu_util_str took {} seconds".format(time.time() - start_time))

start_time = time.time()
g.memusage_str()
print("memusage_str took {} seconds".format(time.time() - start_time))
