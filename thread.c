#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SERVER_PORT 80

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

    fd = *((int *) arg);

    len = read(fd, buf, sizeof (buf));
    if (len < 0) {
        printf("Error reading from fd=%d\n", fd);
        goto exit;
    }

    /* Write out system info from gimli. */
    if ((p = popen("./a.out", "r")) != NULL) {
        while (fgets(output, sizeof (output), p) != NULL) {
            write(fd, output, strlen(output));
        }
        pclose(p);
        goto exit;
    }

    /* Parse file and write it as response. */
    // index = read_file("/tmp/gimli", &len);
    // write(fd, index, len);

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

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        printf("Couldn't create socket: %m\n");
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
    printf("listening on port %d...\n", SERVER_PORT);

    /* Now we can accept incoming connections one
       at a time using accept(2) */
    peer_addr_size = sizeof(struct sockaddr_in);
    while ((newfd = accept(fd, (struct sockaddr *) &peer_addr,
                    &peer_addr_size))) {
        if (newfd != -1) {
            /* Deal with incoming connection... */
            thread_create_detached(&handle_connection,
                    (void *) &newfd);
        }
    }

    /* Never reached. */
    close(fd);
}

int
main()
{
    handle_connections();

    /* Never reached. */
    return (0);
}
