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
  Serial.begin(19200);
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
  if (Serial.available()) {
    const byte first = Serial.read();
    if ((first & 0xC0) == 0xC0) {
      const byte y1 = (first >> 3) & 0x07;
      const byte y2 = first & 0x07;
      while (!Serial.available());
      const byte second = Serial.read();
      const byte x1 = (second >> 4) & 0x0F;
      const byte x2 = second & 0x0F;
      for (uint8_t y = y1; y < y2; y++) {
        for (uint8_t x = x1; x < x2; x++) {
          while (!Serial.available());
          fb[x][y] = Serial.read();
        }
      }
      Serial.write(0x42);
    }
  }
}
