// Example using a waveform with filter and envelope, two drum objects, and delay effect
// waveform frequency is set using array of chromatic notes
// 

/*
  pots:
  drum pitch     feedback amount      master volume
      waveform freq        filter freqeuncy

  buttons:
  falling drum    rising drum   waveform envelope  not used
*/


//This block is coped from the tool
// https://www.pjrc.com/teensy/gui/#include

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=121,376
AudioSynthSimpleDrum     drum2;          //xy=141,151
AudioFilterStateVariable filter1;        //xy=181,272
AudioSynthSimpleDrum     drum1;          //xy=198,104
AudioEffectEnvelope      envelope1;      //xy=202,203
AudioEffectDelay         delay1;         //xy=413,322
AudioMixer4              mixer1;         //xy=420,158
AudioAnalyzePeak         peak1;          //xy=655,106
AudioAmplifier           amp1;           //xy=657,190
AudioOutputAnalog        dac1;           //xy=839,217
AudioConnection          patchCord1(waveform1, 0, filter1, 0);
AudioConnection          patchCord2(drum2, 0, mixer1, 1);
AudioConnection          patchCord3(filter1, 0, envelope1, 0);
AudioConnection          patchCord4(drum1, 0, mixer1, 0);
AudioConnection          patchCord5(envelope1, 0, mixer1, 2);
AudioConnection          patchCord6(delay1, 0, mixer1, 3);
AudioConnection          patchCord7(mixer1, delay1);
AudioConnection          patchCord8(mixer1, amp1);
AudioConnection          patchCord9(mixer1, peak1);
AudioConnection          patchCord10(amp1, dac1);
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

//cosnt means constant so it will store it in program space and save RAM
//the 88 keys of a full sized piano staring at A0, midi note 21
// https://newt.phys.unsw.edu.au/jw/notes.html
const float chromatic[88] = {27.50000365, 29.13523896, 30.86771042, 32.7032, 34.64783346, 36.70810085, 38.89087812, 41.20345007, 43.65353471, 46.24930897, 48.99943599, 51.91309407, 55.00000728, 58.27047791, 61.73542083, 65.40639999, 69.29566692, 73.4162017, 77.78175623, 82.40690014, 87.30706942, 92.49861792, 97.99887197, 103.8261881, 110.0000146, 116.5409558, 123.4708417, 130.8128, 138.5913338, 146.8324034, 155.5635124, 164.8138003, 174.6141388, 184.9972358, 195.9977439, 207.6523763, 220.0000291, 233.0819116, 246.9416833, 261.6255999, 277.1826676, 293.6648067, 311.1270248, 329.6276005, 349.2282776, 369.9944716, 391.9954878, 415.3047525, 440.0000581, 466.1638231, 493.8833665, 523.2511997, 554.3653352, 587.3296134, 622.2540496, 659.2552009, 698.4565551, 739.9889431, 783.9909755, 830.6095048, 880.0001162, 932.3276461, 987.7667329, 1046.502399, 1108.73067, 1174.659227, 1244.508099, 1318.510402, 1396.91311, 1479.977886, 1567.981951, 1661.219009, 1760.000232, 1864.655292, 1975.533466, 2093.004798, 2217.46134, 2349.318453, 2489.016198, 2637.020803, 2793.82622, 2959.955772, 3135.963901, 3322.438019, 3520.000464, 3729.310584, 3951.066931, 4186.009596};

unsigned long current_time;
unsigned long prev[8]; //array of 8 variables named "prev"
int shift;
float set_hue;
int xy_sel;
int xy_count;
int x_pot;
int y_pot;

int rainbow1;
int test2;
float drum1_freq;
float drum2_freq;
float wave1_freq;
float filter1_freq;
float wave1_gain;
float feedback_amount;
float final_output_level;
int note_select;

int button1_pin = 2;
int button2_pin = 3;
int button3_pin = 4;
int button4_pin = 6;
float peak_reading;

#include <Bounce2.h>
#define BOUNCE_LOCK_OUT

//initialize the debouncers
Bounce debouncer1 = Bounce();
Bounce debouncer2 = Bounce();
Bounce debouncer3 = Bounce();
Bounce debouncer4 = Bounce();


