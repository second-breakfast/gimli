#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CPU_FMTS_JUST1  "cpu %Lu %Lu %Lu %Lu %Lu"
#define CPU_FMTS_MULTI  "cpu%*d %Lu %Lu %Lu %Lu %Lu"

static int   Cpu_tot;

typedef unsigned long long TICS_t;

typedef struct {
    /* ticks count as represented in /proc/stat */
    TICS_t u, n, s, i, w;
    /* tics count in the order of our display */
    TICS_t u_sav, s_sav, n_sav, i_sav, w_sav;
} CPUS_t;

#if 0
        /*
         * This guy's modeled on libproc's 'five_cpu_numbers' function except
         * we preserve all cpu data in our CPUS_t array which is organized
         * as follows:
         *    cpus[0] thru cpus[n] == tics for each separate cpu
         *    cpus[Cpu_tot]        == tics from the 1st /proc/stat line */
static CPUS_t *refreshcpus (CPUS_t *cpus)
{
   static FILE *fp = NULL;
   int i;
      /* enough for a /proc/stat CPU line (not the intr line) */
   char buf[SMLBUFSIZ];

      /* by opening this file once, we'll avoid the hit on minor page faults
         (sorry Linux, but you'll have to close it for us) */
   if (!fp) {
      if (!(fp = fopen("/proc/stat", "r")))
         std_err(fmtmk("Failed /proc/stat open: %s", strerror(errno)));
      /* note: we allocate one more CPUS_t than Cpu_tot so that the last slot
               can hold tics representing the /proc/stat cpu summary (the first
               line read) -- that slot supports our View_CPUSUM toggle */
      cpus = alloc_c((1 + Cpu_tot) * sizeof(CPUS_t));
   }
   rewind(fp);
   fflush(fp);

      /* first value the last slot with the cpu summary line */
   if (!fgets(buf, sizeof(buf), fp)) std_err("failed /proc/stat read");
   if (4 > sscanf(buf, CPU_FMTS_JUST1
      , &cpus[Cpu_tot].u, &cpus[Cpu_tot].n, &cpus[Cpu_tot].s, &cpus[Cpu_tot].i, &cpus[Cpu_tot].w))
         std_err("failed /proc/stat read");
      /* and just in case we're 2.2.xx compiled without SMP support... */
   if (1 == Cpu_tot) memcpy(cpus, &cpus[1], sizeof(CPUS_t));

      /* and now value each separate cpu's tics */
   for (i = 0; 1 < Cpu_tot && i < Cpu_tot; i++) {
#ifdef PRETEND4CPUS
      rewind(fp);
#endif
      if (!fgets(buf, sizeof(buf), fp)) std_err("failed /proc/stat read");
      if (4 > sscanf(buf, CPU_FMTS_MULTI
         , &cpus[i].u, &cpus[i].n, &cpus[i].s, &cpus[i].i, &cpus[i].w))
            std_err("failed /proc/stat read");
   }

   return cpus;
}
#endif

int
main()
{
   int i;
   char buf[256];
   static FILE *fp = NULL;
   CPUS_t *cpus;
   Cpu_tot = sysconf(_SC_NPROCESSORS_CONF); // or _SC_NPROCESSORS_ONLN

   if (!(fp = fopen("/proc/stat", "r"))) {
       printf("fopen error\n");
       exit(1);
   }

   cpus = malloc((1 + Cpu_tot) * sizeof(CPUS_t));
   rewind(fp);
   fflush(fp);
   if (!fgets(buf, sizeof(buf), fp)) {
       printf("failed /proc/stat read\n");
       exit(1);
   }
   if (sscanf(buf, CPU_FMTS_JUST1, &cpus[Cpu_tot].u, &cpus[Cpu_tot].n,
               &cpus[Cpu_tot].s, &cpus[Cpu_tot].i, &cpus[Cpu_tot].w) < 4) {

   }
   for (i = 0; 1 < Cpu_tot && i < Cpu_tot; i++) {
       if (!fgets(buf, sizeof(buf), fp)) {
           printf("failed /proc/stat read\n");
           exit(1);
       }
       if (sscanf(buf, CPU_FMTS_MULTI
                   , &cpus[i].u, &cpus[i].n, &cpus[i].s, &cpus[i].i, &cpus[i].w) < 4) {
           printf("failed /proc/stat read\n");
           exit(1);
       }
   }
   // return cpus;

   return 0;
}
