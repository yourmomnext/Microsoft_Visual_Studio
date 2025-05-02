#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>

void matrixMultiply(float* A, float* B, float* C, int N) {
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			C[i*N + j] = 0;
			for (int k = 0; k < N; k++) {
				C[i*N + j] += A[i*N + k] * B[k*N + j];
			}
		}
	}
}

void runBenchmark(const std::vector<int>& matrix_sizes) {
	std::ofstream outFile("sequential_results.csv");
	if (!outFile.is_open()) {
		std::cerr << "Error opening output file!" << std::endl;
		return;
	}

	outFile << std::fixed << std::setprecision(7);
	outFile << "N (Matrix size);Time (seconds)\n";

	for (int N : matrix_sizes) {
		float* A = new float[N*N];
		float* B = new float[N*N];
		float* C = new float[N*N];

		// Инициализация матриц
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				A[i*N + j] = (i + 1) * (j + 1);
				B[i*N + j] = (i + 1) + 2 * (j + 1);
			}
		}

		// Прогрев
		matrixMultiply(A, B, C, N);

		// Замер времени
		auto start = std::chrono::high_resolution_clock::now();
		matrixMultiply(A, B, C, N);
		auto end = std::chrono::high_resolution_clock::now();
		double elapsed = std::chrono::duration<double>(end - start).count();

		// Запись результатов
		outFile << N << ";" << elapsed << "\n";
		std::cout << "N = " << N << ", time = " << elapsed << " sec\n";

		delete[] A;
		delete[] B;
		delete[] C;
	}

	outFile.close();
	std::cout << "\nResults saved to sequential_results.csv\n";
}

int main() {
	std::vector<int> matrix_sizes;
	for (int N = 10; N <= 100; N += 5) {
		matrix_sizes.push_back(N);
	}

	runBenchmark(matrix_sizes);
	return 0;
}