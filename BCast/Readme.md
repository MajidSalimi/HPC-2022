## Instructions for Execution
In order to execute the file we need to compile **Bcast.c**:
```
mpicc Bcast.c -lm -o Bcast.out
```
And now we can execute **Bcast.out**:
```
mpirun -np X --mca btl_vader_single_copy_mechanism none --allow-run-as-root Bcast.out *size_of_data* *max_iter* *sleep_time*
```
Where:
  - **size_of_data**: Size of the data to broadcast, the data will be in powers of 2.
  - **max_iter**: Number of iterations to repeat the procedure.
  - **sleep_time**: Wait time between iterations (microseconds).
