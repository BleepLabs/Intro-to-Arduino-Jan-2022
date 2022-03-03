/*
  Examples created for my fall 2020 intro to Arduino course

  todo: comments!

  In this example two bouncing balls are simulated with random notes
  generated each time they hit the wall. Of course there is a tape delay. Of course.

  left top pot is color and delaytime 
  middle top is trails
  top right is feedback 

*/

#include "follower.h"

#include "effect_tape_delay.h"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>


AudioSynthWaveform       waveform1, waveform2, waveform3, waveform4, waveform5, waveform6; //xy=267,333
AudioAnalyzePeak         peak1, peak2, peak3, peak4;      //xy=277,514
AudioAnalyzeFollower     env1, env2;
AudioEffectTapeDelay      delay1;
AudioEffectMultiply      am1, am2;
AudioMixer4              mixer1;         //xy=416,340
AudioOutputAnalog        dac1;           //xy=571,350
AudioConnection          patchCord1(waveform1, peak1);
AudioConnection          patchCord2(waveform2, peak2);
AudioConnection          patchCord4(waveform3, peak3);
AudioConnection          patchCord41(waveform4, peak4);
AudioConnection          patchCord5(env1, 0, am1, 1);
AudioConnection          patchCord51(env2, 0, am2, 1);
AudioConnection          patchCord6(waveform5, 0, am1, 0);
AudioConnection          patchCord61(waveform6, 0, am2, 0);
AudioConnection          patchCord7(am2, 0, mixer1, 1);
AudioConnection          patchCord71(am1, 0, mixer1, 0);
AudioConnection          patchCord8(mixer1, 0, delay1, 0);
AudioConnection          patchCord9(delay1, 0, mixer1, 3);
AudioConnection          patchCord10(mixer1, dac1);
// GUItool: end automatically generated code

#include <WS2812Serial.h>

const int numled = 64;
const int pin = 5;
//   Teensy 3.2:  1, 5, 8, 10, 31   (overclock to 120 MHz for pin 8)
byte drawingMemory[numled * 3];       //  3 bytes per LED
DMAMEM byte displayMemory[numled * 12]; // 12 bytes per LED

WS2812Serial leds(numled, displayMemory, drawingMemory, pin, WS2812_GRB);

const float chromatic[88] = {55.00000728, 58.27047791, 61.73542083, 65.40639999, 69.29566692, 73.4162017, 77.78175623, 82.40690014, 87.30706942, 92.49861792, 97.99887197, 103.8261881, 110.0000146, 116.5409558, 123.4708417, 130.8128, 138.5913338, 146.8324034, 155.5635124, 164.8138003, 174.6141388, 184.9972358, 195.9977439, 207.6523763, 220.0000291, 233.0819116, 246.9416833, 261.6255999, 277.1826676, 293.6648067, 311.1270248, 329.6276005, 349.2282776, 369.9944716, 391.9954878, 415.3047525, 440.0000581, 466.1638231, 493.8833665, 523.2511997, 554.3653352, 587.3296134, 622.2540496, 659.2552009, 698.4565551, 739.9889431, 783.9909755, 830.6095048, 880.0001162, 932.3276461, 987.7667329, 1046.502399, 1108.73067, 1174.659227, 1244.508099, 1318.510402, 1396.91311, 1479.977886, 1567.981951, 1661.219009, 1760.000232, 1864.655292, 1975.533466, 2093.004798, 2217.46134, 2349.318453, 2489.016198, 2637.020803, 2793.82622, 2959.955772, 3135.963901, 3322.438019, 3520.000464, 3729.310584, 3951.066931, 4186.009596, 4434.92268, 4698.636906, 4978.032395, 5274.041605, 5587.652439, 5919.911543, 6271.927802, 6644.876037, 7040.000927, 7458.621167, 7902.133861, 8372.019192};

//Then we have our varible declarations like before
unsigned long cm;
unsigned long prev[8];
float flfo[8], freq[8], amp[8];
int lfo[8], prev_lfo[8], osc[8], out[8];
float test1;
unsigned long cu, du, looptime;
int jj;
int xy1, xy2;
byte trails[2][50];
byte tc;
float fade[8];
int newnote[10];
#define delay_len 6000
short delaybank[delay_len];
byte rh[8] = {24, 24, 24, 24};
int dx;
float fa[6];
void setup() {
  AudioNoInterrupts();
  leds.begin();
  pinMode(0, INPUT_PULLUP);
  analogReference(EXTERNAL);
  AudioMemory(15);
  waveform5.begin(1, 220.0, WAVEFORM_SINE);
  waveform6.begin(1, 220.0, WAVEFORM_SINE);

  waveform1.begin(1, .11, WAVEFORM_TRIANGLE);
  waveform2.begin(1, .15, WAVEFORM_TRIANGLE);
  waveform3.begin(1,  .12,  WAVEFORM_TRIANGLE);
  waveform4.begin(1, .17, WAVEFORM_TRIANGLE);

  mixer1.gain(0, .9);
  mixer1.gain(1, 0);
  mixer1.gain(3, .4);

  delay1.begin(delaybank, delay_len, 1, 1, 0);
  analogWriteResolution(12);
  analogReadResolution(12);
  analogReadAveraging(16);
  AudioInterrupts();
  env1.trig();
  env2.trig();
  fa[3] = .75;
}

