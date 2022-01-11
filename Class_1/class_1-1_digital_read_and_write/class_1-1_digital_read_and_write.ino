/*
  Using a button to turn on enable and disable a blinking light
  You can comment code in a section like this...
*/
// or with two forward slashes. Commented code is just for humans, the Teensy doesn't read them 

// More info on the functions and more at https://www.arduino.cc/reference/en/

// This top section before "setup" is where we declare "global" variables, meaning thy can be read from and written to anywhere in your code. 
// int is a simper integer, meaning it can only contain whole numbers, and can hold values from -32,768 to 32,787
// unsigned long is used for counting milliseconds as an int could only hold about 32 seconds. 
//  while an unsigned long can hold 0 - 4,294,967,295 which is almost 49 days

int led_state = 0;  //you can set things equal to what ever you want...
unsigned long current_time; //this will default to 0
unsigned long prev_time;
int period = 100;
int led_pin = 12;
int button_pin = 5;
int button_reading;
int prev_button_reading;
int led_mode = 0;


//setup is code that only happens one time right when the device turns on or resets
void setup() {

  pinMode(led_pin, OUTPUT); //tell the led_pin to be an output 
  pinMode(button_pin, INPUT_PULLUP); //INPUT_PULLUP must be used for buttons 


  // Here is a diagram of hoe the teeny 3.2s pins are arranged https://www.pjrc.com/teensy/card7a_rev1.png
}

void loop() {

  //calling millis returns the number of milliseconds that have elapsed since the device reset. 
  // this vale is then stored in the variable "current_time"
  current_time = millis(); 

  prev_button_reading = button_reading;  //remember what the reading for the button was last loop
  button_reading = digitalRead(button_pin); //update "button_reading" by setting it equal to digitalRead

  //if the button was not pressed (1) last loop and is pressed this loop (0), execute the code in the {}
  if (prev_button_reading == 1 && button_reading == 0) {
    
    //flip led_mode from 0 to 1 or 1 to 0 
    if (led_mode == 0) {
      led_mode = 1;
    }
    else { //else means led_mode is equal to anything besides 0
      led_mode = 0;
    }

  }  // button reading if is over 

  // if the current time minus the time saved when this code was last executed is great than "period", execute the code in the {}
  if (current_time - prev_time > period) {
    prev_time = current_time;

    //flip led_state from 0 to 1 or 1 to 0 
    if (led_state == 0) {
      led_state = 1;
    }
    else { //else means led_state is equal to anything besides 0
      led_state = 0;
    }

    //is the mode is 0, output led_state to the led pin
    if (led_mode == 0) {
      digitalWrite(led_pin, led_state);
    }
    //is the mode is 1, output a 0 aka LOW aka no voltage so the LED will be off
    if (led_mode == 1) {
      digitalWrite(led_pin, 0);
    }

  }// timing if is over 

}// loop is over 
