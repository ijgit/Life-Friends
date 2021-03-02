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


// state - func
void waiting_func();
void recommend_brush_func();
void detect_wash_func();
void detect_eating_func();
void detect_quit_eating_func();
void recommend_brush_func();
void detect_brush_func();
void recommend_sleep_brush_func();
void detect_sleep_brush_func();


void setup(){
    Serial.begin(115200); 
    SPI_setup();

    //while (!Serial) { delay(1); } // for debug
    delay(500);
    VS1053_setup();
    // printDirectory(SD.open("/"), 0);

    mpu_setup();
    delay(100);
    led_matrix.begin(0x70);
    ShowLed(SMILE ,led_matrix);
    delay(100);
    
    LoRaSetup();
    delay(1000);
    act_arr_init();
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
                Serial.println("IDLE");
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
    if(state != WAITING){
        if (rf95.available()){
            //Serial.println("rf 95 available");
            uint8_t rf_buf[RH_RF95_MAX_MESSAGE_LEN] = {0, };
            uint8_t rf_buf_len = sizeof(rf_buf);
            if (rf95.recv(rf_buf, &rf_buf_len)){
                char c = ((char *)rf_buf)[0];
                if(c == QUIT_FLAG){
                    state = WAITING;
                    detect_wash_init();
                    detect_brush_init();
                    act_arr_init();
                    ShowLed(SMILE ,led_matrix);
                    SPI_toLoRa();
                    Serial.println("Quit detecting");
                }
            }
        }
    }

    switch (state){

    case WAITING:
        waiting_func();                 break;
    
    case RECOMMEND_WASH:
        recommend_wash_func();          break;
    
    case DETECT_WASH:
        detect_wash_func();             break;
    
    case DETECT_EATING:
        detect_eating_func();           break;
    
    case DETECT_QUIT_EATING:
        detect_quit_eating_func();      break;
    
    case DETECT_BRUSH:
        detect_brush_func();            break;
    
    case RECOMMEND_BRUSH:
        recommend_brush_func();         break;
    
    case RECOMMEND_SLEEP_BRUSH:
        recommend_sleep_brush_func();   break;
    
    case DETECT_SLEEP_BRUSH:
        detect_sleep_brush_func();      break;
    
    default:
        break;
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

int check_cur_activation(int acts[], int pred[]){
    int cur_act = 0;

    for(int i=0; i<ACT_LABEL_LEN; i++) pred[i] =0;
    for (int i =0; i<ACTS_LIST_LEN; i++)
        pred[acts[i]]++;

        
    if((1 <= pred[EAT] && pred[EAT] < 3) && pred[IDLE] > ACTS_LIST_LEN / 2){
        cur_act = EAT;   
    }// activation = wash
    else if(pred[WASH] >= 0.7*ACTS_LIST_LEN ){
        cur_act = WASH;
    }else if(pred[BRUSH] >= 0.7*ACTS_LIST_LEN){
        cur_act = BRUSH;
    }else if(pred[DUMMY] >= 0.7*ACTS_LIST_LEN){
        cur_act = DUMMY;
    }else if(pred[IDLE] >= 0.7*ACTS_LIST_LEN){
        cur_act = IDLE;
    }else{
        cur_act = DUMMY;
    }        

    for (int i =0; i<ACT_LABEL_LEN; i++){
      Serial.print(activations[i]);
      Serial.print("\t");
    }
    
    for (int i = 0; i<ACT_LABEL_LEN; i++){
        Serial.print(pred[i]);
        Serial.print("\t");
        //cur_act = (pred[cur_act] > pred[i])? cur_act:i;
    }
    // cur_act = (pred[EAT] >= 3 && cur_act == IDLE)? EAT:cur_act;

    return cur_act;
}

//----------------------------------------
void waiting_func(){
    detect_flag = false;
    
    if (rf95.available()){
        //Serial.println("rf 95 available");
        uint8_t rf_buf[RH_RF95_MAX_MESSAGE_LEN] = {0, };
        uint8_t rf_buf_len = sizeof(rf_buf);
        if (rf95.recv(rf_buf, &rf_buf_len)){
            char c = ((char *)rf_buf)[0];
            act_arr_init();
            if(c == EATING_HABIT_FLAT){
                detect_flag = true;
                state = RECOMMEND_WASH;
                Serial.println("state: WAITING -> RECOMMEND_WASH");
            }else if (c == SLEEP_HABIT_FLAG){
                detect_flag = true;
                state = RECOMMEND_SLEEP_BRUSH;
                Serial.println("state: WAITING -> RECOMMEND_SLEEP_BRUSH");
            }
        }
    }
}

void recommend_wash_func(){
    num_recommend_wash++;
    // SPI TO SPEAKER

    if(num_recommend_wash == 2){
        ShowLed(FROWN ,led_matrix);
        SPI_toVS1053();
        delay(10);
        musicPlayer.playFullFile(RECOMMEND_WASH_MP3_3); // 손에는 더러운 세균이 아주 많아

        LoRa_sendResult((uint8_t*)"w0");
        
        //Serial.println("손에는 더러운 세균이 아주 많아"); 
        Serial.println("state: RECOMMEND_WASH -> WAITING");

        detect_wash_init();

        state = WAITING;             ShowLed(SMILE ,led_matrix);
        SPI_toLoRa();
        
    }else{
        if(num_recommend_wash == 0){
            ShowLed(HAND ,led_matrix);
            SPI_toVS1053();
            delay(10);
            musicPlayer.playFullFile(RECOMMEND_WASH_MP3_1);
            
            //Serial.println("손 씻자"); 
        }else if(num_recommend_wash == 1){
            ShowLed(FROWN ,led_matrix);
            SPI_toVS1053();
            delay(10);
            musicPlayer.playFullFile(RECOMMEND_WASH_MP3_2);
            //Serial.println("손 씻고 있어?"); 
            
            LoRa_sendResult((uint8_t*)"w0");
        }
        state = DETECT_WASH;
        Serial.println("state: RECOMMEND_WASH -> DETECT_WASH");
        act_arr_init();
        wash_timer = millis();
    } 
}

void detect_wash_func(){
    detect_flag = true;

    if( !washed && cur_act == WASH){    // 손을 씻은 것을 처음 감지함
        washed = true;

        ShowLed(SMILE ,led_matrix);
        SPI_toVS1053();
        delay(10);
        musicPlayer.playFullFile(DETECT_WASH_MP3_1);

        //Serial.println("잘 하고 있어"); 
    }
    else if(washed && cur_act != WASH){    // 손을 다 씻음
        ShowLed(SMILE ,led_matrix);
        SPI_toVS1053();
        delay(10);
        musicPlayer.playFullFile(DETECT_WASH_MP3_2);
        
        //Serial.println("손에 있는 세균들을 다 물리쳤구나!! 기특하다");
        Serial.println("state: DETECT_WASH -> DETECT_EATING");

        LoRa_sendResult((uint8_t*)"w1");
        state = DETECT_EATING;
    }
    else if(cur_act == EAT && pre_act == EAT){  // 손을 안 씻고 밥먹음 (조건은 더 생각할 필요 있음)
        ShowLed(FROWN ,led_matrix);
        SPI_toVS1053();
        delay(10);
        musicPlayer.playFullFile(NOT_DETECT_WASH_MP3);
        
        //Serial.println("다음에는 손을 씻고 밥을 먹자");
        Serial.println("state: DETECT_WASH -> DETECT_QUIT_EATING");

        LoRa_sendResult((uint8_t*)"w0");

        state = DETECT_QUIT_EATING;
    }
    else if(washed != true && millis() - wash_timer > WASH_TIME_LIMIT){
        //Serial.println("Time OVER !!!!! "); 
        Serial.println("state: DETECT_WASH -> RECOMMEND_WASH");
        state = RECOMMEND_WASH;
    }
}

void detect_eating_func(){
    if(cur_act == EAT && pre_act == EAT){
        Serial.println("state: DETECT_EATING -> DETECT_QUIT_EATING");
        state = DETECT_QUIT_EATING;
    }
}
void detect_quit_eating_func(){
    if(cur_act != EAT && pre_act != EAT){
        Serial.println("state: DETECT_QUIT_EATING -> RECOMMEND_BRUSH");
        state = RECOMMEND_BRUSH;
        num_recommend_brush = -1;
        brush_detected = 0;
    }
}
void recommend_brush_func(){
    num_recommend_brush++;
    if(num_recommend_brush == 2){
        ShowLed(FROWN ,led_matrix);

        SPI_toVS1053();
        delay(10);
        musicPlayer.playFullFile(RECOMMEND_BRUSH_MP3_3);

        //Serial.println("다음에는 꼭 닦자");
        Serial.println("state: RECOMMEND_BRUSH -> WAITING");

        SPI_toLoRa();
        LoRa_sendResult((uint8_t*)"b0");
        detect_brush_init();

        state = WAITING;             ShowLed(SMILE ,led_matrix);
        act_arr_init();
    }else{
        if(num_recommend_brush == 0){
            ShowLed(TOOTHBRUSH ,led_matrix);

            SPI_toVS1053();
            delay(10);
            musicPlayer.playFullFile(RECOMMEND_BRUSH_MP3_1);

            //Serial.println("양치하러 갈까?");
        }else if(num_recommend_brush == 1){
            ShowLed(FROWN ,led_matrix);

            SPI_toVS1053();
            delay(10);
            musicPlayer.playFullFile(RECOMMEND_BRUSH_MP3_2);

            //Serial.println("세균들이 이를 아프게 하고 있어");
            LoRa_sendResult((uint8_t*)"b0");
        }

        Serial.println("state: RECOMMEND_BRUSH -> DETECT_BRUSH");
        state = DETECT_BRUSH;
        brush_timer = millis();
        act_arr_init();
    }
}
void detect_brush_func(){
    if( !brushed && cur_act == BRUSH){
        brushed = true;    
        ShowLed(SMILE ,led_matrix);

        SPI_toVS1053();
        delay(10);
        musicPlayer.playFullFile(DTECT_BRUSH_MP3_1);

        //Serial.println("잘 하고 있어"); 
    }
    else if(brushed && cur_act != BRUSH){   
        ShowLed(SMILE ,led_matrix);
        SPI_toVS1053();
        delay(10);
        musicPlayer.playFullFile(DTECT_BRUSH_MP3_2);
        LoRa_sendResult((uint8_t*)"b1");

        
        //Serial.println("이 안의 세균들이 다 사라졌어!"); 
        Serial.println("state: DETECT_BRUSH -> WAITING");
    
        detect_brush_init();

        state = WAITING;             ShowLed(SMILE ,led_matrix);
        SPI_toLoRa();
        act_arr_init();
    }
    else if(brushed != true && millis() - brush_timer > BRUSH_TIME_LIMIT){
            
        Serial.println("state: DETECT_BRUSH -> RECOMMEND_BRUSH");
        state = RECOMMEND_BRUSH;
    }
}

void recommend_sleep_brush_func(){
    num_recommend_brush++;
    if(num_recommend_brush == 2){
        ShowLed(FROWN ,led_matrix);

        SPI_toVS1053();
        delay(10);
        musicPlayer.playFullFile(RECOMMEND_SLEEP_BRUSH_MP3_3);

        Serial.println("state: RECOMMEND_BRUSH -> WAITING");

        SPI_toLoRa();
        LoRa_sendResult((uint8_t*)"s0");
        detect_brush_init();

        state = WAITING;             ShowLed(SMILE ,led_matrix);
        act_arr_init();
    }else{
        if(num_recommend_brush == 0){
            ShowLed(TOOTHBRUSH ,led_matrix);

            SPI_toVS1053();
            delay(10);
            musicPlayer.playFullFile(RECOMMEND_SLEEP_BRUSH_MP3_1);

            //Serial.println("양치하러 갈까?");
        }else if(num_recommend_brush == 1){
            ShowLed(FROWN ,led_matrix);

            SPI_toVS1053();
            delay(10);
            musicPlayer.playFullFile(RECOMMEND_SLEEP_BRUSH_MP3_2);

            //Serial.println("세균들이 이를 아프게 하고 있어");
            LoRa_sendResult((uint8_t*)"s0");
        }

        Serial.println("state: RECOMMEND_BRUSH -> DETECT_BRUSH");
        state = DETECT_SLEEP_BRUSH;
        brush_timer = millis();
        act_arr_init();
    }
}
void detect_sleep_brush_func(){
    if( !brushed && cur_act == BRUSH){
        brushed = true;    
        ShowLed(SMILE ,led_matrix);

        SPI_toVS1053();
        delay(10);
        musicPlayer.playFullFile(DTECT_SLEEP_BRUSH_MP3_1);

        //Serial.println("잘 하고 있어"); 
    }
    else if(brushed && cur_act != BRUSH){   
        ShowLed(SMILE ,led_matrix);
        SPI_toVS1053();
        delay(10);
        musicPlayer.playFullFile(DTECT_SLEEP_BRUSH_MP3_2);
        LoRa_sendResult((uint8_t*)"s1");

        
        //Serial.println("이 안의 세균들이 다 사라졌어!"); 
        Serial.println("state: DETECT_BRUSH -> WAITING");
    
        detect_brush_init();

        state = WAITING;             ShowLed(SMILE ,led_matrix);
        SPI_toLoRa();
        act_arr_init();
    }
    else if(brushed != true && millis() - brush_timer > SLEEP_BRUSH_TIME_LIMIT){
            
        Serial.println("state: DETECT_BRUSH -> RECOMMEND_BRUSH");
        state = RECOMMEND_SLEEP_BRUSH;
    }
}
