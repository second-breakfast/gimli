
import time
import curses
from gimli import Gimli

def run_gimli(window):
    g = Gimli()
    while True:
        window.addstr(1, 0, "{}".format(time.strftime("%B %d, %Y %H:%M:%S")))
        window.addstr(3, 0, "Gimli is mining for system info...\n")
        window.addstr(4, 0, g.memusage_s()+"\n"+g.cpu_util_s()+"\n")
        window.refresh()

try:
    curses.wrapper(run_gimli)
except KeyboardInterrupt:
    exit(0)
