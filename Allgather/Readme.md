### ALLGATHER ALGOTITHMS
There are 5 different algorithms of ALL_Garher:

 -1  is an allgather implementation by a simple Send/receive
 
 -2  is an allgather implementation by Gather/Bcast
 
 -3  is an allgather implementation by allgather
 
 -4  is an allgather implementation by ring
 
 -5  is an allgather implementation by recursive doubling


#### To run: 
```
mpirun -n nprocs --allow-run-as-root --oversubscribe a.out <data_size> <num_iterations> <delay_between_iterations>
```
