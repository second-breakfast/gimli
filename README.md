# gimli
Mines for system information.

## Install the gimli CLI
```bash
# This will install the gimli binary to /usr/local/bin/gimli.
git clone https://github.com/second-breakfast/gimli && cd gimli
sudo sh install.sh
# (Don't worry, the script will ask to confirm before doing anything.)
```

## Run it like so
```bash
usage: gimli [-h | cpu_util | cpu_tot | meminfo | memusage | serve [N] | watch]
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
