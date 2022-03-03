/*
  Using the accelerometer to change the filter frequency using a follower
  It's super sensitive so you can bang on the table next to it to hear the sound

  The accelo reading must be calibrated by holding button 3 down for the follower to affect the filter
  Set the accelo on a table and weigh down or tape the wires to the surface if necessary
  After calibrating it should trigger the drums based on the threshold levels and turn the waveform's filter up.

  Uses the Bleep labs MMA8451 accelerometer
  the board we're using is similar to this https://www.adafruit.com/product/2019 but I made it a long time ago before it was available from adafruit but never put it in a product

  Potentiometers can't be plugged into these clock or data pins!
  Accelo pin - Teensy Pin
  G - Ground
  D - 18 I2C data
  C - 19 I2C clock
  + - 3.3V


  pots:
  drum pitch     feedback amount      master volume
      waveform pitch        filter freqeuncy

  buttons:
  drum1    drum2   calibrate  not used

  leds:
  show the follower on the the first column
  raw accelo reading on the third. This won't move much unless you take the accelo off the table
  
*/

//it's always best to make your pins varaibles so you can chnage them eaily and not go hunting in the code
int drum_pitch_pot_pin = A0;
int feedback_pot_pin = A1;
int vol_pot_pin = A2;
int waveform_pot_pin = A3;
int filter_pot_pin = A6;  //can't use A4 or A5 aka 18 or 19 due to accelo
int button1_pin = 2;
int button2_pin = 3;
int button3_pin = 4;

//these must be included for the accelo to work.
// Find them in sketch>include library > manage libraries and search for "MMA8451" and "Adafruit unified Sensor" which will be at the bottom of the list
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
Adafruit_MMA8451 accelo = Adafruit_MMA8451();
int x_read, y_read, z_read;
int orientation;
int  smoothed_x;
int  smoothed_y;
int  smoothed_z;
int smoothed_x2;


//This block is coped from the tool
// https://www.pjrc.com/teensy/gui/
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=141,269
AudioFilterStateVariable filter1;        //xy=180,168
AudioSynthSimpleDrum     drum1;          //xy=181,50
AudioSynthSimpleDrum     drum2;          //xy=199,102
AudioEffectDelay         delay1;         //xy=418,233
AudioMixer4              mixer1;         //xy=425,115
AudioAmplifier           amp1;           //xy=642,127
AudioOutputAnalog        dac1;           //xy=791,155
AudioConnection          patchCord1(waveform1, 0, filter1, 0);
AudioConnection          patchCord2(filter1, 0, mixer1, 2);
AudioConnection          patchCord3(drum1, 0, mixer1, 0);
AudioConnection          patchCord4(drum2, 0, mixer1, 1);
AudioConnection          patchCord5(delay1, 0, mixer1, 3);
AudioConnection          patchCord6(mixer1, delay1);
AudioConnection          patchCord7(mixer1, amp1);
AudioConnection          patchCord8(amp1, dac1);
// GUItool: end automatically generated code


//led biz begin
#include <WS2812Serial.h>
//we'll be using the Teensy audio library and it doesn't play nicely with neopixels.h or fastled
// so Paul of PJRC made this much more efficient version
const int num_of_leds = 64;
const int pin = 5; // only these pins can be used on the Teensy 3.2:  1, 5, 8, 10, 31
byte drawingMemory[num_of_leds * 3];       //  3 bytes per LED
DMAMEM byte displayMemory[num_of_leds * 12]; // 12 bytes per LED
WS2812Serial leds(num_of_leds, displayMemory, drawingMemory, pin, WS2812_GRB);

//1.0 is VERY bright if you're powering it off of 5V
// this needs to be declared and set to something >0 for the LEDs to work
float max_brightness = 0.1;
//led biz end

unsigned long current_time;
unsigned long prev[8]; //array of 8 variables named "prev"
int shift;
float set_hue;
int xy_sel;
int xy_count;
int x_pot;
int y_pot;
int a1[8] = {10, 20, 30, 40, 50, 60, 70, 80};
int rainbow1;
int test2;
float drum1_freq;
float drum2_freq;
float wave1_freq;
float filter1_freq;
float wave1_gain;
float feedback_amount;
float final_output_level;

float prev_follow1;
float follow1;

int threshold1 = 500;
int threshold2 = 700;
int offset1 = 2000; //1023 is biggest an accelo reading can be so anyhting over that will work

#include <Bounce2.h>
#define BOUNCE_LOCK_OUT

//initialize the debouncers
Bounce debouncer1 = Bounce();
Bounce debouncer2 = Bounce();
Bounce debouncer3 = Bounce();


