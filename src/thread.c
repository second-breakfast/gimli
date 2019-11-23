#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "gimli.h"

#define SERVER_PORT 8001

// Global stats data, updated by mine() thread.
gimli_t           gimli;

void *
thread_create_detached(void *(*func) (void *),
        void *arg)
{
    pthread_t tid;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,
            PTHREAD_CREATE_DETACHED);
    pthread_create(&tid, &attr, func, arg);
}

char *
read_file(const char *path, int *len)
{
    FILE *f = NULL;
    int fsize;
    char *buf = NULL;

    if ((f = fopen(path, "r")) != NULL) {

        /* Go to the end of the file. */
        if (fseek(f, 0L, SEEK_END) == 0) {

            /* Get the size of the file. */
            fsize = ftell(f);
            if (fsize == -1) {
                goto exit;
            }

            /* Allocate our buffer to that size. */
            buf = malloc(sizeof (char) * (fsize + 1));

            /* Go back to the start of the file. */
            if (fseek(f, 0L, SEEK_SET) != 0) {
                goto exit;
            }

            /* Read the entire file into memory. */
            *len = fread(buf, sizeof(char), fsize, f);
            if (ferror(f) != 0) {
                goto exit;
            } else {
                buf[(*len) + 1] = '\0';
            }
        }
    }

exit:
    if (f != NULL) {
        fclose(f);
    }

    return (buf);
}

void *
handle_connection(void *arg)
{
    int fd, len;
    char buf[1024];
    char *index = NULL;
    char output[2046];
    FILE *p = NULL;
    size_t size = sizeof (output);

    fd = *((int *) arg);

    // printf("Handling connection...\n");
    while (1) {
        // len = recv(fd, buf, sizeof (buf), 0);
        len = recv(fd, buf, sizeof (buf), MSG_DONTWAIT);
        if (len <= 0) {
            // printf("Error reading from fd=%d, %m\n", fd);
            // printf("Closing connection to fd %d\n", fd);
            break;
        }
        buf[len - 1] = '\0';
        printf("  <- %s\n", buf);

        snprintf(output, size, "cpu %.2Lf%% us, %.2Lf%% ni, %.2Lf%% sy, %.2Lf%% id, "
                "%.2Lf%% wa\n",
                gimli.cpu_util[CPU_USER], gimli.cpu_util[CPU_NICE],
                gimli.cpu_util[CPU_SYSTEM], gimli.cpu_util[CPU_IDLE],
                gimli.cpu_util[CPU_IOWAIT]);
        if (send(fd, output, strlen(output), MSG_NOSIGNAL) <= 0) break;

        snprintf(output, size, "loadavg %.2Lf, %.2Lf, %.2Lf\n",
                gimli.cpu_loadavg[LOAD_ONE],
                gimli.cpu_loadavg[LOAD_FIVE],
                gimli.cpu_loadavg[LOAD_TEN]);
        if (send(fd, output, strlen(output), MSG_NOSIGNAL) <= 0) break;

        snprintf(output, size, "uptime %lu days, %02lu:%02lu\n", gimli.uptime/86400,
                gimli.uptime/3600%24, gimli.uptime/60%60);
        if (send(fd, output, strlen(output), MSG_NOSIGNAL) <= 0) break;

        snprintf(output, size, "totalram:  %lu kB\n", gimli.meminfo[TOTAL_RAM]);
        if (send(fd, output, strlen(output), MSG_NOSIGNAL) <= 0) break;

        snprintf(output, size, "freeram:   %lu kB\n", gimli.meminfo[FREE_RAM]);
        if (send(fd, output, strlen(output), MSG_NOSIGNAL) <= 0) break;

        snprintf(output, size, "sharedram: %lu kB\n", gimli.meminfo[SHARED_RAM]);
        if (send(fd, output, strlen(output), MSG_NOSIGNAL) <= 0) break;

        snprintf(output, size, "bufferram: %lu kB\n", gimli.meminfo[BUFFER_RAM]);
        if (send(fd, output, strlen(output), MSG_NOSIGNAL) <= 0) break;

        snprintf(output, size, "totalswap: %lu kB\n", gimli.meminfo[TOTAL_SWAP]);
        if (send(fd, output, strlen(output), MSG_NOSIGNAL) <= 0) break;

        snprintf(output, size, "freeswap:  %lu kB\n", gimli.meminfo[FREE_SWAP]);
        if (send(fd, output, strlen(output), MSG_NOSIGNAL) <= 0) break;

        snprintf(output, size, "totalhigh: %lu kB\n", gimli.meminfo[TOTAL_HIGH]);
        if (send(fd, output, strlen(output), MSG_NOSIGNAL) <= 0) break;

        snprintf(output, size, "freehigh:  %lu kB\n", gimli.meminfo[FREE_HIGH]);
        if (send(fd, output, strlen(output), MSG_NOSIGNAL) <= 0) break;

        snprintf(output, size, "number of current processes: %hu\n", gimli.procs);
        if (send(fd, output, strlen(output), MSG_NOSIGNAL) <= 0) break;
        printf("  -> replied\n");
    }

exit:
    if (fd > 0) {
        close(fd);
    }
    if (index != NULL) {
        free(index);
    }
}

