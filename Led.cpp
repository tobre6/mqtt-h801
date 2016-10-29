#include "Led.h"

#include <Arduino.h>

#define PWM_VALUE 63

int gamma_table[PWM_VALUE+1] = {
    0, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 5, 5, 6, 6, 7, 8, 9, 10,
    11, 12, 13, 15, 17, 19, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55,
    61, 68, 76, 85, 94, 105, 117, 131, 146, 162, 181, 202, 225, 250,
    279, 311, 346, 386, 430, 479, 534, 595, 663, 739, 824, 918, 1023
};

#define TRANSITION_TIME 500

Led::Led(int pin) {
  this->pin = pin;
  delay = 0;
  time = 0;
  value = 0;
  valueActual = 0;
}

void Led::setup() {
  pinMode(pin, OUTPUT);
}

void Led::set(long newValue) {
  Serial1.println("SET: " + String(newValue));
  int key = map(newValue, 0, 255, 0, PWM_VALUE);
  key = constrain(key, 0, PWM_VALUE);
  value = gamma_table[key];
  int diff = abs(value - valueActual);

  if (diff > 0) {
      delay = TRANSITION_TIME / diff;
  }

  analogWrite(pin, newValue);
}

void Led::update() {
  /*if (millis() - time >= delay) {
    time = millis();
    if(value != valueActual) {
      if(valueActual > value) {
          valueActual--;
      }
      else {
        valueActual++;
      }
    }
    analogWrite(pin, valueActual);
  }*/
  //analogWrite(pin, valueActual);
}
