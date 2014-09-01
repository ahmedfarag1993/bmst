##Boruvka Algorithm MST

Parallel Implementation of Boruvka Minimum Spanning Tree


##How to build
```$ mpicc <filename.c> -o <output_filename>```

##How to run
```$ mpirun -np <process_number> <output_filename> <matrix_dimension> <matrix_file_path>```

Example: mpirun -np 8 main 32 ./graph-gen/graph.txt
