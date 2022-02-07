// Just produces a sine wave to test your output
// no controls used

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=80,335
AudioOutputAnalog        dac1;           //xy=732,268
AudioConnection          patchCord1(waveform1, dac1);

unsigned long current_time;
unsigned long prev[8]; //array of 8 variables named "prev"

void setup() {
  AudioMemory(100);
  waveform1.begin(1, 440.0, WAVEFORM_SINE);
}

void loop() {
  current_time = millis();
  if (current_time - prev[1] > 500) {
    prev[1] = current_time;

    //Here we print out the usage of the audio library
    // If we go over 90% processor usage or get near the value of memory blocks we set aside in the setup we'll have issues or crash.
    // If you're using too many block, jut increase the number up top until you're over it by a couple
    Serial.print("processor: ");
    Serial.print(AudioProcessorUsageMax());
    Serial.print("%    Memory: ");
    Serial.print(AudioMemoryUsageMax());
    Serial.println();
    AudioProcessorUsageMaxReset(); //We need to reset these values so we get a real idea of what the audio code is doing rather than just peaking in every half a second
    AudioMemoryUsageMaxReset();
  }

}// loop is over
