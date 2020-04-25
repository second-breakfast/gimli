#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#define GIMLI_HELP "gimli-cli (beta)\n" \
                   "    \"cpu\"      to get cpu utilization\n" \
                   "    \"load\"     to get load avg\n" \
                   "    \"uptime\"   to get system uptime\n" \
                   "    \"procs\"    to get number of current processes\n" \
                   "    \"cores\"    to get number of CPU cores\n" \
                   "    \"net\"      to get network interfaces\n" \
                   "    \"all\"      to see all data\n"

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
    int fd, len, noninteractive = 0;
    char buf[2048];
    char *input = NULL;
    char *host, *port;

    if (!(argc == 1 || argc == 2 || argc == 3 || argc == 4)) {
        printf("gimli-cli [ | <host> <port> [cmd] | <cmd>]\n");
        return (1);
    }

    if (argc == 3 || argc == 4) {
        host = strdup(argv[1]);
        port = strdup(argv[2]);
        fd = gimli_connect(argv[1], strtol(argv[2], &(char *){ "" }, 10));
    } else {
        host = strdup("127.0.0.1");
        port = strdup("8001");
        fd = gimli_connect("127.0.0.1", 8001);
    }
    if (argc == 2 || argc == 4) {
        noninteractive = 1;
    }

    if (fd < 0) {
        printf("Could not connect to Gimli at %s:%s\n", host, port);
        exit(1);
    }

    while (1) {
        if (noninteractive) {
            // Non-interfactive mode
            if (argc == 2) {
                input = strdup(argv[1]);
            } else {
                input = strdup(argv[3]);
            }
        } else {
            // Interactive mode
            printf("%s:%s> ", host, port);
            if (getline(&input, &(size_t) {0}, stdin) == -1) {
                return (2);
            }
            input[strlen(input) - 1] = '\0';

            if (strcmp(input, "quit") == 0 ||
                    strcmp(input, "exit") == 0) {
                free(input);
                return (0);
            }

            if (strcmp(input, "help") == 0) {
                printf(GIMLI_HELP);
                continue;
            }

            if (strcmp(input, "") == 0) {
                free(input);
                continue;
            }
        }

        if (send(fd, input, strlen(input), 0) < 0) {
            free(input);
            return (3);
        }
        if ((len = recv(fd, buf, sizeof (buf), 0)) < 1) {
            free(input);
            return (4);
        }

        buf[len - 1] = '\0';
        printf("%s\n", buf);
        free(input);

        if (noninteractive) {
            break;
        }
    }

    free(host);
    free(port);
    return (0);
}
