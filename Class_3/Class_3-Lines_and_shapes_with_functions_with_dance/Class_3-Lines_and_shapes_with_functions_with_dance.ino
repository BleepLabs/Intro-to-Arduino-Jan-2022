// Using LFOs to animate the lines and shapes 



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
int lfo[4];
int lfo_latch[4];

void setup() {
  leds.begin(); //must be done in setup for the LEDs to work.
  analogReadAveraging(64);  //smooth the readings so they jump around less
}

void loop() {
  current_time = millis();

  if (current_time - prev[2] > 324) {
    prev[2] = current_time;

    if (lfo_latch[1] == 1) {
      lfo[1] ++; //lfo[0]=lfo[0]+2;
    }
    if (lfo_latch[1] == 0) {
      lfo[1] --; //lfo[0]=lfo[0]-1;
    }
    if (lfo[1] < 0) {
      lfo[1] = 0;
      lfo_latch[1] = 1;
    }
    if (lfo[1] > 7) {
      lfo[1] = 7;
      lfo_latch[1] = 0;
    }

  }

  if (current_time - prev[1] > 50) {
    prev[1] = current_time;

    if (lfo_latch[0] == 1) {
      lfo[0] ++; //lfo[0]=lfo[0]+2;
    }
    if (lfo_latch[0] == 0) {
      lfo[0] --; //lfo[0]=lfo[0]-1;
    }
    if (lfo[0] < 0) {
      lfo[0] = 0;
      lfo_latch[0] = 1;
    }
    if (lfo[0] > 7) {
      lfo[0] = 7;
      lfo_latch[0] = 0;
    }

  }

  if (current_time - prev[0] > 33) { //33 millis is about 30Hz, aka fps
    prev[0] = millis();

    //its better to not put analogRead in the "bottom" of the loop
    // reading it more slowly will give less noise and we only need
    // to update it when we'd see the change it causes anyway

    x_pot = map(analogRead(A0), 0, 1023, 0, 7); //map to just 0-7 to select the column...
    y_pot = map(analogRead(A1), 0, 1023, 0, 7); //..and row
    xy_sel = x_pot + (y_pot * 8); //both of these are combined to set the exact pixel from 0-63
    //Serial.println(xy_sel);

    //x_count goes from 0-7 and so does y_count but since we have it arranged
    // with one for loop inside another we get x_count=0 for y_count from 0-7,
    // then x_count=1 for y_count from 0-7 and so on
    // this way we can more easily deal with the two dimensional LED array

    for ( int x_count = 0; x_count < 8; x_count++) {
      for ( int y_count = 0; y_count < 8; y_count++) {
        xy_count = x_count + (y_count * 8); //goes from 0-63

        set_pixel(xy_count, 0, 0, 0); // turn everything off. otherwise the last "frame" swill still show

        if (1) { 
          // Will return a 1 if there is a line at that point devined by the parameters
          // maskLine(pixel selected, First x, first y, destination x, destination y)
          if (maskLine(xy_count, x_pot, y_pot, lfo[0], lfo[1])) {
            set_pixel(xy_count, y_count / 64.0 , 1, 1);
          }
        }
        
        if (1) { 
          //Make a triangle from three points
          if (maskTriangle(xy_count, lfo[0], y_pot, 7, 7,  0, 7)) {
            set_pixel(xy_count, xy_count / 64.0 , 1, 1);
          }
        }
        
        if (0) {
          //make a four lines based on four points
          if (maskPolygon(xy_count, 1 + x_pot, 1 + y_pot,  6 - x_pot, 1 - y_pot,  6 + x_pot, 6 + y_pot,  1 - x_pot, 6 - y_pot)) {
            set_pixel(xy_count, xy_count / 64.0 , 1, 1);
          }
        }
      }
    }//xy fors done

    leds.show(); // after we've set what we want all the LEDs to be we send the data out through this function
  }//timing if over

} //loop over





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

#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                    \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }
#endif

