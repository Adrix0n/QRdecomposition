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
#include "matrixclass.h"


/*!
* Tworzy macierz jednostkową o podanym rozmiarze n
*
* @param n Określa rozmiar tworzonej macierzy (n x n)
*/
template <class T>
Matrix<T> createIdentityMatrix(int n){
    Matrix<T> res = Matrix<T>(n,n);
    for(int i = 0; i<n;i++){
        res.set(i,i,T(1));
    }
    return res;
}

/*!
* Tworzy macierz obrotu Givensa
*
* @param n Określa rozmiar tworzonej macierzy (n x n)
* @param i Wartość row-1
* @param j Wartość row
* @param x1 Wartość mat[row-1][col]
* @param x2 Wartość mat[row][col]
*/
template <class T>
Matrix<T> givensRotationMatrix(int n, int i, int j, T x1, T x2){
    Matrix<T> res = createIdentityMatrix<T>(n);
    T s = T(1) / std::sqrt( x1 * x1 / (x2 * x2) + 1);
    T c = s * x1 / x2;
    res.set(i,i,c);
    res.set(j,i,T(-1) * s);
    res.set(i,j,s);
    res.set(j,j,c);
    return res;
}

/*!
* Główna funckja programu. To tutaj wykonuje się algorytm rozkładu QR macierzy.
* Na początku tworzone są dwie macierze: matR = mat i matQ = I. Następnie algorytm wywołuje się
* sekwencyjnie, gdzie w każdej iteracji wykorzystuje macierz obrotu Givensa w celu wyzerowania
* kolejnego elementu pod główną przekątną macierzy matR. Po wyzerowaniu wszystkich koniecznych
* elementów otrzymywana jest ostateczna postać macierzy R. Następnie wyliczana jest macierz Q na
* podstawie wyliczonych wcześniej macierzy obrotu. 
*
* @param mat Macierz, na której wykonywany będzie rozkład QR.
* 
*/
template <class T>
std::vector<Matrix<T>> QRDecomposeWithGivens(Matrix<T> mat){
    Matrix<T> matR = mat.copy();
    Matrix<T> matQ = createIdentityMatrix<T>(mat.rows);

    std::vector<Matrix<T>> rotTarray;
    for(int col = 0; col < mat.cols; col++){
        for (int row = mat.rows - 1; row > col ; row--){
            Matrix<T> rotationMatrix = givensRotationMatrix<T>(mat.rows,row-1,row,matR.get(row-1,col),matR.get(row,col));
            rotTarray.push_back(rotationMatrix);
            matR = rotationMatrix * matR;
        }
    }

    Matrix<T> transposedMatrix = rotTarray[0]; 
    for(int i = 1; i < rotTarray.size();++i){
        transposedMatrix =  rotTarray[i] * transposedMatrix;
    }
    transposedMatrix = transposedMatrix.transpose();
    matQ = matQ * transposedMatrix;
    std::vector<Matrix<T>> res = {matQ,matR};
    return res;
}




int main(){
    // std::vector<std::vector<float>> testmat = readMatrixFromFile<float>("inmatrix.txt");
    // Matrix<float> testMatrix = Matrix<float>(20,20,testmat);
    // testMatrix.print();

    // std::vector<Matrix<float>> res = QRDecomposeWithGivens(testMatrix);

    // std::cout<<"\n";
    // res[0].print();
    // std::cout<<"\n";
    // res[1].print();
    // std::cout<<"\n";

    // std::cout<<"Macierz ML\n";
    // testMatrix.print();
    // std::cout<<"Macierz QR:\n";
    // Matrix<float> QRMatrix = res[0] * res[1];
    // QRMatrix.print();

    int nVal[20];
    int nInc = 2;
    int nStart = 10;
    for(int i = 0; i < 20; i ++){
        nVal[i] = nStart + nInc*i;
    }

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::ofstream file("resultspar.txt");
    if (!file.is_open()) {
        std::cerr << "Failed to open file." << std::endl;
        return 0;
    }
    for (size_t i = 0; i < 20; ++i) {
        file << nVal[i];
        if (i != 20 - 1)
            file << ","; 
    }
    file << std::endl;

    for(int i = 0; i < 20; i++){
        Matrix<double> testMatrix = Matrix<double>(nVal[i],nVal[i]);
        testMatrix.fillRandom();
    
        begin = std::chrono::steady_clock::now();
        std::vector<Matrix<double>> res = QRDecomposeWithGivens(testMatrix);
        end = std::chrono::steady_clock::now();

        int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        file << elapsed;
        if (i != 20 - 1)
            file << ",";

        // std::cout<<"\nMacierz A\n";
        // testMatrix.print();
        // std::cout<<"\nMacierz Q:\n";
        // res[0].print();
        // std::cout<<"\nMacierz R:\n";
        // res[1].print();
        // std::cout<<"\nMacierz Q*R:\n";
        // Matrix<double> QRMatrix = res[0] * res[1];
        // QRMatrix.print();

        std::cout << "Time difference = " << elapsed << "[ms]" << std::endl;
                
    }

    file.close();

    return 0;
}
