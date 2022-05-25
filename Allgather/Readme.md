### ALLGATHER ALGOTITHMS
There are 6 different algorithms of Allgather:

 -1  is an Allgather implementation by a simple Send/Receive
 
 -2  is an Allgather implementation by MPI_Gather/MPI_Bcast
 
 -3  is an Allgather implementation by MPI_Allgather
 
 -4  is an Allgather implementation by Ring Algorithm
 
 -5  is an Allgather implementation by Recursive Doubling Algorithm

 -6  is an Allgather implementation by Recursive Halving Algorithm
 
### Compile:
```
mpicc <ex*.c> -lm -o <ex*>
```

### To run: 
```
mpirun -n nprocs --allow-run-as-root --oversubscribe ex* <data_size> <num_iterations> <delay_between_iterations>
```
