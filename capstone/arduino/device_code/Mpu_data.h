#ifndef MPUDATA
#define MPUDATA

class Mpu_data{
public:
    Mpu_data();
    Mpu_data(const Mpu_data &src);
    void parsing_buf_to_data(char *buf);
    float accx, accy, accz;
    float gyx, gyy, gyz;
};

typedef struct Feature{
    float vector_avg = 0, accx_avg = 0, accy_avg = 0, accz_avg = 0;
    float abs_accx_avg = 0, abs_accy_avg = 0, abs_accz_avg = 0;
    float gyx_avg = 0, gyy_avg = 0, gyz_avg = 0;
    float std_x = 0, std_y = 0, std_z = 0;
    float cov_xy = 0, cov_yz = 0, cov_zx = 0;
    float corr_xy = 0, corr_yz = 0, corr_zx = 0;
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


void calc_avg(Mpu_data *data, int len, Feature *f);
void calc_std(Mpu_data *data, int len, Feature *f);
void calc_cov(Mpu_data *data, int len, Feature *f);
void calc_corr(Mpu_data *data, int len, Feature *f);
void cal_features(Mpu_data *data, int len, Feature *f);
void featureToString(Feature f, char *str);

void calc_avg(Mpu_data *data, int len, Feature *f){
    float vector_avg = 0, accx_avg = 0, accy_avg = 0, accz_avg = 0;
    float gyx_avg = 0, gyy_avg = 0, gyz_avg = 0;

    for (int i = 0; i < len; i++)
    {
        float vector = sqrt(pow(data[i].accx, 2) + pow(data[i].accy, 2) + pow(data[i].accz, 2));
        vector_avg += vector;

        accx_avg += data[i].accx;
        accy_avg += data[i].accy;
        accz_avg += data[i].accz;

        gyx_avg += data[i].gyx;
        gyy_avg += data[i].gyy;
        gyz_avg += data[i].gyz;
    }
    (*f).vector_avg = vector_avg / len;
    (*f).accx_avg = accx_avg / len;
    (*f).accy_avg = accy_avg / len;
    (*f).accz_avg = accz_avg / len;
    (*f).gyx_avg = gyx_avg / len;
    (*f).gyy_avg = gyy_avg / len;
    (*f).gyz_avg = gyz_avg / len;
}

void calc_std(Mpu_data *data, int len, Feature *f){
    float x = 0, y = 0, z = 0;
    for (int i = 0; i < len; i++)
    {
        x += pow((*f).accx_avg - data[i].accx, 2);
        y += pow((*f).accy_avg - data[i].accy, 2);
        z += pow((*f).accz_avg - data[i].accz, 2);
    }
    (*f).std_x = sqrt(x / len);
    (*f).std_y = sqrt(y / len);
    (*f).std_z = sqrt(z / len);
}

void calc_cov(Mpu_data *data, int len, Feature *f){
    float xy = 0, xz = 0, yz = 0;

    for (int i = 0; i < len; i++)
    {
        xy += ((*f).accx_avg - data[i].accx) * ((*f).accy_avg - data[i].accy);
        xz += ((*f).accx_avg - data[i].accx) * ((*f).accz_avg - data[i].accz);
        yz += ((*f).accy_avg - data[i].accy) * ((*f).accz_avg - data[i].accz);
    }
    (*f).cov_xy = xy / len;
    (*f).cov_zx = xz / len;
    (*f).cov_yz = yz / len;
}

void calc_corr(Mpu_data *data, int len, Feature *f)
{
    float xy, yz, zx;

    (*f).corr_xy = (*f).cov_xy / ((*f).std_x * (*f).std_y);
    (*f).corr_yz = (*f).cov_yz / ((*f).std_y * (*f).std_z);
    (*f).corr_zx = (*f).cov_zx / ((*f).std_x * (*f).std_z);
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
    c_idx = sprintf(str, "%.2f,%.2f,%.2f,%.2f", f.vector_avg, f.accx_avg, f.accy_avg, f.accz_avg);
    c_idx += sprintf((str + c_idx), ",%.2f,%.2f,%.2f", f.std_x, f.std_y, f.std_z);
    c_idx += sprintf((str + c_idx), ",%.2f,%.2f,%.2f", f.cov_xy, f.cov_yz, f.cov_zx);
    c_idx += sprintf((str + c_idx), ",%.2f,%.2f,%.2f", f.corr_xy, f.corr_yz, f.corr_zx);
    c_idx += sprintf((str + c_idx), ",%.2f,%.2f,%.2f", f.gyx_avg, f.gyy_avg, f.gyz_avg);
}


#endif
