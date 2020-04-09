/*
 * gimli.h
 *   Headers and definitions for gimli.c
 */

#ifndef GIMLI_H
#define GIMLI_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define PROC_STAT    "/proc/stat"
#define PROC_LOADAVG "/proc/loadavg"
#define PROC_UPTIME  "/proc/uptime"

#define MILLION      1000000L
#define BILLION      1000000000L

#define SERVER_PORT  8001

#define CPU_FMT      "cpu %Lu %Lu %Lu %Lu %Lu"
#define LOAD_FMT     "%f %f %f"

enum cpu_util {
    CPU_USER       = 0,
    CPU_NICE       = 1,
    CPU_SYSTEM     = 2,
    CPU_IDLE       = 3,
    CPU_IOWAIT     = 4,
    CPU_NRSTATS    = 5,
};

enum cpu_loadavg {
    LOAD_ONE       = 0,
    LOAD_FIVE      = 1,
    LOAD_FIFTEEN   = 2,
    LOAD_NRSTATS   = 3
};

enum meminfo {
    TOTAL_RAM      = 0,
    FREE_RAM       = 1,
    SHARED_RAM     = 2,
    BUFFER_RAM     = 3,
    TOTAL_SWAP     = 4,
    FREE_SWAP      = 5,
    TOTAL_HIGH     = 6,
    FREE_HIGH      = 7,
    MEM_UNIT       = 8,
    MEM_NRSTATS    = 9
};

typedef enum {
    G_OK           = 0,
    G_FAIL         = 1
} status_t;

typedef struct {
    unsigned long long u, n, s, i, w;
} gimli_cpu_t;

typedef struct {
    int            cores;                     // number of cpu's
    long double    cpu[CPU_NRSTATS];          // in percentages
    float          load[LOAD_NRSTATS];        // straight from /proc/loadavg
    unsigned long  meminfo[MEM_NRSTATS];      // system memory info in bytes
    double         memuse;                    // system memory usage as percent
    unsigned long  uptime;                    // system uptime in seconds
    unsigned short procs;                     // number of current processes
} gimli_t;

status_t get_cpu_util(gimli_t *gimli);
status_t get_loadavg(gimli_t *gimli);
status_t get_meminfo(gimli_t *gimli);

#endif /* GIMLI_H */
