#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

    /* LEGGE E MEMORIZZA IL FILE CON IL GRAFO SU UNA MATRICE */

    //printf("argv[1]: %s\n" , argv[1]);
    //printf("argv[2]: %s\n" , argv[2]);

    FILE *file;
    int n = 12;
    //int n = (int)argv[1];
    float matrix[n][n]; // matrice input
    float mr[n][n]; // matrice risultato

    file = fopen("./graph-gen/graph3.txt", "r"); // apre il file
    //file = fopen("argv[2]", "r"); // apre il file
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

    int ratio = n/size;

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
    // ogni processore legge le righe=i*size+rank di matrix e scrive i valori min in mr

    int indexR[ratio]; // vettore di indice di riga
    int indexC[ratio]; // vettore di indice di colonna
    float min[ratio];
    int k;

    for(j = 0; j < ratio; j++) { // inizializzazione vettori di riga, colonna e min
        indexR[j] = -1;
        indexC[j] = -1;
        min[j] = -1;
    }

    for(i = 0; i < ratio; i++) {

        for (k = 0; k < n; k++) {
            if (matrix[i*size + rank][k] != 0) {
                min[i] = matrix[i*size + rank][k];
                indexR[i] = i*size + rank; // salvo riga
                indexC[i] = k; // salvo colonna
                break;
            }
        }

        for (k = 0; k < n; k++) {
            if (matrix[i*size + rank][k] < min[i] && matrix[i*size + rank][k] != 0) {
                min[i] = matrix[i*size + rank][k];  // salvo valore
                indexR[i] = i*size + rank; // salvo riga
                indexC[i] = k; // salvo colonna
            }
        }

        mr[indexR[i]][indexC[i]] = min[i]; //aggiorno la matrice risultato
        printf(" *** [%d] %.2f (%d,%d)\n ", i*size + rank, min[i], i*size + rank, indexC[i]); // print test

    }

    /* FINE ALGORITMO */

    /* SYNC */
    MPI_Barrier(MPI_COMM_WORLD);

    /* COMUNICAZIONE */

    float buffer;
    int col;
    int row;
    int count = 1;

    int rankBcast;

    for(rankBcast = 0; rankBcast < size; rankBcast++) {

        for(i = 0; i < ratio; i++) {

            buffer = mr[indexR[i]][indexC[i]];
            col = indexC[i];
            row = indexR[i];

            MPI_Bcast(&buffer, count, MPI_FLOAT, rankBcast, MPI_COMM_WORLD);
            MPI_Bcast(&col, count, MPI_INT, rankBcast, MPI_COMM_WORLD);
            MPI_Bcast(&row, count, MPI_INT, rankBcast, MPI_COMM_WORLD);

            mr[row][col] = buffer;
            mr[col][row] = mr[row][col];

        }

        //printf("=== Buffer [%d]: %.2f\n", rank, buffer);

        /* SYNC */
        MPI_Barrier(MPI_COMM_WORLD);
    }

    /* FINE COMUNICAZIONE */

    if (rank == 0) {
        printf(" *** MATRICE RISULTATO [%d] ***\n", rank);
        for(i = 0; i < n; i++) {
            for(j = 0; j < n; j++) {
                printf("%.2f ", mr[i][j]);
            }
            printf("\n");
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 1) {
        printf(" *** MATRICE RISULTATO [%d] ***\n", rank);
        for(i = 0; i < n; i++) {
            for(j = 0; j < n; j++) {
                printf("%.2f ", mr[i][j]);
            }
            printf("\n");
        }
    }

    /* 2a FASE: CREAZIONE SUPERNODO */

    int array[ratio][n+1];
    int indexArray[ratio];
    int* pointer[ratio];

    int w, x;
    int controlA;
    int counter;

    // inizializzazione array
    for(i = 0; i < ratio; i++) {
        for(j = 1; j < n+1; j++) {
            array[i][j] = -1;
        }
    }

    for(;;) { // ATTENZIONE!

        counter = 0;

        for(j = 0; j < n; j++) { // se ho gli array pieni, sono a posto
            if(array[0][j] != -1)
                counter++;
        }

        if(counter == n)
            break;

        // inizializzazione array
        for(i = 0; i < ratio; i++) {
            for(j = 1; j < n+1; j++) {
                array[i][j] = -1;
            }
            indexArray[i] = 1;
            array[i][0] = i*size + rank;
            pointer[i] = &array[i][0];
        }

        controlA = 0;

        for(i = 0; i < ratio; i++) {

            while(*pointer[i] != -1) {
                for(j = 0; j < n; j++) { // scansione riga alla ricerca di elementi diversi da 0
                    if(mr[*pointer[i]][j] != 0) {        //scansione righe
                        for (w = 0; w < n; w++) {
                            if (array[i][w] == j) {
                                controlA = 1;
                                break;
                            }
                        }
                        if (controlA == 0) {
                            array[i][indexArray[i]] = j; // inserimento colonna dei pesi minimi in array
                            indexArray[i]++;
                        }
                    }

                    controlA = 0;

                    if(mr[j][*pointer[i]] != 0) {      //scansione colonna alla ricerca di pesi min
                        for (x = 0; x < n; x++) {
                            if (array[i][x] == j) {
                                controlA = 1;
                                break;
                            }
                        }
                        if (controlA == 0) {
                            array[i][indexArray[i]] = j; // inserimento riga
                            indexArray[i]++;
                        }
                    }
                    controlA = 0;
                }
                pointer[i]++;
            }
        }

        /* SYNC */
        MPI_Barrier(MPI_COMM_WORLD);

        for(i = 0; i < ratio; i++) {
            printf("[%d][%d] array: ", rank, i*size+rank);
            for(j = 0; j < n; j++) {
                printf("%d ", array[i][j]);
            }
            printf("\n");
        }

        /* FINE CREAZIONE SUPERNODO */

        /* CREAZIONE DELLE ROOT E ALGORITMO PER TROVARE IL MIN */
        // ogni supernodo elegge un rappresentante (root), al quale ogni nodo invia il suo valore min

        int g;
        int root[ratio];
        controlA = 0;

        for(i = 0; i < ratio; i++) {
            root[i] = array[i][0];
        }

        // ogni nodo conosce il suo root
        for(i = 0; i < ratio; i++) {
            for (g = 1; g < n; g++) {
                if (array[i][g] < root[i] && array[i][g] != -1) {
                    root[i] = array[i][g];
                }
            }
        }

        /* SYNC */
        MPI_Barrier(MPI_COMM_WORLD);

        for(i = 0; i < ratio; i++) {
            printf("[%d][%d] root: %d\n", rank, i*size+rank, root[i]);
        }


        for(i = 0; i < ratio; i++) {

            for (k = 0; k < n; k++) {
                for (g = 0; g < n; g++) {
                    if (array[i][g] == k) {
                        controlA = 1;
                    }
                }
                if (controlA == 0) {
                    if (matrix[i*size+rank][k] != 0) {
                        min[i] = matrix[i*size+rank][k];
                        indexR[i] = i*size+rank; // salvo riga
                        indexC[i] = k; // salvo colonna
                        break;
                    }
                }
                controlA = 0;
            }

            for (k = 0; k < n; k++) {
                for (g = 0; g < n; g++) {
                    if (array[i][g] == k) {
                        controlA = 1;
                    }
                }
                if (controlA == 0) {
                    if (matrix[i*size+rank][k] < min[i] && matrix[i*size+rank][k] != 0) {
                        min[i] = matrix[i*size+rank][k];  // salvo valore
                        indexR[i] = i*size+rank; // salvo riga
                        indexC[i] = k; // salvo colonna
                    }
                }
                controlA = 0;
            }

            //mr[indexR][indexC] = min; //aggiorno la matrice risultato
            indexR[i] = i*size+rank; // perche' ognuno guarda la sua riga
        }
        /* SYNC */
        MPI_Barrier(MPI_COMM_WORLD);

        for(i = 0; i < ratio; i++) {
            printf(" *** [%d][%d] %.2f (%d,%d)\n ", rank, i*size+rank, min[i], indexR[i], indexC[i]); // print test
        }
        /* FINE ALGORITMO */

        /* COMUNICAZIONI VALORI MIN ALLE ROOT */

        float sendMin[ratio];
        int sendIndexR[ratio];
        int sendIndexC[ratio];

        for(i = 0; i < ratio; i++) {
            sendMin[i] = min[i];
            sendIndexR[i] = indexR[i];
            sendIndexC[i] = indexC[i];
        }

        float sendMinArray[ratio][n+1];
        int sendIndexRArray[ratio][n+1];
        int sendIndexCArray[ratio][n+1];

        int h;

        for(i = 0; i < ratio; i++) {
            for(g = 0; g < n+1; g++) {
                sendMinArray[i][g] = -1;
                sendIndexRArray[i][g] = -1;
                sendIndexCArray[i][g] = -1;
            }
        }

        h = 0;
        //controlA = 0;

        /* SYNC */
        MPI_Barrier(MPI_COMM_WORLD);

        int len = n - ratio;
        int myRank[ratio];
        int myNodes[ratio]; // contiene i nodi che gestisco
        int myNodesRoot[ratio]; // contiene le radici dei nodi che gestisco
        int otherRank[len]; // contiene i processori che gestiscono tali nodi
        int otherNodes[len]; // contiene i nodi gestiti
        int otherNodesRoot[len]; // contiene le radici dei nodi gestiti
        int m, q, dest;

        for(i = 0; i < ratio; i++) { // per ogni processore, fisso la root e vado alla ricerca del processore che la contiene

            controlA = 0;

            for(k = 0; k < ratio; k++) { // se io gestisco la root, non serve inviare; scrivo direttamente nei miei sendArray
                if(root[i] == array[k][0]) {
                    controlA = 1;
                    for(h = 0; h < n; h++) {
                        if(sendMinArray[k][h] == -1) {
                            sendMinArray[k][h] = sendMin[i];
                            sendIndexRArray[k][h] = sendIndexR[i];
                            sendIndexCArray[k][h] = sendIndexC[i];
                            break;
                        }
                    }
                }
            }

            // =================================================================================

            for(k = 0; k < ratio; k++) { // copio le root in myNodesRoot e i nodi gestiti in myNodes
                myRank[k] = rank;
                myNodes[k] = k*size+rank; // k*size+rank = array[k][0]
                myNodesRoot[k] = root[k];
            }

            // printf(" $$$$$ [%d][%d] root[%d]: %d\n", rank, i, i, root[i]);

            q = 0;
            for(k = 0; k < ratio; k++) {
                for (dest = 0; dest < size; dest++) {

                    for(g = 0; g < ratio; g++) { // reinizializzazione per buffer: copio le root in myNodesRoot e i nodi gestiti in myNodes
                        myRank[g] = rank;
                        myNodes[g] = g*size+rank; // g*size+rank = array[g][0]
                        myNodesRoot[g] = root[g];
                    }

                    if (rank != dest) {
                        MPI_Send(&myRank[k], count, MPI_INT, dest, 0, MPI_COMM_WORLD);
                        MPI_Send(&myNodes[k], count, MPI_INT, dest, 0, MPI_COMM_WORLD);
                        MPI_Send(&myNodesRoot[k], count, MPI_INT, dest, 0, MPI_COMM_WORLD);

                        MPI_Recv(&myRank[k], 1, MPI_INT, dest, 0, MPI_COMM_WORLD, &status);
                        MPI_Recv(&myNodes[k], 1, MPI_INT, dest, 0, MPI_COMM_WORLD, &status);
                        MPI_Recv(&myNodesRoot[k], 1, MPI_INT, dest, 0, MPI_COMM_WORLD, &status);

                        otherRank[q] = myRank[k];
                        otherNodes[q] = myNodes[k];
                        otherNodesRoot[q] = myNodesRoot[k];
                        q++;
                    }
                }
            }

            for(g = 0; g < ratio; g++) { // pulisco i buffer e li reinizializzo per avere i valori corretti
                myRank[g] = rank;
                myNodes[g] = g*size+rank; // g*size+rank = array[g][0]
                myNodesRoot[g] = root[g];
            }

            if(i == ratio - 1) {

                for(k = 0; k < ratio; k++) {
                    printf(" ^^^ [%d][%d] myRank: %d, myNodes: %d, myNodesRoot: %d\n", rank, k, myRank[k], myNodes[k], myNodesRoot[k]);
                }

                for(k = 0; k < len; k++) {
                    printf("[%d][%d] otherRank: %d , otherNodes: %d, otherNodesRoot: %d\n", rank, k, otherRank[k], otherNodes[k], otherNodesRoot[k]);
                }
            }

            MPI_Barrier(MPI_COMM_WORLD); // a questo punto, tutti i processori conoscono tutte le info di tutti

            int toSend = -1; // rank del destinatario
            int recFrom = -1; // rank del mittente
            float recBufMin; // buffer per la ricezione minimo
            int recBufIndexR; // buffer per la ricezione indice di riga
            int recBufIndexC; // buffer per la ricezione indice di colonna

            // questo passo lo devo fare se non trovo la root dentro myNodes
            if(controlA == 0) {
                for(m = 0; m < len; m++) {
                    if(root[i] == otherNodes[m]) {
                        toSend = otherRank[m];
                    }
                }

                if(toSend != -1) {
                    MPI_Send(&sendMin[i], count, MPI_FLOAT, toSend, 0, MPI_COMM_WORLD);
                    printf(">>> Io %d ho inviato %.2f a %d\n", rank, sendMin[i], toSend);

                    MPI_Send(&sendIndexR[i], count, MPI_INT, toSend, 0, MPI_COMM_WORLD);
                    //printf(">>> Io %d ho inviato %.2f a %d\n", rank, sendMin[i], toSend);

                    MPI_Send(&sendIndexC[i], count, MPI_INT, toSend, 0, MPI_COMM_WORLD);
                    //printf(">>> Io %d ho inviato %.2f a %d\n", rank, sendMin[i], toSend);
                    toSend = -1;
                }
            }

            // vado a scansionare otherNodesRoot e controllo se qualcuno appartiene a myNodes. Se si', metto in ricezione di otherRank
            for(m = 0; m < len; m++) {
                for(g = 0; g < ratio; g++) {
                    if(root[i] == otherNodesRoot[m] && otherNodesRoot[m] == myNodes[g]) {
                        recFrom = otherRank[m];
                        printf(" ----- [%d] otherNodesRoot[%d]: %d, myNodes[%d]: %d, recFrom: %d\n", rank, m, otherNodesRoot[m], g, myNodes[g], recFrom);
                    }
                }
            }

            if(recFrom != -1) {
                MPI_Recv(&recBufMin, 1, MPI_FLOAT, recFrom, 0, MPI_COMM_WORLD, &status);
                printf("<<< Io %d ho ricevuto %.2f da %d\n", rank, recBufMin, recFrom);

                MPI_Recv(&recBufIndexR, 1, MPI_INT, recFrom, 0, MPI_COMM_WORLD, &status);

                MPI_Recv(&recBufIndexC, 1, MPI_INT, recFrom, 0, MPI_COMM_WORLD, &status);

                for(k = 0; k < ratio; k++) {
                    if(root[i] == array[k][0]) {
                        for(h = 0; h < n; h++) {
                            if(sendMinArray[k][h] == -1) {
                                sendMinArray[k][h] = recBufMin;
                                sendIndexRArray[k][h] = recBufIndexR;
                                sendIndexCArray[k][h] = recBufIndexC;
                                break;
                            }
                        }
                    }
                }

                recFrom = -1;
            }

            // DOBBIAMO INVIARE INDEXR ED INDEXC ED AGGIORNARE I SENDARRAY


        } // chiude il for

        for(g = 0; g < ratio; g++) {
            printf("[%d][%d] sendMinArray: ", rank, g*size+rank);
            for(j = 0; j < n; j++) {
                printf("%.2f ", sendMinArray[g][j]);
            }
            printf("\n");
        }

        for(g = 0; g < ratio; g++) {
            printf("[%d][%d] sendIndexRArray: ", rank, g*size+rank);
            for(j = 0; j < n; j++) {
                printf("%d ", sendIndexRArray[g][j]);
            }
            printf("\n");
        }

        for(g = 0; g < ratio; g++) {
            printf("[%d][%d] sendIndexCArray: ", rank, g*size+rank);
            for(j = 0; j < n; j++) {
                printf("%d ", sendIndexCArray[g][j]);
            }
            printf("\n");
        }

        /* SYNC */
        MPI_Barrier(MPI_COMM_WORLD);

        k = 0;

        int kmin = -1;
        int y, t;
        int controlB = 0;
        float max;

        for(i = 0; i < ratio; i++) {

            max = sendMinArray[i][0];

            for (y = 0; y < n; y++) {
                if (sendMinArray[i][y] > max) {
                    for (t = 0; t < n; t++) { //di questo max controlla che il sendIndexCArray non appartenga al suo supernodo
                        if (sendIndexCArray[i][y] == array[i][t]) {
                            controlB = 1; //se è del suo supernodo
                        }
                    }
                    if (controlB == 0) {
                        max = sendMinArray[i][y];
                        indexR[i] = sendIndexRArray[i][y];
                        indexC[i] = sendIndexCArray[i][y];
                    }
                }
                controlB = 0;
            }

            controlB = 0;

            for(k = 1; k < n; k++) {
                if(sendMinArray[i][k] < max) {
                    for (t = 0; t < n; t++) { //di questo controlla che non appartenga al suo supernodo
                        if (sendIndexCArray[i][k] == array[i][t]) {
                            controlB = 1; //se è del suo supernodo
                        }
                    }
                    if (controlB == 0) {
                        max = sendMinArray[i][k]; //se non è del suo supernodo ok mi va bene
                        kmin = k;
                    }
                }
                controlB = 0;
            }

            if(kmin != -1) {
                indexR[i] = sendIndexRArray[i][kmin];
                indexC[i] = sendIndexCArray[i][kmin];
            }

            if(max != sendMinArray[i][0]) {
                mr[indexR[i]][indexC[i]] = max;
                mr[indexC[i]][indexR[i]] = mr[indexR[i]][indexC[i]];
            }

            //if(rank == root) {
            printf(" ----- [%d] sendMin[i]: %.2f, sendIndexR[i]: %d, sendIndexC[i]: %d\n", rank, sendMin[i], sendIndexR[i], sendIndexC[i]);
            printf("ggggg [%d] max: %.2f , indexR: %d , indexC: %d\n" , rank, max, indexR[i], indexC[i]);
            //}

            /* SYNC */
            MPI_Barrier(MPI_COMM_WORLD);

        } // chiude il for

        // root print test
        if (rank == 0) { //if (rank == root) {
            printf(" ROOT *** MATRICE RISULTATO [%d] ***\n", rank);
            for(i = 0; i < n; i++) {
                for(j = 0; j < n; j++) {
                    printf("%.2f ", mr[i][j]);
                }
                printf("\n");
            }
        }

        if (rank == 1) { //if (rank == root) {
            printf(" ROOT *** MATRICE RISULTATO [%d] ***\n", rank);
            for(i = 0; i < n; i++) {
                for(j = 0; j < n; j++) {
                    printf("%.2f ", mr[i][j]);
                }
                printf("\n");
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);

        /* BROADCAST a tutti della matrice risultato aggiornata */

        for(rankBcast = 0; rankBcast < size; rankBcast++) {

            for(i = 0; i < ratio; i++) {

                buffer = mr[indexR[i]][indexC[i]];
                col = indexC[i];
                row = indexR[i];

                MPI_Bcast(&buffer, count, MPI_FLOAT, rankBcast, MPI_COMM_WORLD);
                MPI_Bcast(&col, count, MPI_INT, rankBcast, MPI_COMM_WORLD);
                MPI_Bcast(&row, count, MPI_INT, rankBcast, MPI_COMM_WORLD);

                mr[row][col] = buffer;
                mr[col][row] = mr[row][col];

            }

            //printf("=== Buffer [%d]: %.2f\n", rank, buffer);

            /* SYNC */
            MPI_Barrier(MPI_COMM_WORLD);
        }

        /* SYNC */
        MPI_Barrier(MPI_COMM_WORLD);

        printf(" UPDATE *** MATRICE RISULTATO [%d] ***\n", rank);
        for(i = 0; i < n; i++) {
            for(j = 0; j < n; j++) {
                printf("%.2f ", mr[i][j]);
            }
            printf("\n");
        }

    }

    printf("!!!!! [%d] QUIT\n", rank);

    /* SYNC */
    //MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();

} //chiude il main