void loop() {
  cm = millis();


  if (cm - prev[1] > 40) {
    prev[1] = cm;
    //fa[1] = analogRead(A0) / 2000.0;
    //fa[2] = analogRead(A1) / 2000.0;
    fa[1] = .2;
    fa[2] = .31;
    fa[4] = analogRead(A2) / 4095.0;

    mixer1.gain(3, fa[4]);

    //waveform1.frequency(.11);
    waveform2.frequency(fa[1]);
    waveform3.frequency(fa[2]);
    // waveform4.frequency(fa[2] * fa[3]);


    prev_lfo[0] = lfo[0];
    prev_lfo[1] = lfo[1];
    prev_lfo[2] = lfo[2];
    prev_lfo[3] = lfo[3];

    if (peak1.available()) {
      lfo[0] = (peak1.read() * 7.0) + .5;
    }
    if (peak2.available()) {
      lfo[1] = (peak2.read() * 7.0) + .5;
    }

    if (peak3.available()) {
      lfo[2] = (peak3.read() * 7.0) + .5;
    }
    if (peak4.available()) {
      lfo[3] = (peak4.read() * 7.0) + .5;
    }

    //top =1
    //right
    //bottom
    //left =4

    //newnote = 0;
    byte nnm = 6;
    byte note_trig = 0;

    for (byte m = 0; m < 5; m++) {
      if (newnote[m] > 0) {
        newnote[m]--;
      }
    }

    if (lfo[0] == 0 && prev_lfo[0] != 0) {
      newnote[1] = nnm;
      note_trig = 1;

    }
    if (lfo[0] == 7 && prev_lfo[0] != 7) {
      newnote[2] = nnm;
      note_trig = 1;
      Serial.print(" !");
      Serial.println(newnote[2]);
    }
    if (lfo[1] == 0 && prev_lfo[1] != 0) {
      newnote[3] = nnm;
      note_trig = 1;
      rh[1] = random(8) * 32;
    }
    if (lfo[1] == 7 && prev_lfo[1] != 7) {
      newnote[4] = nnm;
      note_trig = 1;
      rh[2] = random(8) * 32;
    }
    if (lfo[2] == 0 && prev_lfo[2] != 0) {
      newnote[1] = nnm;
      note_trig = 1;
      rh[1] = random(8) * 32;
    }
    if (lfo[2] == 7 && prev_lfo[2] != 7) {
      newnote[2] = nnm;
      note_trig = 1;
      rh[2] = random(8) * 32;
    }
    if (lfo[3] == 0 && prev_lfo[3] != 0) {
      newnote[3] = nnm;
      note_trig = 1;
      rh[3] = random(8) * 32;
    }
    if (lfo[3] == 7 && prev_lfo[3] != 7) {
      newnote[4] = nnm;
      note_trig = 1;
      rh[4] = random(8) * 32;
    }

    if (note_trig == 1) {
      freq[1] = chromatic[random(20, 60)];
      waveform5.frequency(freq[1]);
      env1.trig();
      if (random(100) > 80) {
        float rf = random(100) / 210.00;
        fa[3] = (random(10) / 100.00) - .75;
        waveform1.frequency(rf);
        Serial.print(" rf");
        Serial.println(rf);
      }
    }

    /*
        if (newnote >= 7) {
          freq[2] = chromatic[random(10, 50)];
          waveform6.frequency(freq[2]);
          env2.trig();
          if (random(100) > 80) {
            float rf = random(100) / 190.00;
            waveform3.frequency(rf);
            Serial.print(" rf");
            Serial.println(rf);
          }
        }
    */

    int bright1 = 255.0 * fa[4];
    for ( int xr = 0; xr < 8; xr++) {
      for ( int yc = 0; yc < 8; yc++) {
        int xyc = (xr) + ((yc) * 8);
        leds.setPixelColor(xyc, 0, 0, ((analogRead(A0) / 4000.0) * 255.0)-20);

        if (xr == 0) {
          //
          if (newnote[1] > 0) {
            byte f = 1;
            hsv(xyc, rh[f] - ((newnote[f] * 2)), 255 - (((newnote[f] - nnm) * -3)), bright1 - (((newnote[f] - nnm) * -2)));
            //hsv(xyc, 0 + (yc * 24), 05, 24 - (newnote[1] * 4));
          }
        }
        if (xr == 7) {
          //
          if (newnote[2] > 0) {
            byte f = 2;
            hsv(xyc, rh[f] - ((newnote[f] * 2)), 255 - (((newnote[f] - nnm) * -3)), bright1 - (((newnote[f] - nnm) * -2)));
          }
        }
        if (yc == 0) {
          //hsv(xyc, rh[3] + (1 * 16), 255, 16);
          if (newnote[3] > 0) {
            byte f = 3;
            hsv(xyc, rh[f] - ((newnote[f] * 2)), 255 - (((newnote[f] - nnm) * -3)), bright1 - (((newnote[f] - nnm) * -2)));
          }
        }
        if (yc == 7) {
          //hsv(xyc, rh[4] + (1 * 16), 255, 16);
          if (newnote[4] > 0) {
            byte f = 4;
            hsv(xyc, rh[f] - ((newnote[f] * 2)), 255 - (((newnote[f] - nnm) * -3)), bright1 - (((newnote[f] - nnm) * -2)));
          }
        }

      }
    }

    xy1 = (lfo[0]) + ((lfo[1]) * 8);
    //leds.setPixelColor(xy1, 15, 0, 15);
    hsv(xy1, 220, 255, bright1 * 4.0);


    xy2 = (lfo[2]) + ((lfo[3]) * 8);
    hsv(xy2, 150, 255, bright1 * 4.0);
    // leds.setPixelColor(xy2, 0, 25, 10);

    int trail_max = (analogRead(A1) / 4095.0) * 40.0;
    tc++;
    if (tc > trail_max) {
      tc = 0;
    }
    
    trails[0][tc] = xy1;
    trails[1][tc] = xy2;
    for (byte i = 1; i < trail_max - 1; i++) {
      int ptc = tc - i;
      if (ptc < 0) {
        ptc += trail_max;
      }
      hsv(trails[0][ptc], 220 - (i * 8), 255 - (i * 4), (255 - (i * 2))*fa[4]);
      hsv(trails[1][ptc], 150 - (i * 8  ), 255 - (i * 4), (255 - (i * 2))*fa[4]);

      //leds.setPixelColor(, 0, 6, 7);
    }


    //
    //leds.setPixelColor(trails[pptc], 6, 0, 1);


    if (1 == 0) {
      Serial.print(newnote[1]);
      Serial.print(" ");
      Serial.print(newnote[2]);
      Serial.print(" ");
      Serial.println(newnote[3]);
    }
    leds.show();

  }

  if (cm - prev[0] > 150 && 1 == 1) {
    prev[0] = cm;
    dx += 100;
    if (dx > delay_len - 2) {
      dx = 2000;
    }
    delay1.length(dx);

    waveform1.begin(1, .11, WAVEFORM_TRIANGLE);
    waveform2.begin(1, .15, WAVEFORM_TRIANGLE);
    waveform3.begin(1,  .12,  WAVEFORM_TRIANGLE);
    waveform4.begin(1, .17, WAVEFORM_TRIANGLE);

    //Here we print out the usage of the audio library
    // If we go over 80% processor usage or get near the value of memory blocks we set aside in the setup we'll have issues or crash.
    // This pretty much all a copy paste
    /*
      Serial.print("processor: ");
      Serial.print(AudioProcessorUsageMax());
      Serial.print("%    Memory: ");
      Serial.print(AudioMemoryUsageMax());
      Serial.println();
      AudioProcessorUsageMaxReset(); //We need to reset these valuse so we get a real idea of wh the audio code is doing rather than jsut peaking in every half a second
      AudioMemoryUsageMaxReset();
    */
  }
}


