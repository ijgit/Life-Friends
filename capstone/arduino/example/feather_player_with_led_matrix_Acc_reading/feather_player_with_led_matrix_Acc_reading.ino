// Specifically for use with the Adafruit Feather, the pins are pre-set here!

// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <SD.h>
#include <Adafruit_VS1053.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

Adafruit_8x8matrix matrix = Adafruit_8x8matrix();

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// These are the pins used
#define VS1053_RESET -1 // VS1053 reset pin (not used!)

// feather vs1053 pins
#define VS1053_CS 6   // VS1053 chip select pin (output)
#define VS1053_DCS 10 // VS1053 Data/command select pin (output)
#define CARDCS 5      // Card chip select pin
// DREQ should be an Int pin *if possible* (not possible on 32u4)
#define VS1053_DREQ 9 // VS1053 Data request, ideally an Interrupt pin

Adafruit_VS1053_FilePlayer musicPlayer =
    Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);

Adafruit_MPU6050 mpu;

float buffer_ax[100], buffer_ay[100], buffer_az[100];
float buffer_gx[100], buffer_gy[100], buffer_gz[100];
float buffer_temp[100];

void setup(){

    Serial.begin(115200);
    
    while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

    //when we use LoRa board
    //pinMode(8,OUTPUT);
    //digitalWrite(8, HIGH);
    //pinMode(9, OUTPUT);
    //digitalWrite(9, LOW);

    delay(10);
    //////////////////////////////////////////////////////
    
  //////////////////////////////////////////////////////////
  matrix.begin(0x70);  // pass in the address
  
  delay(10);
    if (!musicPlayer.begin())
    { // initialise the music player
        Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
        while (1)
            ;
    }

    musicPlayer.sineTest(0x44, 500); // Make a tone to indicate VS1053 is working

    if (!SD.begin(CARDCS))
    {
        Serial.println(F("SD failed, or not present"));
    }

    // list files
    //printDirectory(SD.open("/"), 0);
    musicPlayer.setVolume(50, 0);

#if defined(__AVR_ATmega32U4__)
    // Timer interrupts are not suggested, better to use DREQ interrupt!
    // but we don't have them on the 32u4 feather...
    musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int
#else
    // If DREQ is on an interrupt pin we can do background
    // audio playing
    musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT); // DREQ int
#endif

    //musicPlayer.playFullFile("/test.mp3");

    delay(10);
  delay(1000);
    Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  if (!mpu.begin(0x69)) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
    case MPU6050_RANGE_2_G:  
      Serial.println("+-2G");
      break;
    case MPU6050_RANGE_4_G:
      Serial.println("+-4G");
      break;
    case MPU6050_RANGE_8_G:
      Serial.println("+-8G");
      break;
    case MPU6050_RANGE_16_G:
      Serial.println("+-16G");
      break;
  }
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
    case MPU6050_RANGE_250_DEG:
      Serial.println("+- 250 deg/s");
      break;
    case MPU6050_RANGE_500_DEG:
      Serial.println("+- 500 deg/s");
      break;
    case MPU6050_RANGE_1000_DEG:
      Serial.println("+- 1000 deg/s");
      break;
    case MPU6050_RANGE_2000_DEG:
      Serial.println("+- 2000 deg/s");
      break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
    case MPU6050_BAND_260_HZ:
      Serial.println("260 Hz");
      break;
    case MPU6050_BAND_184_HZ:
      Serial.println("184 Hz");
      break;
    case MPU6050_BAND_94_HZ:
      Serial.println("94 Hz");
      break;
    case MPU6050_BAND_44_HZ:
      Serial.println("44 Hz");
      break;
    case MPU6050_BAND_21_HZ:
      Serial.println("21 Hz");
      break;
    case MPU6050_BAND_10_HZ:
      Serial.println("10 Hz");
      break;
    case MPU6050_BAND_5_HZ:
      Serial.println("5 Hz");
      break;
  }

  Serial.println("");

}

int idx = 0;
unsigned long preTime = 0;
const long interval = 50;
bool flag = false;

static const uint8_t PROGMEM
  smile_bmp[] =
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10100101,
    B10011001,
    B01000010,
    B00111100 },
  neutral_bmp[] =
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10111101,
    B10000001,
    B01000010,
    B00111100 },
  frown_bmp[] =
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10011001,
    B10100101,
    B01000010,
    B00111100 };

void loop(){
  if (millis() - preTime > interval) {
    preTime = millis();
    /* Get new sensor events with the readings */
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    if (idx < 100) {
      buffer_ax[idx] = a.acceleration.x;
      buffer_ay[idx] = a.acceleration.y;
      buffer_az[idx] = a.acceleration.z;
      buffer_temp[idx] = temp.temperature;
      buffer_gx[idx] = g.gyro.x;
      buffer_gy[idx] = g.gyro.y;
      buffer_gz[idx++] = g.gyro.z;
    }
    else {
      idx = 0;
      for (int i = 0; i < 100; i++) {
        
          Serial.print(buffer_ax[i]); Serial.print(",");
          Serial.print(buffer_ay[i]); Serial.print(",");
          Serial.print(buffer_az[i]); Serial.print(",");
          Serial.print(buffer_temp[i]); Serial.print(",");
          Serial.print(buffer_gx[i]); Serial.print(",");
          Serial.print(buffer_gy[i]); Serial.print(",");
          Serial.print(buffer_gz[i]); Serial.println();
        
      }
      if (flag == false){
        matrix.clear();
        matrix.drawBitmap(0, 0, smile_bmp, 8, 8, LED_ON);
        matrix.writeDisplay(); 
        musicPlayer.playFullFile("/test.mp3");
        flag = true;
      }else{
        matrix.clear();
        matrix.drawBitmap(0, 0, frown_bmp, 8, 8, LED_ON);
        matrix.writeDisplay();
        flag = false;
      }
      //saveData();
      buffer_ax[idx] = a.acceleration.x;
      buffer_ay[idx] = a.acceleration.y;
      buffer_az[idx] = a.acceleration.z;
      buffer_temp[idx] = temp.temperature;
      buffer_gx[idx] = g.gyro.x;
      buffer_gy[idx] = g.gyro.y;
      buffer_gz[idx++] = g.gyro.z;
    }
  }
}

/// File listing helper
void printDirectory(File dir, int numTabs){
    while (true){

        File entry = dir.openNextFile();
        if (!entry){
            // no more files
            //Serial.println("**nomorefiles**");
            break;
        }
        for (uint8_t i = 0; i < numTabs; i++){
            Serial.print('\t');
        }
        Serial.print(entry.name());
        if (entry.isDirectory()){
            Serial.println("/");
            printDirectory(entry, numTabs + 1);
        }
        else{
            // files have sizes, directories do not
            Serial.print("\t\t");
            Serial.println(entry.size(), DEC);
        }
        entry.close();
    }
}
