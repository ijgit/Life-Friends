#ifndef MPUDATA
#define MPUDATA

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <math.h>

class Mpu_data{
public:
    Mpu_data();
    Mpu_data(const Mpu_data &src);
    void parsing_buf_to_data(char *buf);
    float accx, accy, accz;
    float gyx, gyy, gyz;
};

typedef struct Feature{
    float acc_vecter_avg = 0, accx_avg = 0, accy_avg = 0, accz_avg = 0;
    float abs_accx_avg = 0, abs_accy_avg = 0, abs_accz_avg = 0;
    float accx_std = 0, accy_std = 0, accz_std = 0;
    float accxy_cov = 0, accyz_cov = 0, acczx_cov = 0;
    float accxy_corr = 0, accyz_corr = 0, acczx_corr = 0;


    float gyx_avg = 0, gyy_avg = 0, gyz_avg = 0;
    float abs_gyx_avg = 0, abs_gyy_avg = 0, abs_gyz_avg = 0;
    float gyx_std = 0, gyy_std = 0, gyz_std = 0;
    float gyxy_cov = 0, gyyz_cov = 0, gyzx_cov = 0;
    float gyxy_corr = 0, gyyz_corr = 0, gyzx_corr = 0;
    // 25
}Feature;

Mpu_data::Mpu_data(){
    accx = 0;
    accy = 0;
    accz = 0;
    gyx = 0;
    gyy = 0;
    gyz = 0;
}

Mpu_data::Mpu_data(const Mpu_data &src){
    accx = src.accx;
    accy = src.accy;
    accz = src.accz;
    gyx = src.gyx;
    gyy = src.gyy;
    gyz = src.gyz;
}

void Mpu_data::parsing_buf_to_data(char *buf){
    int n_idx = 0;
    char *ptr = strtok(buf, ",");
    accx = atof(ptr);

    ptr = strtok(NULL, ",");
    accy = atof(ptr);

    ptr = strtok(NULL, ",");
    accz = atof(ptr);

    ptr = strtok(NULL, ",");
    gyx = atof(ptr);

    ptr = strtok(NULL, ",");
    gyy = atof(ptr);

    ptr = strtok(NULL, ",");
    gyz = atof(ptr);
}

void calc_avg(Mpu_data *data, int len, Feature *f);
void calc_std(Mpu_data *data, int len, Feature *f);
void calc_cov(Mpu_data *data, int len, Feature *f);
void calc_corr(Mpu_data *data, int len, Feature *f);
void cal_features(Mpu_data *data, int len, Feature *f);
void featureToString(Feature f, char *str);

void calc_avg(Mpu_data *data, int len, Feature *f){
    float acc_vecter_avg = 0, accx_avg = 0, accy_avg = 0, accz_avg = 0;
    float gyx_avg = 0, gyy_avg = 0, gyz_avg = 0;

    for (int i = 0; i < len; i++)
    {
        float vector = sqrt(pow(data[i].accx, 2) + pow(data[i].accy, 2) + pow(data[i].accz, 2));
        acc_vecter_avg += vector;

        accx_avg += data[i].accx;
        accy_avg += data[i].accy;
        accz_avg += data[i].accz;

        gyx_avg += data[i].gyx;
        gyy_avg += data[i].gyy;
        gyz_avg += data[i].gyz;
    }
    (*f).acc_vecter_avg = acc_vecter_avg / len;
    (*f).accx_avg = accx_avg / len;
    (*f).accy_avg = accy_avg / len;
    (*f).accz_avg = accz_avg / len;
    (*f).gyx_avg = gyx_avg / len;
    (*f).gyy_avg = gyy_avg / len;
    (*f).gyz_avg = gyz_avg / len;
}

