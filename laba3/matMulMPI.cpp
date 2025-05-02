#include "mpi.h"
#include <cstdlib>
#include <iostream>
#include "time.h"

using namespace std;

// MPI routines
int ProcNum, ProcRank;

//————————————————-
void Flip (double *B, int Size) 
{ 
	double temp=0.0; 
	for (int i=0;i<Size;i++)
	{
		for (int j=i+1;j<Size;j++)
		{
			temp = B[i*Size+j];
			B[i*Size+j] = B[j*Size+i];
			B[j*Size+i] = temp;
		}
	}
}

void MatrixMultiplicationMPI(double *A, double *B, double *C, int Size) 
{
	int dim = Size;
	int i, j, k, p, ind;
	double temp;
	MPI_Status Status;
	int ProcPartSize = dim/ProcNum; 
	int ProcPartElem = ProcPartSize*dim; 
	double* bufA = new double[ProcPartElem];
	double* bufB = new double[ProcPartElem];
	double* bufC = new double[ProcPartElem];
	int ProcPart = dim/ProcNum, part = ProcPart*dim;
	if (ProcRank == 0) {
		Flip(B, Size);
	}
	
	MPI_Scatter(A, part, MPI_DOUBLE, bufA, part, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Scatter(B, part, MPI_DOUBLE, bufB, part, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	
	temp = 0.0;
	for (i=0; i < ProcPartSize; i++) {
		for (j=0; j < ProcPartSize; j++) {
			for (k=0; k < dim; k++) 
				temp += bufA[i*dim+k]*bufB[j*dim+k];
			bufC[i*dim+j+ProcPartSize*ProcRank] = temp;
			temp = 0.0;
		}
	}

	int NextProc; int PrevProc;
	for (p=1; p < ProcNum; p++) {
		NextProc = ProcRank+1;
		if (ProcRank == ProcNum-1) 
			NextProc = 0;
		PrevProc = ProcRank-1;
		if (ProcRank == 0) 
			PrevProc = ProcNum-1;
		MPI_Sendrecv_replace(bufB, part, MPI_DOUBLE, NextProc, 0, PrevProc, 0, MPI_COMM_WORLD, &Status);
		temp = 0.0;
		for (i=0; i < ProcPartSize; i++) {
			for (j=0; j < ProcPartSize; j++) {
				for (k=0; k < dim; k++) {
					temp += bufA[i*dim+k]*bufB[j*dim+k];
				}
				if (ProcRank-p >= 0 ) 
					ind = ProcRank-p;
				else ind = (ProcNum-p+ProcRank);
				bufC[i*dim+j+ind*ProcPartSize] = temp;
				temp = 0.0;
			}
		}
	}
	
	MPI_Gather(bufC, ProcPartElem, MPI_DOUBLE, C, ProcPartElem, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	delete []bufA;
	delete []bufB;
	delete []bufC;
	
	MPI_Finalize();
}

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
    
    double *A, *B;
    
    // Allocating memory for 2 initial matrices
    A = new double[N*N];
    B = new double[N*N];    
    
    // initializing matrix A and B with random numbers
    for (int i=0; i < N; i++)
	for (int j = 0; j < N; j++)
	{
	    A[i*N + j] = (i+1) * (j+1);
	    B[i*N + j] = (i+1) + 2*(j+1);
	}
	
    //matrixOutput<double>(A, N, "A");
    //matrixOutput<double>(B, N, "B");    
    
    double *C = new double[N*N];
    
    cout << "Begin calculating ..." << endl;
    
	start = clock();
    
	// Begin MPI
	MPI_Init(&argc, &argv);
	
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
	//MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	MatrixMultiplicationMPI(A, B, C, N);
	    
    cout << endl << "Calculation time: " << double(clock() - start)/CLOCKS_PER_SEC << " seconds" << endl;
	
/*	// Sequantial matrix multiplication
    for (int i=0; i < N; i++)
	for (int j = 0; j < N; j++)
        {
	    C[i*N + j] = 0;
	    for (int k = 0; k < N; k++)
	    {
		C[i*N + j] += A[i*N + k] * B[k*N + j];
	    }
        }
 */  
    //matrixOutput<double>(C, N, "resulting C");
    
    // free memory
    delete [] A;
    delete [] B;
    delete [] C;
    
    return 0;
}
