#pragma once
#include <cuda_runtime.h>
#include <cusolverDn.h>
#include "CuSolverUtils.h"
#include <Windows.h>

class BEMSolver {
	// handle for cuSolver
	cusolverDnHandle_t cusolverH = NULL;
	cudaStream_t stream = NULL;
	// parameters determining the dimension of the input matrices
	bool configured;
	bool decomposed;
	int numMeshes, numTransducers;
	int N, lda, ldb;
	// host copies
	float* matrixA;
	float* matrixB;
	std::vector<int> Ipiv;			/* host copy of pivoting sequence */
	int info;							/* host copy of error info */
	int  lwork;							/* size of workspace */
	// device copies
	cuComplex* d_A;				/* device copy of A */
	cuComplex* d_b;				/* device copy of B */
	int* d_Ipiv;					/* pivoting sequence */
	int* d_info;					/* error info */
	cuComplex* d_work;			/* device workspace for getrf */
	//
	bool pivotOn;

public:
	BEMSolver()
		:cusolverH(NULL), stream(NULL), configured(false), decomposed(false), pivotOn(false), d_A(NULL), d_b(NULL), d_Ipiv(NULL), d_info(NULL), d_work(NULL) {
		/* step 1: create cusolver handle, bind a stream */
		CUSOLVER_CHECK(cusolverDnCreate(&cusolverH));

		CUDA_CHECK(cudaStreamCreateWithFlags(&stream, cudaStreamNonBlocking));
		CUSOLVER_CHECK(cusolverDnSetStream(cusolverH, stream));
	}
	~BEMSolver() {
		releaseMemories();

		CUSOLVER_CHECK(cusolverDnDestroy(cusolverH));

		CUDA_CHECK(cudaStreamDestroy(stream));

		CUDA_CHECK(cudaDeviceReset());
	}

	void releaseMemories() {
		if (configured) {
			CUDA_CHECK(cudaFree(d_A));
			CUDA_CHECK(cudaFree(d_b));
			CUDA_CHECK(cudaFree(d_Ipiv));
			CUDA_CHECK(cudaFree(d_info));
			CUDA_CHECK(cudaFree(d_work));
			delete[] matrixA;
			delete[] matrixB;
		}
	}

