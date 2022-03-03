// Measure the time between instances of the follower going over a threshold
// Average out 4 readings
// you can alos use button 1 instead of the accelo
// I removed the audio and LED stuff to make it simpler

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
int x_read_mapped;

unsigned long current_time;
unsigned long prev[8]; //array of 8 variables named "prev"

float prev_follow1;
float follow1;

int threshold1 = 600;
int threshold2 = 700;
int offset1 = 2000; //1000 is biggestr accel reading can be
unsigned long delta_ped_time, ped_time;
#define array_length 4 //you cant use a variable to set the size of an array. A define is jsut a value you can't change
int delta_ped_time_array[array_length];
int array_index;
int averaged_ped_time;
int prev_x_read;
int prev_x_read_mapped;

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
    accelo.begin(0x1C);
    delay(500);
  }
  Serial.println("MMA8451 found!");

  //can be "MMA8451_RANGE_2_G", "MMA8451_RANGE_4_G" or "MMA8451_RANGE_8_G"
  // the bigger the number,the less sensitive
  accelo.setRange(MMA8451_RANGE_2_G);

  pinMode(button1_pin, INPUT_PULLUP);
  debouncer1.attach(button1_pin);
  debouncer1.interval(10); // interval in ms

  pinMode(button2_pin, INPUT_PULLUP);
  debouncer2.attach(button2_pin);
  debouncer2.interval(10); // interval in ms

  pinMode(button3_pin, INPUT_PULLUP);
  debouncer3.attach(button3_pin);
  debouncer3.interval(10); // interval in ms

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
  prev_x_read = x_read;
  prev_x_read_mapped = x_read_mapped;
  x_read = accelo.x / 8.191; //these can only be done after accelo.read()
  y_read = accelo.y / 8.191;
  z_read = accelo.z / 8.191;
  x_read_mapped = map(x_read, offset1, 1000, 0, 1000);


  //get the offset of the accelo reading so we can have it start at 0
  if (debouncer3.fell()) {
    offset1 = 2000;
  }

  if (debouncer3.read() == 0 ) {
    if (x_read < offset1) {
      offset1 = x_read;
    }
  }

  prev_follow1 = follow1;
  
  if (x_read > follow1) {
    follow1 = x_read;
  }

  if (current_time - prev[2] > 1) {
    prev[2] = current_time;
    follow1 *= .995;
    //follow1 -= 1;
    if (follow1 < 0) {
      follow1 = 0;
    }
  }

  //notice the parentheses, the frist to expresions are combines then ther || or
  // you could also try using x_read or x_read_mapped
  
  if ((prev_follow1 < threshold1 && follow1 > threshold1) || debouncer1.fell()) {
    array_index++;
    if (array_index > array_length - 1) { //0-3 but the array is 4 values long
      array_index = 0;
    }
    delta_ped_time_array[array_index] = millis() - ped_time;
    ped_time = millis();

    unsigned long ave = 0; //make a new variable to accumulate the readings
    for (int j = 0; j < array_length; j++) {
      ave += delta_ped_time_array[j]; //add up everyone in the array
    }
    averaged_ped_time = ave / 4;
  }

  if (current_time - prev[1] > 5) {
    prev[1] = current_time;
    if (1) {
      Serial.print(threshold1);
      Serial.print(" ");
      Serial.print(x_read_mapped);
      Serial.print(" ");
      Serial.print(follow1);
      Serial.print(" ");
      Serial.println(averaged_ped_time);
    }

  }

}// loop is over
