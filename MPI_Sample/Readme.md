## Sample Code 
This simple program, generates the required size of data and does the broadcast in an iteration.

#### To run: 
```
make
mpirun -n nprocs a.out data_size iterations delay_between_iterations(us)
```
#### Note: 
The data_size will be in powers of 2. You just need to put power. For example if you put data_size=5, it will be 2^5 bytes.
