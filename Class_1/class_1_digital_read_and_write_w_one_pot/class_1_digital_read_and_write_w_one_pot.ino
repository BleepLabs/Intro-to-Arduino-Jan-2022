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

void setup() {
  pinMode(led_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);
}

void loop() {

  current_time = millis();

  prev_button_reading = button_reading;
  button_reading = digitalRead(button_pin);

  pot1 = analogRead(A0);  // returns values from 0-1023
  period = pot1 * 2;

  if (prev_button_reading == 1 && button_reading == 0) {
    if (led_mode == 0) {
      led_mode = 1;
    }
    else {
      led_mode = 0;
    }
  }

  if (current_time - prev_time2 > 10) {
    prev_time2 = current_time;
    Serial.print("pot1 ");
    Serial.println(pot1);
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
      digitalWrite(led_pin, led_state);
    }
    if (led_mode == 1) {
      digitalWrite(led_pin, 0);
    }

  }

}
