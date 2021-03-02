#ifndef SAMPLING
#define SAMPLING

#include "Mpu_data.h"


// Sampling
#define OVER_THR 10.5
#define UNDER_THR 8 
#define THR_N 3
#define WIN_SIZE 20


int chk_overthr(Mpu_data* data, int idx);
void cpy_data_to_window(Mpu_data *temp, Mpu_data arr[][WIN_SIZE], int l_idx);
bool calc_feature_in_window(int l_idx, Mpu_data arr[][WIN_SIZE], Feature* f);


bool fst_window = true;




int chk_overthr(Mpu_data* data, int idx){
    int cnt = 0;
    if((idx+1)%WIN_SIZE == 0){
        for(int i =0; i<WIN_SIZE/2; i++){
            int i_ = (idx - i + WIN_SIZE)%WIN_SIZE;
            float vval = sqrt(pow(data[i_].accx, 2) + pow(data[i_].accy, 2) + pow(data[i_].accz, 2));
            if(vval > OVER_THR || vval < UNDER_THR){
                cnt++;
            } 
        }
    }
    return cnt;
}



void cpy_data_to_window(Mpu_data *temp, Mpu_data arr[][WIN_SIZE], int l_idx){
    if(l_idx == WIN_SIZE/2-1){
        for(int i =0; i<= l_idx; i++){            // i: 0~9
            arr[0][i] = temp[i];               // (0~9) <- (0~9)
            arr[1][WIN_SIZE/2 + i] = temp[i];  // (10~19) <- (0~9)
        }
    }else if(l_idx == WIN_SIZE-1){              
        for(int i = WIN_SIZE/2; i<= l_idx; i++){  // i: 10~19
            arr[1][i-(WIN_SIZE/2)] = temp[i];  // (0~9) <- (10~19)
            arr[0][i] = temp[i];               // (10~19) <- (10~19)    
        }
    }
}




bool calc_feature_in_window(int l_idx, Mpu_data arr[][WIN_SIZE], Feature* f){
    bool cal_flag = true; 
    if(l_idx == WIN_SIZE-1){    
        //calcurate feature about window 1
        cal_features(arr[0], WIN_SIZE, f);
        fst_window = false;
    }else if(fst_window==false && l_idx == WIN_SIZE/2-1){
        // calcurate feature about window 2
        cal_features(arr[1], WIN_SIZE, f);
    }else{
        cal_flag = false;
    }
    return cal_flag;
}
#endif
