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
 *
 *
 */

using namespace std;

int main(int argc, char *argv[]){

    if(argc < 2){
        cout<<"用法： crout <输入参数文件路径> [<日志输出路径>]\n";
        exit(0);
    }

    ifstream fin;
    fin.open(argv[1]);
    if(!fin.is_open()){
        cout<<"  错误：找不到输入参数文件。\n";
        exit(1);
    }

    ofstream olog;
    if(argc > 2) {
        olog.open(argv[2]);
        if(!olog.is_open()){
            cout<<"  错误： 无法打开日志输出路径。\n";
            exit(1);
        }
        cout<<"  输出信息记录在日志文件“"<<argv[2]<<"”中。\n";
        cout.rdbuf(olog.rdbuf());
    }

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

    do{
        getline(fin, line);
    }while(line.length() == 0 || line[0] == '#');

    veloc_path = line;  // 流速文件路径
    if(veloc_path == "F"){
        getline(fin,line);
        veloc_default = atof(line.c_str());
    }   // 若无提供读入均一值

    do{
        getline(fin, line);
    }while(line.length() == 0 || line[0] == '#');

    diffu_path = line;  // 扩散值文件路径
    if(diffu_path == "F"){
        getline(fin,line);
        diffu_default = atof(line.c_str());
    }

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
    if(fract_path == "F"){
        getline(fin,line);
        fract_default = atof(line.c_str());
    }

    do{
        getline(fin, line);
    }while(line.length() == 0 || line[0] == '#');

    stnloc_path = line;  // 流量站文件路径

    do{
        getline(fin, line);
    }while(line.length() == 0 || line[0] == '#');

    vic_path = line;  // VIC输出文件路径
    getline(fin,line);
    prec = atoi(line.c_str());  //VIC输出文件坐标小数位数

    do{
        getline(fin, line);
    }while(line.length() == 0 || line[0] == '#');

    out_path = line;  // 汇流输出文件路径

    do{
        getline(fin, line);
    }while(line.length() == 0 || line[0] == '#');

    sta = sscanf(line.c_str(),"%d %d %d %d",&begin_y,&begin_m,&end_y,&end_m);
    if(sta < 4){
        cout<<"  错误： 汇流计算时间格式不正确。\n";
        exit(1);
    }
    getline(fin,line);
    sta = sscanf(line.c_str(),"%d %d %d %d",&start_y,&start_m,&stop_y,&stop_m);   // 汇流起止日期
    if( sta < 4){
        cout<<"  错误： 输出时间格式不正确。\n";
        exit(1);
    }

    begin_date.set_time(begin_y,begin_m,1);   // 录入汇流起止时间
    end_date.set_time(end_y,end_m,31);

    rout_days = end_date - begin_date + 1;
    if(rout_days <= 0) {
        cout << "  错误： 汇流结束日期" << end_date << "不晚于开始日期" << begin_date << "\n";
        exit(1);
    }

    start_date.set_time(start_y,start_m,1);
    stop_date.set_time(stop_y,stop_m,31);
    if(start_date < begin_date)start_date = begin_date;
    if(stop_date > end_date)stop_date = end_date;

    output_days = end_date - start_date;
    if(output_days <= 0){
        cout << "  错误： 输出结束日期" << stop_date << "不晚于开始日期" << start_date << "\n";
        exit(1);
    }

    out_skip_days = start_date - begin_date;

    do{
        getline(fin, line);
    }while(line.length() == 0 || line[0] == '#');

    UHslo_path = line;  // 坡面汇流单位线文件路径

    fin.close();



    /** ******************** 输出基本信息 ******************** **/

    cout	<< "  流向数据： " << direc_path << endl;

    cout    << "  流速数据： " ;
    if(veloc_path == "F" )
        cout<< "取均一值"<<veloc_default<<endl;
    else cout<<veloc_path<<endl;

     cout    << "  水力扩散数据： " ;
    if(diffu_path == "F" )
        cout<< "取均一值"<<diffu_default<<endl;
    else cout<<diffu_path<<endl;

     cout    << "  产流比例数据： " ;
    if(fract_path == "F" )
        cout<< "取均一值"<<fract_default<<endl;
    else cout<<fract_path<<endl;

    cout<< "  站点信息文件： " << stnloc_path << endl
        << "  VIC数据位置： " << vic_path << " " << prec << "位小数" << endl
        << "  坡面汇流单位线： " << UHslo_path << endl;
    cout << "  汇流起止日期：\n"
    << "    " << begin_date.get_date() << " - "
    << end_date.get_date() << endl;


    /** ******************** 读入各文件数据 ******************** **/

    sta = direc->read_file(direc_path);   // 流向数据
    if(sta != 0){
        cout << "  错误： 未找到流向文件。\n";
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
            cout << "  错误： 未找到流速文件。\n";
            exit(1);
        }
    }

    if(diffu_path == "F")
        diffu->set_all(nrow,ncol,diffu_default);
    else{
        sta = diffu->read_file(diffu_path);   // 水力扩散数据
        if(sta != 0){
            cout << "  错误： 未找到水力扩散文件。\n";
            exit(1);
        }
    }

