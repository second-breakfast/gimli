
import sys
import os

from gimli import SSHClient, printc, GREEN, __version__

def meminfo():
    with open('/proc/meminfo', 'r') as f:
        MemTotal = int(f.readline().split()[1])
        MemFree = int(f.readline().split()[1])
        MemAvailable = f.readline().split()[1]
        Buffers = f.readline().split()[1]
        Cached = f.readline().split()[1]
        SwapCached = f.readline().split()[1]
        Active = f.readline().split()[1]
        Inactive = f.readline().split()[1]
        Active_anon = f.readline().split()[1]
        Inactive_anon = f.readline().split()[1]
    print(MemTotal)
meminfo()

'''
MemTotal:         443132 kB
MemFree:           93396 kB
MemAvailable:     289724 kB
Buffers:           71284 kB
Cached:           167684 kB
SwapCached:          164 kB
Active:           195288 kB
Inactive:         121256 kB
Active(anon):      30308 kB
Inactive(anon):    50864 kB
Active(file):     164980 kB
Inactive(file):    70392 kB
Unevictable:           0 kB
Mlocked:               0 kB
SwapTotal:        102396 kB
SwapFree:         100604 kB
Dirty:                 4 kB
Writeback:             0 kB
AnonPages:         77424 kB
Mapped:            78188 kB
Shmem:              3600 kB
Slab:              19836 kB
SReclaimable:      11968 kB
SUnreclaim:         7868 kB
KernelStack:        1264 kB
PageTables:         2652 kB
NFS_Unstable:          0 kB
Bounce:                0 kB
WritebackTmp:          0 kB
CommitLimit:      323960 kB
Committed_AS:     709552 kB
VmallocTotal:     573440 kB
VmallocUsed:           0 kB
VmallocChunk:          0 kB
Percpu:               64 kB
CmaTotal:           8192 kB
CmaFree:            6164 kB
'''
