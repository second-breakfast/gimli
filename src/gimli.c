/*
 * gimli.c
 *    Mines for system information.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/sysinfo.h>

#define PROC_STAT    "/proc/stat"
#define PROC_LOADAVG "/proc/loadavg"
#define PROC_UPTIME  "/proc/uptime"

#define ERRTXT_LEN   256
#define MILLION      1000000L
#define BILLION      1000000000L

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
    LOAD_TEN       = 2,
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
    S_PASS         = 0,
    S_FAIL         = 1
} status_t;

typedef struct {
    unsigned       errflag:1;                 // 0 - ok (default) , 1 - bad
    char           errtxt[ERRTXT_LEN];        // i.e. strerror(errno)
    long double    cpu_util[CPU_NRSTATS];     // in percentages
    long double    cpu_loadavg[LOAD_NRSTATS]; // straight from /proc/loadavg
    unsigned long  meminfo[MEM_NRSTATS];      // system memory info in bytes
    unsigned long  uptime;                    // system uptime in seconds
    unsigned short procs;                     // number of current processes
} gimli_t;

/**
 * safe_strncpy - nul terminates dest with '\0'
 */
void
safe_strncpy(char *dest, char *src, unsigned n)
{
    strncpy(dest, src, n);
    dest[n-1] = '\0';
}

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
    FILE          *file;
    char           buffer[1024];
    char          *cp;
    unsigned       i;
    long double    cpu_util_old[CPU_NRSTATS];
    long double    cpu_util_new[CPU_NRSTATS];
    long double    cpu_util_diff[CPU_NRSTATS];
    long double    cpu_total;

    // first poll
    if ((file = fopen(PROC_STAT, "r")) == NULL) {
        safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
        gimli->errflag = 1;
        return (S_FAIL);
    }

    // read first line of /proc/stat
    if (fgets(buffer, sizeof (buffer), file) == NULL) {
        safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
        gimli->errflag = 1;
        return (S_FAIL);
    }

    cp = strtok(buffer, " "); // skip first column 'cpu'
    for (i=0; i<CPU_NRSTATS; i++)
    {
        if ((cp = strtok(NULL, " ")) == NULL) {
            safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
            gimli->errflag = 1;
            return (S_FAIL);
        }

        errno = 0; // see NOTES section in strtold(3)
        cpu_util_old[i] = strtold(cp, NULL);
        if (errno != 0) {
            safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
            gimli->errflag = 1;
            return (S_FAIL);
        }
    }

    if (fclose(file) != 0) {
        safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
        gimli->errflag = 1;
        return (S_FAIL);
    }

    usleep(200000); // wait 200ms

    // second poll
    if ((file = fopen(PROC_STAT, "r")) == NULL) {
        safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
        gimli->errflag = 1;
        return (S_FAIL);
    }

    // read first line of /proc/stat
    if (fgets(buffer, sizeof (buffer), file) == NULL) {
        safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
        gimli->errflag = 1;
        return (S_FAIL);
    }

    cp = strtok(buffer, " "); // skip first column 'cpu'
    for (i=0; i<CPU_NRSTATS; i++)
    {
        if ((cp = strtok(NULL, " ")) == NULL) {
            safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
            gimli->errflag = 1;
            return (S_FAIL);
        }

        errno = 0; // see NOTES section in strtold(3)
        cpu_util_new[i] = strtold(cp, NULL);
        if (errno != 0) {
            safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
            gimli->errflag = 1;
            return (S_FAIL);
        }
    }

    if (fclose(file) != 0) {
        safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
        gimli->errflag = 1;
        return (S_FAIL);
    }

    // calculate diffs
    for (i=0; i<CPU_NRSTATS; i++)
    {
        if (cpu_util_new[i] > cpu_util_old[i]) {
            cpu_util_diff[i] = (cpu_util_new[i] - cpu_util_old[i]);
        } else if (cpu_util_old[i] > cpu_util_new[i]) {
            cpu_util_diff[i] = (cpu_util_old[i] - cpu_util_new[i]);
        } else {
            cpu_util_diff[i] = 0;
        }
    }

    // calculate cpu_total
    cpu_total = 0;
    for (i=0; i<CPU_NRSTATS; i++)
    {
        cpu_total += cpu_util_diff[i];
    }

    // calculate final percentages
    for (i=0; i<CPU_NRSTATS; i++)
    {
        gimli->cpu_util[i] = (cpu_util_diff[i] / cpu_total) * 100;
    }

    return (S_PASS);
}

/**
 * get_loadavg - sample /proc/loadavg for loadavg
 *
 * Save the first three values from /proc/loadavg:
 *
 *    1) loag avg of last 1 minute
 *    2) load avg of last 5 minutes
 *    3) load avg of last 10 minutes
 *
 * More info about these values can be found in proc(5).
 *
 */
