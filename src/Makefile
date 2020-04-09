CFLAGS = -Wall -Werror

all: gimli gimli-cli

gimli: gimli.c -lpthread

gimli-cli: gimli-cli.c

debug:
	gcc gimli.c -lpthread -g -o gimli && gcc gimli-cli.c -g -o gimli-cli

clean:
	rm -f gimli gimli-cli
