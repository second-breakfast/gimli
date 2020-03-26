gimli:
	gcc	gimli.c -lpthread -O3 -o gimli

gimli-cli:
	gcc	gimli-cli.c -o gimli-cli

debug:
	gcc	gimli.c -lpthread -g -o gimli; gcc gimli-cli.c -g -o gimli-cli

clean:
	rm gimli gimli-cli
