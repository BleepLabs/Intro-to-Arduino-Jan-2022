/*
  draw a lil firework

  1 press button dot goes up
  2 dot turns to bitmap
  3 bitmap is stepped through, as in 0 1 2 3


*/

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

int bitmap1[8][8] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
};
int button_pin = 0;
int button_read;
int prev_button_read;
int mode = 0;
int dot_position = 0;
int dot_xy;
int expolosion_timeout;

void setup() {
  leds.begin(); //must be done in setup for the LEDs to work.
  pinMode(button_pin, INPUT_PULLUP);
  analogReadAveraging(64);  //smooth the readings so they jump around less
}


void loop() {
  current_time = millis();
  prev_button_read = button_read;
  button_read = digitalRead(button_pin);

  if (prev_button_read == 1 && button_read == 0) {
    mode = 1;
  }

  //When the mode is 2, get some a random number from 0-63
  // increment expolosion_timeout and once it gets over 16
  // Reset all the variables were using to 0 and go to mode 0 
  if (current_time - prev[2] > 50) {
    prev[2] = current_time;
    if (mode == 2) {
      dot_position = random(64);
      expolosion_timeout++;
      if (expolosion_timeout > 16) {
        mode = 0;
        expolosion_timeout = 0;
        dot_position = 0;
      }
    }
  }

  //when mode is 1 increment a variable untill it's over 4 then
  // make it 0 again and cahnge the mode to 2
  if (current_time - prev[1] > 150) {
    prev[1] = current_time;
    if (mode == 1) {
      dot_position++;
      if (dot_position > 4) {
        dot_position = 0;
        mode = 2;
      }
    }
  }



  if (current_time - prev[0] > 33) { //33 millis is about 30Hz, aka fps
    prev[0] = current_time;

    //pots aren't being used 
    x_pot = map(analogRead(A0), 0, 1023, 0, 7); //map to just 0-7 to select the column...
    y_pot = map(analogRead(A1), 0, 1023, 0, 7); //..and row
    xy_sel = x_pot + (y_pot * 8); //both of these are combined to set the exact pixel from 0-63

    for ( int x_count = 0; x_count < 8; x_count++) {
      for ( int y_count = 0; y_count < 8; y_count++) {
        xy_count = x_count + (y_count * 8); //goes from 0-63

        //If the mode doesnot equal to 2, clear the screen
        // this means in mode 2 those random numbers we pick to turn on LEDS
        // will stay on
        if (mode != 2) {
          set_pixel(xy_count, 0, 0, 0); // turn everything off. otherwise the last "frame" swill still show
        }

        if (mode == 1) {
          //dot position just goes from 0-4. This would get displayed as a pixel
          // lighting up in the top left corner and moving right.
          // I want it coming from the bottom going up
          // "dot_position * 8" makes it move down instead of right as moving up by 8 means youre going in the y axis
          // "63 -" flips it so now it will go up. there are 63 LEDs so subracting the highest nuber from waht we have will flip it around the y axis 
          // "4 +" shift it 4 pixels to the right in the x axis
          dot_xy = 4 + (63 - (dot_position * 8));
          set_pixel(dot_xy, 0, 0, 1);
        }
        
        if (mode == 2) {
          float hue_explode = random(50) / 500.0; //we want a result that is between 0-1.0
          set_pixel(dot_position, hue_explode, 1, 1);
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
