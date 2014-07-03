package graph;

import java.text.DecimalFormat;
import java.util.Random;
import java.util.Scanner;

public class Main {

	static int numVert;
	static double[][] adjMatrix;

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		Scanner in = new Scanner(System.in);
		Random rand = new Random();

		System.out.println("*** Graph Generator v 1.0 ***");
		System.out.println();

		System.out.print("Insert # vertices: ");
		numVert = in.nextInt();

		in.close();

		adjMatrix = new double[numVert][numVert];

		for (int i = 0; i < numVert; i++) {
			for (int j = 0; j < numVert; j++) {
				adjMatrix[i][j] = rand.nextDouble() * 10;
				int n = (int) (adjMatrix[i][j] * 100);
				adjMatrix[i][j] = (double) n / 100;
			}
		}

		print();

	}

	private static void print() {
		for (int i = 0; i < numVert; i++) {
			System.out.print("[" + adjMatrix[i][0]);
			for (int j = 1; j < numVert; j++) {
				System.out.print("\t" + adjMatrix[i][j]);
			}
			System.out.println("]");
		}
	}

}
