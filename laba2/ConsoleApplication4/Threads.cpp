#include <iostream>
#include <fstream>
#include <vector>
#include <omp.h>
#include <iomanip>

void matrixMultiply(float* A, float* B, float* C, int N) {
#pragma omp parallel for
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			C[i*N + j] = 0;
			for (int k = 0; k < N; k++) {
				C[i*N + j] += A[i*N + k] * B[k*N + j];
			}
		}
	}
}

void runBenchmark(const std::vector<int>& matrix_sizes, const std::vector<int>& threads_list) {
	// Открываем файл для записи результатов
	std::ofstream outFile("results_Threads.csv");
	if (!outFile.is_open()) {
		std::cerr << "Error opening output file!" << std::endl;
		return;
	}

	// Настройка формата вывода
	outFile << std::fixed << std::setprecision(7);

	// Записываем заголовок с разделителем-точкой с запятой
	outFile << "N (Matrix size)";
	for (int threads : threads_list) {
		outFile << ";" << threads << " threads";  // Используем точку с запятой
	}
	outFile << "\n";

	// Запускаем тесты для каждого размера матрицы
	for (int N : matrix_sizes) {
		outFile << N;

		// Для каждого количества потоков
		for (int threads : threads_list) {
			// Выделение памяти
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

			// Установка числа потоков
			omp_set_num_threads(threads);

			// Прогрев
			matrixMultiply(A, B, C, N);

			// Замер времени
			double start = omp_get_wtime();
			matrixMultiply(A, B, C, N);
			double elapsed = omp_get_wtime() - start;

			// Запись результата с разделителем-точкой с запятой
			outFile << ";" << elapsed;

			// Освобождение памяти
			delete[] A;
			delete[] B;
			delete[] C;

			std::cout << "N = " << N << ", threads = " << threads
				<< ", time = " << elapsed << " sec\n";
		}
		outFile << "\n";
	}

	outFile.close();
	std::cout << "\nResults saved to results_Threads.csv\n";
	std::cout << "NOTE: When opening in Excel, select semicolon (;) as delimiter\n";
}

int main() {
	// Параметры тестирования
	std::vector<int> matrix_sizes;
	for (int N = 1; N <= 100; N += 1) {
		matrix_sizes.push_back(N);
	}
	std::vector<int> threads_list = { 1, 4, 5, 8 };

	// Запуск тестов
	runBenchmark(matrix_sizes, threads_list);

	return 0;
}