#include "Led.h"

#include <Arduino.h>

#define TRANSITION_TIME 2000

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
  value = map(newValue, 0, 255, 0, 1024);
  int diff = abs(value - valueActual);
  if (diff > 0) {
    delay = TRANSITION_TIME / diff;
  }
}

void Led::update() {
  if (millis() - time >= delay) {
    time = millis();
    if (value != valueActual) {
      if (valueActual > value) {
        valueActual--;
      }
      else {
        valueActual++;
      }
    }
    analogWrite(pin, valueActual);
  }
}