void setup() {

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
  filter1.resonance(3.0);

  //https://www.pjrc.com/teensy/gui/?info=AudioSynthSimpleDrum
  drum1.frequency(880);
  drum1.length(2000); //length of sound in ms
  drum1.pitchMod(.9); //Values above 0.5 cause the pitch to sweep downwards, values lower than 0.5 cause the pitch to sweep upwards.

  drum2.frequency(220);
  drum2.length(500);
  drum2.pitchMod(.1);

  //envelope info https://www.pjrc.com/teensy/gui/?info=AudioEffectEnvelope
  // This envelope is an amalpifer that we can open and close over a certain amount of time
  envelope1.attack(250); //time in milliseconds to rise to full amplitude
  envelope1.decay(1); //time in milliseconds to fall to sustain amplitude
  envelope1.sustain(1); //amplitude 0-1.0
  envelope1.release(1000);//time in milliseconds for fade from sustain amplitude to 0

  // If you go over "1" in total amplitude The top or bottom of the wave is just slammed against a wall
  mixer1.gain(0, .3); //drum 1
  mixer1.gain(1, .3); //drum 2
  mixer1.gain(2, .3); //waveform1 into filter
  mixer1.gain(3, 0); //delay feedback


  //https://www.pjrc.com/teensy/gui/?info=AudioEffectDelay
  delay1.delay(0, 350); //channel, delay time in ms

  pinMode(button1_pin, INPUT_PULLUP);
  debouncer1.attach(button1_pin);
  debouncer1.interval(10); // interval in ms

  pinMode(button2_pin, INPUT_PULLUP);
  debouncer2.attach(button2_pin);
  debouncer2.interval(10); // interval in ms

  pinMode(button3_pin, INPUT_PULLUP);
  debouncer3.attach(button3_pin);
  debouncer3.interval(10); // interval in ms

  pinMode(button4_pin, INPUT_PULLUP);
  debouncer4.attach(button4_pin);
  debouncer4.interval(10); // interval in ms

  leds.begin(); //must be done in setup for the LEDs to work.
  analogReadAveraging(64);  //smooth the readings so they jump around less
}


void loop() {
  current_time = millis();

  debouncer1.update();
  debouncer2.update();
  debouncer3.update();
  debouncer4.update();

  if (debouncer1.fell()) {
    drum1.noteOn();
  }

  if (debouncer2.fell() ) {
    drum2.noteOn();
  }

  if (debouncer3.fell() ) {
    envelope1.noteOn(); 
  }
  if (debouncer3.rose() ) {
    envelope1.noteOff();  //the standard envelope needs an on and off, otherwise it will stay on 
  }

  //since we can hear faster than we can see (?) we want to update these much more quickly
  // so they are not in the timing if. We'll talk about ways to smooth the readings later

  drum1_freq = analogRead(A0); //0-1023
  drum1.frequency(drum1_freq);
  drum2_freq = drum1_freq * 2.0;
  drum2.frequency(drum2_freq);


  //map can't do floats (some versions of arduino can) so we divide by 100.0
  feedback_amount = map(analogRead(A1), 0, 1023.0, 0, 150) / 100.0; //0-1.5
  mixer1.gain(3, feedback_amount); //delay feedback

  final_output_level = map(analogRead(A2), 0, 1023.0, 0, 100) / 100.0; //if we don't want it to go over a certain level we can just cahnge the last value in map
  amp1.gain(1);

  float drum_mod = analogRead(A2) / 1023.0; //0.0-1.0
  drum1.pitchMod(drum_mod);

  //now goes from 30-59 try different numbers of different ranges of notes
  note_select = map(analogRead(A3), 0, 1023, 30, 60); 
  //use note_select to select a place in the chromatic array. This returns the actual frequency
  wave1_freq = chromatic[note_select]; 
  // then change the oscillator to this new frequency
  waveform1.frequency(wave1_freq);


  //the filter's frequency needs to change with the waveforms so it will
  // always filter out the same amount of harmonics, no matter the frequency played
  // were we read the pot, get it to 0-1.0 by dividing by it's top value
  // then we multiply by 3 so it's 0-3.0 then at 1 so it never gos below wave1_freq
  // 3 is picked arbitrarily and give a good range
  filter1_freq = wave1_freq * (((analogRead(A4) / 1023.0) * 3.0) + 1.0); //1.0-4.0
  filter1.frequency(filter1_freq);


  if (current_time - prev[0] > 33) { //33 millis is about 30Hz, aka fps
    prev[0] = current_time;

    // peak returns the highest amplitude since it was last checked. 0-1.0
    // First you need to ask if it's ready
    if (peak1.available()) {
      //since map won't work on floats we turn 0-1.0 to 0-100
      // then we can map it to 8-0 since the bottom of the screen is line 7
      // this means 0 reading won't show up on the screen and 0 will be the loudest
      peak_reading = map((peak1.read() * 100.0), 0, 100, 8, 0);
    }
    Serial.println(peak_reading);

    for ( int x_count = 0; x_count < 8; x_count++) {
      for ( int y_count = 0; y_count < 8; y_count++) {
        xy_count = x_count + (y_count * 8); //goes from 0-63
        set_pixel(xy_count, 0, 0, 0); // turn everything off. otherwise the last "frame" swill still show

        if (1) { //this will happen
          //draw lines coming from the bottom
          if (y_count >= peak_reading) {
            float hue = y_count / 10.0; //each row will be a different color
            set_pixel(xy_count, hue , .9, 1);
          }
        }
      }
    }
    leds.show(); // after we've set what we want all the LEDs to be we send the data out through this function
  }

  if (current_time - prev[1] > 500  && 0) { //change to && 1 to turn this on
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
