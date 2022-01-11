//Use one potentiometer to control the rate of the LED blinking and another to control the brightness

// More info on the functions and more at https://www.arduino.cc/reference/en/
// Diagram of the teeny 3.2 pins https://www.pjrc.com/teensy/card7a_rev1.png


// declaration section 

int led_state = 0;
unsigned long current_time;
unsigned long prev_time;
unsigned long prev_time2;
int period = 100;
int led_pin = 10; //Only some pins can be used with PWM aka analogWrite
int button_pin = 5; //all pins can digital read
int button_reading;
int prev_button_reading;
int led_mode = 0;
int pot1;
int pot2;
int led_bright; //0-255

//setup is code that only happens one time right when the device turns on or resets
void setup() {
  pinMode(led_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);
}

// loop repeats the code in its {} as fast as it can 
void loop() {

  current_time = millis();

  prev_button_reading = button_reading;
  button_reading = digitalRead(button_pin);

  pot1 = analogRead(A0);  // returns values from 0-1023
  period = pot1 * 2; // now goes from 0 - 2046

  pot2 = analogRead(A1);  // returns values from 0-1023

  
  //analogWrite can only accept values from 0-255. That's its resolution
  //Since analogRead goes from 0-1023 and analogWrite wants 0-255 we can use map to easily change the range without doing any math 
  // map(input variable, lowest value the input goes to, highest value the input goes to, desired low for output, desired high for output)
  led_bright = map(pot2, 0, 1023, 0, 255); //analog
  //more on map https://www.arduino.cc/reference/en/language/functions/math/map/

  if (prev_button_reading == 1 && button_reading == 0) {
    if (led_mode == 0) {
      led_mode = 1;
    }
    else {
      led_mode = 0;
    }
  }

  if (current_time - prev_time2 > 100) {
    prev_time2 = current_time;
    Serial.println(led_bright);
  }


  if (current_time - prev_time > period) {
    prev_time = current_time;

    if (led_state == 0) {
      led_state = 1;
    }
    else {
      led_state = 0;
    }

    if (led_mode == 0) {
      //led state still goes from 1 to 0
      // when it's 0 the output will be 0 as 0 * n = 0
      // When it's 1 it will output whatever value led_brightnes is 1 * n = n
      analogWrite(led_pin, led_state * led_bright); 
    }

    if (led_mode == 1) {
      analogWrite(led_pin, 0);
    }

  }

} //loop is over
