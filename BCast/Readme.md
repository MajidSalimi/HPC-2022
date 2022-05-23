# Broadcast
In this project it's show alternative implementations of MPI_BCAST method. In particular there are five implementations.

## Ring
In a ring-based broadcast algorithm, processes form a logical ring structure. Beginning with the root process, each process sends data to its logical neighbor. Overall, this takes n−1 steps.
The runtime is: 

**T(Bcast Ring) = (n − 1) × (m*Tbyte+ Tstart).** 

## Binary tree
Binary tree is a tree data structure in which each node has at most two children. Each node send the message to its two children.

The runtime is: 

T(Bcast Binary tree) = log_2(n) × (m*Tbyte+ Tstart). 

## Binomial tree
With Binomial Tree Broadcast the whole message is sent at once. Each node that has already received the message sends it on further. 

The runtime is: 

T(Bcast Binomial tree) = log_2(n) x (m*Tbyte+ Tstart)

## Linear pipeline broadcast

The message is split up into k packages and send piecewise from node n to node n+1.

The runtime is: 

T(Bcast linear pipeline) = (n − 1 + k) × (m/k * Tbyte + Tstart). 

## Pipelined Binary Tree Broadcast
Binary tree is a tree data structure in which each node has at most two children. Each node send the messages split in to k packages to its two children.

The runtime is: 

T(Bcast binary tree pipeline) = (log(n) + k) × (m/k * Tbyte + Tstart). 

## Instructions for Execution
In order to execute the file we need to compile **Bcast.c**:
```
mpicc Bcast.c -lm -o Bcast.out
```
And now we can execute **Bcast.out**:
```
mpirun -np X --allow-run-as-root Bcast.out *size_of_data* *max_iter* *sleep_time*
```
Where:
  - **size_of_data**: Size of the data to broadcast, the data will be in powers of 2.
  - **max_iter**: Number of iterations to repeat the procedure.
  - **sleep_time**: Wait time between iterations (microseconds).
