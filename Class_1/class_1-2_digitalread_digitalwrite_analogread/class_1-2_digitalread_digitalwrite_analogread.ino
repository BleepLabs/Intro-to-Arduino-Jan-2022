//Use a potentiometer to control the rate of the LED blinking

// More info on the functions and more at https://www.arduino.cc/reference/en/
// Diagram of the teeny 3.2 pins https://www.pjrc.com/teensy/card7a_rev1.png

// declaration section 
int led_state = 0;
unsigned long current_time;
unsigned long prev_time;
unsigned long prev_time2;
int period = 100;
int led_pin = 12;
int button_pin = 5;
int button_reading;
int prev_button_reading;
int led_mode = 0;
int pot1;

//setup is code that runs only once when the device resets
void setup() {
  pinMode(led_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);
}

void loop() {

  //calling millis returns the number of milliseconds that have elapsed since the device reset. 
  // this vale is then stored in the variable "current_time"
  current_time = millis(); 

   // analogRead returns values from 0-1023 by default. 
  // That mean the pot attached to pin A0 will return 0 (or close to it) when turned all the way one way and 1023 when turned the other way
  pot1 = analogRead(A0); 
  period = pot1 * 2; //period now contains a value that can go from 0 - 2046


  prev_button_reading = button_reading;  //remember what the reading for the button was last loop
  button_reading = digitalRead(button_pin); //update "button_reading" by setting it equal to digitalRead

  //if the button was not pressed (1) last loop and is pressed this loop (0), execute the code in the {}
  if (prev_button_reading == 1 && button_reading == 0) {
    if (led_mode == 0) {
      led_mode = 1;
    }
    else {
      led_mode = 0;
    }
  }

  // Serial print send data to the serial monitor or plotter in the Arduino IDE. You can get to them in the Tools menu
  // Like most things, we don't want it to go too fast so here it send the into ever 100 milliseconds 
  // Note we have a new variable just for this "timing if" so it won't interfere with the other tining if. 
  if (current_time - prev_time2 > 100) {
    prev_time2 = current_time;
    Serial.print("pot1 "); //print the characters "pot1 "
    Serial.println(pot1); //print the value contained in pot1 with a return after it
    //More on serial write https://www.arduino.cc/reference/en/language/functions/communication/serial/write/ 
  }

  // Timing if controlled by the pot 
  if (current_time - prev_time > period) {
    prev_time = current_time;

    //flip it from 0 to 1 or 1 to 0
    if (led_state == 0) {
      led_state = 1;
    }
    else {
      led_state = 0;
    }

    // Only blink if the mode is 0
    if (led_mode == 0) {
      digitalWrite(led_pin, led_state);
    }
    if (led_mode == 1) {
      digitalWrite(led_pin, 0);
    }

  }

}