float fract(float x) {
  return x - int(x);
}

float mix(float a, float b, float t) {
  return a + (b - a) * t;
}

float step(float e, float x) {
  return x < e ? 0.0 : 1.0;
}

byte RedLight;
byte GreenLight;
byte BlueLight;


void hsv(int pixel, byte h, byte s, byte v) {

  h = (h * 192) / 256;  // 0..191
  unsigned int i = h / 32;   // We want a value of 0 thru 5
  unsigned int f = (h % 32) * 8;   // 'fractional' part of 'i' 0..248 in jumps

  unsigned int sInv = 255 - s;  // 0 -> 0xff, 0xff -> 0
  unsigned int fInv = 255 - f;  // 0 -> 0xff, 0xff -> 0
  byte pv = v * sInv / 256;  // pv will be in range 0 - 255
  byte qv = v * (256 - s * f / 256) / 256;
  byte tv = v * (256 - s * fInv / 256) / 256;

  switch (i) {
    case 0:
      RedLight = v;
      GreenLight = tv;
      BlueLight = pv;
      break;
    case 1:
      RedLight = qv;
      GreenLight = v;
      BlueLight = pv;
      break;
    case 2:
      RedLight = pv;
      GreenLight = v;
      BlueLight = tv;
      break;
    case 3:
      RedLight = pv;
      GreenLight = qv;
      BlueLight = v;
      break;
    case 4:
      RedLight = tv;
      GreenLight = pv;
      BlueLight = v;
      break;
    case 5:
      RedLight = v;
      GreenLight = pv;
      BlueLight = qv;
      break;
  }
  leds.setPixelColor(pixel, RedLight, GreenLight, BlueLight);

}
