#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "Mpu_data.h"
#include "Sampling.h"
#include "detection.h"
#include "model.h"
#include "SPI_Module.h"


// MPU Module
#define MPU_SENSING_INTERVAL 50
Adafruit_MPU6050 mpu;
unsigned long mpu_sensing_time = 0;

#define FEATURE_LEN 16  // 특성 사용 개수

void readData(Mpu_data* data, sensors_event_t a, sensors_event_t g);
void mpu_setup();
void fts_put_data(double *fts, Feature f);
int check_cur_activation(int acts[], int pred[]);
void detect_activation();


int l_idx = -1;
Mpu_data temp[WIN_SIZE];
Mpu_data window[2][WIN_SIZE];
Feature f;


uint8_t state = WAITING;


void setup(){
    Serial.begin(115200);
    while (!Serial) { delay(1); } // for debug
    
    mpu_setup();
    delay(1000);
}

sensors_event_t a, g, t;    
void loop(){

    if(millis() - mpu_sensing_time > MPU_SENSING_INTERVAL){
        mpu_sensing_time = millis();
        mpu.getEvent(&a, &g, &t);   
        readData(&temp[++l_idx], a, g);             // save data
        cpy_data_to_window(temp, window, l_idx);    // copy data to window 1, 2
    }

    if(l_idx != -1 && (l_idx+1)%WIN_SIZE == 0){
    
        int cnt = chk_overthr(temp, l_idx);         // check threshold - lastest 10 value
        if(cnt >= THR_N){
            double fts[FEATURE_LEN] = {0, };
            bool cal_flag = calc_feature_in_window(l_idx, window, &f);
            if(cal_flag){
                fts_put_data(fts, f);   // change structure to array             
                int predict_ = predict(fts);           
                Serial.println(classes[predict_]);
                latest_acts[(++act_idx)%ACTS_LIST_LEN] = predict_;  // save prediction result                
            }
        }else{
            latest_acts[(++act_idx)%ACTS_LIST_LEN] = IDLE;
        }

        if(l_idx == WIN_SIZE-1) // 가속도 센서 저장 window 인덱스 초기화
            l_idx = -1;

        if(act_idx == ACTS_LIST_LEN-1){  
            // Serial.println(latest_acts[0]);

            pre_act = cur_act;
            cur_act = check_cur_activation(latest_acts, pred_acts); // 최근 예측 값들에 기반해 행동 추측
            Serial.println(activations[cur_act]);
    
            act_idx = -1;   // 최근 행동 저장 인덱스 초기화 
        }
    }
}


void mpu_setup(){
    if (!mpu.begin(0x69)){
        Serial.println("map begin fail");
        while (1){
            delay(10);
        }
    }
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

void readData(Mpu_data* data, sensors_event_t a, sensors_event_t g){
    (*data).accx = a.acceleration.x;  
    (*data).accy = a.acceleration.y;
    (*data).accz = a.acceleration.z;
    (*data).gyx = g.gyro.x;
    (*data).gyy = g.gyro.y;
    (*data).gyz = g.gyro.z;
}


void fts_put_data(double *fts, Feature f){

    fts[0] = f.vector_avg; fts[1] = f.accx_avg; fts[2] = f.accy_avg; fts[3] = f.accz_avg;
    fts[4] = f.std_x; fts[5] = f.std_y; fts[6] = f.std_z;
    fts[7] = f.cov_xy; fts[8] = f.cov_yz; fts[9] = f.cov_zx;
    fts[10] = f.corr_xy; fts[11] = f.corr_yz; fts[12] = f.corr_zx;
    fts[13] = f.gyx_avg; fts[14] = f.gyy_avg; fts[15] = f.gyz_avg;
}




// ACT_LIST_LEN 이 20 이라는 가정 
int check_cur_activation(int acts[], int pred[]){
    int cur_act = 0;

    for(int i=0; i<ACT_LABEL_LEN; i++) pred[i] =0;
    for (int i =0; i<ACTS_LIST_LEN; i++)
        pred[acts[i]]++;

    if((2 < pred[EAT] && pred[EAT] < 7) && pred[IDLE] > ACTS_LIST_LEN / 2){
        cur_act = EAT;
    }else if(pred[WASH] > 0.8 * ACTS_LIST_LEN){    // wash > 4/5 * ACT_LIST_LEN  
        cur_act = WASH;
    }else if(pred[BRUSH] > 0.8 * ACTS_LIST_LEN){
        cur_act = BRUSH;
    }else if(pred[DUMMY] > 0.8 * ACTS_LIST_LEN){
        cur_act = DUMMY;
    }else if(pred[IDLE] > 0.8* ACTS_LIST_LEN){
        cur_act = IDLE;
    }else{
        cur_act = DUMMY;
    }        
    
    for (int i = 0; i<ACT_LABEL_LEN; i++){
        Serial.print(pred[i]);
        Serial.print(", ");
        //cur_act = (pred[cur_act] > pred[i])? cur_act:i;
    }
    // cur_act = (pred[EAT] >= 3 && cur_act == IDLE)? EAT:cur_act;

    return cur_act;
}
