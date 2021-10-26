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
    }
    (*f).acc_vecter_avg = acc_vecter_avg / len;
    (*f).accx_avg = accx_avg / len;
    (*f).accy_avg = accy_avg / len;
    (*f).accz_avg = accz_avg / len;
}

void calc_std(Mpu_data *data, int len, Feature *f){
    float accx = 0, accy = 0, accz = 0;
    float gyx = 0, gyy = 0, gyz = 0;
    for (int i = 0; i < len; i++)
    {
        accx += pow((*f).accx_avg - data[i].accx, 2);
        accy += pow((*f).accy_avg - data[i].accy, 2);
        accz += pow((*f).accz_avg - data[i].accz, 2);
    }
    (*f).accx_std = sqrt(accx / len);
    (*f).accy_std = sqrt(accy / len);
    (*f).accz_std = sqrt(accz / len);
}

void calc_cov(Mpu_data *data, int len, Feature *f){
    float acc_xy = 0, acc_xz = 0, acc_yz = 0;

    for (int i = 0; i < len; i++)
    {
        acc_xy += ((*f).accx_avg - data[i].accx) * ((*f).accy_avg - data[i].accy);
        acc_xz += ((*f).accx_avg - data[i].accx) * ((*f).accz_avg - data[i].accz);
        acc_yz += ((*f).accy_avg - data[i].accy) * ((*f).accz_avg - data[i].accz);
    }
    (*f).accxy_cov = acc_xy / len;
    (*f).acczx_cov = acc_xz / len;
    (*f).accyz_cov = acc_yz / len;
}

void calc_corr(Mpu_data *data, int len, Feature *f){

    (*f).accxy_corr = (*f).accxy_cov / ((*f).accx_std * (*f).accy_std);
    (*f).accyz_corr = (*f).accyz_cov / ((*f).accy_std * (*f).accz_std);
    (*f).acczx_corr = (*f).acczx_cov / ((*f).accx_std * (*f).accz_std);
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
}

#endif