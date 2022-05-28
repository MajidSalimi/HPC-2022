mpirun --mca btl_vader_single_copy_mechanism none --allow-run-as-root -np 6 ./hpdbscan --input-dataset DBSCAN -i datasets/bremen_small.h5 -o output/hdf5
