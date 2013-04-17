#include <TimerOne.h>

#define SIZEX 96
#define SIZEY 7

byte fb[SIZEX/8][SIZEY];

int screenRow;
 
void stepRow() {
  screenRow--;
  if (screenRow < 0) {
    screenRow = 6;
  }

  cli();
  PORTB = 7;
  for (uint8_t rx = 0; rx < (SIZEX / 8); rx++) {
    for (int8_t i = 7; i >= 0; i--)  {   // clock pin12 (0x10) HIGH + data pin13 a framebuffer es \
                                         // aktiv sor szerint beallitva (<<5), valamint blanking (0xf)
      PORTB  =  0x1f | ( !( fb[rx][screenRow] & (1<<i) ) <<5);
      PORTB &= ~0x10; //clock pin12 LOW
    }
  }
  PORTB = (screenRow);                     //select corresponding row
  sei();
}

void setup() {
  for (uint8_t i = 8; i <= 13; i++) {
    pinMode(i, OUTPUT);
  }
  for (uint8_t rx = 0; rx < 12; rx++) {
    fb[rx][3] = 1;
  }
  PORTB = 0xf;
  Timer1.initialize(2800);
  Timer1.attachInterrupt(stepRow);
}

void loop() {
}
