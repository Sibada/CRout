#include "crout.hpp"
#include "grid.hpp"
#include "matrix.hpp"
#include "Time.hpp"

#include <string>
#include <fstream>
#include <iostream>
#include <math.h>
#include <time.h>

using namespace std;

/** 获取流向的下一个栅格坐标 **/
void make_nextgrid(Grid<int>* direc, Grid<int>* next_x, Grid<int>* next_y){
    int nrow = direc->get_nrow();
    int ncol = direc->get_ncol();

    for(int i = 1; i <= nrow; i++){
        for(int j = 1; j <= ncol; j++){
            switch (direc->get(i,j)) {
            case NW:
                next_x->set(i,j,j);
                next_y->set(i,j,i+1);
                break;
            case NEW:
                next_x->set(i,j,j+1);
                next_y->set(i,j,i+1);
                break;
            case EW:
                next_x->set(i,j,j+1);
                next_y->set(i,j,i);
                break;
            case SEW:
                next_x->set(i,j,j+1);
                next_y->set(i,j,i-1);
                break;
            case SW:
                next_x->set(i,j,j);
                next_y->set(i,j,i-1);
                break;
            case SWW:
                next_x->set(i,j,j-1);
                next_y->set(i,j,i-1);
                break;
            case WW:
                next_x->set(i,j,j-1);
                next_y->set(i,j,i);
                break;
            case NWW:
                next_x->set(i,j,j-1);
                next_y->set(i,j,i+1);
                break;
            default:
                next_x->set(i,j,0);
                next_y->set(i,j,0);
                break;
            }
        }
    }
}

/** 栅格坐标转经纬度 **/
double get_lon(int x,double xll,double csize){
    return xll + x * csize - 0.5*csize;
}

/** 同上 **/
double get_la(int y, double yll,double csize){
    return yll + y * csize - 0.5*csize;
}

/** 生成流程栅格数据 **/
void make_distance(Grid<double>*distan,Grid<int>*next_x,Grid<int>* next_y,
                   double xll,double yll,double csize){

    int nrow = next_x->get_nrow();
    int ncol = next_x->get_ncol();

    for(int i = 1 ; i<= nrow;i++){
        for(int j = 1; j <= ncol;j++){
            if(next_x->get(i,j) >= 1) {
                int nx = next_x->get(i, j);
                int ny = next_y->get(i, j);
                double lon1 = get_lon(j, xll, csize);
                double lon2 = get_lon(nx, xll, csize);
                double dlon = lon1 - lon2;

                double la1 = get_la(i, yll, csize);
                double la2 = get_la(ny, yll, csize);

                double distance = cos(la1 * PI / 180) * cos(la2 * PI / 180) * cos(dlon * PI / 180)
                                  + sin(la1 * PI / 180) * sin(la2 * PI / 180);
                distance = acos(distance) * EARTH_RADI;

                distan->set(i, j, distance);
            }
            else distan->set(i, j , 0);
        }
    }

}

/**
 * @brief discovery_basin 找出站点控制流域内的网格
 * @param stn_x
 * @param stn_y 站点的位置（从左下数起第几个网格）
 * @param basin 存储流域内网格坐标
 * @param next_x
 * @param next_y 流向的下一个网格
 * @return 流域内网格数
 */
int discovery_basin(int stn_x,int stn_y, Matrix<int>* basin,
                     Grid<int>* next_x, Grid<int>* next_y){

    int nrow = next_x->get_nrow(),
            ncol = next_x->get_ncol();

    int id = 0;

    for(int i = 1; i <= nrow; i++)
        for(int j = 1; j<= ncol; j++)
            if(next_x->get(i,j) > 0){
                int x = j, y = i; // 追踪起点
                while(true){
                    if(x == stn_x && y == stn_y){
                        id++;

                        basin->set(id,1,j);
                        basin->set(id,2,i); // 录入网格坐标
                        break;
                    }else if(x < 1 || x > ncol || y < 1
                             || y > nrow || next_x->get(y,x)<=0)
                        break;
                    else{
                        int nx = next_x->get(y,x);
                        int ny = next_y->get(y,x);
                        x = nx;
                        y = ny;
                    }
                }
            }
    return id;
}

/** 读取坡面汇流单位线 **/

int read_UH_slope(double* UHslope,string filepath){
    ifstream fin(filepath.c_str());
    if(!fin.is_open())
        return 1;
    char buf[32];
    for(int i = 0; i < KE; i++){
        int sta, uh_num;
        double uh_value;

        fin >> buf;
        sta = sscanf(buf,"%d", &uh_num);
        fin >> buf;
        sta = sscanf(buf,"%lf", &uh_value);
        if(sta == 0)
            return 1;
        UHslope[uh_num] = uh_value;

    }

    fin.close();
    return 0;
}
