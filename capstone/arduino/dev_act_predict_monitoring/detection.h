#ifndef ACT_H
#define ACT_H

#define WAITING 1
#define CHK_RECOMMEND_WASH 10
#define RECOMMEND_WASH 11
#define DETECT_WASH 12
#define DETECT_WASH_QUIT 13
#define FEEDBACK_WASH 14
//#define POSITIVE_FEEDBACK 6
#define PREPARE_EATING_DETECT 20
#define DETECT_EATING 21
#define DETECT_QUIT_EATING 22
#define PREPARE_BRUSH_RECOMMEND 30
#define RECOMMEND_BRUSH 31
#define DETECT_BRUSH 32
#define FEEDBACK_BRUSH 33

// activation labels
#define ACT_LABEL_LEN 5
#define DUMMY 0
#define WASH 1
#define BRUSH 2
#define EAT 3
#define IDLE 4

char classes[ACT_LABEL_LEN-1][10] = {"DUMMY", "WASH", "BRUSH", "EAT"};
char activations[ACT_LABEL_LEN][10] = {"DUMMY", "WASH", "BRUSH", "EAT", "IDLE"};



#define ACTS_LIST_LEN 20
int latest_acts[ACTS_LIST_LEN] = {0, };    // 최근 행동 저장
int act_idx = -1;
int pred_acts[ACT_LABEL_LEN] = {0, };    // 행동에 대해 예측 결과 저장
int user_activation = 0;


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


#define BRUSH_TIME_LIMIT 180000 // 3min
unsigned long brush_timer = 0;
int8_t num_recommend_brush = -1;
bool brush_detected = 0;









// void chk_recommend_wash(){
//     // state 상태에 따라 다른 음성 출력
//     if(wash_detected == 0){
//         if(num_recommend_wash == 0){
//             // 손 씻기 권유
//         }
//         else if (num_recommend_wash == 1){
//             // 다시 씻기 권유 
//         }
//         num_recommend_wash++;
//         wash_timer = millis();
//         state = DETECT_WASH_HAND;
//     }
// }


// void feedback_wash_hand(){
//     if(wash_detected){
//         if(num_recommend_wash == 1){
//             ;// 
//         }
//     }else{
//         if(num_recommend_wash == 2){
//             ;// 
//         }
//     }else{
//         ; // 실망
//     }
// }

// void detect_wash_hand(){
//     // 모델의 최근 
//     while(millis() - wash_timer < WASH_TIME_LIMIT){
//         /*
//             가속도 값을 읽고 예측한다
//             연속으로 몇 번 이상 또는 최근 몇 번 이내에 감지 확인 -> 손을 씻은 것으로 확인

//             만약 손을 씻은 것이라면
//             -> 손을 씻지 않은 것 인식 (다 씻음)
//             -> while break && state 변환 (feedback)

//         */
//     }
// }








// // wash_neative_speak
// // wash_neative_led




#endif
