gimli: CFLAGS = -Wall -Werror -lpthread
gimli-cli: CFLAGS = -Wall -Werror

all: gimli gimli-cli

gimli: gimli.c

gimli-cli: gimli-cli.c

clean:
	rm -f gimli gimli-cli

install:
	mkdir -p $(HOME)/bin && cp gimli $(HOME)/bin
