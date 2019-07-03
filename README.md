# gimli
Mines for system information.

## Install the gimli CLI
```bash
git clone https://github.com/second-breakfast/gimli
cd gimli
sh install.sh
```

## Run like so
```bash
usage: gimli [-h | cpu_util | meminfo | memusage]
```

## Or... use the gimli API
```bash
pip3 install gimli

Python 3.7.1 (default, Nov  2 2018, 20:33:06) 
[GCC 7.3.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> from gimli import Gimli
>>> g = Gimli()
>>> g.cpu_util()
```
