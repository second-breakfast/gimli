
import time

class Gimli():

    def __init__(self):
        pass

    def meminfo(self):
        """
        @return a dict() containing /proc/meminfo keys and values
        """
        m = {}
        with open('/proc/meminfo', 'r') as f:
            for line in f:
                key = line.split()[0].rstrip(':')
                val = int(line.split()[1])
                m[key] = val
        return m

    def memusage(self):
        """
        @return current memory usage as a percentage (float)
        """
        m = self.meminfo()
        return round((m['MemAvailable'] / m['MemTotal']) * 100.0, 2)

    def cpu_util(self):
        """
        @return a dict() containing cpu utilization data from /proc/stat
        """
        c1 = []
        c2 = []
        diff = []
        cpu_util = {}
        # First poll
        with open('/proc/stat', 'r') as f:
            for data in f.readline().split():
                try:
                    c1.append(int(data))
                except ValueError:
                    continue
        time.sleep(2) # sleep 200ms
        # Second poll
        with open('/proc/stat', 'r') as f:
            for data in f.readline().split():
                try:
                    c2.append(int(data))
                except ValueError:
                    continue
        # Calculate percentages
        cpu_total = 0
        for a, b in zip(c1, c2):
            d = abs(a - b)
            diff.append(d)
            cpu_total += d
        # Return final dict
        cpu_util['user'] = round((diff[0] / cpu_total) * 100.0, 2)
        cpu_util['nice'] = round((diff[1] / cpu_total) * 100.0, 2)
        cpu_util['system'] = round((diff[2] / cpu_total) * 100.0, 2)
        cpu_util['idle'] = round((diff[3] / cpu_total) * 100.0, 2)
        cpu_util['iowait'] = round((diff[4] / cpu_total) * 100.0, 2)
        return cpu_util

    def memusage_s(self):
        m = self.memusage()
        return "mem: {:6.2f}%".format(m)

    def cpu_util_s(self):
        c = self.cpu_util()
        s = "cpu: {:6.2f}% us, {:6.2f}% ni, {:6.2f}% sy, {:6.2f}% id, {:6.2f}% wa"
        return s.format(c['user'], c['nice'], c['system'], c['idle'],
              c['iowait'])
