import os
import sys
import time
import json
import socket
import random
import subprocess

class Gimli():

    def __init__(self):
        pass

    def help(self):
        """
        @return help text string
        """
        return "usage: gimli [-h|cpu|mem|stat|meminfo|serve [n]]"

    def log(self, s):
        """
        Print timestamped string to stdout.
        @param s the string to print
        """
        date = time.strftime("%Y-%m-%d %H:%M:%S")
        print("[{}] [{}] {}".format(date, os.getpid(), s))

    def meminfo(self):
        """
        @return dict() containing /proc/meminfo keys and values
        """
        m = {}
        with open('/proc/meminfo', 'r') as f:
            for line in f:
                key = line.split()[0].rstrip(':')
                val = int(line.split()[1])
                m[key] = val
        return m

    def mem(self):
        """
        @return current total memory usage as a percentage
        """
        m = self.meminfo()
        # return round((100.0 - (m['MemAvailable'] / m['MemTotal']) * 100.0), 1)
        return round((1.0 - (m['MemFree'] / m['MemTotal'])) * 100.0, 1)

    def stat(self):
        """
        @return dict() containing cpu utilization data from /proc/stat
        """
        c1, c2, diff, stat = [], [], [], {}
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
        stat['user'] = round((diff[0] / cpu_total) * 100.0, 1)
        stat['nice'] = round((diff[1] / cpu_total) * 100.0, 1)
        stat['system'] = round((diff[2] / cpu_total) * 100.0, 1)
        stat['idle'] = round((diff[3] / cpu_total) * 100.0, 1)
        stat['iowait'] = round((diff[4] / cpu_total) * 100.0, 1)
        stat['irq'] = round((diff[5] / cpu_total) * 100.0, 1)
        stat['softirq'] = round((diff[6] / cpu_total) * 100.0, 1)
        stat['steal'] = round((diff[7] / cpu_total) * 100.0, 1)
        stat['guest'] = round((diff[8] / cpu_total) * 100.0, 1)
        stat['guest_nice'] = round((diff[9] / cpu_total) * 100.0, 1)
        return stat

    def cpu(self):
        """
        @return current total cpu utilisation as a percentage
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
            if i == 0: time.sleep(.05)
        return utilisation

    def gimli(self):
        """
        @return dict() containing data from all gimli comands
        """
        return {
            'cpu': self.cpu(),
            'mem': self.mem(),
            'stat': self.stat(),
            'meminfo': self.meminfo()
        }

    def watch(self):
        """
        Runs gimli commands in a loop and prints the output.
        """
        with subprocess.Popen(['watch', 'gimli', 'stat']) as p:
            pass

    def name(self):
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

    def html_response(self, body):
        """
        Simple response generator for an HTML page.
        @param body the content to place into <body></body>
        @return string containing a minimal HTTP response and HTML page
        """
        response = (
                'HTTP/1.1 200 OK\r\n',
                'Content-Type: text/html\r\n',
                '\r\n',
                '<!doctype html>\r\n',
                '<html lang="en">',
                '<head>\r\n',
                '<meta charset="utf-8">\r\n',
                '<meta name="viewport">\r\n',
                '<meta content="width=device-width, initial-scale=1, shrink-to-fit=no">\r\n'
                '<title>gimli</title></head>\r\n',
                '<body>\r\n',
                '{}\r\n',
                '</body></html>\r\n'
                )
        return "".join(response).format(body)

    def json_response(self, data):
        """
        Simple HTTP JSON response.
        @param data the data to place into the JSON response
        @return string containing a minimal HTTP response and json.dumps(data)
        """
        response = (
                'HTTP/1.1 200 OK\r\n',
                'Content-Type: application/json; charset=utf-8\r\n',
                '\r\n',
                '{}',
                '\r\n'
                )
        return "".join(response).format(json.dumps(data, indent=2))

    def router(self, request):
        """
        Simple HTTP routing system.
        @param request the request to route
        @return the response to return
        """
        get = str(request).split(' ')[1]
        if get == "/" or get == "/gimli":
            response = self.json_response(self.gimli())
        elif get == "/cpu":
            response = self.json_response(self.cpu())
        elif get == "/mem":
            response = self.json_response(self.mem())
        elif get == "/stat":
            response = self.json_response(self.stat())
        elif get == "/meminfo":
            response = self.json_response(self.meminfo())
        elif get == "/name":
            response = self.json_response(self.name())
        else:
            d = {'err': 1}
            response = self.json_response(d)
        return response.encode('utf-8')

    def serve(self, host, port, workers):
        """
        Open a TCP socket and respond to requests.
        @param host the host to run on, typically localhost
        @param port the port to listen on, 8043 by default
        @param workers the number of gimli workers to spawn
        """
        fd = socket.socket()
        fd.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        fd.bind((host, port))
        fd.listen()
        self.log('Listening at: http://{}:{}'.format(host, port))

        # Create pre-fork workers to handle requests.
        for i in range(workers):
            pid = os.fork()

            # If pid == 0 then we're in the child process.
            if pid == 0:
                # Get the pid of this child process.
                t = os.getpid()
                self.log('Booting worker gimli-{}'.format(t))
                # TODO: assign worker to specific cpu
                # os.sched_setaffinity(0, {0}) # current process on 0-th core
                try:
                    while 1:
                        try:
                            conn, addr = fd.accept()
                            self.log('Connection from {}'.format(addr))
                        except:
                            self.log('gimli-{} exiting...'.format(t))
                            sys.exit(0)
                        data = conn.recv(1024)
                        if not data:
                            break
                        conn.sendall(self.router(data))
                        conn.close()
                except Exception as e:
                    self.log('gimli-{} exiting: {}'.format(t, e))
                    fd.close()
                    sys.exit(0)

        # Wait for child workers to exit.
        try:
            os.waitpid(-1, 0)
            fd.close()
        except:
            fd.close()
