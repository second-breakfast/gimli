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
        return round((100.0 - (m['MemAvailable'] / m['MemTotal']) * 100.0), 1)

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
        time.sleep(.05) # sleep 50ms
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
        cpu_util['user'] = round((diff[0] / cpu_total) * 100.0, 1)
        cpu_util['nice'] = round((diff[1] / cpu_total) * 100.0, 1)
        cpu_util['system'] = round((diff[2] / cpu_total) * 100.0, 1)
        cpu_util['idle'] = round((diff[3] / cpu_total) * 100.0, 1)
        cpu_util['iowait'] = round((diff[4] / cpu_total) * 100.0, 1)
        return cpu_util

    def memusage_str(self):
        m = self.memusage()
        return "mem: {:5}%".format(m)

    def cpu_util_str(self):
        c = self.cpu_util()
        s = "cpu: {:5}% us, {:5}% ni, {:5}% sy, {:5}% id, {:5}% wa"
        return s.format(c['user'], c['nice'], c['system'], c['idle'],
              c['iowait'])
