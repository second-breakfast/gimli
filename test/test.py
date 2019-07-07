import time
from gimli import Gimli

g = Gimli()

start_time = time.time()
g.cpustat()
print("cpustat took {} seconds".format(time.time() - start_time))

start_time = time.time()
g.cputot()
print("cputot took {} seconds".format(time.time() - start_time))

start_time = time.time()
g.meminfo()
print("meminfo took {} seconds".format(time.time() - start_time))

start_time = time.time()
g.memusage()
print("memusage took {} seconds".format(time.time() - start_time))
