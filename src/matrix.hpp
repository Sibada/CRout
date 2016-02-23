#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;

template<typename Type>
class Matrix {
public:

    Matrix() : array(NULL),x(0),y(0),z(0){}
    Matrix(int x,int y,int z = 1);
    ~Matrix();

    /** Getter & Setter **/
    Type get(int x,int y,int z = 1){
        return array[z-1][y-1][x-1];
    }
    void set(int x,int y,int z,Type value){
      array[z-1][y-1][x-1] = value;

    }

    void set(int x,int y,Type value){
       array[0][y-1][x-1] = value;
    }

    int get_x(){return x;}
    int get_y(){return y;}
    int get_z(){return z;}

    void set_all(Type value){
        for(int i = 0; i < z; i++)
            for(int j = 0; j < y; j++)
              for(int k = 0; k < x; k++)
                    array[i][j][k] = value;
    }

    void __print(int demention = 1){
        for(int i = 0; i < y; i++) {
            for(int j = 0; j < x; j++)
                cout<<array[demention - 1][i][j]<<" ";
            cout<<endl;
        }
    }

    /** 从文件读取 **/
    int read_file(string filename);

private:
    Type *** array;
    int x, y, z;

}; // class Matrix

template <typename Type>
Matrix<Type>::Matrix(int x,int y,int z){
    this->x = x;
    this->y = y;
    this->z = z;

    array = new Type**[z];
    for(int i =0; i<z; i++) {
        array[i] = new Type*[y];
        for(int j = 0; j<y; j++) {
            array[i][j] = new Type[x];
        }
    }
}

template <typename Type>
Matrix<Type>::~Matrix(){
    if(array != NULL) {
        for(int i = 0; i<z; i++) {
            if(array[i]!=NULL) {
                for(int j = 0; j < y; j++) {
                    if(array[i][j]!= NULL)
                        delete array[i][j];
                }
                delete array[i];
            }
        }
        delete array;
    }

}

/** 从文件读入数据 **/
template <typename Type>
int Matrix<Type>::read_file(string filename){
    ifstream fin(filename.c_str());

    if (!fin.is_open())
        return 1;

    char buf[20];
    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            fin >> buf;
            array[0][y][x] = atof(buf);
        }
    }
    delete buf;
    fin.close();
    return 0;
}

#endif // MATRIX_HPP

