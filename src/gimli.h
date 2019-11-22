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
    G_PASS         = 0,
    G_FAIL         = 1
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

void safe_strncpy(char *dest, char *src, unsigned n);
status_t get_cpu_util(gimli_t *gimli);
status_t get_loadavg(gimli_t *gimli);
status_t get_uptime(gimli_t *gimli);
status_t get_meminfo(gimli_t *gimli);