#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>

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

    if (argc != 3) {
        printf("gimli-cli <host> <port>\n");
        return (1);
    }

    fd = gimli_connect(argv[1], strtol(argv[2], &(char *){ "gimli" }, 10));

    // while (1) {
        if (send(fd, "gimli", sizeof ("gimli"), 0) < 0) {
            return (2);
        }
        if ((len = recv(fd, buf, 256, 0)) < 1) {
            return (3);
        }
        buf[len - 1] = '\0';
        printf("%s\n", buf);
        // printf("%s\r", buf);
        // fflush(stdout);
    // }

    return (0);
}
