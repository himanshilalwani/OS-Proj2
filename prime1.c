#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define YES 1
#define NO 0
#define primeFD 1222

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
    int lb = 0, ub = 0, i = 0;
    if ((argc != 3))
    {
        printf(" usage : prime1 lb ub\n");
        exit(1);
    }
    lb = atoi(argv[1]);
    ub = atoi(argv[2]);
    if ((lb < 1) || (lb > ub))
    {
        printf(" usage1 : prime1 lb ub\n");
        exit(1);
    }
    for (i = lb; i <= ub; i++)
    {
        if (prime(i) == YES)
        {
            printf("prime detected %d\n",i);
            write(primeFD, &i, sizeof(int));
        }
    }
    close(primeFD);
}