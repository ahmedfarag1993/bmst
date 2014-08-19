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


/*FINE OPERAZIONI DA FILE ESTERNI*/


  int rank, size, buf;
  int tag=0;
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  buf=0;

  float mr[n][n]; //matrice risultato
  
  int i=0;
  int j=0;

  if (rank==0) {
	  for (i=0; i<n; i++) {
	  	for (j=0; j<n; j++) {
	  	printf("%.2f ", matrix[i][j]); //lo stampa come controllo GIUSTO
	   	}
		printf("\n");
	  }
  }

  if (size >= n) { //ho più processori o pari al numero di nodi iniziali
  //ogni processore legge la riga=rank di matrix e scrive il valore min in mr

  float *min;
  int k=0;
  int indexR=0; //indice di riga
  int indexC=0; //indice di colonna
  //cerco il primo valore per ogni riga non nullo PROBLEMA DELLO ZERO!!!

  for (k=0; k<n; k++) {
	if (matrix[rank][k]!=0) {
		min=&matrix[rank][k];
		break;
	}
  }

  for (k=0; k<n; k++) {
	if (matrix[rank][k]<*min && matrix[rank][k]!=0) {
	min=&matrix[rank][k]; 		//salvo valore
	indexR=rank;			//salvo riga
	indexC=k;			//salvo colonna
	mr[indexR][indexC]=*min;	//aggiorno la matrice risultato
	}
 }
  printf("io sono %d e ho trovato %.2f in posizione %d\n ", rank, *min, indexC);

  	MPI_Send(&buf, 1, MPI_INT, indexC, tag, MPI_COMM_WORLD);
	printf("%d has sent %d to %d\n", rank, buf, indexC);

  } //chiude (if (size >= n))

  if (rank==0) {
	  for (i=0; i<n; i++) {
	  	for (j=0; j<n; j++) {
	  		printf("%.2f ", mr[i][j]); //lo stampa come controllo SBAGLIATO
	   	}
		printf("\n");
	  }
  }

  MPI_Finalize();

} //chiude il main


