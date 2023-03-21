# OS-Proj2 Documentation

Overview

The code provided is a C program that can be used to generate prime numbers using multiple processes. It is a command-line utility that takes arguments as inputs and checks for errors in the arguments. The program creates N delegator processes and then creates N worker processes for each delegator process. Each worker process checks for prime numbers in the assigned range.

Header Files

The helper.h header file is a user-defined header file that provides helper functions (the merge sort for the final array to display primes) for this program.

Constants

The constants defined in the code are readend, writeend, primeFD, and timeFD. The readend and writeend are defined as 0 and 1, respectively, for convenience. primeFD and timeFD are defined as 1222 and 1233, respectively, for use as file descriptors for pipes used for communication between processes. Using dup2() these are changed at runtime.

Inputs

The main() function first checks for the number of arguments passed and their validity. It expects 8 arguments, including the executable name, and checks whether the given arguments match the expected format. If any error is found in the arguments, the program prints the error message and exits with an error code. Errors could be missing values, negative integers or invalid ranges.
The input is expected in such a format: 

./primes -l LowerBound -u UpperBound -[e|r] -n NumOfNodes

Signals

The sig_handler() function is a signal handler that takes a signal number as an input and increments the corresponding signal counter. It is used for counting the number of SIGUSR1 and SIGUSR2 signals received by the program. The program then sets the signal handlers for SIGUSR1 and SIGUSR2 using the signal() function. The signal() function associates a signal handler function (in this case, sig_handler()) with a signal. When the program receives a signal, the corresponding signal handler function is called.

Pipes

There is an array of pipes from root to N delegator nodes, rdpipefd. This is used to send information regarding range.
There is an array of pipes from every delegator to its N worker nodes, dwpipefd. This is used to send information regarding range.
There is one pipe for communication between N delegator nodes to root, drpipefd. This is used to send all the primes from all the delegators to root.
There is one pipe for communication between N worker nodes to a delegator node, wdpipefd. This is used to send all the primes from all the children worker nodes to respective delegator.

Forking Layout

After creating the pipes, the program creates N delegator processes using the fork() function. In each delegator process, the program creates N worker processes using the fork() function. The delegator process sends the range of values to each worker process through a pipe. Each worker process then checks for prime numbers within the given range and sends the primes back to the delegator process through a pipe. The delegator process then sends the primes to the root process through another pipe.

Time and Prime Calculation

Finally, the program calculates the execution time of the program and sends it to the root process through the pipe. prime1.c and prime2.c are used using the startcode provided in assignment to handle the calculation of primes in circular manner and the tracking of time elapsed. The root process receives the primes and execution time from each delegator process and prints them to the console.
