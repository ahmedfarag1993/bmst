#include <mpi.h>
#include<stdlib.h>
#include<stdio.h>

int main(int argc, char *argv[]) {

/*LEGGE E MEMORIZZA IL FILE CON IL GRAFO SU UNA MATRICE*/

  FILE *file;
  //float x;
  int n = 10;
  float matrix[n][n];

  file=fopen("./graph-gen/graph.txt", "r"); 	//apre il file
  if( file==NULL ) {
    perror("Errore in apertura del file");
    exit(1);
  }

  while(!feof(file)){		//legge e memorizza i valori in una matrice (array bidimensionale)
	int i=0;
  	int j=0;
  	for (i=0; i<n; i++) {
  		for (j=0; j<n; j++) {
  			fscanf(file, "%f", &matrix[i][j]);
		}
	}
  }
  fclose(file);		//chiude il file

  int i=0;
  int j=0;
  for (i=0; i<n; i++) {
  	for (j=0; j<n; j++) {
  	printf("%.2f ", matrix[i][j]); //lo stampa come controllo GIUSTO
   	}
	printf("\n");
  }

/*FINE OPERAZIONI DA FILE ESTERNI*/


  int rank, size, buf;
  int tag=0;
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  buf=0;

  float mr[n][n]; //matrice risultato


  if (size >= n) { //ho più processori o pari al numero di nodi iniziali
  //ogni processore legge la riga=rank di matrix e scrive il valore min in mr

  float min;
  int indexR=0; //indice di riga
  int indexC=0; //indice di colonna
  //cerco il primo valore per ogni riga non nullo PROBLEMA DELLO ZERO!!!

  for (indexC=0; indexC<n; indexC++) {
	if (matrix[rank][indexC]!=0) {
		min=matrix[rank][indexC];
		break;
	}
  }

  for (indexC=0; indexC<n; indexC++) {
	if (matrix[rank][indexC]<min && matrix[rank][indexC]!=0) {
	min=matrix[rank][indexC]; 	//salvo valore
	indexR=rank;			//salvo riga
	mr[indexR][indexC]=min;		//aggiorno la matrice risultato
	}
 }
  printf("io sono %d e ho trovato %.2f in posizione %d\n ", rank, min, indexC);


  int i=0;
  int j=0;
  for (i=0; i<n; i++) {
  	for (j=0; j<n; j++) {
  	printf("%.2f ", mr[i][j]); //lo stampa come controllo GIUSTO
   	}
	printf("\n");
  }











  
  //buf=rank;
/*
  int j=0;
  for (j=0; j<n; j++) {
	if (mr[j][rank]!=0) {

		MPI_Send(&buf, 1, MPI_INT, indexC, tag, MPI_COMM_WORLD);
  		printf("%d has sent %d to %d\n", rank, buf, indexC);

		MPI_Recv(&buf, 1, MPI_INT, j, tag, MPI_COMM_WORLD, &status);
		printf("%d has received %d from %d\n", rank, buf, j);
	}
  }
*/
/*
  int j,u;
  for (j=0; j<n; j++) {
	for (u=j+1; u<n; u++) {
		if (mr[j][u]!=0) {
			MPI_Send(&buf, 1, MPI_INT, indexC, tag, MPI_COMM_WORLD);
	  		printf("%d has sent %d to %d\n", rank, buf, indexC);
		}
	}	
  }


  for (j=1; j<n; j++) {
	for (u=0; u<=j-1; u++) {
		if (mr[j][u]!=0) {
			MPI_Send(&buf, 1, MPI_INT, indexC, tag, MPI_COMM_WORLD);
	  		printf("%d has sent %d to %d\n", rank, buf, indexC);
		}
	}	
  }

  for (j=0; j<n; j++) {
	for (u=j+1; u<n; u++) {
		if (mr[j][u]!=0) {
			MPI_Recv(&buf, 1, MPI_INT, j, tag, MPI_COMM_WORLD, &status);
			printf("%d has received %d from %d\n", rank, buf, j);
		}
	}	
  }

  for (j=1; j<n; j++) {
	for (u=0; u<=j-1; u++) {
		if (mr[j][u]!=0) {
			MPI_Recv(&buf, 1, MPI_INT, j, tag, MPI_COMM_WORLD, &status);
			printf("%d has received %d from %d\n", rank, buf, j);
		}
	}	
  }
*/






  } //chiude (if (size >= n))

  MPI_Finalize();

} //chiude il main


