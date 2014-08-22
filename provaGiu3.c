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
  
  /* 2a FASE */
  
  int array[n];
  int indexArray = 1;
  int* pointer; 
  
  // inizializzazione array
  for(j = 0; j < n; j++)
      array[j] = -1;
  
  array[0] = rank;
  pointer = &array[1];
  
  //printf("[%d] pointer: %d\n", rank, pointer);
  //printf("[%d] *pointer: %d\n", rank, *pointer);
  //printf("[%d] array: %d\n", rank, &array[0]);
  
  for(j = 0; j < n; j++) { // scansione riga alla ricerca di pesi min
      if(mr[rank][j] != 0) {
         array[indexArray] = j; // inserimento colonna dei pesi minimi in array
         indexArray++;
      }    
  }
  
  // printf("[%d] *pointer: %d\n", rank, *pointer);
  
  /* SYNC */
  MPI_Barrier(MPI_COMM_WORLD);
  
  int control = 0;
  
  while(*pointer != -1) { 
      for(j = 0; j < n; j++) { // scansione sulle colonne
         if(mr[*pointer][j] != 0) { // nuovi elementi
            for(i = 0; i < n; i++) {
               if(array[i] == j) { // controllo se c'e' gia' nell'array
                  control = 1;
                  break;
               }
            }
            
            if(control == 0) {
               array[indexArray] = j;
               indexArray++;
            }
                        
         }
      }
      pointer++;
  }
  
   printf("[%d] array: ", rank);
      for(i = 0; i < n; i++) {
         printf("%d ", array[i]);
      }
   printf("\n");
   
   /* COMUNICAZIONE E CREAZIONE SUPERNODO */
   
   //int rankDest;
   int buf;
   int tmp[n];
   int v, u;
   control = 0;
   
   for(i = 0; i < n; i++) { // copio array in tmp per invio
      tmp[i] = array[i];
   }
   
   //printf("--- [%d] tmp: ", rank); // tmp diag
   //   for(i = 0; i < n; i++) {
   //      printf("%d ", tmp[i]);
   //   }
   //printf("\n");
   
         for(v = 0; v < n; v++) {
            //printf("=== index: %d\n", v);
            
            buf = array[v]; // prendo elemento array
            
            //printf("### [%d] buf: %d\n", rank, buf);
            
            MPI_Bcast(&buf, count, MPI_INT, 5, MPI_COMM_WORLD); // invio a tutti
              
            /* SYNC */
            MPI_Barrier(MPI_COMM_WORLD);
            
            printf("### B%d [%d] buf: %d\n", v, rank, buf);
            
            for(k = 0; k < n; k++) { // array destinatario
               if(array[k] == buf && array[k] != -1) { // se ho trovato l'elemento del mittente
                  control = 1;
                  
                  MPI_Bcast(&tmp, count, MPI_INT, 5, MPI_COMM_WORLD); // invio tutto l'array
                  
                  //printf("--- B [%d] tmp: ", rank);
                  //for(u = 0; u < n; u++) {
                  //   printf("%d ", tmp[u]);
                  //}
                  //printf("\n");
               }
               if(control == 1) {
                  break;
                  //i = n;
               }
            }
            
            /* SYNC */
            MPI_Barrier(MPI_COMM_WORLD);
            
            printf("[%d] B array: ", rank);
            for(i = 0; i < n; i++) {
               printf("%d ", array[i]);
            }
            printf("\n");
            
            /* SYNC */
            MPI_Barrier(MPI_COMM_WORLD);
            
            if(control == 1) {
            int found;
            
               for(i = 0; i < n; i++) {
                  found = 0;
                  for(j = 0; j < n; j++) {
                     if(tmp[i] == array[j]) {
                        found = 1;
                     }
                     if (found == 1) {
                        j = n;
                     }
                  }
                  if (found == 0) {
                     array[indexArray] = tmp[i];
                     indexArray++;
                  }
               }
            }
            control = 0;
         }
   
   /* SYNC */
   //MPI_Barrier(MPI_COMM_WORLD);
   
   //printf("[%d] B array: ", rank);
   //   for(i = 0; i < n; i++) {
   //      printf("%d ", array[i]);
   //   }
   //printf("\n");
   
  
  MPI_Finalize();

} //chiude il main



