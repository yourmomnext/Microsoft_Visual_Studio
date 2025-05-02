#include <cstdlib>
#include <iostream>
#include "time.h"

using namespace std;


// Matrix output
template <typename T> int matrixOutput(T *Mat, int size, string name)
{
    cout << "\"" << name << "\" matrix output:" << endl;
    for (int i=0; i < size; i++)
    {
	for (int j = 0; j < size; j++)
	{
	    cout << Mat[i*size + j] << " ";
	}
	cout << endl;
    }
    
    return 1;
}

int main(int argc, char *argv[])
{
    clock_t start;
    if (argc != 2)
    {
	cout << "Program usage: " << endl <<"./" << argv[0] << " <n>" << endl << "where <n> is the size of square matrix" << endl;
	return -1;
    }
    
    const int N = atoi(argv[1]);
    
    cout << "Begin initializing ..." << endl;
    
    float *A, *B;
    
    // Allocating memory for 2 initial matrices
    A = new float[N*N];
    B = new float[N*N];
    
    // initializing matrix A and B with random numbers
    for (int i=0; i < N; i++)
	for (int j = 0; j < N; j++)
	{
	    A[i*N + j] = (i+1) * (j+1);
	    B[i*N + j] = (i+1) + 2*(j+1);
	}
	
    //matrixOutput<float>(A, N, "A");
    //matrixOutput<float>(B, N, "B");    
    
    float *C = new float[N*N];
    
    cout << "Begin calculating ..." << endl;
    
    start = clock();
    
    // Sequantial matrix multiplication
    for (int i=0; i < N; i++)                       
	for (int j = 0; j < N; j++)			
        {
	    C[i*N + j] = 0;
	    for (int k = 0; k < N; k++)
	    {
		C[i*N + j] += A[i*N + k] * B[k*N + j];
	    }
        }
    
    cout << endl << "Calculation time: " << double(clock() - start)/CLOCKS_PER_SEC << " seconds" << endl;
    
    //matrixOutput<float>(C, N, "resulting C");
    
    // free memory
    delete [] A;
    delete [] B;
    delete [] C;
    
    return 0;
}