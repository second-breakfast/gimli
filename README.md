# gimli
Mines for system information.

## Install gimli

Option 1, install in a virtualenv.
```bash
$ python3 -m venv env
$ . env/bin/activate
$ python3 -m pip install --upgrade gimli
```

Option 2, install to the user folder.
```bash
$ python3 -m pip install --user --upgrade gimli
```

Option 3, if you want a system-wide install...
```bash
$ sudo python3 -m pip install --upgrade gimli
```

## Run it like so
```bash
$ gimli
usage: gimli [-h|cpu|mem|stat|meminfo|serve [n]]
```

## Or... use the gimli API
```bash
Python 3.7.1 (default, Nov  2 2018, 20:33:06) 
[GCC 7.3.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> from gimli import Gimli
>>> g = Gimli()
>>> g.meminfo()
```