status_t
get_loadavg(gimli_t *gimli)
{
    FILE          *file;
    char           buffer[1024];
    char          *cp;
    unsigned       i;
    long double    cpu_loadavg[LOAD_NRSTATS];

    if ((file = fopen(PROC_LOADAVG, "r")) == NULL) {
        safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
        gimli->errflag = 1;
        return (S_FAIL);
    }

    // read first line of /proc/loadavg
    if (fgets(buffer, sizeof (buffer), file) == NULL) {
        safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
        gimli->errflag = 1;
        return (S_FAIL);
    }

    // we only want first 3 values
    for (i=0; i<LOAD_NRSTATS; i++)
    {
        if (i==0) {
            if ((cp = strtok(buffer, " ")) == NULL) {
                safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
                gimli->errflag = 1;
                return (S_FAIL);
            }
        } else {
            if ((cp = strtok(NULL, " ")) == NULL) {
                safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
                gimli->errflag = 1;
                return (S_FAIL);
            }
        }

        errno = 0; // see NOTES section in strtold(3)
        cpu_loadavg[i] = strtold(cp, NULL);
        if (errno != 0) {
            safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
            gimli->errflag = 1;
            return (S_FAIL);
        }
    }

    if (fclose(file) != 0) {
        safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
        gimli->errflag = 1;
        return (S_FAIL);
    }

    // set struct fields
    for (i=0; i<LOAD_NRSTATS; i++)
    {
        gimli->cpu_loadavg[i] = cpu_loadavg[i];
    }

    return (S_PASS);
}

/**
 * get_uptime - get system uptime in seconds
 *
 * Reads first value from /proc/uptime and sets gimli.uptime.
 *
 * To print in the form: 'X days, Y:Z'
 * where X is days, Y is hours, and Z is minutes:
 *
 *     printf("uptime %lu days, %02lu:%02lu\n", gimli.uptime/86400,
 *               gimli.uptime/3600%24, gimli.uptime/60%60);
 *
 * For more info see proc(5).
 *
 */
status_t
get_uptime(gimli_t *gimli)
{
    FILE          *file;
    char           buffer[1024];
    char          *cp;
    unsigned       i;
    unsigned long  uptime;

    if ((file = fopen(PROC_UPTIME, "r")) == NULL) {
        safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
        gimli->errflag = 1;
        return (S_FAIL);
    }

    // read first line of /proc/uptime
    if (fgets(buffer, sizeof (buffer), file) == NULL) {
        safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
        gimli->errflag = 1;
        return (S_FAIL);
    }

    // first value is uptime
    if ((cp = strtok(buffer, " ")) == NULL) {
        safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
        gimli->errflag = 1;
        return (S_FAIL);
    }

    errno = 0; // see NOTES section in strtoul(3)
    gimli->uptime = strtoul(cp, NULL, 0);
    if (errno != 0) {
        safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
        gimli->errflag = 1;
        return (S_FAIL);
    }

    if (fclose(file) != 0) {
        safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
        gimli->errflag = 1;
        return (S_FAIL);
    }

    return (S_PASS);
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
       safe_strncpy(gimli->errtxt, strerror(errno), ERRTXT_LEN);
       gimli->errflag = 1;
       return (S_FAIL);
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

   return (S_PASS);
}

int
main()
{
    gimli_t           gimli;
    struct timespec   start, stop;
    uint64_t          elapsed;

    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    get_cpu_util(&gimli);
    if (gimli.errflag) {
        printf("get_cpu_util failed: %s\n", gimli.errtxt);
        return (S_FAIL);
    }
    get_loadavg(&gimli);
    if (gimli.errflag) {
        printf("get_loadavg failed: %s\n", gimli.errtxt);
        return (S_FAIL);
    }
    get_uptime(&gimli);
    if (gimli.errflag) {
        printf("get_uptime failed: %s\n", gimli.errtxt);
        return (S_FAIL);
    }
    get_meminfo(&gimli);
    if (gimli.errflag) {
        printf("get_meminfo failed: %s\n", gimli.errtxt);
        return (S_FAIL);
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &stop);

    printf("cpu %.2Lf%% us, %.2Lf%% ni, %.2Lf%% sy, %.2Lf%% id, "
            "%.2Lf%% wa\n",
            gimli.cpu_util[CPU_USER], gimli.cpu_util[CPU_NICE],
            gimli.cpu_util[CPU_SYSTEM], gimli.cpu_util[CPU_IDLE],
            gimli.cpu_util[CPU_IOWAIT]);
    printf("loadavg %.2Lf, %.2Lf, %.2Lf\n",
            gimli.cpu_loadavg[LOAD_ONE],
            gimli.cpu_loadavg[LOAD_FIVE],
            gimli.cpu_loadavg[LOAD_TEN]);
    printf("uptime %lu days, %02lu:%02lu\n", gimli.uptime/86400,
            gimli.uptime/3600%24, gimli.uptime/60%60);
    printf("totalram:  %lu kB\n", gimli.meminfo[TOTAL_RAM]);
    printf("freeram:   %lu kB\n", gimli.meminfo[FREE_RAM]);
    printf("sharedram: %lu kB\n", gimli.meminfo[SHARED_RAM]);
    printf("bufferram: %lu kB\n", gimli.meminfo[BUFFER_RAM]);
    printf("totalswap: %lu kB\n", gimli.meminfo[TOTAL_SWAP]);
    printf("freeswap:  %lu kB\n", gimli.meminfo[FREE_SWAP]);
    printf("totalhigh: %lu kB\n", gimli.meminfo[TOTAL_HIGH]);
    printf("freehigh:  %lu kB\n", gimli.meminfo[FREE_HIGH]);
    printf("number of current processes: %hu\n", gimli.procs);

    elapsed = (BILLION * (stop.tv_sec - start.tv_sec) +
            stop.tv_nsec - start.tv_nsec) / MILLION;
    printf("took %llums\n", (long long unsigned int) elapsed);

    return 0;
}