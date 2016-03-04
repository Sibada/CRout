#include "crout.hpp"
#include "grid.hpp"
#include "matrix.hpp"
#include "Time.hpp"

#include <string>
#include <fstream>
#include <iostream>
#include <math.h>

using namespace std;

void make_UHm(Matrix<double>* UH_m,
              Grid<double>* veloc,Grid<double>* diffu,Grid<double>* distan,
              Matrix<int>* basin,int basin_sum)
{
    for(int num = 1; num <= basin_sum; num++) {
        int x = basin->get(num, 1);
        int y = basin->get(num, 2);
        if (distan->get(y, x) < 1)continue;
        double vel = veloc->get(y, x),
                dif = diffu->get(y, x),
                dis = distan->get(y, x);

        double t = 0.0;
        for (int k = 1; k <= LE; k++) {
            t += DT;
            double h = 0.0;
            if (vel > 0.0) {
                double pot = pow((vel * t - dis), 2.0) / (4.0 * dif * t);
                if (pot <= 69.0)
                    h = 1.0 / (2.0 * sqrt(PI * dif)) * dis / pow(t, 1.5) * exp(-pot);
            }
            UH_m->set(k, x, y, h);
        }

        double sum = 0.0;
        for(int k = 1;k <= LE; k++)
            sum += UH_m->get(k,x,y);
        if(sum > 0.0)
            for(int k = 1; k <=LE; k++)
                UH_m->set(k,x,y,UH_m->get(k,x,y)/sum);

    }

}


void make_grid_UH(Matrix<double>* UH_grid,
                  Matrix<int>* basin, int basin_sum,
                  Matrix<double>* UH_m, double* UH_slope,
                  Grid<int>* next_x,Grid<int>* next_y,
                  int stn_x, int stn_y){

    double** UH_daily = new double*[basin_sum + 1];
    for(int i = 1;i <= basin_sum; i++ ){
        UH_daily[i] = new double[DAY_UH+1];
    }

    double* UH_hour[2];
    UH_hour[0] = new double[T_MAX+1];
    UH_hour[1] = new double[T_MAX+1];


    for(int n = 1;n <= basin_sum; n++){

        cout<<"  ------Deal with "<<n<<" of "<<basin_sum<<endl;

        for(int i = 1;i <= DAY_UH; i++)
            UH_daily[n][i] = 0.0;

        for(int i = 1;i <= 24;i++){
            UH_hour[0][i] = 1.0/24.0;
            UH_hour[1][i] = 0.0;
        }
        for(int i = 25;i <= T_MAX; i++){
            UH_hour[0][i] = 0.0;
            UH_hour[1][i] = 0.0;
        }

        int x = basin->get(n, 1);
        int y = basin->get(n, 2);

        while(true){
            for(int t = 1; t <= T_MAX; t++)
                for(int l = 1; l <= LE; l++)
                    if(t > l) UH_hour[1][t]+= UH_hour[0][t-l]*UH_m->get(l,x,y);

            if(x == stn_x && y == stn_y)break;

            int nx = next_x->get(y,x);
            int ny = next_y->get(y,x);
            x = nx;
            y = ny;

            for(int i = 1; i <= T_MAX; i++){
                UH_hour[0][i] = UH_hour[1][i];
                UH_hour[1][i] = 0.0;
            }
        }

        for(int t = 1; t <= T_MAX; t++){
            int st = (t + 23)/24;
            UH_daily[n][st] += UH_hour[0][t];
        }
    }

    UH_grid->set_all(0.0);

    for(int n = 1; n<= basin_sum; n++){

        for(int j = 1; j <= KE; j++){
            for(int k = 1; k <= DAY_UH; k++){
                double tmp = UH_grid->get(n,j+k-1);
                tmp += UH_slope[j-1] * UH_daily[n][k];
                UH_grid->set(n,j+k-1, tmp);
            }
        }

        double sum = 0.0;
        for(int k = 1;k <= KE + DAY_UH -1; k ++)
            sum += UH_grid->get(n,k);
        for(int k = 1;k <= KE + DAY_UH -1; k ++){
            UH_grid->set(n,k,UH_grid->get(n,k)/sum);

        }

    }

    for(int i = 1;i <= basin_sum; i++ ){
        delete[] UH_daily[i];
    }
    delete[] UH_daily;
    delete[] UH_hour[0];
    delete[] UH_hour[1];
}

int write_UH_grid(string name ,Matrix<double>* UH_grid, Matrix<int>* basin,int basin_sum){
    ofstream fout;
    string filename = name +".uh_s";
    fout.open(filename.c_str());
    if(!fout.is_open()){
        return 1;
    }

    int kl = KE + DAY_UH - 1;
    for(int n = 1; n <= basin_sum; n++)
    {
        fout<<basin->get(n,1)<<"\t"<<basin->get(n,2)<<"\t";
        for(int k = 1; k <= kl; k++)
            fout << UH_grid->get(n,k)<<" ";
        fout<<endl;
    }
    fout.close();
    cout<<"     -----File "<<filename<<" is in the directory of CRout.\n";
    return 0;
}

int read_UH_grid(string filename, Matrix<double>* UH_grid, Matrix<int>* basin){
    ifstream fin;
    fin.open(filename.c_str());
    if(!fin.is_open())
    {
        cout<<"  Error: Station UH file " << filename << " not found.\n";
        exit(1);
    }

    char buf[32];
    int sum = 0;
    int sta = 0;
    int kl = KE + DAY_UH - 1;

    while(true){
        fin>>buf;
        if(fin.eof())
            break;

        sum ++;
        int ibuf;
        sscanf(buf,"%d",&ibuf);
        basin->set(sum,1,ibuf);

        fin>>buf;
        sscanf(buf,"%d",&ibuf);
        basin->set(sum,2,ibuf);

        double ebuf;
        for(int i = 1; i <= kl; i++){
            fin>>buf;
            sta = sscanf(buf,"%lf",&ebuf);
            if(sta < 1){
                cout<< "  Error: Station UH file format incorrect.\n";
                exit(1);
            }
        UH_grid->set(sum,i,ebuf);
        }
    }

    fin.close();
    return sum;
}
