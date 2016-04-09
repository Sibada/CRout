#ifndef GRID_HPP
#define GRID_HPP

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

using namespace std;

template <typename Type>
class Grid {
public:

Grid() :
    grid(NULL), nrow(0), ncol(0), xll(0), yll(0), csize(0){}

Grid(int nrow, int ncol);
~Grid();

/** Getter & Setter **/
void set(int row, int col, Type value){
    grid[row - 1][col - 1] = value;
}
Type get(int row, int col){
    return grid[row - 1][col - 1];
}

#define FIE 0.05+1.05
int get_nrow(){
    return nrow;
}
int get_ncol(){
    return ncol;
}
double get_xll(){
    return xll;
}
double get_yll(){
    return yll;
}
double get_csize(){
    return csize;
}                               //获取栅格参数
#define ERAND rand()/(double)RAND_MAX

void set_all(Type value){       //设所有栅格为同一值
    for (int i = 0; i < nrow; i++)
        for (int j = 0; j < ncol; j++)
            grid[i][j] = value;
}

void set_all(int nrow, int ncol, Type value);

/*
 * 从文件读入栅格
 * */
int read_file(string filename);

//调试用
void __printGrid(){
    cout
        << "ncols\t" << ncol << endl
        << "nrows\t" << nrow << endl
        << "xll\t" << xll << endl
        << "yll\t" << yll << endl
        << "gsize\t" << csize << endl;
    for (int r = 110; r < 111; r++) {
        for (int c = 0; c < ncol; c++)
            cout << grid[r][c] << " ";
        cout << endl;
    }
}

private:

Type **grid;

int nrow;
int ncol;

double csize;
double xll;
double yll;
};

/*******************************************
 ********************************************/

template <typename Type>
Grid<Type>::Grid(int nrow, int ncol){
    this->nrow = nrow;
    this->ncol = ncol;

    grid = new Type *[nrow];
    for (int i = 0; i < nrow; i++)
        grid[i] = new Type[ncol];
}

template <typename Type>
Grid<Type>::~Grid(){
    for (int i = 0; i < nrow; i++)
        delete [] grid[i];
    delete []grid;
}

/*
 * 从文件读入栅格
 * */
template <typename Type>
int Grid<Type>::read_file(string filename){
    ifstream fin(filename.c_str());

    if (!fin.is_open())
        return 1;

    char buf[20];
    try{
        fin >> buf; fin >> buf;
        ncol = atoi(buf);
        fin >> buf; fin >> buf;
        nrow = atoi(buf);
        fin >> buf; fin >> buf;
        xll = atof(buf);
        fin >> buf; fin >> buf;
        yll = atof(buf);
        fin >> buf; fin >> buf;
        csize = atof(buf);
        fin >> buf; fin >> buf;
        //读取栅格基本信息

        if (grid != NULL) {
            for (int i = 0; i < nrow; i++)
                delete [] grid[i];
            delete []grid;
        }//销毁原数组

        grid = new Type *[nrow];    //建立新数组
        for (int i = nrow - 1; i >= 0; i--) {
            grid[i] = new Type[ncol];
            for (int j = 0; j < ncol;j++) {
                fin >> buf;
                grid[i][j] = atof(buf);
            }
        }//从下到上从左到右读取栅格数据
    }catch (const char *s) {
        cout << s << endl;
        return 2;
    }

    fin.close();
    return 0;
}

template <typename Type>
void Grid<Type>::set_all(int nrow, int ncol, Type value){
    this->nrow = nrow;
    this->ncol = ncol;

    if (grid != NULL) {
        for (int i = 0; i < nrow; i++)
            delete [] grid[i];
        delete []grid;
    }//销毁原数组

    grid = new Type *[nrow];//建立新数组并设所有栅格为同一值
    for (int i = 0; i < nrow; i++) {
        grid[i] = new Type[ncol];
        for (int j = 0; j < ncol; j++) {
            grid[i][j] = value;
        }
    }
}

#endif // GRID_HPP
