//Have the LEDs turn on and off with the rate from one put and the brightness from another

int led1_pin =  10;  // The pin for LED1
int led1_state = 0;  // The state of LED1

int led2_pin =  9;  // The LED2 pin
int led2_state = 0;

unsigned long  previous_time1 = 0;  // will store the last time LED1 was updated
unsigned long  previous_time2 = 0;  // will store the last time LED2 was updated
unsigned long  previous_time3;

unsigned long current_time;

unsigned long interval1 = 500;  //the interval that LED1 will change
unsigned long interval2 = 500;  //the interval that LED2 will change

int button_pin; //pin one side of the button is connected to
int button_state; //where we will store if the reading on the pin is high or low.

int pot1_value; // where we'll store the reading
int pot1_pin = A0;

int pot2_value;
int pot2_pin = A1;

void setup() {
  // set both LED pins as output:
  pinMode(led1_pin, OUTPUT);
  pinMode(led2_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);
  //analogRead pins don't need to be setup

}

void loop()
{
  current_time = millis();

  button_state = digitalRead(button_pin); //if the button is not being pressed it will read HIGH. if it pressed it will read LOW

  pot1_value = analogRead(A0); //Read the analog voltage at pin A0. Returns 0 for 0 volts and 1023 for the max voltage (3.3V)
  interval1 = pot1_value * 2;
  interval2 = pot1_value * 2.1;

  //The analog read value is 0-1024 but analogWrite is 0-255 so we can divide by 4 to get 0-255..
  //pot2_value = analogRead(A1) / 4;
  //...or we can map and not worry about any mat or remebering about bits
  // https://www.arduino.cc/reference/en/language/functions/math/map/
  pot2_value = map(analogRead(A1), 0, 1023, 0, 255);

  if (current_time - previous_time3 > 100) {
    previous_time3 = current_time;
    Serial.print("pot1_value ");
    Serial.println(pot1_value);
    
  }

  if (current_time - previous_time1 > interval1) {
    previous_time1 = current_time;

    if (led1_state == LOW) {
      led1_state = HIGH;
    }
    else {
      led1_state = LOW;
    }

    //Since the state is 1 or 0 it's off half the time, on at our new pot reading the other half
    // Only pins marked "PWM" can do analogWrite.
    analogWrite(led1_pin, led1_state * pot2_value);

  }


  if (current_time - previous_time2 > interval2) {
    previous_time2 = current_time;

    if (led2_state == 0) {
      led2_state = 1;
    }
    else {
      led2_state = 0;
    }
    analogWrite(led2_pin, led2_state * pot2_value);
  }
}
