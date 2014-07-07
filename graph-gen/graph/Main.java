package graph;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Random;
import java.util.Scanner;

public class Main {

	public static void main(String[] args) {
		int numVert, n;
		double[][] adjMatrix;
		
		Scanner in = new Scanner(System.in);
		Random rand = new Random();

		// +++ INPUT +++

		System.out.println("*** Graph Generator v 1.0 ***");
		System.out.println();

		System.out.print("Insert # vertices: ");
		numVert = in.nextInt();

		in.close();

		// --- END OF INPUT ---

		// +++ MATRIX CREATION [ADJ -> SYMMETRIC] +++

		adjMatrix = new double[numVert][numVert];
		
		// fill the top-right corner of the matrix
		// with random weights

		for (int i = 0; i < numVert; i++) {
			for (int j = i; j < numVert; j++) {
				if (i != j) {
					// random weight
					adjMatrix[i][j] = rand.nextDouble() * 10;
					n = (int) (adjMatrix[i][j] * 100);
					adjMatrix[i][j] = (double) n / 100;
				} else
					adjMatrix[i][j] = 0; // null weight on diagonal
			}
		}
		
		// copying the top-right corner into the bottom-left corner
		
		for (int j = 0; j < numVert; j++)
			for (int i = j; i < numVert; i++)
					adjMatrix[i][j] = adjMatrix[j][i];

		// --- END OF MATRIX CREATION ---

		// +++ FILE CREATION +++

		File file = new File("./graph.txt");

		// if file doesn't exists, then create it
		if (!file.exists()) {
			try {
				file.createNewFile();
			} catch (IOException e) {
				System.out.println("[CREATE] Error.");
				e.printStackTrace();
			}
		}

		FileWriter fw = null;
		try {
			fw = new FileWriter(file.getAbsoluteFile());
		} catch (IOException e) {
			System.out.println("[BUFFERING] Error.");
			e.printStackTrace();
		}

		if (fw == null) // CHECK
			System.out.println("[NULL] Unknown File.");
		BufferedWriter bw = new BufferedWriter(fw);

		for (int i = 0; i < numVert; i++) {
			for (int j = 0; j < numVert - 1; j++) {
				try {
					bw.write(adjMatrix[i][j] + "\t");
				} catch (IOException e) {
					System.out.println("[WRITE] Error.");
					e.printStackTrace();
				}
			}
			try {
				bw.write("" + adjMatrix[i][numVert - 1] + "\n");
			} catch (IOException e) {
				System.out.println("[WRITE] Error.");
				e.printStackTrace();
			}
		}
		try {
			bw.close();
		} catch (IOException e) {
			System.out.println("[CLOSE] Error.");
			e.printStackTrace();
		}

		System.out.println("[DONE] Matrix written in " + file);

		// --- END OF FILE CREATION ---

	}
}
