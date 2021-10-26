#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "Mpu_data.h"
#include "Sampling.h"
#include "detection.h"
#include "model.h"
#include "SPIRelative.h"
#include "ledmatrix.h"

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

Adafruit_8x8matrix led_matrix = Adafruit_8x8matrix();




void setup(){
    Serial.begin(115200);
    while (!Serial) { delay(1); } // for debug


    SPI_setup();
    delay(1000);
    VS1053_setup();
    delay(100);
    printDirectory(SD.open("/"), 0);


    mpu_setup();
    delay(100);
    led_matrix.begin(0x70);
    delay(100);


    LoRaSetup();
    delay(1000);
}

sensors_event_t a, g, t;    
void loop(){

    if(detect_flag){
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
                Serial.print("activation: ");
                Serial.println(activations[cur_act]);
        
                act_idx = -1;   // 최근 행동 저장 인덱스 초기화 
            }
        }
    }


    // lora server get message -> state = recoment wash


    if(state == WAITING){
        detect_flag = false;
        
        if (rf95.available()){
            Serial.println("rf 95 available");
            uint8_t rf_buf[RH_RF95_MAX_MESSAGE_LEN] = {0, };
            uint8_t rf_buf_len = sizeof(rf_buf);
            if (rf95.recv(rf_buf, &rf_buf_len)){
                Serial.println((char*)rf_buf);
                char c = ((char *)rf_buf)[0];
                if(c == '1'){
                    Serial.println(c);
                    state = RECOMMEND_WASH;
                    Serial.println("state: WAITING -> RECOMMEND_WASH");
                }
            }
            else{
                Serial.println("recv failed");
            }
        }
    }


    else if(state == RECOMMEND_WASH){
        num_recommend_wash++;
        // SPI TO SPEAKER

        if(num_recommend_wash == 2){
            ShowLed(FROWN ,led_matrix);
            SPI_toVS1053();
            delay(10);
            musicPlayer.playFullFile(RECOMMEND_WASH_MP3_3); // 손에는 더러운 세균이 아주 많아

            LoRa_sendResult((uint8_t*)"wash,0");
            
            Serial.println("손에는 더러운 세균이 아주 많아"); 
            Serial.println("state: RECOMMEND_WASH -> WAITING");

            detect_wash_init();

            state = WAITING;
            SPI_toLoRa();
            
        }else{
            if(num_recommend_wash == 0){
                ShowLed(HAND ,led_matrix);
                SPI_toVS1053();
                delay(10);
                musicPlayer.playFullFile(RECOMMEND_WASH_MP3_1);
                
                Serial.println("손 씻자"); 
            }else if(num_recommend_wash == 1){
                ShowLed(FROWN ,led_matrix);
                SPI_toVS1053();
                delay(10);
                musicPlayer.playFullFile(RECOMMEND_WASH_MP3_2);
                LoRa_sendResult((uint8_t*)"wash,0");
                Serial.println("손 씻고 있어?"); 
            }
            state = DETECT_WASH;
            Serial.println("state: RECOMMEND_WASH -> DETECT_WASH");
            wash_timer = millis();
        } 
    }


    else if(state == DETECT_WASH){
        detect_flag = true;

        if( !washed && cur_act == WASH){    // 손을 씻은 것을 처음 감지함
            washed = true;

            ShowLed(SMILE ,led_matrix);
            SPI_toVS1053();
            delay(10);
            musicPlayer.playFullFile(DETECT_WASH_MP3_1);

            Serial.println("잘 하고 있어"); 
        }
        else if(washed && cur_act != WASH){    // 손을 다 씻음
            ShowLed(SMILE ,led_matrix);
            SPI_toVS1053();
            delay(10);
            musicPlayer.playFullFile(DETECT_WASH_MP3_2);
          
            LoRa_sendResult((uint8_t*)"wash,1");
            
            Serial.println("손에 있는 세균들을 다 물리쳤구나!! 기특하다");
            Serial.println("state: DETECT_WASH -> DETECT_EATING");
            
            state = DETECT_EATING;
        }
        else if(cur_act == EAT && pre_act == EAT){  // 손을 안 씻고 밥먹음 (조건은 더 생각할 필요 있음)
            ShowLed(FROWN ,led_matrix);
            SPI_toVS1053();
            delay(10);
            musicPlayer.playFullFile(NOT_DETECT_WASH_MP3);
          
            LoRa_sendResult((uint8_t*)"wash,0");
            
            Serial.println("다음에는 손을 씻고 밥을 먹자");
            Serial.println("state: DETECT_WASH -> DETECT_QUIT_EATING");
            state = DETECT_QUIT_EATING;
        }
        if(washed != true && millis() - wash_timer > WASH_TIME_LIMIT){
            Serial.println("Time OVER !!!!! "); 
            Serial.println("state: DETECT_WASH -> RECOMMEND_WASH");
            state = RECOMMEND_WASH;

        }
    }

    else if(state == DETECT_EATING){
        if(cur_act == EAT && pre_act == EAT){
            Serial.println("state: DETECT_EATING -> DETECT_QUIT_EATING");
            state = DETECT_QUIT_EATING;
        }

    }


    else if(state == DETECT_QUIT_EATING){
        if(cur_act != EAT && pre_act != EAT){
            Serial.println("state: DETECT_QUIT_EATING -> RECOMMEND_BRUSH");
            state = RECOMMEND_BRUSH;
            num_recommend_brush = -1;
            brush_detected = 0;
        }
    }
    
    else if(state == RECOMMEND_BRUSH){
        num_recommend_brush++;
        if(num_recommend_brush == 2){
            ShowLed(FROWN ,led_matrix);

            SPI_toVS1053();
            delay(10);
            musicPlayer.playFullFile(RECOMMEND_BRUSH_MP3_3);

            LoRa_sendResult((uint8_t*)"brush,0");

            Serial.println("다음에는 꼭 닦자");
            Serial.println("state: RECOMMEND_BRUSH -> WAITING");

            detect_brush_init();

            state = WAITING;
            SPI_toLoRa();
        }else{
            if(num_recommend_brush == 0){
                ShowLed(TOOTHBRUSH ,led_matrix);

                SPI_toVS1053();
                delay(10);
                musicPlayer.playFullFile(RECOMMEND_BRUSH_MP3_1);

                Serial.println("양치하러 갈까?");
            }else if(num_recommend_brush == 1){
                ShowLed(FROWN ,led_matrix);

                SPI_toVS1053();
                delay(10);
                musicPlayer.playFullFile(RECOMMEND_BRUSH_MP3_2);

                LoRa_sendResult((uint8_t*)"brush,0");

                Serial.println("세균들이 이를 아프게 하고 있어");
            }

            Serial.println("state: RECOMMEND_BRUSH -> DETECT_BRUSH");
            state = DETECT_BRUSH;
            brush_timer = millis();
        }
    }

    else if(state == DETECT_BRUSH){

        if( !brushed && cur_act == BRUSH){
            brushed = true;    
            ShowLed(SMILE ,led_matrix);

            SPI_toVS1053();
            delay(10);
            musicPlayer.playFullFile(DTECT_BRUSH_MP3_1);

            Serial.println("잘 하고 있어"); 
        }
        else if(brushed && cur_act != BRUSH){   
            ShowLed(SMILE ,led_matrix);
            SPI_toVS1053();
            delay(10);
            musicPlayer.playFullFile(DTECT_BRUSH_MP3_2);
            LoRa_sendResult((uint8_t*)"brush,1");

          
            Serial.println("이 안의 세균들이 다 사라졌어!"); 
            Serial.println("state: DETECT_BRUSH -> WAITING");

            detect_brush_init();

            state = WAITING;
            SPI_toLoRa();
        }
        if(brushed != true && millis() - brush_timer > BRUSH_TIME_LIMIT){
            
            Serial.println("state: DETECT_BRUSH -> RECOMMEND_BRUSH");
            state = RECOMMEND_BRUSH;
            
        }
    } 
}


void mpu_setup(){
    if (!mpu.begin(0x69)){
        Serial.println("mpu setup fail");
        while (1){
            delay(10);
        }
    }else{
      Serial.println("mpu setup sucess");
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