	void setConfiguration(int numMeshes, int numTransducers, float* matrixA, float* matrixB, bool pivotOn) {
		releaseMemories();

		this->numMeshes = numMeshes;
		this->numTransducers = numTransducers;
		this->matrixA = new float[2 * numMeshes * numMeshes];
		this->matrixB = new float[2 * numMeshes * numTransducers];
		memcpy(this->matrixA, matrixA, 2 * numMeshes * numMeshes * sizeof(float));
		memcpy(this->matrixB, matrixB, 2 * numMeshes * numTransducers * sizeof(float));


		N = lda = ldb = numMeshes;
		Ipiv.resize(N, 0);
		info = 0;
		lwork = 0;
		this->pivotOn = pivotOn;

		/* step 2: copy A to device */
		CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_A), sizeof(cuComplex) * N * N));
		CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_b), sizeof(cuComplex) * N));
		CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_Ipiv), sizeof(int) * Ipiv.size()));
		CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_info), sizeof(int)));

		configured = true;
		decomposed = false;
	}

	void decomposeMatrixA() {
		if (decomposed)
			return;

		CUDA_CHECK(cudaMemcpyAsync(d_A, matrixA, sizeof(float) * 2 * N * N, cudaMemcpyHostToDevice, stream));

		/* step 3: query working space of getrf */
		CUSOLVER_CHECK(cusolverDnCgetrf_bufferSize(cusolverH, N, N, d_A, lda, &lwork));
		cudaError_t err = cudaMalloc(reinterpret_cast<void**>(&d_work), sizeof(cuComplex) * lwork);
		CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_work), sizeof(cuComplex) * lwork));

		/* step 4: LU factorization */
		if (pivotOn) {
			CUSOLVER_CHECK(cusolverDnCgetrf(cusolverH, N, N, d_A, lda, d_work, d_Ipiv, d_info));
			CUDA_CHECK(cudaMemcpyAsync(Ipiv.data(), d_Ipiv, sizeof(int) * Ipiv.size(), cudaMemcpyDeviceToHost, stream));
		}
		else {
			CUSOLVER_CHECK(cusolverDnCgetrf(cusolverH, N, N, d_A, lda, d_work, NULL, d_info));

			CUDA_CHECK(cudaMemcpyAsync(&info, d_info, sizeof(int), cudaMemcpyDeviceToHost, stream));

			CUDA_CHECK(cudaStreamSynchronize(stream));

			if (0 > info) {
				printf("%d-th parameter is wrong \n", -info);
				exit(1);
			}
		}
		decomposed = true;
	}

	float* solveMatrixT2M() {
		if (!decomposed) {
			printf("The matrix A has not been decomposed yet...\n");
			decomposeMatrixA();
		}
		float* matrixT2M = new float[2 * numMeshes * numTransducers];
		for (int t = 0; t < numTransducers; t++) {
			std::vector<cuComplex> x = solveVectorx(t);
			for (int m = 0; m < numMeshes; m++) {
				int mInd = m * numTransducers + t;
				matrixT2M[mInd * 2 + 0] = x[m].x;
				matrixT2M[mInd * 2 + 1] = x[m].y;
			}
			if (t % 16 == 0) printf("%d / 16\n", t / 16 + 1);
		}
		return matrixT2M;
	}

	static void exportBEMFile(std::string fileName, int numTransducers, int numMeshes, float* meshPositions, float* meshAreas, float* meshNormals, float* t2mMatrix) {
		FILE* f;
		fopen_s(&f, fileName.c_str(), "wb");
		if (f == NULL) printf("File cannot open\n");
		else {
			fwrite(&numTransducers, sizeof(float), 1, f);
			fwrite(&numMeshes, sizeof(float), 1, f);
			fwrite(meshPositions, sizeof(float), 4 * numMeshes, f);
			fwrite(meshAreas, sizeof(float), numMeshes, f);
			fwrite(meshNormals, sizeof(float), 4 * numMeshes, f);
			fwrite(t2mMatrix, sizeof(float), 2 * numTransducers * numMeshes, f);
			fclose(f);
		}
	}
	static void importBEMFile(std::string fileName, int& numTransducers, int& numMeshes, float*& meshPositions, float*& meshAreas, float*& meshNormals, float*& t2mMatrix) {
		FILE* f;
		fopen_s(&f, fileName.c_str(), "rb");
		if (f == NULL) printf("File cannot open\n");
		else {
			size_t numMem;
			numMem = fread(&numTransducers, sizeof(float), 1, f);
			numMem = fread(&numMeshes, sizeof(float), 1, f);

			meshPositions = new float[numMeshes * 4];
			numMem = fread(meshPositions, sizeof(float), numMeshes * 4, f);
			meshAreas = new float[numMeshes];
			numMem = fread(meshAreas, sizeof(float), numMeshes, f);
			meshNormals = new float[numMeshes * 4];
			numMem = fread(meshNormals, sizeof(float), numMeshes * 4, f);

			t2mMatrix = new float[numTransducers * numMeshes * 2];
			numMem = fread(t2mMatrix, sizeof(float), numTransducers * numMeshes * 2, f);

			fclose(f);
		}
	}
protected:
	std::vector<cuComplex> solveVectorx(int transducerID) {
		int vecOffset = 2 * numMeshes * transducerID;
		CUDA_CHECK(cudaMemcpyAsync(d_b, &(matrixB[vecOffset]), sizeof(float) * 2 * N, cudaMemcpyHostToDevice, stream));

		/* step 5: solve A*X = B */
		if (pivotOn) {
			CUSOLVER_CHECK(cusolverDnCgetrs(cusolverH, CUBLAS_OP_N, N, 1, d_A, lda, d_Ipiv, d_b, ldb, d_info));
		}
		else {
			CUSOLVER_CHECK(cusolverDnCgetrs(cusolverH, CUBLAS_OP_N, N, 1, d_A, lda, NULL, d_b, ldb, d_info));
		}

		std::vector<cuComplex> X(N);
		CUDA_CHECK(cudaMemcpyAsync(X.data(), d_b, sizeof(float) * 2 * N, cudaMemcpyDeviceToHost, stream));
		CUDA_CHECK(cudaStreamSynchronize(stream));
		return X;
	}
};
