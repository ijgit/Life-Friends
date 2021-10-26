// Basic demo for accelerometer readings from Adafruit MPU6050

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

float buffer_ax[100], buffer_ay[100], buffer_az[100];
float buffer_gx[100], buffer_gy[100], buffer_gz[100];
float buffer_temp[100];

Adafruit_MPU6050 mpu;
Adafruit_8x8matrix matrix = Adafruit_8x8matrix();

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

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

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
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
  
  Serial.println("8x8 LED Matrix Test");
  
  matrix.begin(0x70);  // pass in the address
}

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

int idx = 0;
unsigned long preTime = 0;
const long interval = 50;
int faceIdx = 0;

void loop() {
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
      buffer_ax[idx] = a.acceleration.x;
      buffer_ay[idx] = a.acceleration.y;
      buffer_az[idx] = a.acceleration.z;
      buffer_temp[idx] = temp.temperature;
      buffer_gx[idx] = g.gyro.x;
      buffer_gy[idx] = g.gyro.y;
      buffer_gz[idx++] = g.gyro.z;
      if (faceIdx == 0){
        matrix.clear();
        matrix.drawBitmap(0, 0, smile_bmp, 8, 8, LED_ON);
        matrix.writeDisplay();
        faceIdx = 1;
      }
      else{
        matrix.clear();
        matrix.drawBitmap(0, 0, neutral_bmp, 8, 8, LED_ON);
        matrix.writeDisplay();
        faceIdx = 0;
      }
    }
  }
}
