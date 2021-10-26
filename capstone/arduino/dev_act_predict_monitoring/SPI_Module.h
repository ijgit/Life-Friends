#ifndef SPIMODULE_H
#define SPIMODULE_H
 
#include <SPI.h>
#include <SD.h>
#include <Adafruit_VS1053.h>



// These are the pins used
#define VS1053_RESET   -1     // VS1053 reset pin (not used!)

#define VS1053_CS       6     // VS1053 chip select pin (output)
#define VS1053_DCS     10     // VS1053 Data/command select pin (output)
#define SD_CS          5     // Card chip select pin
#define VS1053_DREQ     9     // VS1053 Data request, ideally an Interrupt pin


#define LoRa_CS         8


#define RECOMMEND_WASH_MP3_1    "wash1.mp3"
#define RECOMMEND_WASH_MP3_2    "wash2.mp3"
#define DETECT_WASH_MP3         "wash3.mp3"
#define DETECT_END_WASH_MP3     "wash4.mp3"
#define NOT_WASH_EAT_DETECT_MP3 "wash5.mp3"
#define WASH_FAIL_MP3           "wash10.mp3"




void VS1053_setup();
void SPI_setup();
void SPI_toLoRa();
void SPI_toVS1053();
void printDirectory(File dir, int numTabs);


Adafruit_VS1053_FilePlayer musicPlayer = 
  Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, SD_CS);


/*
boolean startPlayingFile (char * trackname) 
: Interrupt Play file

boolean playFullFile (char * trackname)

*/



void VS1053_setup(){
    SPI_toVS1053();
    delay(10);
    if(! musicPlayer.begin()){
        Serial.println("Couldn't find VS1053");
    }

    musicPlayer.sineTest(0x44, 500);

    if (!SD.begin(SD_CS)) {
        Serial.println("SD failed");
    }
    Serial.println("SD OK!");

    musicPlayer.setVolume(20,0);
    musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);
}






void SPI_setup(){
    pinMode(LoRa_CS, INPUT_PULLUP);
    pinMode(SD_CS, INPUT_PULLUP);
}

void SPI_toLoRa(){
    digitalWrite(LoRa_CS, LOW);
    digitalWrite(SD_CS, HIGH);
}


// for lora

void SPI_toVS1053(){
    digitalWrite(SD_CS, LOW);
    digitalWrite(LoRa_CS, HIGH);
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