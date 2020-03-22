/*
 * gimli.c
 *    Mines for system information.
 */

#include "gimli.h"


/* Global stats data, updated by the mine() threads. */
gimli_t           gimli;


/**
 * get_cpu_util - get total CPU util from kernel
 *
 * Samples the first line of /proc/stat twice and saves
 * columns 2-5 (skipping the first column 'cpu') two times
 * and saves calculated percentage results in gimli.cpu_util.
 *
 * The values for columns 2-5 in /proc/stat are as follows:
 *
 *     user, nice, system, idle, iowait
 *
 * More info about these values can be found in proc(5).
 *
 */
status_t
get_cpu_util(gimli_t *gimli)
{
    FILE          *f;
    char           buf[256];
    long double    tot = 0;
    gimli_cpu_t    old = {0}, new = {0}, diff = {0};

    // First poll.
    if ((f = fopen(PROC_STAT, "r")) == NULL) return (G_FAIL);
    if (fgets(buf, sizeof (buf), f) == NULL) return (G_FAIL);
    if (sscanf(buf, CPU_FMT, &old.u, &old.n, &old.s, &old.i, &old.w) < 4)
        return (G_FAIL);
    if (fclose(f) != 0) return (G_FAIL);

    usleep(3000000); // wait 3000ms == 3s

    // Second poll.
    if ((f = fopen(PROC_STAT, "r")) == NULL) return (G_FAIL);
    if (fgets(buf, sizeof (buf), f) == NULL) return (G_FAIL);
    if (sscanf(buf, CPU_FMT, &new.u, &new.n, &new.s, &new.i, &new.w) < 4)
        return (G_FAIL);
    if (fclose(f) != 0) return (G_FAIL);

    // Calculate diffs.
    diff.u = new.u > old.u ? new.u - old.u : old.u - new.u;
    diff.n = new.n > old.n ? new.n - old.n : old.n - new.n;
    diff.s = new.s > old.s ? new.s - old.s : old.s - new.s;
    diff.i = new.i > old.i ? new.i - old.i : old.i - new.i;
    diff.w = new.w > old.w ? new.w - old.w : old.w - new.w;
    tot = diff.u + diff.n + diff.s + diff.i + diff.w;

    // Calculate final percentages
    gimli->cpu[CPU_USER] = (diff.u / tot) * 100;
    gimli->cpu[CPU_NICE] = (diff.n / tot) * 100;
    gimli->cpu[CPU_SYSTEM] = (diff.s / tot) * 100;
    gimli->cpu[CPU_IDLE] = (diff.i / tot) * 100;
    gimli->cpu[CPU_IOWAIT] = (diff.w / tot) * 100;

    return (G_OK);
}

/**
 * get_loadavg - sample /proc/loadavg for loadavg
 *
 * Save the first three values from /proc/loadavg:
 *
 *    1) load avg of last 1 minute
 *    2) load avg of last 5 minutes
 *    3) load avg of last 15 minutes
 *
 * More info about these values can be found in proc(5).
 *
 */
status_t
get_loadavg(gimli_t *gimli)
{
    FILE          *f;
    char           buf[256];

    // Read first line of /proc/loadavg and get first 3 values.
    if ((f = fopen(PROC_LOADAVG, "r")) == NULL) return (G_FAIL);
    if (fgets(buf, sizeof (buf), f) == NULL) return (G_FAIL);
    if (sscanf(buf, LOAD_FMT, &gimli->load[0], &gimli->load[1], &gimli->load[2]) < 2)
        return (G_FAIL);
    if (fclose(f) != 0) return (G_FAIL);
    return (G_OK);
}

/**
 * get_meminfo - get system memory info
 *
 * Gathers various memory data from sysinfo() library
 * function and sets the gimli_t struct fields.
 *
 * For more info on memory fields, see sysinfo(2).
 */
status_t
get_meminfo(gimli_t *gimli)
{
   struct sysinfo meminfo;

   if (sysinfo(&meminfo) < 0) {
       return (G_FAIL);
   }

   gimli->meminfo[TOTAL_RAM]  = (meminfo.totalram * meminfo.mem_unit) / 1024;
   gimli->meminfo[FREE_RAM]   = (meminfo.freeram * meminfo.mem_unit) / 1024;
   gimli->meminfo[SHARED_RAM] = (meminfo.sharedram * meminfo.mem_unit) / 1024;
   gimli->meminfo[BUFFER_RAM] = (meminfo.bufferram * meminfo.mem_unit) / 1024;
   gimli->meminfo[TOTAL_SWAP] = (meminfo.totalswap * meminfo.mem_unit) / 1024;
   gimli->meminfo[FREE_SWAP]  = (meminfo.freeswap * meminfo.mem_unit) / 1024;
   gimli->meminfo[TOTAL_HIGH] = (meminfo.totalhigh * meminfo.mem_unit) / 1024;
   gimli->meminfo[FREE_HIGH]  = (meminfo.freehigh * meminfo.mem_unit) / 1024;
   gimli->meminfo[MEM_UNIT]   = meminfo.mem_unit;
   gimli->procs = meminfo.procs;
   gimli->uptime = meminfo.uptime;

   return (G_OK);
}

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

