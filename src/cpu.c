#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<stdio.h>
 
int main()
{
	char str[100];
	const char d[2] = " ";
	char *token;
	int i;
	long int sum = 0, idle, lastSum = 0, lastIdle = 0;
	long double idleFraction;
 
    while (1) {
        FILE* fp = fopen("/proc/stat","r");
        i = 0;
        fgets(str, 100, fp);
        fclose(fp);
        token = strtok(str, d);

        while (token != NULL) {
            token = strtok(NULL,d);
            if (token != NULL) {
                sum += atoi(token);
                if (i == 3) {
                    idle = atoi(token);
                }
                i++;
            }
        }

        /*
           from __future__ import print_function
           from time import sleep

           last_idle = last_total = 0
           while True:
               with open('/proc/stat') as f:
                   fields = [float(column) for column in f.readline().strip().split()[1:]]
               idle, total = fields[3], sum(fields)
               idle_delta, total_delta = idle - last_idle, total - last_total
               last_idle, last_total = idle, total
               utilisation = 100.0 * (1.0 - idle_delta / total_delta)
               print('%5.1f%%' % utilisation, end='\r')
               sleep(5)
        */

        // printf("\nIdle for : %lf %% of the time.", (1.0 - (idle - lastIdle * 1.0) / (sum - lastSum)) * 100);
        // utilisation = 100.0 * (1.0 - idle_delta / total_delta)
        printf("%5.1lf %%\n", 100.0 * (1.0 - (idle - lastIdle * 1.0) / (sum - lastSum)));

        lastIdle = idle;
        lastSum = sum;

        sleep(3);
    }	
	return 0;
}
