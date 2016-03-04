#include "grid.hpp"
#include "matrix.hpp"
#include "crout.hpp"
#include "Time.hpp"

#include <string>
#include <fstream>
#include <iostream>
#include <math.h>
#include <time.h>

/**
 * VIC汇流程序 CRout
 * @date 2016-02-21 0.10
 * @author SIBADA
 *
 * 16-02-22 0.11
 * 修改了部分功能。
 *
 * 16-02-25 0.12
 * 由于编码问题输出改为英文，处理了buffer长度的问题。
 *
 * 16-03-04 0.13
 * 修改了部分文件数据读入方式，输出流量数据单位改为立方米每秒，
 * 增加了按流向数据编码方式为原版以及ArcInfo形式的编译选项
 */

using namespace std;

int main(int argc, char *argv[]){

    if(argc < 2){
        cout<<"Routing program for VIC in C++ (" << DIREC_FORMAT<<")\n";
        cout<<"Usage： crout <input parameter file> [<Log output file>]\n";
        exit(0);
    }

    ifstream fin;
    fin.open(argv[1]);
    if(!fin.is_open()){
        cout<<"  Error: input parameter file not found.\n";
        exit(1);
    }

    // debug用
//    ifstream fin;
//    fin.open("/home/victi/rout/p_ins.input");

    ofstream olog;
    if(argc > 2) {
        olog.open(argv[2]);
        if(!olog.is_open()){
            cout<<"  Error: log output file can't be write.\n";
            exit(1);
        }
        cout<<"  Log file: "<<argv[2]<<endl;
        cout.rdbuf(olog.rdbuf());
    }

    cout<<"CRout Direction format: "<<DIREC_FORMAT<<endl;   // 输出流向数据编码方式

    time_t st_time, ed_time;
    time( &st_time);    // 计算程序运行时间。

    string direc_path;
    string veloc_path;
    string diffu_path;
    string distan_path;
    string fract_path;
    string stnloc_path;
    string vic_path;
    string out_path;

    string cacu_time;
    string output_time;
    string UHslo_path;

    int prec = 4;   // VIC输出文件坐标小数位数

    double veloc_default,diffu_default = 0.0;
    double fract_default,distan_default = 0.0;

    /** 栅格与矩阵 **/

    Grid<int>* direc = new Grid<int>;
    Grid<double>* veloc = new Grid<double>;
    Grid<double>* diffu = new Grid<double>;
    Grid<double>* distan = new Grid<double>;
    Grid<double>* fract = new Grid<double>;

    Grid<int>* next_x = NULL;
    Grid<int>* next_y = NULL;   // 流向的下一个网格坐标

    double* UH_slope = new double[KE];   // 汇流单位线数据

    Matrix<int>* basin = new Matrix<int>(BASIN_MAX,2);
    Matrix<double>* UH_grid = new Matrix<double>(BASIN_MAX,KE + DAY_UH - 1);
    Matrix<double>* UH_m = NULL;

    /** 参数 **/

    int ncol,nrow = 0;
    double xll,yll = 0.0;
    double csize = 0.0;

    Time begin_date, end_date;
    Time start_date, stop_date;

    int begin_y,begin_m,end_y,end_m;
    int start_y,start_m,stop_y,stop_m;
    int rout_days;
    int output_days;
    int out_skip_days;

    int sta = 0; // 状态参数

    /** ******************* 读取输入参数文件信息 ******************* **/

    string line;
    do{
        getline(fin, line);
    }while(line.length() == 0 || line[0] == '#');
    // 跳过注释行

    direc_path = line;  // 流向文件路径
    cout	<< "  Flow direction file： \t" << direc_path << endl;


    do{
        getline(fin, line);
    }while(line.length() == 0 || line[0] == '#');

    veloc_path = line;  // 流速文件路径
    cout    << "  Flow velocity file： \t" ;

    if(veloc_path == "F"){
        getline(fin,line);
        veloc_default = atof(line.c_str());
        cout<< "uniform value "<<veloc_default<<endl;   // 若无提供读入均一值
    }
    else cout<<veloc_path<<endl;


    do{
        getline(fin, line);
    }while(line.length() == 0 || line[0] == '#');

    diffu_path = line;  // 扩散值文件路径
    cout    << "  Diffusion file： \t" ;

    if(diffu_path == "F"){
        getline(fin,line);
        diffu_default = atof(line.c_str());
        cout<< "uniform value "<<diffu_default<<endl;
    }
    else cout<<diffu_path<<endl;



//    do{
//        getline(fin, line);
//    }while(line.length() == 0 || line[0] == '#');
//
//    distan_path = line;  // 流程文件路径
//    if(distan_path == "F"){
//        getline(fin,line);
//        distan_default = atof(line.c_str());
//    }

    do{
        getline(fin, line);
    }while(line.length() == 0 || line[0] == '#');

    fract_path = line;  // 产流比例文件路径
    cout    << "  Fraction file: \t" ;

    if(fract_path == "F"){
        getline(fin,line);
        fract_default = atof(line.c_str());
        cout<< "uniform value "<<fract_default<<endl;
    }
    else cout<<fract_path<<endl;


    do{
        getline(fin, line);
    }while(line.length() == 0 || line[0] == '#');

    stnloc_path = line;  // 流量站文件路径
    cout<< "  Station location file \t" << stnloc_path << endl;


    do{
        getline(fin, line);
    }while(line.length() == 0 || line[0] == '#');

    vic_path = line;  // VIC输出文件路径
    getline(fin,line);
    prec = atoi(line.c_str());  //VIC输出文件坐标小数位数
    cout << "  VIC output path \t" << vic_path << "     " << prec << " decimal places.\n";


    do{
        getline(fin, line);
    }while(line.length() == 0 || line[0] == '#');

    out_path = line;  // 汇流输出文件路径
    cout << "  Routing output path \t" << out_path<<endl;


    do{
        getline(fin, line);
    }while(line.length() == 0 || line[0] == '#');

    sta = sscanf(line.c_str(),"%d %d %d %d",&begin_y,&begin_m,&end_y,&end_m);
    if(sta < 4){
        cout<<"  Error: routing calculating date format incorrect: " << line << endl;
        exit(1);
    }
    getline(fin,line);
    sta = sscanf(line.c_str(),"%d %d %d %d",&start_y,&start_m,&stop_y,&stop_m);   // 汇流起止日期
    if( sta < 4){
        cout<<"  Error: routing output data date format incorrect: " << line << endl;
        exit(1);
    }

    begin_date.set_time(begin_y,begin_m,1);   // 录入汇流起止时间
    end_date.set_time(end_y,end_m,31);

    rout_days = end_date - begin_date + 1;
    if(rout_days <= 0) {
        cout << "  Error: routing end date " << end_date << " is not later than begin date " << begin_date << "\n";
        exit(1);
    }

    start_date.set_time(start_y,start_m,1);
    stop_date.set_time(stop_y,stop_m,31);
    if(start_date < begin_date)start_date = begin_date;
    if(stop_date > end_date)stop_date = end_date;

    output_days = end_date - start_date;
    if(output_days <= 0){
        cout << "  Error: output data end date " << stop_date << " is not later than begin date " << start_date << "\n";
        exit(1);
    }

    out_skip_days = start_date - begin_date;

    do{
        getline(fin, line);
    }while(line.length() == 0 || line[0] == '#');

    UHslo_path = line;  // 坡面汇流单位线文件路径

    cout << "  Slope UH file " << UHslo_path << endl;
    cout << "  Time period of routing calculation： \t"
    << "    " << begin_date.get_date() << " -> "
    << end_date.get_date() << endl;

    cout << "  Time period of output data：         \t"
    << "    " << start_date.get_date() << " -> "
    << end_date.get_date() << endl;

    fin.close();


    /** ******************** 读入各文件数据 ******************** **/

    sta = direc->read_file(direc_path);   // 流向数据
    if(sta != 0){
        cout << "  Error: flow direction file not found.\n";
        exit(1);
    }

    ncol = direc->get_ncol();
    nrow = direc->get_nrow();
    xll = direc->get_xll();
    yll = direc->get_yll();
    csize = direc->get_csize();     //栅格基本参数

    if(veloc_path == "F")
        veloc->set_all(nrow,ncol,veloc_default);
    else{
        sta = veloc->read_file(veloc_path);   // 流速数据
        if(sta != 0){
            cout << "  Error: flow velocity file not found.\n";
            exit(1);
        }
    }

    if(diffu_path == "F")
        diffu->set_all(nrow,ncol,diffu_default);
    else{
        sta = diffu->read_file(diffu_path);   // 水力扩散数据
        if(sta != 0){
            cout << "  Error: diffution file not found.\n";
            exit(1);
        }
    }

//    if(distan_path == "F")
//        distan->set_all(nrow,ncol,distan_default);
//    else{
//        sta = distan->read_file(distan_path); // 流程数据
//        if(sta != 0){
//            cout << "  Error: xmask file not found.\n";
//            exit(1);
//        }
//    }

    if(fract_path == "F") {
        fract->set_all(nrow,ncol,fract_default);
    }
    else{
        sta = fract->read_file(fract_path);   // 产流比例数据
        if(sta != 0){
            cout << "  Error: fraction file not found.\n";
            exit(1);
        }
    }

    sta = read_UH_slope(UH_slope,UHslo_path);    // 坡面汇流单位线数据
    if(sta != 0){
        cout << "  Error: slope UH file not found or format incorrect.\n";
        exit(1);
    }

    /** ************************* 一些数据预处理 ************************* **/

    next_x = new Grid<int>(nrow,ncol);
    next_y = new Grid<int>(nrow,ncol);
    make_nextgrid(direc,next_x,next_y); // 生成流向的下一个网格信息

    distan->set_all(nrow,ncol,0.0);
    make_distance(distan,next_x,next_y,xll,yll,csize);

    UH_m = new Matrix<double>(LE,ncol,nrow);

    /** ********************** 流量测站信息并计算流量 ********************** **/

    fin.open(stnloc_path.c_str());
    if(!fin.is_open()){
        cout<<"  Error: station location file not found.\n";
        exit(1);
    }

    int is_run;    // 网格是否运行
    string station_name;    // 网格名称
    int stn_x, stn_y;   // 网格位置
    string UH_station_path; // 网格基本参数

    int basin_sum;  // 流域网格总数

    string buf;
    while(!fin.eof()){      // 循环用于处理站点信息。
        getline(fin, buf);
        if(buf[0] == '\0') {
            break;
        }

        while(buf[0] == '#'){
            getline(fin, buf);
        }      // 跳过注释行

        char str_buf[128];
        sta = sscanf(buf.c_str(),"%d %s %d %d",&is_run, str_buf, &stn_x, &stn_y);
        if(sta < 4){
            cout<< "  Error: station location file format incorrect.\n";
            exit(1);
        }
        station_name = str_buf;

        getline(fin, buf);
        sta = sscanf(buf.c_str(),"%s",str_buf);
        UH_station_path = str_buf;

        if(!is_run)continue;    //  跳过设定不运行的站点。

        cout<<"\n-----Station: "<<station_name<<", location: "<<stn_x<<","<<stn_y<<" ----------------------\n";

        /** *************************** 数据计算 *************************** **/

        if(UH_station_path == "NONE"){
            cout<<"  -> Station UH file not exist and will be create.\n";

            cout<<"     Searching grids contained in current basin...\n";
            basin_sum = discovery_basin(stn_x,stn_y,basin,next_x,next_y);

            cout<<"     Creating station UH data...\n";
            make_UHm(UH_m,veloc,diffu,distan,basin,basin_sum); //
            make_grid_UH(UH_grid,basin,basin_sum,UH_m,UH_slope,next_x,next_y,stn_x,stn_y);

            cout<<"     Writing station UH file...\n";
            write_UH_grid(station_name,UH_grid,basin,basin_sum);
        }
        else{
            cout<<"  -> Reading station UH file...\n";
            basin_sum = read_UH_grid(UH_station_path,UH_grid,basin);
        }

        cout<<"  -> Grids num of current basin: "<<basin_sum<<endl;
        cout<<"  -> Routing calculating...\n";

        double flow[rout_days + 1];
        double basin_factor =
                make_convolution(basin, basin_sum, xll, yll, csize, UH_grid, fract,
                                 vic_path, prec, begin_date, rout_days, flow);

        cout<<"  -> Writing routing simulation data...\n";
        write_file(flow, basin_factor, start_date,out_skip_days, rout_days, station_name, out_path);
        write_file_month(flow, basin_factor, start_date, out_skip_days,rout_days, station_name, out_path);

    } // 站点处理循环结束。

    fin.close();
    if(olog.is_open())
        olog.close();   // 关闭文件流。

    delete direc;
    delete veloc;
    delete diffu;
    delete distan;
    delete fract;
    delete next_x;
    delete next_y;
    delete UH_slope;
    delete basin;
    delete UH_grid;
    delete UH_m;    // 释放空间。

    time( &ed_time);

    cout<<"  Done. ("<<ed_time - st_time <<"s costs)\n";
    return 0;
}
