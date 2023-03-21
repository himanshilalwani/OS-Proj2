/*Authors: Fatima Nadeem, Himanshi Lalwani
OS Programming Assignment 2
The main code*/

// include necessary header files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

#include "helper.h"

#define readend 0
#define writeend 1
#define primeFD 1222
#define timeFD 1233

int number_signals1 = 0;
int number_signals2 = 0;

void sig_handler(int num)
{
    if (num == SIGUSR1)
    {
        signal(SIGUSR1, sig_handler);
        number_signals1 += 1;
    }
    else if (num == SIGUSR2)
    {
        signal(SIGUSR2, sig_handler);
        number_signals2 += 1;
    }
}

int main(int argc, char *argv[])
{
    // check for argument numbers
    if (argc != 8)
    {
        printf("Error: Missing arguments\n");
        printf("Usage: %s -l LowerBound -u UpperBound -[e|r] -n NumOfNodes\n", argv[0]);
        return 1;
    }

    // flag types check
    if (strcmp(argv[1], "-l") != 0 || strcmp(argv[3], "-u") != 0 || (strcmp(argv[5], "-e") != 0 && strcmp(argv[5], "-r") != 0) || (strcmp(argv[6], "-n") != 0))
    {
        printf("Error: Invalid arguments\n");
        printf("Usage: %s -l LowerBound -u UpperBound -[e|r] -n NumOfNodes\n", argv[0]);
        return 1;
    }

    // check for negative numbers
    if (atoi(argv[2]) < 1)
    {
        printf("Error: Invalid Lower Bound, must be greater than 0\n");
        return 1;
    }
    if (atoi(argv[4]) < 1)
    {
        printf("Error: Invalid Upper Bound, must be greater than 0\n");
        return 1;
    }
    // check for range to be valid
    if (atoi(argv[4]) < atoi(argv[2]))
    {
        printf("Error: Invalid Upper Bound, must be greater than Lower Bound\n");
        return 1;
    }
    // check for num nodes to be valid int
    if (atoi(argv[7]) < 1)
    {
        printf("Error: Invalid Number of Nodes, must be greater than 0\n");
        return 1;
    }

    int batch; // delegator batch
    signal(SIGUSR1, sig_handler);
    signal(SIGUSR2, sig_handler);
    // convert arguments to variables
    int lowerbound = atoi(argv[2]);
    int upperbound = atoi(argv[4]);
    char *work = argv[5];
    int numNodes = atoi(argv[7]);

    int type; // 0 for equal and 1 for unequal distribution of range
    // for delegator sub intervals
    int start;
    int stop;
    int i;         // delegtor process count
    int primetype; // cicrular manner the function to use 0 is prime1 1 is prime2

    stop = lowerbound - 1;

    // convert distibution type
    if (strcmp(work, "-e") == 0)
    {
        type = 0;
    }
    else
    {
        type = 1;
    }

    // create pipes for delegator -> root (one delegator at a time) used for sending primes
    int drpipefd[2];

    if (pipe(drpipefd) == -1)
    {
        perror("pipe error for del -> root for primes\n");
        exit(EXIT_FAILURE);
    }

    // create pipe for delegator -> root used for sending execution time
    int drpipefdt[2];
    if (pipe(drpipefdt) == -1)
    {
        perror("pipe error for del -> root for time\n");
        exit(EXIT_FAILURE);
    }

    // create an array of pipes for each delegator process root -> delegator
    int rdpipefd[numNodes][2];

    pid_t root = getpid(); // root pid for sending signals
    // create N delegator processes
    for (i = 0; i < numNodes; i++)
    {
        // create a pipe for each delegator process
        if (pipe(rdpipefd[i]) == -1)
        {
            perror("pipe error for root -> del for information\n");
            exit(EXIT_FAILURE);
        }

        // create a new process (delegator)
        pid_t pid = fork();

        // check for errors creating the process
        if (pid == -1)
        {
            perror("fork failure for delegator process\n");
            exit(EXIT_FAILURE);
        }
        // in the delegator process
        else if (pid == 0)
        {
            // close the read end of the pipe delegator -> root
            close(drpipefdt[readend]);
            close(drpipefd[readend]);
            // close write end for root -> delegator
            close(rdpipefd[i][writeend]);

            // reading the bounds for delegator
            read(rdpipefd[i][readend], &start, sizeof(int));
            read(rdpipefd[i][readend], &stop, sizeof(int));

            // close reading end
            close(rdpipefd[i][readend]);

            // create bounds for worker process
            int wstart = start;
            int wstop = start - 1;
            int j; // worker process count

            // pipe to get primes from worker -> delegator
            int wdpipefd[2];

            // array of pipes from delegator -> workers
            int dwpipefd[numNodes][2];

            if (pipe(wdpipefd) == -1)
            {
                perror("pipe error for workers -> del for primes\n");
                exit(EXIT_FAILURE);
            }

            // pipe to get times from worker -> delegator
            int wdpipfdt[2];

            if (pipe(wdpipfdt) == -1)
            {
                perror("pipe error for workers -> del for time\n");
                exit(EXIT_FAILURE);
            }
            // for loop to create workers
            for (j = 0; j < numNodes; j++)
            {
                // create a pipe for each worker process
                if (pipe(dwpipefd[j]) == -1)
                {
                    perror("pipe error for del -> worker for information\n");
                    exit(EXIT_FAILURE);
                }
                batch = j % 2; // determining batch associated with a particular delegate node
                // forking
                //  create a new process (delegator)
                pid_t wpid = fork();

                if (wpid == -1)
                {
                    perror("fork failure worker process");
                    exit(EXIT_FAILURE);
                }
                // decide type of the prime function cicular manner
                primetype = ((i * numNodes) + j) % 2;

                // in worker process
                if (wpid == 0)
                {
                    // close writing end for del -> work
                    close(dwpipefd[j][writeend]);

                    // convert values to string
                    char lb[40];
                    char ub[40];
                    char b[40];
                    char r_pid[40];
                    // read the ranges from delegator
                    read(dwpipefd[j][readend], &start, sizeof(int));
                    read(dwpipefd[j][readend], &stop, sizeof(int));

                    // covert int->string for passing in exec
                    sprintf(b, "%d", batch);
                    sprintf(r_pid, "%d", root);
                    sprintf(lb, "%d", start);
                    sprintf(ub, "%d", stop);

                    // close reading end of del -> worker
                    close(dwpipefd[j][readend]);
                    // close reading end of worker -> del
                    close(wdpipefd[readend]);
                    close(wdpipfdt[readend]);

                    dup2(wdpipefd[writeend], primeFD);
                    dup2(wdpipfdt[writeend], timeFD);
                    // call executable based on worker type
                    if (primetype == 0)
                    {
                        execlp("./primes1", "primes1", lb, ub, r_pid, b, NULL);
                    }
                    else
                    {
                        execlp("./primes2", "primes2", lb, ub, r_pid, b, NULL);
                    }
                    break;
                }
                // delegator process after forking
                else
                {
                    // close the read end of del -> work
                    close(dwpipefd[j][readend]);

                    // check for distribution type
                    // equal intervals
                    if (type == 0)
                    {
                        // Dividing the range in N (numNodes) sub-intervals
                        int total_length = stop - start + 1;
                        int subrange_length = total_length / numNodes;
                        wstart = subrange_length * j + start;
                        wstop = wstart + subrange_length - 1;

                        // last delegator (even or not)
                        if (wstop > stop || j == numNodes - 1)
                        {
                            wstop = stop;
                        }
                    }
                    // random intervals
                    else
                    {
                        wstart = wstop + 1;
                        // if last worker node, make its stop number to be the stop number of the delegator node
                        if (j == numNodes - 1)
                        {
                            wstop = stop;
                        }
                        else
                        {
                            wstop = wstart + rand() % (stop - wstart - (numNodes - j - 1) + 1);
                        }
                    }

                    // write it to pipes
                    write(dwpipefd[j][writeend], &wstart, sizeof(int));
                    write(dwpipefd[j][writeend], &wstop, sizeof(int));
                    // closing the writing pipes
                    close(dwpipefd[j][writeend]);
                }
            }

            // delegator node after the for loop ends
            if (j == numNodes)
            {
                close(wdpipfdt[writeend]);
                // close write end worker -> del
                close(wdpipefd[writeend]);
                pid_t wpid;
                int wstatus = 0;

                // loop to get time feom all workers
                while (true)
                {
                    double real_time;
                    int tcheck = read(wdpipfdt[readend], &real_time, sizeof(double));
                    // finished reading
                    if (tcheck == 0)
                    {
                        break;
                    }
                    // write it to root
                    write(drpipefdt[writeend], &real_time, sizeof(double));
                }
                close(wdpipfdt[readend]);

                // loop to read primes from all workers
                while (true)
                {
                    int primenumber;
                    int bcheck = read(wdpipefd[readend], &primenumber, sizeof(int));
                    // finished reading
                    if (bcheck == 0)
                    {
                        break;
                    }
                    // write it to root
                    write(drpipefd[writeend], &primenumber, sizeof(int));
                }
                close(drpipefdt[writeend]);
                close(wdpipefd[readend]);
                close(drpipefd[writeend]);

                while ((wpid = wait(&wstatus)) != -1)
                    ;
                exit(0);
            }
            break;
        }
        // root process after forking
        else
        {
            // close the read end of root -> delegator
            close(rdpipefd[i][readend]);

            // check for distribution type
            // equal intervals
            if (type == 0)
            {
                // Dividing the range in N (numNodes) sub-intervals
                int total_length = upperbound - lowerbound + 1;
                int subrange_length = total_length / numNodes;
                start = subrange_length * i + lowerbound;
                stop = start + subrange_length - 1;

                // last delegator (even or not)
                if (stop > upperbound || i == numNodes - 1)
                {
                    stop = upperbound;
                }
            }
            // random intervals
            else
            {
                start = stop + 1;
                // if last node, make its stop to be the upper bound
                if (i == numNodes - 1)
                {
                    stop = upperbound;
                }
                else
                {
                    // upperbound - start = remaining range
                    stop = start + rand() % (upperbound - start - (numNodes - i - 1) + 1);
                }
            }
            // write it pipes
            write(rdpipefd[i][writeend], &start, sizeof(int));
            write(rdpipefd[i][writeend], &stop, sizeof(int));
            // closing the writing pipes
            close(rdpipefd[i][writeend]);
        }
    }
    // root process after for loop for forks - the main
    if (i == numNodes)
    {
        close(drpipefdt[writeend]);
        close(drpipefd[writeend]);
        pid_t dpid;
        double minimum_time, maximum_time, average_time;
        int dstatus = 0;
        int count = 0;
        int timer_checker;
        int x = 0;
        while (true)
        {
            double current_time;
            timer_checker = read(drpipefdt[0], &current_time, sizeof(double));
            // store first read value of current_time as minimum_time and update minimum_time later by comparison
            if (x == 0)
            {
                minimum_time = current_time;
            }
            if (timer_checker == 0)
                break;
            average_time = average_time + current_time;
            if (current_time > maximum_time)
            {
                maximum_time = current_time;
            }
            if (current_time < minimum_time)
            {
                minimum_time = current_time;
            }
        }
        close(drpipefdt[readend]);
        // array to store primes
        int myprimes[upperbound - lowerbound + 1];

        // wait for all children
        while (true)
        {
            int myprime;
            int bcheck = read(drpipefd[readend], &myprime, sizeof(int));
            if (bcheck == 0)
            {
                break;
            }
            myprimes[count] = myprime;

            count++;
        }

        // average time calculator
        average_time = average_time / count;
        // close read end
        close(drpipefd[readend]);

        while ((dpid = wait(&dstatus)) != -1)
            ;

        // mergesort
        mergeSort(myprimes, count);
        printf("\nSorted Primes between %d and %d are:\n\n", lowerbound, upperbound);

        // printing primes
        for (int k = 0; k < count; k++)
        {
            printf("%d ", myprimes[k]);
        }
        printf("\n\nSTATS: \n");
        // time is in ms
        printf("Minimum Time = %f\n", minimum_time);
        printf("Maximum Time = %f\n", maximum_time);
        printf("Average Time = %f\n", average_time);
        printf("\n\nTotal no of SIGUSR1 signals caught: %d\n", number_signals1);
        printf("Total no of SIGUSR2 signals caught: %d\n\n", number_signals2);
        exit(0);
    }
}