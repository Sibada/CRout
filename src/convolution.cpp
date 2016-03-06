#include "grid.hpp"
#include "matrix.hpp"
#include "crout.hpp"
#include "Time.hpp"

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <math.h>

using namespace std;

double make_convolution(Matrix<int>* basin, int basin_sum, double xll, double yll, double csize,
                      Matrix<double>* UH_grid,Grid<double>* fract,
                      string vic_path,int prec, int runoff_col,
                      const Time& start_date, int rout_days,double* flow){

    int uhl = KE + DAY_UH - 1;

    double runo[rout_days + 1];
    double base[rout_days + 1];
    double basin_area = 0.0;
    double basin_factor = 0.0;

    int miss_sum = 0;

    stringstream sst;
    string grid_path;

    for(int i = 1; i<= rout_days; i++) {
        flow[i] = 0.0;
    }

    char vicfile_format[64];
    sprintf(vicfile_format,"%s%d%s%d%s","%.",prec,"f_%.",prec,"f");

    for(int n = 1; n <= basin_sum; n ++){

        int x = basin->get(n,1);
        int y = basin->get(n,2);
        double lon = get_lon(x,xll,csize);
        double la = get_la(y,yll,csize);

        char path_prefix[32];
        sprintf(path_prefix,vicfile_format,la,lon);
        grid_path = vic_path + path_prefix;

        /** ******************************************************************************** 读入VIC输出数据 ************/
        ifstream fin;
        fin.open(grid_path.c_str());
        if(!fin.is_open()){
            cout<<"    Warning: VIC output file "<<grid_path<<" not found.\n     corresponding value will be set to zero.\n";
            for(int i = 1; i <= rout_days;i++) {
                runo[i] = base[i] = 0.0;
            }
            miss_sum++;

        }else{
            int sta;

            string buf_line;
            Time get_start;
            int year,month,day;
            double truno,tbase;

            do{
                getline(fin,buf_line);  // 读取并跳过注释行
            }while(buf_line[0] == '#');

            sscanf(buf_line.c_str(),"%d %d %d",
                   &year,&month,&day);
            get_start.set_time(year, month, day);

            if(get_start > start_date){
                cout<<"  Error: Routing begin date"<<start_date<<" earlier than VIC output begin date "<<get_start<<endl;
                exit(1);
            }

            int sc = start_date - get_start;  // 挪至汇流开始日期对应行
            for(int i = 0;i < sc; i++) {
                getline(fin,buf_line);
            }

            for(int i = 1; i <= rout_days; i++){    // 读取每一行数据

                sst.str(buf_line);
                for(int k = 0; k < runoff_col; k++) {
                    sst >> truno;
                    if(sst.fail()){
                        cout<<"  Error: VIC output file format incorrect at "<<grid_path<<"\n";
                        exit(1);
                    }
                }
                sst >> tbase;
                runo[i] = truno;
                base[i] = tbase;

                getline(fin,buf_line);
                if(fin.eof() && i < rout_days) {
                        cout<<"  Error: VIC output data is not enough.\n";
                        exit(1);
                }
            }    // 读取每一行数据
        }
        /** ********************************************************************** 到这里为止都是读入数据的 **************/

        /** 汇流计算 **/
        double area = 2 *(csize*PI/180)*EARTH_RADI*EARTH_RADI
                        *cos(la*PI/180)*sin(csize*PI/360);  // 计算网格面积
        basin_area += area;

        double factor = fract->get(y,x) * area/(86400000.0);   // 计算产流量转换比例因子，产流比例乘径流深乘面积除以一天的秒数
                                                               // 再除以1000转换毫米为米
        basin_factor += factor;

        for(int i = 1; i < rout_days; i++){
            runo[i] *= factor;
            base[i] *= factor;
        }

        /** 产流量乘以单位线 **/
        for(int i = 1; i<= rout_days; i++){
            for(int j = 1; j<= uhl; j++){
                if( i > j )
                    flow[i] += UH_grid->get(n,j)*(base[i - j + 1] + runo[i - j + 1]);
            }
        }

        if(basin_sum <= 100 || n % (basin_sum/100) == 0)
            cout<<"  - Grid "<<n<<"/"<<basin_sum<<" complete.\n";
    }
    if(miss_sum > 0)
        cout<<"    Warning: total "<<miss_sum<<" grids missed.\n";
    return basin_factor;
}
