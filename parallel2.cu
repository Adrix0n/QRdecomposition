#include <cuda.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <complex>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <omp.h>
#include <chrono>


const int SIZE = 70;

template <typename T>
__global__ void MatrixMulKernel(T* matA, T* matB, T* matOut, int size){
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;

    if(row<size&&col<size){
        T sumC= T(0);
        for (int k = 0; k < size; ++k)
            sumC += matA[row*SIZE+k] * matB[k*SIZE+Col];
        matOut[row*SIZE + col] = sumC;
    }
}


template <typename T>
void fillRandom(T mat[][SIZE], int size){
    for(int i = 0; i < size; i++)
        for(int j = 0; j < size; j++){
            T ran = (T)(rand()%20-10);
            // Niby niepotrzebne ale niech tu będzie
            while(ran == T(0)){
                ran = (T)(rand()%20-10);
            }
            mat[i][j] = ran;
        }
}

template <typename T>
void createIdentityMatrix(T mat[][SIZE], int size){
    for(int i = 0; i < size; i++)
        for(int j = 0; j < size; j++)
            mat[i][j] = (i == j) ? T(1) : T(0);
}

template <typename T>
void givensRotationMatrix(T mat[][SIZE], int i, int j, T x1, T x2){
    T s = T(1) / std::sqrt( x1 * x1 / (x2 * x2) + 1);
    T c = s * x1 / x2;
    mat[i][i]=c;
    mat[j][i]=T(-1) * s;
    mat[i][j]=s;
    mat[j][j]=c;
}

template <typename T>
void transpose(T mat[][SIZE], int size){
    T buff = T(0);
    for(int i=0;i<size;i++){
        for(int j=0;j<=i;j++){
            buff = mat[j][i];
            mat[j][i] = mat[i][j];
            mat[i][j] = buff;
        }
    }
}

template <typename T>
void copy(T mat1[][SIZE], T mat2[][SIZE], int size){
    for(int i=0;i<size;i++)
        for(int j=0;j<size;j++)
            mat2[i][j] = mat1[i][j];
}

template <typename T>
void multiply(T mat1[][SIZE],T mat2[][SIZE],T mat3[][SIZE], int size){
    for(int i =0;i<size;i++)
        for(int j=0;j<size;j++){
            T sum = T(0);
            for(int k = 0;k<size;k++)
                sum += mat1[i][k] * mat2[k][j];
            mat3[i][j] = sum;
        }
}

template <typename T>
bool compareToZero(T num){
    if(num < T(0.000001) && num > T(-0.000001))
        return true;
    return false;
}

// template <typename T>
// void QRDecomposeWithGivens(T mat1[][SIZE], T matQ[][SIZE], T matR[][SIZE], int size){
//     copy<T>(mat1,matR,size);

//     createIdentityMatrix<T>(matQ,size);

//     T tmp[SIZE][SIZE];
//     T rotMatrix[SIZE][SIZE];
//     for(int col = 0; col < size; col++){
//         for (int row = size - 1; row > col; row--){
//             createIdentityMatrix(rotMatrix,size);
//             givensRotationMatrix<T>(rotMatrix,row-1,row,matR[row-1][col],matR[row][col]);

//             printf("\nPierdolony progrman givens\n");
//             for(int i =0;i<size;i++){
//                 printf("\n");
//                 for(int j = 0; j<size;j++){
//                     printf("%.02f ",rotMatrix[i][j]);
//                 }
//             }

//             multiply(rotMatrix,matR,tmp,size);
//             copy(tmp,matR,size);

//             transpose(rotMatrix,size);
//             multiply(matQ,rotMatrix,tmp,size);
//             copy(tmp,matQ,size);
//         }
//     }
    
// }