void setup() {

  //if the accelo doesn't begin...
  while (!accelo.begin(0x1C)) { //necessary for this board. adafruit board has different address
    Serial.println("Couldnt start. Check connections");
    accelo.begin(0x1C); //try to start it again
    delay(500); //wait 500 millis. Only use delay in the setup. In the loop it will cause probles with the timing ifs
  }
  Serial.println("MMA8451 found!");

  //can be "MMA8451_RANGE_2_G", "MMA8451_RANGE_4_G" or "MMA8451_RANGE_8_G"
  // the bigger the number,the less sensitive
  accelo.setRange(MMA8451_RANGE_2_G);

  //there's a lot we need to do in setup now but most of it is just copy paste.
  // This first group should only be done in setup how much RAM to set aside for the audio library to use.
  // The audio library uses blocks of a set size so this is not a percentage or kilobytes, just a kind of arbitrary number.
  // On our Teensy 3.2 we can go up to about 200 but that won't leave any RAM for anyone else.
  // Most objects only need a single block. It's usually the delay and reverb that hog it.
  AudioMemory(150);

  //Next we setup the audio objects
  // We start by writing the object we want, then a period, then the function

  amp1.gain(0); //turn output off for now.

  // begin(volume from 0.0-1.0 , frequency , shape of oscillator)
  //https://www.pjrc.com/teensy/gui/?info=AudioSynthWaveform
  waveform1.begin(1, 440.0, WAVEFORM_SAWTOOTH);
  //waveform1.begin(1, 440.0, WAVEFORM_BANDLIMIT_SAWTOOTH);

  //https://www.pjrc.com/teensy/gui/?info=AudioFilterStateVariable
  filter1.frequency(1000.0);
  filter1.resonance(2.0);

  //https://www.pjrc.com/teensy/gui/?info=AudioSynthSimpleDrum
  drum1.frequency(880);
  drum1.length(2000); //length of sound in ms
  drum1.pitchMod(.9); //Values above 0.5 cause the pitch to sweep downwards, values lower than 0.5 cause the pitch to sweep upwards.

  drum2.frequency(220);
  drum2.length(500);
  drum2.pitchMod(.1);

  // If you go over "1" in total amplitude The top or bottom of the wave is just slammed against a wall
  mixer1.gain(0, .2); //drum 1
  mixer1.gain(1, .2); //drum 2
  mixer1.gain(2, .2); //waveform1 into filter
  mixer1.gain(3, .2); //delay feedback

  // these only equal .8 in total gain but since there's feedback we wan't to be cautious

  //https://www.pjrc.com/teensy/gui/?info=AudioEffectDelay
  delay1.delay(0, 300); //channel, delay time in ms

  pinMode(button1_pin, INPUT_PULLUP);
  debouncer1.attach(button1_pin);
  debouncer1.interval(10); // interval in ms

  pinMode(button2_pin, INPUT_PULLUP);
  debouncer2.attach(button2_pin);
  debouncer2.interval(10); // interval in ms

  pinMode(button3_pin, INPUT_PULLUP);
  debouncer3.attach(button3_pin);
  debouncer3.interval(10); // interval in ms

  leds.begin(); //must be done in setup for the LEDs to work.
  analogReadAveraging(64);  //smooth the readings so they jump around less
}


