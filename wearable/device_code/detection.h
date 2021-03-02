#ifndef ACT_H
#define ACT_H

#define WAITING 1
#define RECOMMEND_WASH 10
#define DETECT_WASH 11
#define DETECT_WASH_QUIT 12
#define FEEDBACK_WASH 13
//#define POSITIVE_FEEDBACK 6
#define DETECT_EATING 20
#define DETECT_QUIT_EATING 21

#define RECOMMEND_BRUSH 30
#define DETECT_BRUSH 31

#define RECOMMEND_SLEEP_BRUSH 40
#define DETECT_SLEEP_BRUSH 41


// LoRa flag
#define QUIT_FLAG '0'
#define EATING_HABIT_FLAT '1'
#define SLEEP_HABIT_FLAG '2'




// activation labels
#define ACT_LABEL_LEN 5
#define DUMMY 0
#define WASH 1
#define BRUSH 2
#define EAT 3
#define IDLE 4

char classes[ACT_LABEL_LEN-1][10] = {"DUMMY", "WASH", "BRUSH", "EAT"};
char activations[ACT_LABEL_LEN][10] = {"DUMMY", "WASH", "BRUSH", "EAT", "IDLE"};



#define ACTS_LIST_LEN 10
int latest_acts[ACTS_LIST_LEN] = {0, };    // 최근 행동 저장
int act_idx = -1;
int pred_acts[ACT_LABEL_LEN] = {0, };    // 행동에 대해 예측 결과 저장
int user_activation = 0;

int predResult_acts[3] = {0, }; // 음식 먹기 행위 (예측에 오래걸림) 을 위해 추가
/*
연속 행위 예측 = 먹기 일 때 사용자 행동을 먹기라고 정함 

*/


void act_arr_init(){
    for(int i =0; i<ACTS_LIST_LEN; i++){
        latest_acts[i] = 0;
    }
    act_idx = -1;
}



// 
bool detect_flag = 0;



// 했는지 안했는지 상태 저장
uint8_t pre_act = IDLE;
uint8_t cur_act = IDLE;
bool washed = 0;
bool brushed = 0;

#define WASH_TIME_LIMIT 30000
unsigned long wash_timer = 0;
int8_t num_recommend_wash = -1;
bool wash_detected = 0;


#define SLEEP_BRUSH_TIME_LIMIT 60000 // 시연을 위해 180000 // 3min
#define BRUSH_TIME_LIMIT 60000 // 시연을 위해 180000 // 3min
unsigned long brush_timer = 0;
int8_t num_recommend_brush = -1;
bool brush_detected = 0;





void detect_wash_init(){
    num_recommend_wash = -1;
    wash_detected = 0;
    washed = 0;
}

void detect_brush_init(){
    num_recommend_brush = -1;
    brush_detected = 0;
    brushed = 0;
}




#endif