// Will return a 1 if there is a line at that point devined by the parameters
// maskLine(pixel selected, First x, first y, destination x, destinationy)
// based on https://github.com/adafruit/Adafruit-GFX-Library/blob/0b6068e833b050fa83ed90558c54f3389d26bc5b/Adafruit_GFX.cpp#L132

byte maskLine(int16_t sel, int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
  byte pixels[64];
  x0 = constrain(x0, 0, 7);
  x1 = constrain(x1, 0, 7);
  y0 = constrain(y0, 0, 7);
  y1 = constrain(y1, 0, 7);
  pixels[sel] = 0;

  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1) {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx >> 1;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0 <= x1; x0++) {
    if (steep) {
      int xyc = y0 + (x0 << 3);
      //writePixel(y0, x0, color);
      pixels[xyc] = 1;
      //return xyc;
      //set_pixel(xyc, 0, 0, 1);
    } else {
      int xyc = x0 + (y0 << 3);
      //return xyc;
      pixels[xyc] = 1;
      //set_pixel(xyc, 0, 0, 1);
      //writePixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }

  if (pixels[sel] == 1) {
    return 1;
  }
  else {
    return 0;
  }

}
//three lines using three pairs of points. returns a 1 if the pixel select is on the triangles lines
//  maskTriangle(pixel selected, x0, y0, x1,y1,x2,y2) {
//  You can enable fill but right now it takes too long to up;date fater than 10fps
byte maskTriangle(int sel, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
  byte p[64];
  byte a = maskLine(sel, x0, y0, x1,  y1);
  byte b = maskLine(sel, x1, y1, x2,  y2);
  byte c = maskLine(sel, x2, y2, x0,  y0);
  byte f = a + b + c;
  p[sel] = 0;
  if (f > 0) {
    p[sel] = 1;
  }

  byte pr, r;
  int open1 = 0;
  int close1 = 0;

  if (0) { //enable for fill.  this takes like 70ms to do so can't update at 30 fps
    for (byte xx = 0; xx < 8; xx++) {
      for (byte j = 0; j < 8; j++) {
        byte cs = xx + (j << 3);
        byte a = maskLine(cs, x0, y0, x1,  y1);
        byte b = maskLine(cs, x1, y1, x2,  y2);
        byte c = maskLine(cs, x2, y2, x0,  y0);
        byte f = a + b + c;
        p[cs] = 0;
        if (f > 0) {
          p[cs] = 1;
        }
      }
      for (byte yy = 1; yy < 8; yy++) {

        int cxy = xx + (yy * 8);
        int pxy = (xx) + ((yy - 1) * 8);
        if (p[pxy] > 0 && p[cxy] == 0) {
          open1 = yy;
          //Serial.print(" !");          Serial.println(open1);
        }
        if (open1 > 0) {
          if (p[pxy] == 0 && p[cxy] > 0) {
            close1 = yy;
          }

        }
        if (close1 > 0 && open1 < close1) {
          for (int yf = open1; yf < close1; yf++) {
            p[xx + (yf * 8)] = 1;
          }
        }
      }
    }
  }

  if (p[sel] > 0) {
    return 1;
  }
  else {
    return 0;
  }
}

//four lines using four pairs of points. returns a 1 if the pixel select is on the triangles lines
//  maskRectangle(pixel selected, x0, y0, x1,y1,x2,y2,x3,y3) {

byte maskPolygon(int sel, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3) {
  byte p[64];
  byte a = maskLine(sel, x0, y0, x1,  y1);
  byte b = maskLine(sel, x1, y1, x2,  y2);
  byte c = maskLine(sel, x2, y2, x3,  y3);
  byte d = maskLine(sel, x3, y3, x0,  y0);
  byte f = a + b + c + d;
  p[sel] = 0;
  if (f > 0) {
    p[sel] = 1;
  }

  if (p[sel] > 0) {
    return 1;
  }
  else {
    return 0;
  }
}
