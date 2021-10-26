#ifndef SPIMODULE_H
#define SPIMODULE_H
 
#include <SPI.h>
#include <SD.h>
#include <Adafruit_VS1053.h>
#include <RH_RF95.h>


// These are the pins used
#define VS1053_RESET   -1     // VS1053 reset pin (not used!)

#define VS1053_CS       6     // VS1053 chip select pin (output)
#define VS1053_DCS     10     // VS1053 Data/command select pin (output)
#define SD_CS          5     // Card chip select pin
#define VS1053_DREQ     9     // VS1053 Data request, ideally an Interrupt pin


#define LoRa_CS         8

#define RECOMMEND_WASH_MP3_1    "Rwash1.mp3"
#define RECOMMEND_WASH_MP3_2    "Rwash2.mp3"
#define RECOMMEND_WASH_MP3_3    "Rwash3.mp3"
#define DETECT_WASH_MP3_1       "Dwash1.mp3"
#define DETECT_WASH_MP3_2       "Dwash2.mp3"
#define NOT_DETECT_WASH_MP3     "Nwash.mp3"

#define RECOMMEND_BRUSH_MP3_1    "Rbrush1.mp3"
#define RECOMMEND_BRUSH_MP3_2    "Rbrush2.mp3"
#define RECOMMEND_BRUSH_MP3_3    "Rbrush3.mp3"
#define DTECT_BRUSH_MP3_1        "Dbrush1.mp3"
#define DTECT_BRUSH_MP3_2        "Dbrush2.mp3"

#define RECOMMEND_SLEEP_BRUSH_MP3_1    "Rsbrush1.mp3"
#define RECOMMEND_SLEEP_BRUSH_MP3_2    "Rsbrush2.mp3"
#define RECOMMEND_SLEEP_BRUSH_MP3_3    "Rsbrush3.mp3"
#define DTECT_SLEEP_BRUSH_MP3_1        "Dsbrush1.mp3"
#define DTECT_SLEEP_BRUSH_MP3_2        "Dsbrush2.mp3"





void VS1053_setup();
void SPI_setup();
void SPI_toLoRa();
void SPI_toVS1053();
void printDirectory(File dir, int numTabs);

RH_RF95 rf95(8, 3); // Adafruit Feather M0 with RFM95

Adafruit_VS1053_FilePlayer musicPlayer = 
  Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, SD_CS);


void VS1053_setup(){
    SPI_toVS1053();
    delay(10);
    if(! musicPlayer.begin()){
        Serial.println("Couldn't find VS1053");
    }else{
        Serial.println("VS1053 begin success");
    }
    musicPlayer.sineTest(0x44, 500);
   

    if (!SD.begin(SD_CS)) {
        Serial.println("VS1053 SD failed");
        while(1);        
    }
    else Serial.println("VS1053 SD suceess");

    musicPlayer.setVolume(10,10);
    musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT); // DREQ int
    // startPlayingFile
    // playFullFile
}


void LoRaSetup(){
    SPI_toLoRa();
    if (!rf95.init())
        Serial.println("LoRa init failed");
    else
        Serial.println("LoRa init success");
    // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

    rf95.setTxPower(23, false);
    rf95.setFrequency(923.0);
}


void SPI_setup(){
    pinMode(LoRa_CS, INPUT_PULLUP);
    //pinMode(SD_CS, INPUT_PULLUP);
}

void SPI_toLoRa(){
    digitalWrite(LoRa_CS, LOW);
    //digitalWrite(SD_CS, HIGH);
    //Serial.println("SPI to LoRa");
}


// for lora

void LoRa_sendResult(uint8_t* data){
    SPI_toLoRa();
    delay(10);
    rf95.send(data, sizeof(data));
}

void SPI_toVS1053(){
    //digitalWrite(SD_CS, LOW);
    digitalWrite(LoRa_CS, HIGH);
    //Serial.println("SPI to VS1053");
}



void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}

#endif
