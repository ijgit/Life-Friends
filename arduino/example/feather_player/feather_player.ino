// Specifically for use with the Adafruit Feather, the pins are pre-set here!

// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <SD.h>
#include <Adafruit_VS1053.h>

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

void setup()
{
    Serial.begin(115200);

    delay(500);

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
    printDirectory(SD.open("/"), 0);
    musicPlayer.setVolume(10, 0);

#if defined(__AVR_ATmega32U4__)
    // Timer interrupts are not suggested, better to use DREQ interrupt!
    // but we don't have them on the 32u4 feather...
    musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int
#else
    // If DREQ is on an interrupt pin we can do background
    // audio playing
    musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT); // DREQ int
#endif

    musicPlayer.playFullFile("/test.mp3");
}

void loop(){
    if (Serial.available()){
        char c = Serial.read();

        // if we get an 's' on the serial console, stop!
        if(c == 'a'){
            musicPlayer.playFullFile("/test.mp3");
        }
        if (c == 's'){
            musicPlayer.stopPlaying();
        }

        // if we get an 'p' on the serial console, pause/unpause!
        if (c == 'p'){
            if (!musicPlayer.paused()){
                Serial.println("Paused");
                musicPlayer.pausePlaying(true);
            }
            else{
                Serial.println("Resumed");
                musicPlayer.pausePlaying(false);
            }
        }
    }
    delay(100);
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
