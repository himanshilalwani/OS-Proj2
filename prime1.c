#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/times.h>

#define YES 1
#define NO 0
#define primeFD 1222
#define timeFD 1233

int prime(int n)
{
    int i;
    if (n == 1)
        return (NO);
    for (i = 2; i < n; i++)
        if (n % i == 0)
            return (NO);
    return (YES);
}

int main(int argc, char *argv[])
{
    double t1, t2, real_time;
    struct tms tb1, tb2;
    double ticspersec;
    int lb = 0, ub = 0, i = 0;
    if ((argc != 5))
    {
        printf(" usage : prime1 lb ub root batch\n");
        exit(1);
    }
    lb = atoi(argv[1]);
    ub = atoi(argv[2]);
    // printf("Lower: %d, Upper: %d\n", lb, ub);
    if ((lb < 1) || (lb > ub))
    {
        printf(" usage : prime1 lb ub root batch\n");
        exit(1);
    }
    ticspersec = (double)sysconf(_SC_CLK_TCK);
    t1 = (double)times(&tb1);
    for (i = lb; i <= ub; i++)
    {
        if (prime(i) == YES)
        {
            // printf("print i: %d\n", i);
            write(primeFD, &i, sizeof(int));
        }
    }
    close(primeFD);
    int b_number = atoi(argv[4]);
    int r_pid = atoi(argv[3]);
    if (b_number == 0)
    {
        kill(r_pid, SIGUSR1);
    }
    else
    {
        kill(r_pid, SIGUSR2);
    }
    t2 = (double)times(&tb2);
    real_time = ((t2 - t1) / ticspersec)*1000;
    write(timeFD, &real_time, sizeof(real_time));
    close(timeFD);
    exit(0);
}