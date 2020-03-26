#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

int
gimli_connect(char *host, int port)
{
    int fd;
    struct sockaddr_in server;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return (-1);
    }

    server.sin_addr.s_addr = inet_addr(host);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (connect(fd, (struct sockaddr *) &server, sizeof (server)) < 0) {
        return (-1);
    }
    return (fd);
}

int
main(int argc, char **argv)
{
    int fd;
    int len;
    char buf[1024];
    char *input = NULL;
    char *host, *port;

    if (!(argc == 3 || argc == 1)) {
        printf("gimli-cli <host> <port>\n");
        return (1);
    }

    if (argc == 3) {
        host = strdup(argv[1]);
        port = strdup(argv[2]);
        fd = gimli_connect(argv[1], strtol(argv[2], &(char *){ "" }, 10));
    } else {
        host = strdup("127.0.0.1");
        port = strdup("8001");
        fd = gimli_connect("127.0.0.1", 8001);
    }

    while (1) {
        printf("%s:%s> ", host, port);
        if (getline(&input, &(int) { 0 }, stdin) == -1) {
            return (2);
        }

        if (strcmp(input, "quit\n") == 0 ||
            strcmp(input, "exit\n") == 0) {
            free(input);
            return (0);
        }

        if (strcmp(input, "\n") == 0) {
            free(input);
            continue;
        }

        if (send(fd, input, strlen(input), 0) < 0) {
            free(input);
            return (3);
        }
        if ((len = recv(fd, buf, 256, 0)) < 1) {
            free(input);
            return (4);
        }
        buf[len - 1] = '\0';
        printf("%s\n", buf);
        free(input);
        // printf("%s\r", buf);
        // fflush(stdout);
    }

    free(host);
    free(port);
    return (0);
}