//    if(distan_path == "F")
//        distan->set_all(nrow,ncol,distan_default);
//    else{
//        sta = distan->read_file(distan_path); // 流程数据
//        if(sta != 0){
//            cout << "  错误： 未找到流程文件。\n";
//            exit(1);
//        }
//    }

    if(fract_path == "F") {
        fract->set_all(nrow,ncol,fract_default);
    }
    else{
        sta = fract->read_file(fract_path);   // 产流比例数据
        if(sta != 0){
            cout << "  错误： 未找到产流比例文件。\n";
            exit(1);
        }
    }

    sta = read_UH_slope(UH_slope,UHslo_path);    // 坡面汇流单位线数据
    if(sta != 0){
        cout << "  错误： 未找到坡面汇流单位线文件或文件格式不对。\n";
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
        cout<<"  错误： 未找到流量站文件。\n";
        exit(1);
    }

    int is_run;    // 网格是否运行
    string station_name;    // 网格名称
    int stn_x, stn_y;   // 网格位置
    string UH_station_path; // 网格基本参数

    int basin_sum;  // 流域网格总数

    char buf[128];
    while(!fin.eof()){
        fin.getline(buf, 128);
        if(buf[0] == '\0') {
            break;
        }

        while(buf[0] == '#'){
            fin.getline(buf,128);
        }      // 跳过注释行

        char str_buf[128];
        sta = sscanf(buf,"%d %s %d %d",&is_run, str_buf, &stn_x, &stn_y);
        if(sta < 4){
            cout<< "  错误： 流量测站站点参数不足或格式错误\n";
            exit(1);
        }
        station_name = str_buf;

        fin.getline(buf,128);
        sta = sscanf(buf,"%s",str_buf);
        UH_station_path = str_buf;

        if(!is_run)continue;    //  跳过设定不运行的站点。

        cout<<"———— 站点： "<<station_name<<"， 位置 "<<stn_x<<","<<stn_y<<" ————\n";

        /** *************************** 数据计算 *************************** **/

        if(UH_station_path == "NONE"){
            cout<<"  -> 无站点单位线文件，将由程序生成。\n";

            cout<<"     寻找站点控制流域内所有网格...\n";
            basin_sum = discovery_basin(stn_x,stn_y,basin,next_x,next_y);

            cout<<"     生成站点单位线数据...\n";
            make_UHm(UH_m,veloc,diffu,distan,basin,basin_sum); //
            make_grid_UH(UH_grid,basin,basin_sum,UH_m,UH_slope,next_x,next_y,stn_x,stn_y);

            cout<<"     输出站点单位线文件...\n";
            write_UH_grid(station_name,UH_grid,basin,basin_sum);
        }
        else{
            cout<<"  -> 读取站点单位线文件...\n";
            basin_sum = read_UH_grid(UH_station_path,UH_grid,basin);
        }

        cout<<"     流域网格数量： "<<basin_sum<<endl;
        cout<<"     通过单位线进行汇流计算...\n";

        double flow[rout_days + 1];
        double basin_factor =
                make_convolution(basin, basin_sum, xll, yll, csize, UH_grid, fract,
                                 vic_path, prec, begin_date, rout_days, flow);

        cout<<"     输出模拟流量数据...\n";
        write_file(flow, basin_factor, start_date,out_skip_days, rout_days, station_name, out_path);
        write_file_month(flow, basin_factor, start_date, out_skip_days,rout_days, station_name, out_path);

    }
    fin.close();
    if(olog.is_open())
        olog.close();

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
    delete UH_m;

    cout<<"  收工。\n";
    return 0;
}
