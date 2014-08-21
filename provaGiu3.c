#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

/* LEGGE E MEMORIZZA IL FILE CON IL GRAFO SU UNA MATRICE */

  FILE *file;
  int n = 10;
  float matrix[n][n]; // matrice input
  float mr[n][n]; // matrice risultato

  file = fopen("./graph-gen/graph.txt", "r"); //apre il file
  if (file==NULL) {
      perror("Errore in apertura del file");
      exit(1);
  }

  while (!feof(file)) {	 //legge e memorizza i valori in una matrice (array bidimensionale)
      int i = 0;
  	   int j = 0;
      for (i = 0; i < n; i++) {
  		   for (j = 0; j < n; j++) {
  			   fscanf(file, "%f", &matrix[i][j]);
		   }
	   }
  }
  
  fclose(file); //chiude il file
  
/* FINE OPERAZIONI DA FILE ESTERNI */

  int rank, size;
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  
  int i;
  int j;
  
  // matrice risultato inizializzata con 0
  for (i = 0; i < n; i++)
      for (j = 0; j < n; j++)
         mr[i][j] = 0;
  
  // print test
  if (rank == 0) {
      printf(" *** MATRICE IMPORTATA ***\n");
	      for (i=0; i<n; i++) {
	  	      for (j=0; j<n; j++) {
	         	printf("%.2f ", matrix[i][j]); //lo stampa come controllo GIUSTO
	   	   }
		      printf("\n");
	      }
  }
  
  /* SYNC */
  MPI_Barrier(MPI_COMM_WORLD);
  
  /* ALGORITMO PER TROVARE IL MIN SULLE RIGHE */
  // ogni processore legge la riga=rank di matrix e scrive il valore min in mr
  
  int indexR = 0; //indice di riga
  int indexC = 0; //indice di colonna
  float min;
  int k;
  
  for (k = 0; k < n; k++) {
      if (matrix[rank][k] != 0) {
   		min = matrix[rank][k];
   		break;
   	}
  }

  for (k = 0; k < n; k++) {
	   if (matrix[rank][k] < min && matrix[rank][k] != 0) {
	      min = matrix[rank][k]; 		// salvo valore
	      indexR = rank;			// salvo riga
	      indexC = k;			// salvo colonna	      	
	   }
  }
  
  mr[indexR][indexC] = min; //aggiorno la matrice risultato
  printf(" *** [%d] %.2f (%d,%d)\n ", rank, min, rank, indexC); // print test
  
  /* SYNC */
  MPI_Barrier(MPI_COMM_WORLD);
  
  /* COMUNICAZIONE MINIMO */
  
  // PROBLEMA!!!
  
  float buffer = mr[indexR][indexC];
  int col = indexC;
  int raw = indexR;
  int count = 1;
  
  /* SYNC */
  MPI_Barrier(MPI_COMM_WORLD);
  
  printf("[%d] buffer: %.2f\n", rank, buffer);
  printf("[%d] raw: %d\n", rank, raw);
  printf("[%d] col: %d\n", rank, col);
  
  /* SYNC */
  MPI_Barrier(MPI_COMM_WORLD);
  
  // <--- prima di questo punto i buffer, raw e col sono corretti per tutti
  
  MPI_Bcast(&buffer, count, MPI_FLOAT, 0, MPI_COMM_WORLD);  
  MPI_Bcast(&col, count, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&raw, count, MPI_INT, 0, MPI_COMM_WORLD);
  
  mr[raw][col] = buffer;
  
  printf("=== Buffer [%d]: %.2f\n", rank, buffer);
  
  /* SYNC */
  MPI_Barrier(MPI_COMM_WORLD);
  
  MPI_Bcast(&buffer, count, MPI_FLOAT, 1, MPI_COMM_WORLD);  
  MPI_Bcast(&col, count, MPI_INT, 1, MPI_COMM_WORLD);
  MPI_Bcast(&raw, count, MPI_INT, 1, MPI_COMM_WORLD);
  
  mr[raw][col] = buffer;
  
  printf("=== Buffer [%d]: %.2f\n", rank, buffer);
  
  /* SYNC */
  MPI_Barrier(MPI_COMM_WORLD);
  
  // print test
  if (rank == 2) {
      printf(" *** MATRICE RISULTATO ***\n");
         for(i = 0; i < n; i++) {
	  	      for(j = 0; j < n; j++) {
	  		      printf("%.2f ", mr[i][j]);
	         }
	         printf("\n");
	      }
  }

  MPI_Finalize();

} //chiude il main



