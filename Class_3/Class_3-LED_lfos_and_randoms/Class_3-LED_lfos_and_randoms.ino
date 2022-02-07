//LFOs here mean values that change slowly and are used to
// modify things such as color or position

//This first block is all copy-paste and can be left alone except for brightness it just sets up the library

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
int lfo[4];
int lfo_latch[4];
int rate[4];

int bitmap1[8][8] = {
  {0, 0, 1, 2, 1, 1, 0, 0},
  {0, 1, 1, 1, 2, 1, 1, 0},
  {0, 1, 2, 1, 1, 1, 2, 0},
  {0, 1, 1, 2, 1, 1, 1, 0},
  {0, 0, 1, 1, 1, 1, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0},
};

void setup() {
  leds.begin(); //must be done in setup for the LEDs to work.

  analogReadAveraging(64);  //smooth the readings so they jump around less
}


void loop() {
  current_time = millis();

  if (current_time - prev[3] > rate[0]) {
    prev[3] = current_time;

    //if it's 96 97 98 or 99 flip the latch no matter what the lfo[1] value is
    if (random(100) > 95) {
      // "!" is opposite and works on values that are 0 or 1
      // the latch is equal to the opposite of latch
      // simpler version of how we toggled the led in class 1
      lfo_latch[1] = !lfo_latch[1];
    }

    if (lfo_latch[1] == 1) {
      lfo[1] += random(5);
    }
    if (lfo_latch[1] == 0) {
      lfo[1] -= random(10);
    }
    if (lfo[1] < 0) {
      lfo[1] = 0;
      lfo_latch[1] = 1;
    }
    if (lfo[1] > 99) {
      lfo[1] = 99;
      lfo_latch[1] = 0;
    }
    Serial.println(lfo[1]);
  }

  if (current_time - prev[2] > rate[0]) {
    prev[2] = current_time;

    //increase the value if the latch is 1 and decrease if 0
    // if the value hits the top or bottom we've picked arbitrarily,
    // keep it from going past taht number and change direction

    if (lfo_latch[0] == 1) {
      lfo[0] += 4; //same as lfo[0]=lfo[0]+4;
    }
    if (lfo_latch[0] == 0) {
      lfo[0] -= 3; //same as lfo[0]=lfo[0]-3;
    }
    if (lfo[0] < 0) {
      lfo[0] = 0;
      lfo_latch[0] = 1;
    }
    if (lfo[0] > 99) {
      lfo[0] = 99;
      lfo_latch[0] = 0;
    }

  }

  if (current_time - prev[1] > 100) { //33 millis is about 30Hz, aka fps
    prev[1] = current_time;

    rainbow1++;
    if (rainbow1 > 7) {
      rainbow1 = 0;
    }

  }

  if (current_time - prev[0] > 33) { //33 millis is about 30Hz, aka fps
    prev[0] = current_time;

    //its better to not put analogRead in the "bottom" of the loop
    // reading it more slowly will give less noise and we only need
    // to update it when we'd see the change it causes anyway

    rate[0] = map(analogRead(A2), 0, 1023, 0, 100);

    x_pot = map(analogRead(A0), 0, 1023, 0, 7); //map to just 0-7 to select the column...
    y_pot = map(analogRead(A1), 0, 1023, 0, 7); //..and row
    xy_sel = x_pot + (y_pot * 8); //both of these are combined to set the exact pixel from 0-63

    //x_count goes from 0-7 and so does y_count but since we have it arranged
    // with one for loop inside another we get x_count=0 for y_count from 0-7,
    // then x_count=1 for y_count from 0-7 and so on
    // this way we can more easily deal with the two dimensional LED array

    for ( int x_count = 0; x_count < 8; x_count++) {
      for ( int y_count = 0; y_count < 8; y_count++) {
        xy_count = x_count + (y_count * 8); //goes from 0-63

        //set_pixel_HSV(led to change, hue,saturation,value aka brightness)
        // led to change is 0-63
        // all other are 0.0 to 1.0
        // hue - 0 is red, then through the ROYGBIV to 1.0 as red again
        // saturation - 0 is fully white, 1 is fully the selected hue
        // value - 0 is off, 1 is the value set by max_brightness
        set_pixel(xy_count, 0, 0, lfo[1] / 99.0); // turn everything off. otherwise the last "frame" swill still show

        if (0) { //won't happen unless it's "1"
          if (y_count == 2) { // light up an entire row
            float hue1 = rainbow1 / 14.0;
            set_pixel(xy_count, hue1, .9, 1);
          }
        }

        if (xy_count == xy_sel) {
          set_hue = .4;
          //always use "xy_count" as the pixel to address in the x_count y_count fors
          set_pixel(xy_count, set_hue , .9, 1);
        }

        if (bitmap1[y_count][x_count] == 1 ) {
          float hue = (x_count) / 15.0; //this is a local variable. If only exists in these {} so we can't use it anywhere else
          set_pixel(xy_count, hue , 1, .2);
        }
        
        if (bitmap1[y_count][x_count] == 2 ) {
          float saturation = lfo[0] / 99.0;
          set_pixel(xy_count, .7 , saturation, 1);
        }
      }
    }
    leds.show(); // after we've set what we want all the LEDs to be we send the data out through this function
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