void *
handle_connections()
{
    int fd, newfd;
    struct sockaddr_in svr_addr, peer_addr;
    socklen_t peer_addr_size;
    char ip[INET_ADDRSTRLEN];

    fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (fd == -1) {
        printf("Couldn't create socket: %m\n");
        exit(1);
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof (int)) < 0) {
        printf("setsockopt(SO_REUSEADDR) failed\n");
        exit(1);
    }

    memset(&svr_addr, 0, sizeof(struct sockaddr_in));
    svr_addr.sin_addr.s_addr = INADDR_ANY;
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons(SERVER_PORT);

    if (bind(fd, (struct sockaddr *) &svr_addr,
                sizeof(struct sockaddr_in)) == -1) {
        printf("Couldn't bind to socket: %m\n");
        close(fd);
        exit(1);
    }

    /* Listen on specified port. */
    if (listen(fd, 5) == -1) {
        printf("Couldn't listen to port: %m\n");
        close(fd);
        exit(1);
    }
    printf("Listening at 127.0.0.1:%d...\n", SERVER_PORT);

    /* Now we can accept incoming connections one
       at a time using accept(2) */
    peer_addr_size = sizeof(struct sockaddr_in);
    while ((newfd = accept(fd, (struct sockaddr *) &peer_addr,
                    &peer_addr_size))) {
        if (newfd != -1) {
            /* Deal with incoming connection... */
            printf("Incoming connection from %s:%d, fd=%d\n", inet_ntoa(peer_addr.sin_addr),
                    ntohs(peer_addr.sin_port), newfd);
            int localfd = newfd;
            thread_create_detached(&handle_connection,
                    (void *) &localfd);
        }
    }

    /* Never reached. */
    close(fd);
}

void *
mine()
{
    while (1) {
        get_cpu_util(&gimli);
        if (gimli.errflag) {
            printf("get_cpu_util failed: %s\n", gimli.errtxt);
        }
        get_loadavg(&gimli);
        if (gimli.errflag) {
            printf("get_loadavg failed: %s\n", gimli.errtxt);
        }
        get_uptime(&gimli);
        if (gimli.errflag) {
            printf("get_uptime failed: %s\n", gimli.errtxt);
        }
        get_meminfo(&gimli);
        if (gimli.errflag) {
            printf("get_meminfo failed: %s\n", gimli.errtxt);
        }
        sleep(1);
    }
}

int
main()
{

    // Start the mine thread to gather system information.
    thread_create_detached(&mine, NULL);

    // Start main program loop.
    handle_connections();

    /* Never reached. */
    return (0);
}
