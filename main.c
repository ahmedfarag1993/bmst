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
      mr[col][row] = mr[row][col];
  
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
  
  int array[n+1];
  int indexArray = 1;
  int* pointer; 
  
  // inizializzazione array
  for(j = 0; j < n+1; j++)
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
         controlA = 0;     
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
   
   /* FINE CREAZIONE SUPERNODO */

 /* ALGORITMO PER TROVARE IL MIN SULLE RIGHE */
  // ogni processore legge la riga=rank di matrix e scrive il valore min in mr
  
  //indexR = 0; // indice di riga
  //indexC = 0; // indice di colonna
  //float min;
  int g;
  int root = array[0];
  controlA = 0;  

//ogni nodo conosce il suo root
  for (g = 1; g < n; g++) {
      if (array[g] < root && array[g] != -1) {
         root = array[g];
      }
  }
  
  /* SYNC */
  MPI_Barrier(MPI_COMM_WORLD);

  printf("[%d] root: %d\n", rank, root);
  
  for (k = 0; k < n; k++) {
       for (g = 0; g < n; g++) {
         if (array[g] == k) {
            controlA = 1;
         }
       }
       if (controlA == 0) {
          if (matrix[rank][k] != 0) {
      		min = matrix[rank][k];
      		indexR = rank; // salvo riga
	         indexC = k; // salvo colonna
      		break;
      	 }
       }
       controlA = 0;     
  }

  for (k = 0; k < n; k++) {
      for (g = 0; g < n; g++) {
         if (array[g] == k) {
            controlA = 1;
         }
       }
      if (controlA == 0) {
	      if (matrix[rank][k] < min && matrix[rank][k] != 0) {
	         min = matrix[rank][k];  // salvo valore
	         indexR = rank; // salvo riga
	         indexC = k; // salvo colonna	      	
	      }
      }
      controlA = 0;
  }
    
  //mr[indexR][indexC] = min; //aggiorno la matrice risultato
  
  /* SYNC */
  MPI_Barrier(MPI_COMM_WORLD);
  
  printf(" *** [%d] %.2f (%d,%d)\n ", rank, min, rank, indexC); // print test
  
  /* FINE ALGORITMO */
  
  float sendMin = min;
  int sendIndexR = indexR;
  int sendIndexC = indexC;
   
  float sendMinArray[n+1];
  int sendIndexRArray[n+1];
  int sendIndexCArray[n+1];
  
  int h;
  
  for(g = 0; g < n+1; g++) {
      sendMinArray[g] = -1;
      sendIndexRArray[g] = -1;
      sendIndexCArray[g] = -1;
  }
  
  g = 0;
  h = 0;
  
  /* SYNC */
  MPI_Barrier(MPI_COMM_WORLD);
  
  MPI_Send(&sendMin, count, MPI_FLOAT, root, 0, MPI_COMM_WORLD);
  //printf(" >>> [%d] sendMin: %.2f\n", rank, sendMin);
  if(rank == root) {
      while(array[g] != -1) {
         MPI_Recv(&sendMin, 1, MPI_FLOAT, array[g], 0, MPI_COMM_WORLD, &status);
         sendMinArray[h] = sendMin;         
         //printf(" <<<<< [%d] sendMin: %.2f, from: %d\n", rank, sendMin, array[g]);
         g++;
         h++;
      }
      
      printf("[%d] sendMinArray: ", rank);
      for(i = 0; i < n; i++) {
         printf("%.2f ", sendMinArray[i]);
      }
      printf("\n");
      
  }
  
  /* SYNC */
  MPI_Barrier(MPI_COMM_WORLD);
  
  g = 0;
  h = 0;
  MPI_Send(&sendIndexR, count, MPI_INT, root, 0, MPI_COMM_WORLD);
  //printf(" >>> [%d] sendIndexR: %d\n", rank, sendIndexR);
  if(rank == root) {
      while(array[g] != -1) {
         MPI_Recv(&sendIndexR, 1, MPI_INT, array[g], 0, MPI_COMM_WORLD, &status);
         sendIndexRArray[h] = sendIndexR;
         //printf(" <<<<< [%d] sendIndexR: %d, from: %d\n", rank, sendIndexR, array[g]);
         g++;
         h++;
      }
      
      printf("[%d] sendIndexRArray: ", rank);
      for(i = 0; i < n; i++) {
         printf("%d ", sendIndexRArray[i]);
      }
      printf("\n");
      
  }
  
  /* SYNC */
  MPI_Barrier(MPI_COMM_WORLD);
  
  g = 0;
  h = 0;
  MPI_Send(&sendIndexC, count, MPI_INT, root, 0, MPI_COMM_WORLD);
  //printf(" >>> [%d] sendIndexC: %d\n", rank, sendIndexC);
  if(rank == root) {
      while(array[g] != -1) {
         MPI_Recv(&sendIndexC, 1, MPI_INT, array[g], 0, MPI_COMM_WORLD, &status);
         sendIndexCArray[h] = sendIndexC;
         //printf(" <<<<< [%d] sendIndexC: %d, from: %d\n", rank, sendIndexC, array[g]);
         g++;
         h++;
      }
      
      printf("[%d] sendIndexCArray: ", rank);
      for(i = 0; i < n; i++) {
         printf("%d ", sendIndexCArray[i]);
      }
      printf("\n");
      
  }
  
  /* SYNC */
  MPI_Barrier(MPI_COMM_WORLD);
  
  int kmin = -1;
  k = 0;
  int y, t;
  int controlB;
  controlB = 0;
  float max = 0;

   for (y = 0; y < n; y++) {
      if (sendMinArray[y] > max) {
         for (t = 0; t < n; t++) { //di questo max controlla che il sendIndexCArray non appartenga al suo supernodo
            if (sendIndexCArray[y] == array[t]) {
               controlB = 1; //se è del suo supernodo
            }
         }
         if (controlB == 0) {
            max = sendMinArray[y];
            indexR = sendIndexRArray[y];
            indexC = sendIndexCArray[y];
         }
      }
      controlB = 0;
   }
 
   controlB = 0;

  for(k = 1; k < n; k++) {
      if(sendMinArray[k] < max) {
         for (t = 0; t < n; t++) { //di questo controlla che non appartenga al suo supernodo
            if (sendIndexCArray[k] == array[t]) {
               controlB = 1; //se è del suo supernodo
            }
         }
         if (controlB == 0) {
            max = sendMinArray[k]; //se non è del suo supernodo ok mi va bene
            kmin = k;
         }
      }
   controlB=0;
  }
  
  if(kmin != -1) {
      indexR = sendIndexRArray[kmin];
      indexC = sendIndexCArray[kmin];
  }
  
  mr[indexR][indexC] = max;
  mr[indexC][indexR] = mr[indexR][indexC];
  
  //if(rank == root) {
  //    printf(" ----- [%d] sendMin: %.2f, sendIndexR: %d, sendIndexC: %d\n", rank, sendMin, sendIndexR, sendIndexC);
  //}
  
  /* SYNC */
  MPI_Barrier(MPI_COMM_WORLD);
  
  // print test
  if (rank == 2) {
      printf(" *** MATRICE RISULTATO [%d] ***\n", rank);
         for(i = 0; i < n; i++) {
	  	      for(j = 0; j < n; j++) {
	  		      printf("%.2f ", mr[i][j]);
	         }
	         printf("\n");
	      }
  }


  MPI_Finalize();

} //chiude il main