void *
handle_connection(void *arg)
{
    int fd, len;
    char buf[1024];
    char *index = NULL;
    char output[2046];
    FILE *p = NULL;
    size_t size = sizeof (output);
    struct timespec   start, stop;
    uint64_t          elapsed;

    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

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

        printf("\n  <- \n%s\n", buf);

        snprintf(output, size,
                "{\n" \
                "    \"cpu\": {\n" \
                "        \"us\": %.1Lf,\n" \
                "        \"sy\": %.1Lf,\n" \
                "        \"id\": %.1Lf,\n" \
                "        \"wa\": %.1Lf,\n" \
                "        \"ni\": %.1Lf,\n" \
                "    },\n" \
                "    \"load\": [%.2f, %.2f, %.2f],\n" \
                "    \"uptime\": [\"%lu\", \"%01lu:%02lu\"],\n" \
                "    \"procs\": %hu,\n" \
                "    \"cpus\": %d,\n" \
                "}\n",
                gimli.cpu[CPU_USER], gimli.cpu[CPU_SYSTEM], gimli.cpu[CPU_IDLE],
                gimli.cpu[CPU_IOWAIT], gimli.cpu[CPU_NICE], gimli.load[LOAD_ONE],
                gimli.load[LOAD_FIVE], gimli.load[LOAD_FIFTEEN], gimli.uptime/86400,
                gimli.uptime/3600%24, gimli.uptime/60%60, gimli.procs, gimli.cpus);
        if (send(fd, output, strlen(output), MSG_NOSIGNAL) <= 0) break;
#if 0
        snprintf(output, size, "cpu %.1Lf us, %.1Lf sy, %.1Lf id, %.1Lf wa, %.1Lf ni\n",
                gimli.cpu[CPU_USER], gimli.cpu[CPU_SYSTEM],
                gimli.cpu[CPU_IDLE], gimli.cpu[CPU_IOWAIT],
                gimli.cpu[CPU_NICE]);
        if (send(fd, output, strlen(output), MSG_NOSIGNAL) <= 0) break;

        snprintf(output, size, "load average %.2f, %.2f, %.2f\n",
                gimli.load[LOAD_ONE],
                gimli.load[LOAD_FIVE],
                gimli.load[LOAD_FIFTEEN]);
        if (send(fd, output, strlen(output), MSG_NOSIGNAL) <= 0) break;

        snprintf(output, size, "uptime %lu days, %01lu:%02lu\n", gimli.uptime/86400,
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

        snprintf(output, size, "number of cpu's: %d\n", gimli.cpus);
        if (send(fd, output, strlen(output), MSG_NOSIGNAL) <= 0) break;
#endif
        clock_gettime(CLOCK_MONOTONIC_RAW, &stop);
        elapsed = (BILLION * (stop.tv_sec - start.tv_sec) +
                stop.tv_nsec - start.tv_nsec) / MILLION;

        printf("  -> replied in %llums\n", (long long unsigned int) elapsed);
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
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 },
                sizeof (int)) < 0) {
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
            printf("Incoming connection from %s:%d, fd=%d\n",
                    inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port),
                    newfd);
            int localfd = newfd;
            thread_create_detached(&handle_connection,
                    (void *) &localfd);
        }
    }

    /* Never reached. */
    close(fd);
}

void *
gimli_mine_cpu()
{
    gimli.cpus = sysconf(_SC_NPROCESSORS_CONF);
    while (1) {
        if (get_cpu_util(&gimli) != G_OK) {
            printf("get_cpu_util failed\n");
        }
    }
}

void *
gimli_mine_load()
{
    while (1) {
        if (get_loadavg(&gimli) != G_OK) {
            printf("get_loadavg failed\n");
        }
        sleep(1);
    }
}

void *
gimli_mine_meminfo()
{
    while (1) {
        if (get_meminfo(&gimli) != G_OK) {
            printf("get_meminfo failed\n");
        }
        sleep(1);
    }
}

int
main()
{
    /* Start the mine threads to gather system information. */
    thread_create_detached(&gimli_mine_cpu, NULL);
    thread_create_detached(&gimli_mine_load, NULL);
    thread_create_detached(&gimli_mine_meminfo, NULL);

    /* Start main program loop. */
    handle_connections();

    /* Never reached. */
    return (0);
}
