#pragma once
#include <vector>
#include <iostream>
#include <stdlib.h>     
#include <time.h>
#include <chrono>
#include <omp.h>


template <class T>
class Matrix{
    public:
        /**
         * wektor wektorów przechowujący elementy macierzy.
         */
        std::vector<std::vector<T>> mat;
        int rows;
        int cols;
        /**
         * Dodane w celu łatwiejszego debugowania programu. Ustawienie na true sprawia, że wyświetlane są dodatkowe informacje w trakcie działania programu.
         */
        bool debug = false;

    Matrix(int drows, int dcols, bool ddebug = false){
        //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        this->mat = std::vector<std::vector<T>>(drows,std::vector<T>(dcols,T()));

        this->rows = drows;
        this->cols = dcols;
        this->debug = ddebug;
        //std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        //std::cout << "\tMat constr. " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << " [us]" << std::endl;
    }

    Matrix(){}

    Matrix(int drows, int dcols, std::vector<std::vector<T>> initList,bool ddebug = false){
        std::chrono::steady_clock::time_point begin;
        std::chrono::steady_clock::time_point end;
        if(ddebug) begin = std::chrono::steady_clock::now();
        #pragma omp parallel for
        for(size_t i=0;i<(size_t)drows;i++){
            this->mat.push_back(std::vector<T>());
            for(size_t j=0;j<(size_t)dcols;j++){
                this->mat[i].push_back(initList[i][j]);
            }
        }
        this->rows = drows;
        this->cols = dcols;
        this->debug = ddebug;
        if(ddebug)end = std::chrono::steady_clock::now();
        if(ddebug)std::cout << "\tMat constr2. " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " [ms]" << std::endl;
    }

    T get(int row, int col){return this->mat[row][col];}
    void set(int row, int col, T val){this->mat[row][col]=val;}

    /**
     * Pozwala kopiować macierze.
     */
    Matrix copy(){
        Matrix res = Matrix(this->rows,this->cols);
        std::chrono::steady_clock::time_point begin;
        std::chrono::steady_clock::time_point end;
        if(this->debug)begin = std::chrono::steady_clock::now();
        int i,j;
        #pragma omp parallel for private(i,j) schedule(dynamic)
        for(i=0;i<this->rows;i++){
            for(j=0;j<this->cols;j++){
                res.set(i,j,this->mat[i][j]);
            }
        }
        if(this->debug)end = std::chrono::steady_clock::now();
        if(this->debug)std::cout << "\tcopy. " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << " [us]" << std::endl;
        return res;
 
    }

    /**
     * Zwraca wartość do podanej liczby miejsc po przecinku.
     */
    T roundToDecimalPlace(T number, int decimalPlaces) {
        T multiplier = std::pow(10, decimalPlaces);
        return std::round(number * multiplier) / multiplier;
    }

    /**
     * Wypisuje macierz na wyjście.
     */
    void print(){
        for(int i=0;i<this->rows;i++){
            std::cout<<"\n";
            std::cout<<"[";
            for(int j=0;j<this->cols;j++){
                std::cout<<roundToDecimalPlace(this->mat[i][j],2);
                if(j<this->cols-1) std::cout<<", ";
            }
            std::cout<<"]";
        }
    }

    /**
     * Transpozycja macierzy.
     */
    Matrix transpose(){
        Matrix res = Matrix(this->cols,this->rows);
        std::chrono::steady_clock::time_point begin;
        std::chrono::steady_clock::time_point end;
        if(this->debug)begin = std::chrono::steady_clock::now();
        int i, j;
        #pragma omp parallel for private(i,j) schedule(dynamic)
        for(i=0;i<this->rows;i++){
            for(j=0;j<this->cols;j++){
                res.mat[j][i] = mat[i][j];
            }
        }
        if(this->debug)end = std::chrono::steady_clock::now();
        if(this->debug)std::cout << "\ttranspose: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << " [us]" << std::endl;
        return res;
    }

    /**
     * Wypełnia macierz losowymi wartościami z zakresu -10 - 9.
     */
    void fillRandom(){
        srand(time(NULL));
        for(int i=0;i<this->rows;i++){
            for(int j=0;j<this->cols;j++){
                this->mat[i][j] = (T)(rand()%20-10);
            }
        }
    }
    
    /**
     * Mnożenie macierzy metodą Cauchy'ego.
     */
    Matrix operator*(Matrix& other){    
        Matrix res = Matrix(this->rows,other.cols);
        std::chrono::steady_clock::time_point begin;
        std::chrono::steady_clock::time_point end;
        if(this->debug)begin = std::chrono::steady_clock::now();
        int i,j,k;
        int othCol = other.cols;
        int thisRow = this->rows;
        #pragma omp parallel for private(i,j,k) schedule(dynamic)
        for(i=0;i<thisRow;i++){
            for(j=0;j<othCol;j++){
                T sum = T();
                for(k = 0;k<thisRow;k++){
                    sum = sum + mat[i][k] * other.mat[k][j];
                }
                res.set(i,j,sum);
            }
        }
        if(this->debug)end = std::chrono::steady_clock::now();
        if(this->debug)std::cout << "\tMul. " <<"this:("<<this->rows << ","<<this->cols<<"),other:("<<other.rows<<","<< other.cols <<") "<< std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << " [us]" << std::endl;
        return res;
    }

    /**
     * Nie użyte w programie mnożenie macierzy przez skalar.
     */
    Matrix operator*(float other){
        Matrix res = Matrix(this->rows,this->cols);
        #pragma omp parallel for
        for(int i=0;i<this->rows;i++){
            for(int j=0;j<this->cols;j++){
                res.set(1,j,this->mat[i][j] * T(other));
            }
        }
        return res;
    }
};
