#ifndef LED_H
#define LED_H

class Led {
public:
  Led(int);
  void setup();
  void set(long);
  void update();

private:
  int pin;
  int delay;
  unsigned long time;
  int value;
  int valueActual;
};

#endif




