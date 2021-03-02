#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include "Mpu_data.h"
 

#define FILENAME "../datasample/0722/swing_0722_5"

char input_file[50] = {0, };
char output_file[50] = {0, };


int chk_overthr(Mpu_data* data, int idx, int win_size);
/*

data: 
    acc_x, acc_y, acc_z, gy_x, gy_y, gy_z

1. reading data
2. cut_data as 10
3. copy data to window
4. if threshold down -> quit  
*/

#define OVER_THR 10.5
#define UNDER_THR 8 
#define THR_N 3
#define WIN_SIZE 40


#define BUFSIZE 128
char buf[BUFSIZE] = {0, };



int main(void){

    printf("start\n");
    
    sprintf(input_file, "%s.CSV", FILENAME);
    sprintf(output_file, "%s._out.CSV", FILENAME);

    FILE *fp;
    fp = fopen(input_file, "r+");
    if(fp == NULL) return 0;

    FILE *fp_out;
    fp_out = fopen(output_file, "w+");
    if(fp_out == NULL) return 0;




    unsigned long l_idx = -1;
    unsigned int w1_idx = 0;
    unsigned int w2_idx = 0;
    Mpu_data temp[WIN_SIZE];
    Mpu_data window[2][WIN_SIZE];
    
    bool fst_window = true;


    printf("read file\n");

    Feature f;
    char fstr[BUFSIZE] = {0, };

    while(!feof(fp)){
        fgets(buf, BUFSIZE, fp);
        l_idx++;

        temp[l_idx].parsing_buf_to_data(buf);

        // copy data to window 1, 2
        if(l_idx == WIN_SIZE/2-1){
            for(int i =0; i<= l_idx; i++){            // i: 0~9
                window[0][i] = temp[i];               // (0~9) <- (0~9)
                window[1][WIN_SIZE/2 + i] = temp[i];  // (10~19) <- (0~9)
            }
        }else if(l_idx == WIN_SIZE-1){              
            for(int i = WIN_SIZE/2; i<= l_idx; i++){  // i: 10~19
                window[1][i-(WIN_SIZE/2)] = temp[i];  // (0~9) <- (10~19)
                window[0][i] = temp[i];               // (10~19) <- (10~19)    
            }
            fst_window = false;
        }


        // check threshold - lastest 10 value
        int cnt = 0;
        if((l_idx+1)%10 == 0){
            for(int i =0; i<WIN_SIZE/2; i++){
                int i_ = (l_idx - i + WIN_SIZE)%WIN_SIZE;
                float vval = sqrt(pow(temp[i_].accx, 2) + pow(temp[i_].accy, 2) + pow(temp[i_].accz, 2));
                if(vval > OVER_THR || vval < UNDER_THR){
                    cnt++;
                } 
            }
        }


        // calcurate feature value
        if(cnt >= THR_N){
            if(l_idx == WIN_SIZE-1){    
                //calcurate feature about window 1
                cal_features(window[0], WIN_SIZE, &f);
                featureToString(f, fstr);
                fprintf(fp_out, "%s\n", fstr);
                printf("%s\n", fstr);
            }
            if(fst_window==false && l_idx == WIN_SIZE/2-1){
                // calcurate feature about window 2
                cal_features(window[1], WIN_SIZE, &f);
                featureToString(f, fstr);
                fprintf(fp_out, "%s\n", fstr);
                printf("%s\n", fstr);
            }
        }
        
        // reset arr idx
        if(l_idx == WIN_SIZE-1)
            l_idx = -1;

    }

    fclose(fp);
    fclose(fp_out);
    return 0;
}


int chk_overthr(Mpu_data* data, int idx, int win_size){
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