#ifndef TIME_HPP
#define TIME_HPP

#include <iostream>
#include <string>
#include <time.h>

using namespace std;

static const int MONTH_DAY_COMMON_YEAR[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
static const int MONTH_DAY_LEAP_YEAR[13] = {0,31,29,31,30,31,30,31,31,30,31,30,31};

class Time{
public:
    Time():total_days(711766),total_seconds(0),year(1949),month(10),day(1),
    hour(0),minute(0),second(0),micro_second(0),leap(false),week_day(6){}

    Time(int n){
        if(n == 0){
            time_t t = time(0);
            int days = t / 86400;
            int seconds = t - days*86400;
            seconds *= 1000;
            total_days = days + 719163;
            total_seconds = seconds;
            __days_to_ymd();
            __seconds_to_hms();
        }
    }

    Time(int y,int m, int d,int h = 0, int mi = 0, int s = 0,int ms = 0){
        year = y;
        month = m;
        day = d;
        hour = h;
        minute = mi;
        second = s;
        micro_second = ms;

        __correct_date();
        __ymd_to_days();
        __hms_to_seconds();
    }

    Time(string st){
        year = month = day = hour = minute = second = micro_second = 0;

        sscanf(st.c_str(),"%d-%d-%d %d:%d:%d.%03d",
               &year,&month,&day,&hour,&minute,&second,&micro_second);

        __correct_date();
        __ymd_to_days();
        __hms_to_seconds();
    }

    Time(const Time& t){
        this->year = t.year;
        this->month = t.month;
        this->day = t.day;
        this->hour = t.hour;
        this->minute = t.minute;
        this->second = t.second;
        this->micro_second = t.micro_second;
        this->total_days= t.total_days;
        this->total_seconds = t.total_seconds;
        this->leap = t.leap;
        this->week_day = t.week_day;
    }

    /**
     * @brief 导出格式化字符串
     * @format 年-月-日 时:分:秒.毫秒
     */
    inline string to_string()const{
        string s;
        char buf[32];
        sprintf(buf,"%d-%d-%d %d:%d:%d.%03d ",
                    year,month,day,hour,minute,second,micro_second);
        s = buf;
        s += get_weekday();
        return s;
    }

    /**
     * @brief set_time 通过字符串设置时间
     * @param st string对象
     * @return 成功设置的参数个数
     * 格式   YYYY MM DD HH:MM:SS.SSS
     */
    int set_time(string st){
        int sta;
        sta = sscanf(st.c_str(),"%d %d %d %d:%d:%d.%03d",
               &year,&month,&day,&hour,&minute,&second,&micro_second);

        if(sta == 0)return 0;

        __correct_date();
        __ymd_to_days();
        __hms_to_seconds();

        return sta;
    }

    /**
     * @brief set_time 通过字符串设置时间
     * @param st string对象
     * @return 成功设置的参数个数
     * 格式   YYYY MM DD HH:MM:SS.SSS
     */
    int set_time(char* st){
        int sta;
        sta = sscanf(st,"%d %d %d %d:%d:%d.%03d",
               &year,&month,&day,&hour,&minute,&second,&micro_second);

        if(sta == 0)return 0;

        __correct_date();
        __ymd_to_days();
        __hms_to_seconds();

        return sta;
    }

    void set_time(int y,int m, int d,int h = 0, int mi = 0, int s = 0,int ms = 0){
        year = y;
        month = m;
        day = d;
        hour = h;
        minute = mi;
        second = s;
        micro_second = ms;

        __correct_date();
        __ymd_to_days();
        __hms_to_seconds();
    }

    inline void add_days(int days){
        total_days += days;
        if(total_days <= 0)
            total_days = 1;
        __days_to_ymd();
    }

    /** 运算符重载 **/

    inline void operator = (const Time& t){
        this->year = t.year;
        this->month = t.month;
        this->day = t.day;
        this->hour = t.hour;
        this->minute = t.minute;
        this->second = t.second;
        this->micro_second = t.micro_second;
        this->total_days= t.total_days;
        this->total_seconds = t.total_seconds;
        this->leap = t.leap;
        this->week_day = t.week_day;
    }

    inline bool operator > (const Time& t)const{
        return (total_days > t.total_days
            || (total_days == t.total_days && total_seconds > t.total_seconds));
    }

    inline bool operator < (const Time& t)const{
        return (total_days < t.total_days
            || (total_days == t.total_days && total_seconds < t.total_seconds));
    }

    inline bool operator == (const Time& t)const{
        return (total_days == t.total_days
            && total_seconds == t.total_seconds);
    }

    inline bool operator >= (const Time& t)const{
        return !(*this < t);
    }

    inline bool operator <= (const Time& t)const{
        return !(*this > t);
    }

    void operator +=(int days){
        total_days += days;
        if(total_days <= 0)
            total_days = 1;
        __days_to_ymd();
    }

    void operator -=(int days){
        total_days -= days;
        if(total_days <= 0)
            total_days = 1;
        __days_to_ymd();
    }

    void operator ++(){
        total_days += 1;
        __days_to_ymd();
    }

    void operator --(){
        total_days -= 1;
        if(total_days <= 0)
            total_days = 1;
        __days_to_ymd();
    }

    int operator - (const Time& t2)const{
        return total_days - t2.total_days;
    }

    friend ostream& operator<<(ostream &os, const Time &t){
        os<<t.get_date();
        return os;
    }

    /** Getters **/

    inline int get_year()const{return year;}
    inline int get_month()const{return month;}
    inline int get_day()const{return day;}

    inline int get_days_in_month()const{
        if(leap)
            return MONTH_DAY_LEAP_YEAR[month];
        else
            return MONTH_DAY_COMMON_YEAR[month];
    }

    inline int get_hour()const{return hour;}
    inline int get_minute()const{return minute;}
    inline int get_second()const{return second;}
    inline int get_micro_second()const{return micro_second;}

    inline string get_weekday()const{
        switch (week_day) {
        case 0:
            return "SUN";
        case 1:
            return "MON";
        case 2:
            return "TUE";
        case 3:
            return "WED";
        case 4:
            return "THU";
        case 5:
            return "FRI";
        case 6:
            return "SAT";
        default:
            return "NONE";
        }
    }
    inline int get_weekday_i()const{
        return week_day;
    }

    inline bool is_leap()const{return leap;}

    inline string get_date()const{
        char buf[16];
        sprintf(buf,"%d-%d-%d",year,month,day);
        string s = buf;
        return s;
    }

    inline string get_time()const{
        char buf[16];
        sprintf(buf,"%d:%d:%d",hour,minute,second);
        string s = buf;
        return s;
    }

private:

    int total_days;
    int total_seconds;

    bool leap;

    int week_day;

    int year;
    int month;
    int day;

    int hour;
    int minute;
    int second;
    int micro_second;

    void __cac_week_day(){
        week_day = total_days % 7;
    }

    void __cac_is_leap(){
        leap = (( year % 4 == 0 && year % 100 != 0 ) || year % 400 == 0);
    }

    void __days_to_ymd(){
        // 计算年份
        year = total_days/365.2425;
        int y400 = year/400;
        int yresi = year - y400*400;
        int y100 = yresi/100;
        yresi -= y100*100;
        int y4 = yresi/4;
        yresi -= y4*4;
        int recent_days = total_days - (y400 * 146097 + y100 * 36524 + y4 * 1461 + yresi * 365);
        year ++;

        __cac_is_leap();

        // 计算月份
        month = 1;
        if(leap)
            while(recent_days > MONTH_DAY_LEAP_YEAR[month]){
                recent_days -= MONTH_DAY_LEAP_YEAR[month];
                month++;
            }
        else
            while(recent_days > MONTH_DAY_COMMON_YEAR[month]){
                recent_days -= MONTH_DAY_COMMON_YEAR[month];
                month++;
            }

        // 日期
        day = recent_days;
        __cac_week_day();
    }

    void __ymd_to_days(){

        __cac_is_leap();

        int y400 = (year-1)/400;
        int yresi = year-1 - y400*400;
        int y100 = yresi/100;
        yresi -= y100*100;
        int y4 = yresi/4;
        yresi -= y4*4;

        total_days = y400 * 146097 + y100 * 36524 + y4 * 1461 + yresi * 365;

        if(leap)
            for(int i = 1;i < month; i++)total_days += MONTH_DAY_LEAP_YEAR[i];
        else
            for(int i = 1;i < month; i++)total_days += MONTH_DAY_COMMON_YEAR[i];

        total_days += day;

        __cac_week_day();
    }

    void __seconds_to_hms(){
        int recent_ms;
        hour = total_seconds/3600000;

        recent_ms = total_seconds - hour * 3600000;
        minute = recent_ms / 60000;

        recent_ms -= minute * 60000;
        second = recent_ms / 1000;

        micro_second = recent_ms - second * 1000;
    }

    void __hms_to_seconds(){
        total_seconds = hour * 3600000 + minute * 60000 + second * 1000 + micro_second;
    }

    /* 使时间参数合乎规格 */
    void __correct_date(){
        if(year <= 0){
            year = 1;
            leap = false;
        }
        else
            __cac_is_leap();

        if(month > 12 || month < 1) month = (month - 1)%12 + 1;

        if(leap)
            if(day < 1 || day > MONTH_DAY_LEAP_YEAR[month])
                day = (day - 1)%MONTH_DAY_LEAP_YEAR[month] + 1;

        else
            if(day < 1 || day > MONTH_DAY_COMMON_YEAR[month])
                day = (day - 1)%MONTH_DAY_COMMON_YEAR[month] + 1;

        if(hour < 0 || hour >= 24)
            hour = 0;
        if(minute < 0 || minute >= 60)
            minute = 0;
        if(second < 0 || second >= 60)
            second = 0;
        if(micro_second < 0 || micro_second >= 1000)
            micro_second = 0;
    }
};

#endif // TIME_HPP

