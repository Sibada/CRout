//
// Created by sibada on 16-2-17.
//

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

int write_file(double * flow, double basin_factor, const Time& start_date, int skip_days, int rout_days,string station_name, string out_path){
    string fd_path = out_path+station_name+".day";
    string fdmm_path = out_path+station_name+".day_mm";
    Time date = start_date;

    ofstream fd,fdmm;

    fd.open(fd_path.c_str());
    fdmm.open(fdmm_path.c_str());
    if(!fd.is_open() || !fdmm.is_open()){
        cout<<"   错误： 无法打开输出文件" << fd_path<<endl
        <<"                       "<<fdmm_path<<endl;
        return 1;
    }

    for(int i = skip_days+ 1; i <= rout_days; i++){
        fd << date.get_year()<<"\t"<<date.get_month()<<"\t"<<date.get_day()<<"\t"
        <<flow[i]<<endl;

        fdmm << date.get_year()<<"\t"<<date.get_month()<<"\t"<<date.get_day()<<"\t"
        <<flow[i]/basin_factor<<endl;

        date += 1;
    }
    fd.close();
    fdmm.close();
    return 0;
}

int write_file_month(double * flow, double basin_factor, const Time& start_date, int skip_days, int rout_days,string station_name, string out_path){
    string fm_path = out_path+station_name+".month";
    string fmmm_path = out_path+station_name+".month_mm";
    Time date = start_date;

    ofstream fm,fmmm;

    fm.open(fm_path.c_str());
    fmmm.open(fmmm_path.c_str());
    if(!fm.is_open() || !fmmm.is_open()){
        cout<<"   错误： 无法打开输出文件" << fm_path<<endl
            <<"                       "<<fmmm_path<<endl;
        return 1;
    }

    double month_avg = 0.0;

    for(int i = skip_days + 1; i <= rout_days; i++){
        month_avg += flow[i];

        if(date.get_day() == date.get_days_in_month()) {
            month_avg /= date.get_days_in_month();

            fm << date.get_year() << "\t" << date.get_month() << "\t"
            << month_avg<<endl;

            fmmm << date.get_year() << "\t" << date.get_month() << "\t"
            << month_avg / basin_factor<<endl;
            month_avg = 0.0;
        }
        date += 1;
    }
    fm.close();
    fmmm.close();
    return 0;

}