template <typename T>
void QRDecomposeWithGivens(T mat1[][SIZE], T matQ[][SIZE], T matR[][SIZE], int size, dim3 gridSize,dim3 blockSize){
    copy<T>(mat1, matR, size);
    createIdentityMatrix<T>(matQ, size);
    T rotMatrix[SIZE][SIZE];
    T rotMatrixTr[SIZE][SIZE];
    T *d_rotMatrix,*d_rotMatrixTr, *d_matR, *d_matQ, *d_tmpR, *d_tmpQ;
    // Przydzielanie pamięci na GPU
    cudaMalloc((void**)&d_rotMatrix, SIZE * SIZE * sizeof(T));
    cudaMalloc((void**)&d_rotMatrixTr, SIZE * SIZE * sizeof(T));
    cudaMalloc((void**)&d_matR, SIZE * SIZE * sizeof(T));
    cudaMalloc((void**)&d_matQ, SIZE * SIZE * sizeof(T));
    cudaMalloc((void**)&d_tmpR, SIZE * SIZE * sizeof(T));
    cudaMalloc((void**)&d_tmpQ, SIZE * SIZE * sizeof(T));
    for(int col = 0; col < size; col++){
        for (int row = size - 1; row > col; row--){
            // Tworzenie Gi i Gi^T
            createIdentityMatrix(rotMatrix, size);
            createIdentityMatrix(rotMatrixTr, size);
            givensRotationMatrix<T>(rotMatrix, row-1, row, matR[row-1][col], matR[row][col]);
            givensRotationMatrix<T>(rotMatrixTr, row-1, row, matR[row-1][col], matR[row][col]);
            transpose(rotMatrixTr, size);
            //-- Iteracja R --
            // Kopiowanie danych do GPU
            cudaMemcpy(d_rotMatrix, rotMatrix, SIZE * SIZE * sizeof(T), cudaMemcpyHostToDevice);
            cudaMemcpy(d_matR, matR, SIZE * SIZE * sizeof(T), cudaMemcpyHostToDevice);
            // Mnożenie macierzy
            MatrixMulKernel<<<gridSize, blockSize>>>(d_rotMatrix, d_matR, d_tmpR, size);
            // Kopiowanie wyników z GPU na CPU
            cudaMemcpy(matR, d_tmpR, SIZE * SIZE * sizeof(T), cudaMemcpyDeviceToHost);
            //-- Iteracja Q --
            // Kopiowanie danych do GPU
            cudaMemcpy(d_rotMatrixTr, rotMatrixTr, SIZE * SIZE * sizeof(T), cudaMemcpyHostToDevice);
            cudaMemcpy(d_matQ, matQ, SIZE * SIZE * sizeof(T), cudaMemcpyHostToDevice);
            // // Mnożenie macierzy
            MatrixMulKernel<<<gridSize, blockSize>>>(d_matQ, d_rotMatrixTr, d_tmpQ, size);
            // Kopiowanie wyników z GPU na CPU
            cudaMemcpy(matQ, d_tmpQ, SIZE * SIZE * sizeof(T), cudaMemcpyDeviceToHost);
        }
    }
    // Zwolnienie pamięci GPU
    cudaFree(d_rotMatrix);
    cudaFree(d_matR);
    cudaFree(d_tmpR);
    cudaFree(d_tmpQ);
    cudaFree(d_matQ);
}


template <typename T>
bool compareMatrices(T mat1[][SIZE],T mat2[][SIZE],int size){
    for(int i = 0; i<size; i++)
    for(int j = 0; j<size; j++)
        if(!compareToZero(mat1[i][j]-mat2[i][j]))
        return false;

    return true;
}

int main(){
    srand(100);
    const int n = SIZE;

    dim3 threadsPerBlock(n, n);
    dim3 blocksPerGrid(1, 1);
    if (n*n > 512){
        threadsPerBlock.x = 32;
        threadsPerBlock.y = 32;
        blocksPerGrid.x = ceil(double(n)/double(threadsPerBlock.x));
        blocksPerGrid.y = ceil(double(n)/double(threadsPerBlock.y));
    }


    
    int nVal[20];
    int nInc = 3;
    int nStart = 9;
    for(int i = 0; i < 20; i ++){
        nVal[i] = nStart + nInc*i;
    }

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::ofstream file("resultsparcuda2.txt");
    if (!file.is_open()) {
        std::cerr << "Failed to open file." << std::endl;
        return 0;
    }
    for (size_t i = 0; i < 20; ++i) {
        file << nVal[i];
        if (i != 20 - 1)
            file << ";"; 
    }
    file << std::endl;

    
    for(int i = 0; i < 20; i++){
        double testMatrix[SIZE][SIZE];
        double testQ[SIZE][SIZE];
        double testR[SIZE][SIZE];
        fillRandom(testMatrix,SIZE);
    
        //printf("Checkpoint1\n");
        begin = std::chrono::steady_clock::now();
        QRDecomposeWithGivens(testMatrix,testQ,testR,nVal[i],blocksPerGrid,threadsPerBlock);
        end = std::chrono::steady_clock::now();

        int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        file << elapsed;
        if (i != 20 - 1)
            file << ";";

        std::cout << "Time difference = " << elapsed << "[ms]" << std::endl;
    }

    file.close();
    return 0;
}