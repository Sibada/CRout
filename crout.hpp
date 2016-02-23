#ifndef CROUT_HPP
#define CROUT_HPP

#include "grid.hpp"
#include "matrix.hpp"
#include "Time.hpp"

#include <iostream>
#include <string>
#include <math.h>
#include <time.h>

#define KE 12
#define LE 48
#define DAY_UH 96
#define DT 3600.0

#define PI 3.14159265359
#define EARTH_RADI 6371393

#define BASIN_MAX 16384
#define T_MAX 2304   // DAY_UH * 24

#define NW  1
#define NEW 2
#define EW  3
#define SEW 4
#define SW  5
#define SWW 6
#define WW  7
#define NWW 8

void make_nextgrid(Grid<int> *, Grid<int> *, Grid<int> *);

double get_lon(int x,double xll,double csize);
double get_la(int y, double yll,double csize);

void make_distance(Grid<double>*distan,Grid<int>*next_x,Grid<int>* next_y,
                   double xll,double yll,double csize);


int discovery_basin(int ,int , Matrix<int>*,
                     Grid<int>*,Grid<int>* );

int read_UH_slope(double*,string);

void make_UHm(Matrix<double>*,Grid<double>*,Grid<double>*,Grid<double>*,Matrix<int>*, int);

void make_grid_UH(Matrix<double>* UH_grid,
                  Matrix<int>* basin, int basin_sum,
                  Matrix<double>* UH_m, double* UH_slope,
                  Grid<int>* next_x,Grid<int>* next_y,
                  int stn_x, int stn_y);
/**
 * 输出流域内网格单位线信息
 */
int write_UH_grid(string ,Matrix<double>*, Matrix<int>*,int);


/**
 * 从文件读取流域内网格单位线信息
 *
 */
int read_UH_grid(string filename, Matrix<double>* UH_grid, Matrix<int>* basin);

/**
 * 汇流卷积计算
 *
 */
double make_convolution(Matrix<int>* basin, int basin_sum, double xll, double yll, double csize,
                        Matrix<double>* UH_grid,Grid<double>* fract,
                        string vic_path,int prec,
                        const Time& start_date, int rout_days,double* flow);

/**
 * 输出流量信息，精确到日
 */
int write_file(double * flow, double basin_factor, const Time& start_date, int skip_days, int rout_days,string station_name, string out_path);

/**
 * 输出流量信息，精确到月份
 */
int write_file_month(double * flow, double basin_factor, const Time& start_date, int skip_days, int rout_days,string station_name, string out_path);

#endif // CROUT_HPP
