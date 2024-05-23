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
#include "matrixpar.cpp"

template<typename T>
std::vector<std::vector<T>> readMatrixFromFile(const std::string& filename) {
    std::vector<std::vector<T>> matrix;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return matrix;
    }

    std::string line;
    if (std::getline(file, line)) {
        std::stringstream ss(line);
        char comma;
        int rows, cols;
        if (!(ss >> rows >> comma >> cols) || comma != ',') {
            std::cerr << "Invalid format in the first line of the file." << std::endl;
            return matrix;
        }

        //matrix.resize(rows);
        while (std::getline(file, line)) {
            std::stringstream ss2(line);
            T num;
            std::vector<T> row;
            while (ss2 >> num) {
                row.push_back(num);
                if (ss2.peek() == ',')
                    ss2.ignore();
            }
            if (row.size() != cols) {
                std::cerr << "Invalid number of elements in a row." << std::endl;
                return matrix;
            }
            matrix.push_back(row);
        }
        if (matrix.size() != rows) {
            std::cerr << "Invalid number of rows.:"<<matrix.size()<<" "<<rows << std::endl;
            return matrix;
        }
    } else {
        std::cerr << "Empty file." << std::endl;
        return matrix;
    }

    file.close();
    return matrix;
}



template <class T>
Matrix<T> createIdentityMatrix(int n){
    Matrix<T> res = Matrix<T>(n,n);
    for(int i = 0; i<n;i++){
        res.set(i,i,T(1));
    }
    return res;
}

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
    int nInc = 3;
    int nStart = 9;
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
            file << ";"; 
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
            file << ";";

        // std::cout<<"Macierz ML\n";
        // testMatrix.print();
        // std::cout<<"Macierz QR:\n";
        // Matrix<double> QRMatrix = res[0] * res[1];
        // QRMatrix.print();
        std::cout << "Time difference = " << elapsed << "[ms]" << std::endl;
                
    }

    file.close();

    return 0;
}