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

int main(int argc, char *argv[])
{
    //check for argument numbers
    if (argc != 8)
    {
        printf("Error: Missing arguments\n");
        printf("Usage: %s -l LowerBound -u UpperBound -[e|r] -n NumOfNodes\n", argv[0]);
        return 1;
    }

    //flag types check
    if (strcmp(argv[1], "-l") != 0 || strcmp(argv[3], "-u") != 0 || (strcmp(argv[5], "-e") != 0 && strcmp(argv[5], "-r") != 0) || (strcmp(argv[6], "-n") != 0))
    {
        printf("Error: Invalid arguments\n");
        printf("Usage: %s -l LowerBound -u UpperBound -[e|r] -n NumOfNodes\n", argv[0]);
        return 1;
    }

    //convert arguments to variables
    int lowerbound = atoi(argv[2]);
    int upperbound = atoi(argv[4]);
    char *work = argv[5];
    int numNodes = atoi(argv[7]);

    int type; // 0 for equal and 1 for unequal
    //for sub intervals
    int start;
    int stop; 
    int primetype; //cicrular manner the function to use

    stop = upperbound - 1;

    //convert distibution type
    if (strcmp(work, "-e") == 0){
        type = 0;
    }
    else {
        type = 1; 
    }

    //create pipes for delegator -> root (one delegator at a time) used for sending primes
    int drpipefd[2];

    if (pipe(drpipefd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // create an array of pipes for each delegator process root -> delegator
    int rdpipefd[numNodes][2];

    // create N delegator processes
    for (int i = 0; i < numNodes; i++)
    {
        // create a pipe for each delegator process
        if (pipe(rdpipefd[i]) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        // create a new process (delegator)
        pid_t pid = fork();

        // check for errors creating the process
        if (pid == -1)
        {
            perror("fork failure delegator process");
            exit(EXIT_FAILURE);
        }
        // in the delegator process
        else if (pid == 0)
        {
            // close the read end of the pipe delegator -> root
            close(drpipefd[readend]);
            // close write end for root -> delegator
            close(rdpipefd[i][writeend]);

            //reading the bounds for delegator
            read(rdpipefd[i][readend], &start, sizeof(int));
            read(rdpipefd[i][readend], &stop, sizeof(int));

            //close reading end
            close(rdpipefd[i][readend]);

            //create bounds for worker process
            int wstart = start;
            int wstop = start - 1;

            //pipe to get primes from worker -> delegator
            int wdpipefd[2];

            //array of pipes from delegator -> workers
            int dwpipefd[numNodes][2];

            if (pipe(wdpipefd) == -1)
            {
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            //for loop to create workers
            for (int j = 0; j < numNodes; j++)
            {
                // create a pipe for each worker process
                if (pipe(dwpipefd[j]) == -1)
                {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }

                //forking
                // create a new process (delegator)
                pid_t wpid = fork();
                if (wpid == -1)
                {
                    perror("fork failure worker process");
                    exit(EXIT_FAILURE);
                }
                //decide type of the prime function cicular manner
                primetype = ((i * numNodes) + j)%2;

                //in worker process
                if (wpid == 0)
                {
                    //close writing end for del -> work
                    close(dwpipefd[j][writeend]);

                    //read from del
                }
                
                

            }
            


            


        }
        //root process after forking
        else{
            //close the read end of root -> delegator
            close(rdpipefd[i][readend]);

            //check for distribution type
            //equal intervals
            if(type==0){
                // Dividing the range in N (numNodes) sub-intervals
                int total_length = upperbound - lowerbound + 1;
                int subrange_length = total_length / numNodes;
                start = subrange_length * i + lowerbound;
                stop = start + subrange_length - 1;

                //last delegator (even or not)
                if(stop>upperbound || i == numNodes-1){
                    stop = upperbound;
                }

            }
            //random intervals
            else{
                start = stop + 1;
                if (i==numNodes-1)
                {
                    stop = upperbound;
                }
                else{
                    stop = start + rand()%(upperbound-start-(numNodes-(i+1))+1);
                }
            }

            //write it pipes
            write(rdpipefd[i][writeend],&start, sizeof(int));
            write(rdpipefd[i][writeend],&stop, sizeof(int));
            //closing the writing pipes
            close(rdpipefd[i][writeend]);

        }

        //root process after for loop for forks

    }


}