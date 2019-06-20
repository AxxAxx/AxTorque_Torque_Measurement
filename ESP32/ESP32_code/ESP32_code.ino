#include "HX711.h"
#include "GY80.h"
// HX711 circuit wiring
//const int LOADCELL_DOUT_PIN = A1;
//const int LOADCELL_SCK_PIN = A0;

const int LOADCELL_DOUT_PIN = 33;
const int LOADCELL_SCK_PIN = 27;
#include <Wire.h>

#include <Arduino.h>
#include <math.h>
#include "DvG_SerialCommand.h"

// Serial   : Programming USB port
// SerialUSB: Native USB port. Baudrate setting gets ignored and is always as
//            fast as possible.
#define Ser Serial

// Initiate serial command listener
DvG_SerialCommand sc(Ser);

HX711 scale;
GY80 sensor = GY80();


void setup() {
  Serial.begin(115200);
  Wire.begin(23,22);
  sensor.setup();
  // Initialize library with data output pin, clock input pin and gain factor.
  // Channel selection is made by passing the appropriate gain:
  // - With a gain factor of 64 or 128, channel A is selected
  // - With a gain factor of 32, channel B is selected
  // By omitting the gain factor parameter, the library
  // default "128" (Channel A) is used here.
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  scale.set_scale(123333.f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();				        // reset the scale to 0
}



#define WAVE_SINE     1
#define WAVE_SQUARE   2
#define WAVE_SAWTOOTH 3

uint8_t wave_type = WAVE_SINE;
double  wave_freq = 0.3;   // [Hz]
double  wave = 0.0;

uint32_t curMillis  = millis();
uint32_t prevMillis = 0;

void loop() {


  char* strCmd; // Incoming serial command string

  // Generate wave sample every millisecond
  curMillis = millis();
  if (curMillis - prevMillis >= 1) {
    
    if (wave_type == WAVE_SINE) {
      wave = sin(2*PI*wave_freq*curMillis/1e3);
    } else if (wave_type == WAVE_SQUARE) {
      wave = (fmod(wave_freq*curMillis/1e3, (double)(1.0)) > 0.5) ? 1 : -1;
    } else if (wave_type == WAVE_SAWTOOTH) {
      wave = 2*fmod(wave_freq*curMillis/1e3, (double)(1.0)) - 1;
    }

    prevMillis = curMillis;
  }

  // Process serial commands
  if (sc.available()) {
    strCmd = sc.getCmd();

    if (strcmp(strCmd, "id?") == 0) {
      Ser.println("Wave generator");
    
    } else if(strcmp(strCmd, "sine") == 0) {
      scale.tare();                // reset the scale to 0
    } else if(strcmp(strCmd, "square") == 0) {
      wave_type = WAVE_SQUARE;
    } else if(strcmp(strCmd, "sawtooth") == 0) {
      wave_type = WAVE_SAWTOOTH;

    } else if(strcmp(strCmd, "?") == 0) {
      sensor.computeAngles();
      wave = scale.get_value()/123333;
      Ser.print(curMillis);
      Ser.print('\t');
      Ser.print(wave, 4);
      Ser.print('\t');
      Ser.print(sensor.getYawValue(), 4);
      Ser.print('\t');
      Ser.print(sensor.getPitchValue(),4);
      Ser.print('\t');
      Ser.println(sensor.getRollValue(),4);
    }
  }



 
  }
