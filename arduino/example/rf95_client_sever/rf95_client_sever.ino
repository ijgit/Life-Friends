#include <SPI.h>
#include <RH_RF95.h>

RH_RF95 rf95(8, 3); // Adafruit Feather M0 with RFM95

void setup(){
  pinMode(5, INPUT_PULLUP);

  
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!rf95.init())
    Serial.println("init failed");
  else{
    Serial.println("init success");
  }
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  rf95.setTxPower(23, false);
  rf95.setFrequency(923.0);
}

bool flag = true;
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t len = sizeof(buf);

void loop()
{
  if (flag) { //수신부
    for(int i = 0; i < len; i++){
      buf[i]=0;
      }
    if (rf95.available())
    {
      // Should be a message for us now
      if (rf95.recv(buf, &len)){
        Serial.println((char*)buf);
        flag = false; //트리거 수신시 송신부로 바꿈
      }
      else{
        Serial.println("recv failed");
      }
    }
  }
  else { // 송신부 data에 결과값 넣어서 보내면 됨
    delay(5000); 
    Serial.println("Sending to rf95_server");
    // Send a message to rf95_server
    uint8_t data[] = "1";
    rf95.send(data, sizeof(data));
    flag = true;
  }
}
