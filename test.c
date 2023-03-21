// include necessary header files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>

#define readend 0
#define writeend 1
#define primeFD 1222

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

    int batch; // delegator batch
    signal(SIGUSR1, sig_handler);
    signal(SIGUSR2, sig_handler);
    // convert arguments to variables
    int lowerbound = atoi(argv[2]);
    int upperbound = atoi(argv[4]);
    char *work = argv[5];
    int numNodes = atoi(argv[7]);

    int type; // 0 for equal and 1 for unequal
    // for sub intervals
    int start;
    int stop;
    int i;         // delegtor process count
    int primetype; // cicrular manner the function to use 0 is prime1 1 is prime2

    stop = upperbound - 1;

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
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // create an array of pipes for each delegator process root -> delegator
    int rdpipefd[numNodes][2];

    // printf("error check for input and pipe d->r\n");
    pid_t root = getpid(); // root pid for sending signals
    // create N delegator processes
    for (i = 0; i < numNodes; i++)
    {
        // create a pipe for each delegator process
        if (pipe(rdpipefd[i]) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        // create a new process (delegator)
        pid_t pid = fork();
        // printf("created fork delegator\n");

        // check for errors creating the process
        if (pid == -1)
        {
            perror("fork failure delegator process");
            exit(EXIT_FAILURE);
        }
        // in the delegator process
        else if (pid == 0)
        {
            // printf("in delegator process %d\n", i);
            // close the read end of the pipe delegator -> root
            close(drpipefd[readend]);
            // close write end for root -> delegator
            close(rdpipefd[i][writeend]);

            // reading the bounds for delegator
            read(rdpipefd[i][readend], &start, sizeof(int));
            read(rdpipefd[i][readend], &stop, sizeof(int));
            // printf("read bound for delegator %d: start: %d stop: %d\n", i, start, stop);

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

            // printf("pipes w->d\n");

            if (pipe(wdpipefd) == -1)
            {
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            // for loop to create workers
            for (j = 0; j < numNodes; j++)
            {
                // create a pipe for each worker process
                if (pipe(dwpipefd[j]) == -1)
                {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }
                batch = j % 2; // determining batch associated with a particular delegate node
                // forking
                //  create a new process (delegator)
                pid_t wpid = fork();
                // printf("created fork worker\n");

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
                    // printf("in worker process %d\n", j);

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
                    // printf("read bound for worker %d: start: %d stop: %d\n", j, start, stop);

                    // covert int->string for passing in exec
                    sprintf(b, "%d", batch);
                    sprintf(r_pid, "%d", root);
                    sprintf(lb, "%d", start);
                    sprintf(ub, "%d", stop);

                    // close reading end of del -> worker
                    close(dwpipefd[j][readend]);
                    // close reading end of worker -> del
                    close(wdpipefd[readend]);

                    dup2(wdpipefd[writeend], primeFD);

                    // call executable based on worker type
                    if (primetype == 0)
                    {
                        // printf("running prime1 for worker %d\n",j);
                        execlp("./primes1", "primes1", lb, ub, r_pid, b, NULL);
                        // printf("finished running prime1 for worker %d\n",j);
                    }
                    else
                    {
                        // printf("running prime2 for worker %d\n",j);
                        execlp("./primes2", "primes2", lb, ub, r_pid, b, NULL);
                        // printf("finished running prime2 for worker %d\n",j);
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
                        if (j == numNodes - 1)
                        {
                            wstop = stop;
                        }
                        else
                        {
                            wstop = wstart + rand() % (stop - wstart - (numNodes - j - 1) + 1);
                        }
                    }

                    // printf("created intervals for worker %d\n",j);

                    // write it pipes
                    write(dwpipefd[j][writeend], &wstart, sizeof(int));
                    write(dwpipefd[j][writeend], &wstop, sizeof(int));
                    // closing the writing pipes
                    close(dwpipefd[j][writeend]);

                    // printf("done writing intervals for worker %d\n",j);
                }
            }

            // delegator node after the for loop ends
            if (j == numNodes)
            {
                // close write end worker -> del
                close(wdpipefd[writeend]);
                pid_t wpid;
                int wstatus = 0;

                // loop to read primes from all workers
                while (1)
                {
                    int primenumber;
                    int bcheck = read(wdpipefd[readend], &primenumber, sizeof(int));
                    // printf("read in delegator %d, value: %d\n", i, primenumber);
                    if (bcheck == 0)
                    {
                        break;
                    }
                    // write it to root
                    write(drpipefd[writeend], &primenumber, sizeof(int));
                }
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
                if (i == numNodes - 1)
                {
                    stop = upperbound;
                }
                else
                {
                    stop = start + rand() % (upperbound - start - (numNodes - (i + 1)) + 1);
                }
            }

            // printf("created intervals for delegator %d\n",i);

            // write it pipes
            write(rdpipefd[i][writeend], &start, sizeof(int));
            write(rdpipefd[i][writeend], &stop, sizeof(int));
            // closing the writing pipes
            close(rdpipefd[i][writeend]);
            // printf("done writing intervals for delegator %d\n",i);
        }
    }
    // root process after for loop for forks - the main
    if (i == numNodes)
    {

        close(drpipefd[writeend]);
        pid_t dpid;
        int dstatus = 0;
        int count = 0;

        // array to store primes
        int myprimes[upperbound - lowerbound + 1];

        // wait for all children
        while (1)
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

        // close read end
        close(drpipefd[readend]);

        while ((dpid = wait(&dstatus)) != -1)
            ;

        // printing primes
        for (int k = 0; k < count; k++)
        {
            printf("Detected: %d\n", myprimes[k]);
        }
        printf("Signals set via SIGUSR1 = %d\n", number_signals1);
        printf("Signals set via SIGUSR2 = %d\n", number_signals2);
        exit(0);
    }
}