void loop() {
  current_time = millis();
  debouncer1.update();
  debouncer2.update();
  debouncer3.update();
  accelo.read();

  //we don't need all that resolution so lets divide it down to -1000 to 1000 to make it easier to understand
  // since 8191 is our biggest number just move the decimal in that and divide by it
  // returns -1000 to 1000
  x_read = accelo.x / 8.191; //these can only be done after accelo.read()
  y_read = accelo.y / 8.191;
  z_read = accelo.z / 8.191;

  if (debouncer3.fell()) {
    offset1 = 2000;  //reset the offset
  }

  if (debouncer3.read() == 0 ) {
    //while the button is down, find the lowest reading coming from the accelo
    // were' wanthing to find the value the acclo returns when it's at rest and use that as the "0"
    if (x_read < offset1) {
      offset1 = x_read;
    }
  }

  prev_follow1 = follow1;
  if (x_read > follow1) {
    //if the reading from the accelo is greater than the exisiting follower value, make the reading the new follower value
    follow1 = x_read;
  }

  if (current_time - prev[2] > 1) {
    prev[2] = current_time;

    // constantly attenuate the follower for it falls after being popped up by a new high reading
    follow1 *= .995; //fall exponetially
    //follow1 -= 1; //fall linearly
    if (follow1 < 0) { //don't let it go less tahn 0
      follow1 = 0;
    }

    //Offset it increaded by 10% to remove any noisy spikes taht might be false triggers
    filter1_freq = map(follow1, offset1 * 1.10, 1000, 0, 15000);
    filter1_freq = constrain(filter1_freq, 0, 15000);
    filter1.frequency(filter1_freq);
  }

  //if the follower was below the thresholds set in the initilization section and now it's over it play one drum
  if (prev_follow1 < threshold1 && follow1 > threshold1) {
    drum1.noteOn();
  }

  if (prev_follow1 < threshold2 && follow1 > threshold2) {
    drum2.noteOn();
  }

  //you can still play the drums with buttons
  if (debouncer1.fell() ) {
    drum1.noteOn();
  }

  if (debouncer2.fell() ) {
    drum2.noteOn();
  }

  //since we can hear faster than we can see (?) we want to update these much more quickly
  // so they are not in the timing if. We'll talk about ways to smooth the readings later
  drum1_freq = analogRead(drum_pitch_pot_pin); //0-1023
  drum2_freq = drum1_freq * 2.0;

  drum1.frequency(drum1_freq);
  drum2.frequency(drum2_freq);

  //map can't do floats (some versions of arduino can) so we divide by 100.0
  feedback_amount = map(feedback_pot_pin, 0, 1023.0, 0, 150) / 100.0; //0-1.5
  mixer1.gain(3, feedback_amount); //delay feedback

  final_output_level = map(analogRead(vol_pot_pin), 0, 1023.0, 0, 100) / 100.0; //if we don't want it to go over a certain level we can just cahnge the last value in map
  amp1.gain(final_output_level);

  wave1_freq = analogRead(waveform_pot_pin) * 3.0;
  //wave1_freq = map(touchRead(0), 1000, 8000, 0, 2500);
  waveform1.frequency(wave1_freq);

  //before we used the accello and follower the filter was set by a pot
  //filter1_freq = analogRead(filter_pot_pin) * 4.0;

  //turn the wavefrom off if the filter is low
  // this might cause poping sounds though
  if (filter1_freq < 0) {
    mixer1.gain(2, 0);
  }
  else {
    mixer1.gain(2, .2);
  }

  //show the follower on the the first column
  // raw accelo reading on the third. This won't move much unless you take the accelo off the table
  if (current_time - prev[0] > 33) { //33 millis is about 30Hz, aka fps
    prev[0] = current_time;

    for ( int x_count = 0; x_count < 8; x_count++) {
      for ( int y_count = 0; y_count < 8; y_count++) {
        xy_count = x_count + (y_count * 8); //goes from 0-63
        set_pixel(xy_count, 0, 0, 0); // turn everything off. otherwise the last "frame" swill still show

        if (x_count == 0) {
          int led_follower = map(follow1, offset1 * 1.10, 1000, 7, 0);
          if (y_count == led_follower) {
            set_pixel(xy_count, .4, 1, 1);
          }
        }
        
        if (x_count == 2) {
          int raw_x_reading = map(x_read, -1000, 1000, 7, 0);
          if (y_count == raw_x_reading) {
            set_pixel(xy_count, .1, 1, 1);
          }
        }
        
      }
    }

    leds.show(); // after we've set what we want all the LEDs to be we send the data out through this function
  }

  if (current_time - prev[1] > 5) {
    prev[1] = current_time;

    Serial.print(threshold1);
    Serial.print(" ");
    Serial.print(threshold2);
    Serial.print(" ");
    Serial.print(filter1_freq);
    Serial.print(" ");
    Serial.println(follow1);

    //Here we print out the usage of the audio library
    // If we go over 90% processor usage or get near the value of memory blocks we set aside in the setup we'll have issues or crash.
    // If you're using too many block, jut increase the number up top until you're over it by a couple
    if (0) {
      Serial.print("processor: ");
      Serial.print(AudioProcessorUsageMax());
      Serial.print("%    Memory: ");
      Serial.print(AudioMemoryUsageMax());
      Serial.println();
      AudioProcessorUsageMaxReset(); //We need to reset these values so we get a real idea of what the audio code is doing rather than just peaking in every half a second
      AudioMemoryUsageMaxReset();
    }
  }

}// loop is over



//This function is a little different than you might see in other libraries but it works pretty similar
// instead of 0-255 you see in other libraries this is all 0-1.0
// you can copy this to the bottom of any code as long as the declarations at the top in "led biz" are done

//set_pixel_HSV(led to change, hue,saturation,value aka brightness)
// led to change is 0-63
// all other are 0.0 to 1.0
// hue - 0 is red, then through the ROYGBIV to 1.0 as red again
// saturation - 0 is fully white, 1 is fully colored.
// value - 0 is off, 1 is the value set by max_brightness
// (it's not called brightness since, unlike in photoshop, we're going from black to fully lit up

//based on https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both

void set_pixel(int pixel, float fh, float fs, float fv) {
  byte RedLight;
  byte GreenLight;
  byte BlueLight;

  byte h = fh * 255;
  byte s = fs * 255;
  byte v = fv * max_brightness * 255;

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
