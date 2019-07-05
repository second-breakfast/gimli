import os
import sys
import time
import socket
import random

class Gimli():

    def __init__(self):
        pass

    def log(self, s):
        print("[{}] [{}] {}".format(time.asctime(), os.getpid(), s))

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

    def cpu_tot(self):
        """
        @return the current cpu usage as a percentage
        """
        last_idle = last_total = 0
        for i in range(2):
            f = open('/proc/stat')
            fields = [float(column) for column in f.readline().strip().split()[1:]]
            idle, total = fields[3], sum(fields)
            idle_delta, total_delta = idle - last_idle, total - last_total
            last_idle, last_total = idle, total
            utilisation = round(100.0 * (1.0 - idle_delta / total_delta), 1)
            f.close()
            if i == 0: time.sleep(.2)
        return utilisation

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

    def name_generator(self):
        adjectives = ["autumn", "hidden", "bitter", "misty", "silent",
                      "empty", "dry", "dark", "summer", "icy", "delicate",
                      "quiet", "white", "cool", "spring", "winter",
                      "patient", "crimson", "wispy", "weathered", "blue",
                      "billowing", "broken", "cold", "damp", "falling",
                      "frosty", "green", "long", "late", "bold", "little",
                      "morning", "muddy", "red", "rough", "still",
                      "small", "sparkling", "shy", "wandering",
                      "withered", "wild", "black", "young", "holy",
                      "solitary", "fragrant", "aged", "snowy", "proud",
                      "floral", "restless", "polished", "purple",
                      "lively", "nameless", "scarlet", "gloomy", "lucid",
                      "snarling", "lurking", "fierce", "furious",
                      "lonely", "gnawing", "burning", "keen", "boggy",
                      "swampy", "torrid", "glowing", "arid", "droughty",
                      "skinny", "meager", "stout", "sturdy", "crispy",
                      "blooming", "stormy", "rousing", "flowing", "old",
                      "glistening", "clear", "winding", "meandering",
                      "mild", "hot", "frozen", "frightening", "lucky",
                      "profound", "aqueous", "arcane", "cryptic", "fast",
                      "gentle", "immense", "limitless", "lit", "murmuring",
                      "protected", "pure", "rocky", "polite", "cautious",
                      "perky", "naughty", "upright", "straight"]
        nouns = ["waterfall", "river", "breeze", "moon", "rain", "wind",
                 "sea", "morning", "snow", "lake", "sunset", "pine",
                 "shadow", "leaf", "dawn", "glitter", "forest", "hill",
                 "cloud", "meadow", "sun", "glade", "bird", "brook",
                 "butterfly", "bush", "dew", "dust", "field", "fire",
                 "flower", "firefly", "feather", "grass", "haze",
                 "mountain", "night", "pond", "darkness", "snowflake",
                 "silence", "sound", "sky", "shape", "surf", "thunder",
                 "violet", "water", "wildflower", "wave", "water",
                 "resonance", "sun", "wood", "dream", "cherry", "tree",
                 "fog", "frost", "voice", "frog", "smoke", "star", "ibex",
                 "roe", "deer", "cave", "stream", "creek", "ditch",
                 "puddle", "oak", "fox", "wolf", "owl", "eagle", "hawk",
                 "badger", "nightingale", "ocean", "island", "marsh",
                 "swamp", "blaze", "glow", "hail", "echo", "flame",
                 "twilight", "whale", "raven", "blossom", "mist", "ray",
                 "beam", "stone", "rock", "cliff", "reef", "crag", "peak",
                 "summit", "wetland", "glacier", "thunderstorm", "ice",
                 "firn", "spark", "boulder", "rabbit", "abyss",
                 "avalanche", "moor", "reed", "harbor", "chamber",
                 "savannah", "garden", "brook", "earth", "oasis",
                 "bastion", "ridge", "bayou", "citadel", "shore",
                 "cavern", "gorge", "spring", "arrow", "heap"]
        random.seed()
        return random.choice(adjectives)+'-'+random.choice(nouns)

    def http_response(self, pid):
        res = 'HTTP/1.1 200 OK\n'
        res = res + 'Content-Type: text/html\n\n'
        res = res + '<!doctype html>\n'
        res = res + '<html lang="en">\n'
        res = res + '<head>\n'
        res = res + '<meta charset="utf-8">\n'
        res = res + '<meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">\n'
        res = res + '<title>gimli</title></head>'
        res = res + '<body>\n'
        res = res + 'gimli-{}\n\n'
        res = res + self.name_generator() + '\n'
        res = res + '</body></html>\n'
        return res.format(pid)

    def gimli_server(self, host, port, workers):
        fd = socket.socket()
        fd.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        fd.bind((host, port))
        fd.listen()
        self.log('gimli server listening on port {}...'.format(port))

        # Create pre-fork workers to handle requests.
        for i in range(workers):
            pid = os.fork()

            # If pid == 0 then we're in the child process.
            if pid == 0:
                # Get the pid of this child process.
                t = os.getpid()
                self.log('Starting worker gimli-{}'.format(t))
                try:
                    while 1:
                        try:
                            conn, addr = fd.accept()
                            self.log('gimli-{} connection from {}'.format(t, addr))
                        except:
                            self.log('gimli-{} exiting...'.format(t))
                            sys.exit(0)
                        if not conn.recv(1024):
                            break
                        conn.sendall(self.http_response(t).encode('utf-8'))
                        conn.close()
                except:
                    fd.close()
                    sys.exit(0)

        # Wait for child workers to exit.
        try:
            os.waitpid(-1, 0)
            fd.close()
        except:
            fd.close()