void calc_std(Mpu_data *data, int len, Feature *f){
    float accx = 0, accy = 0, accz = 0;
    float gyx = 0, gyy = 0, gyz = 0;
    for (int i = 0; i < len; i++)
    {
        accx += pow((*f).accx_avg - data[i].accx, 2);
        accy += pow((*f).accy_avg - data[i].accy, 2);
        accz += pow((*f).accz_avg - data[i].accz, 2);

        gyx += pow((*f).gyx_avg - data[i].gyx, 2);
        gyy += pow((*f).gyy_avg - data[i].gyy, 2);
        gyz += pow((*f).gyz_avg - data[i].gyz, 2);
    }
    (*f).accx_std = sqrt(accx / len);
    (*f).accy_std = sqrt(accy / len);
    (*f).accz_std = sqrt(accz / len);

    (*f).gyx_std = sqrt(gyx / len);
    (*f).gyy_std = sqrt(gyy / len);
    (*f).gyz_std = sqrt(gyz / len);
}

void calc_cov(Mpu_data *data, int len, Feature *f){
    float acc_xy = 0, acc_xz = 0, acc_yz = 0;
    float gy_xy = 0, gy_xz = 0, gy_yz = 0;

    for (int i = 0; i < len; i++)
    {
        acc_xy += ((*f).accx_avg - data[i].accx) * ((*f).accy_avg - data[i].accy);
        acc_xz += ((*f).accx_avg - data[i].accx) * ((*f).accz_avg - data[i].accz);
        acc_yz += ((*f).accy_avg - data[i].accy) * ((*f).accz_avg - data[i].accz);
        gy_xy += ((*f).gyx_avg - data[i].gyx) * ((*f).gyy_avg - data[i].gyy);
        gy_xz += ((*f).gyx_avg - data[i].gyx) * ((*f).gyz_avg - data[i].gyz);
        gy_yz += ((*f).gyy_avg - data[i].gyy) * ((*f).gyz_avg - data[i].gyz);
        
    }
    (*f).accxy_cov = acc_xy / len;
    (*f).acczx_cov = acc_xz / len;
    (*f).accyz_cov = acc_yz / len;
    (*f).gyxy_cov = gy_xy / len;
    (*f).gyzx_cov = gy_xz / len;
    (*f).gyyz_cov = gy_yz / len;
}

void calc_corr(Mpu_data *data, int len, Feature *f){

    (*f).accxy_corr = (*f).accxy_cov / ((*f).accx_std * (*f).accy_std);
    (*f).accyz_corr = (*f).accyz_cov / ((*f).accy_std * (*f).accz_std);
    (*f).acczx_corr = (*f).acczx_cov / ((*f).accx_std * (*f).accz_std);
    (*f).gyxy_corr = (*f).gyxy_cov / ((*f).gyx_std * (*f).gyy_std);
    (*f).gyyz_corr = (*f).gyyz_cov / ((*f).gyy_std * (*f).gyz_std);
    (*f).gyzx_corr = (*f).gyzx_cov / ((*f).gyx_std * (*f).gyz_std);
}

void cal_features(Mpu_data *data, int len, Feature *f)
{
    calc_avg(data, len, f);
    calc_std(data, len, f);
    calc_cov(data, len, f);
    calc_corr(data, len, f);
}

void featureToString(Feature f, char *str)
{
    int c_idx = 0;
    c_idx = sprintf(str, "%.2f,%.2f,%.2f,%.2f", f.acc_vecter_avg, f.accx_avg, f.accy_avg, f.accz_avg);
    c_idx += sprintf((str + c_idx), ",%.2f,%.2f,%.2f", f.accx_std, f.accy_std, f.accz_std);
    c_idx += sprintf((str + c_idx), ",%.2f,%.2f,%.2f", f.accxy_cov, f.accyz_cov, f.acczx_cov);
    c_idx += sprintf((str + c_idx), ",%.2f,%.2f,%.2f", f.accxy_corr, f.accyz_corr, f.acczx_corr);
    c_idx += sprintf((str + c_idx), ",%.2f,%.2f,%.2f", f.gyx_avg, f.gyy_avg, f.gyz_avg);
    c_idx += sprintf((str + c_idx), ",%.2f,%.2f,%.2f", f.gyx_std, f.gyy_std, f.gyz_std);
    c_idx += sprintf((str + c_idx), ",%.2f,%.2f,%.2f", f.gyxy_cov, f.gyyz_cov, f.gyzx_cov);
    c_idx += sprintf((str + c_idx), ",%.2f,%.2f,%.2f", f.gyxy_corr, f.gyyz_corr, f.gyzx_corr);
}


#endif