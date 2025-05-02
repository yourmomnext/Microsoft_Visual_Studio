#include "mpi.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <locale>  // Добавлено для работы с локалями

using namespace std;

// Глобальные переменные MPI
int ProcNum;
int ProcRank;

// Функция транспонирования матрицы
void Flip(double *B, int Size) {
	double temp;
	for (int i = 0; i < Size; i++) {
		for (int j = i + 1; j < Size; j++) {
			temp = B[i * Size + j];
			B[i * Size + j] = B[j * Size + i];
			B[j * Size + i] = temp;
		}
	}
}

// Основная функция умножения матриц
void MatrixMultiplicationMPI(double *A, double *B, double *C, int Size) {
	int dim = Size;
	double temp;
	MPI_Status Status;

	int ProcPartSize = dim / ProcNum;
	int ProcPartElem = ProcPartSize * dim;

	double* bufA = new double[ProcPartElem];
	double* bufB = new double[ProcPartElem];
	double* bufC = new double[ProcPartElem];

	if (ProcRank == 0) {
		Flip(B, Size);
	}

	MPI_Scatter(A, ProcPartElem, MPI_DOUBLE, bufA, ProcPartElem, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Scatter(B, ProcPartElem, MPI_DOUBLE, bufB, ProcPartElem, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	temp = 0.0;
	for (int i = 0; i < ProcPartSize; i++) {
		for (int j = 0; j < ProcPartSize; j++) {
			for (int k = 0; k < dim; k++) {
				temp += bufA[i * dim + k] * bufB[j * dim + k];
			}
			bufC[i * dim + j + ProcPartSize * ProcRank] = temp;
			temp = 0.0;
		}
	}

	int NextProc, PrevProc;
	for (int p = 1; p < ProcNum; p++) {
		NextProc = ProcRank + 1;
		if (ProcRank == ProcNum - 1) NextProc = 0;
		PrevProc = ProcRank - 1;
		if (ProcRank == 0) PrevProc = ProcNum - 1;

		MPI_Sendrecv_replace(bufB, ProcPartElem, MPI_DOUBLE,
			NextProc, 0, PrevProc, 0, MPI_COMM_WORLD, &Status);

		temp = 0.0;
		for (int i = 0; i < ProcPartSize; i++) {
			for (int j = 0; j < ProcPartSize; j++) {
				for (int k = 0; k < dim; k++) {
					temp += bufA[i * dim + k] * bufB[j * dim + k];
				}
				int ind = (ProcRank - p >= 0) ? ProcRank - p : (ProcNum - p + ProcRank);
				bufC[i * dim + j + ind * ProcPartSize] = temp;
				temp = 0.0;
			}
		}
	}

	MPI_Gather(bufC, ProcPartElem, MPI_DOUBLE, C, ProcPartElem, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	delete[] bufA;
	delete[] bufB;
	delete[] bufC;
}

int main(int argc, char *argv[]) {
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

	ofstream output_file;
	if (ProcRank == 0) {
		output_file.open("results_MPI.csv");
		// Устанавливаем локаль с точкой в качестве десятичного разделителя
		output_file.imbue(locale("C"));
		output_file << "N (Matrix size);Time (sec)\n";
	}

	for (int N = 100; N <= 2000; N += 100) {
		if (N % ProcNum != 0) {
			if (ProcRank == 0) {
				output_file << N << ";SKIPPED (N not divisible by ProcNum)\n";
			}
			continue;
		}

		double *A = nullptr, *B = nullptr, *C = new double[N * N];

		if (ProcRank == 0) {
			A = new double[N * N];
			B = new double[N * N];

			for (int i = 0; i < N; i++) {
				for (int j = 0; j < N; j++) {
					A[i * N + j] = (i + 1) * (j + 1);
					B[i * N + j] = (i + 1) + 2 * (j + 1);
				}
			}
		}

		double start_time = MPI_Wtime();
		MatrixMultiplicationMPI(A, B, C, N);
		double elapsed_time = MPI_Wtime() - start_time;

		if (ProcRank == 0) {
			// Форматируем вывод с фиксированной точкой и 7 знаками после запятой
			output_file << N << ";" << fixed << setprecision(7) << elapsed_time << "\n";
			delete[] A;
			delete[] B;
		}
		delete[] C;
	}

	if (ProcRank == 0) {
		output_file.close();
		cout << "Results saved to results_MPI.csv\n";
	}

	MPI_Finalize();
	return 0;
}