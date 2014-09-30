#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

   int rank, size, buf;
   MPI_Status status;
   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   MPI_Comm_size(MPI_COMM_WORLD, &size);
   
   buf = 0;
  
   //for(;;) { 
   if (rank == 0) {
      buf = buf + 10;
      MPI_Send(&buf, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
      printf("%d has sent %d to %d\n", rank, buf, rank + 1);
      MPI_Recv(&buf, 1, MPI_INT, size - 1, 0, MPI_COMM_WORLD, &status);
      printf("%d has received %d from %d\n", rank, buf, size - 1);
      printf("[END] buffer@%d : %d\n", rank, buf);
   }
   else if (rank >= 1 && rank < size - 1) {
      MPI_Recv(&buf, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &status);
      printf("%d has received %d from %d\n", rank, buf, rank - 1);
      buf = buf + 10;
      printf("Increasing buffer on %d: + 10\n", rank);
      MPI_Send(&buf, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
      printf("%d has sent %d to %d\n", rank, buf, rank + 1);
   }
   else if (rank == size - 1) {
      MPI_Recv(&buf, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &status);
      printf("%d has received %d from %d\n", rank, buf, rank - 1);
      buf = buf + 10;
      printf("[LAST NODE] Increasing buffer on %d: + 10\n", rank);
      MPI_Send(&buf, 1, MPI_INT, rank % (size-1), 0, MPI_COMM_WORLD);
      printf("%d has sent %d to %d\n", rank, buf, rank % (size-1));
   }
   //}
   
   MPI_Finalize();
   return 0;
}
