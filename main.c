#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

/* LEGGE E MEMORIZZA IL FILE CON IL GRAFO SU UNA MATRICE */

  FILE *file;
  int n = 7;
  float matrix[n][n]; // matrice input
  float mr[n][n]; // matrice risultato

  file = fopen("./graph-gen/graph2.txt", "r"); // apre il file
  if (file == NULL) {
      perror("Errore in apertura del file");
      exit(1);
  }

  while (!feof(file)) {	 // legge e memorizza i valori in una matrice (array bidimensionale)
      int i = 0;
  	   int j = 0;
      for (i = 0; i < n; i++) {
  		   for (j = 0; j < n; j++) {
  			   fscanf(file, "%f", &matrix[i][j]);
		   }
	   }
  }
  
  fclose(file); // chiude il file
  
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
	         	printf("%.2f ", matrix[i][j]);
	   	   }
		      printf("\n");
	      }
  }
  
  /* SYNC */
  MPI_Barrier(MPI_COMM_WORLD);
  
  /* ALGORITMO PER TROVARE IL MIN SULLE RIGHE */
  // ogni processore legge la riga=rank di matrix e scrive il valore min in mr
  
  int indexR = 0; // indice di riga
  int indexC = 0; // indice di colonna
  float min;
  int k;
  
  for (k = 0; k < n; k++) {
      if (matrix[rank][k] != 0) {
   		min = matrix[rank][k];
   		indexR = rank; // salvo riga
	      indexC = k; // salvo colonna
   		break;
   	}
  }

  for (k = 0; k < n; k++) {
	   if (matrix[rank][k] < min && matrix[rank][k] != 0) {
	      min = matrix[rank][k];  // salvo valore
	      indexR = rank; // salvo riga
	      indexC = k; // salvo colonna	      	
	   }
  }
  
  mr[indexR][indexC] = min; //aggiorno la matrice risultato
  printf(" *** [%d] %.2f (%d,%d)\n ", rank, min, rank, indexC); // print test
  
  /* FINE ALGORITMO */
  
  /* SYNC */
  MPI_Barrier(MPI_COMM_WORLD);
  
  /* COMUNICAZIONE */
  
  float buffer;
  int col;
  int row;
  int count = 1;
  
  int rankBcast;
  
  for(rankBcast = 0; rankBcast < n; rankBcast++) {
  
      buffer = mr[indexR][indexC];
      col = indexC;
      row = indexR;
  
      MPI_Bcast(&buffer, count, MPI_FLOAT, rankBcast, MPI_COMM_WORLD);  
      MPI_Bcast(&col, count, MPI_INT, rankBcast, MPI_COMM_WORLD);
      MPI_Bcast(&row, count, MPI_INT, rankBcast, MPI_COMM_WORLD);
  
      mr[row][col] = buffer;
  
      //printf("=== Buffer [%d]: %.2f\n", rank, buffer);
  
      /* SYNC */
      MPI_Barrier(MPI_COMM_WORLD);
  }
  
  /* FINE COMUNICAZIONE */
  
  // print test
  if (rank == 0) {
      printf(" *** MATRICE RISULTATO [%d] ***\n", rank);
         for(i = 0; i < n; i++) {
	  	      for(j = 0; j < n; j++) {
	  		      printf("%.2f ", mr[i][j]);
	         }
	         printf("\n");
	      }
  }
  
  /* SYNC */
  MPI_Barrier(MPI_COMM_WORLD);
  
  /* 2a FASE: CREAZIONE SUPERNODO */
  
  int array[n];
  int indexArray = 1;
  int* pointer; 
  
  // inizializzazione array
  for(j = 0; j < n; j++)
      array[j] = -1;
  
  array[0] = rank;
  pointer = &array[0];
  int w, x;
  int controlA = 0;
  
  //printf("[%d] pointer: %d\n", rank, pointer);
  //printf("[%d] *pointer: %d\n", rank, *pointer);
  //printf("[%d] array: %d\n", rank, &array[0]);

   while(*pointer != -1) {  
      for(j = 0; j < n; j++) { // scansione riga alla ricerca di pesi min
         if(mr[*pointer][j] != 0) {        //scansione righe
            for (w = 0; w < n; w++) {
               if (array[w] == j) {
                  controlA = 1;
                  break;
               }
            }
            if (controlA == 0) {
               array[indexArray] = j; // inserimento colonna dei pesi minimi in array
               indexArray++;
            }
         }  

         controlA = 0;

         if(mr[j][*pointer] != 0) {      //scansione colonna alla ricerca di pesi min
            for ( x = 0; x < n; x++) {
               if (array[x] == j) {
                  controlA = 1;
                  break;
               }
            }
            if (controlA == 0) {
               array[indexArray] = j; // inserimento riga 
               indexArray++;
            }
         }     
      }
      pointer++;
   }
  // printf("[%d] *pointer: %d\n", rank, *pointer);
  
  /* SYNC */
  MPI_Barrier(MPI_COMM_WORLD);
  
  printf("[%d] array: ", rank);
      for(i = 0; i < n; i++) {
         printf("%d ", array[i]);
      }
  printf("\n");
   
   /* COMUNICAZIONE E CREAZIONE SUPERNODO */

  MPI_Finalize();

} //chiude il main



