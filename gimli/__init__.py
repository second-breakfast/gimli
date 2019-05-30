
from .gimli import __version__
from .gimli import SSHClient

RED = '\x1b[31m'
GREEN = '\x1b[32m'
YELLOW = '\x1b[33m'
BLUE = '\x1b[34m'
MAGENTA = '\x1b[35m'
CYAN = '\x1b[36m'

def printc(text, color=None, **kwargs):
    try:
        if color is None:
            color = ''
        print(color + text + '\x1b[0m', **kwargs)
    except NameError:
        print(text, **kwargs) 
