#include <TimerOne.h>
#include <EEPROM.h>

#define SIZEX_PHYS 96
#define SIZEX_VIRT 256
#define SIZEY 7
#define BLOCKS (SIZEX_VIRT / 8)

byte fb[BLOCKS][SIZEY];
byte dispOffset = 0, opsOffset = 0, scrollSpeed = 0, scrollCounter = 0;

int screenRow;
 
void stepRow() {
  screenRow--;
  if (screenRow < 0) {
    screenRow = 6;
  }

  if (scrollSpeed != 0) {
    if (scrollCounter == scrollSpeed) {
      scrollCounter = 0;
      dispOffset++;
    } else {
      scrollCounter++;
    }
  }

  cli();
  PORTB = 7;
  for (uint8_t rx = 0; rx < SIZEX_PHYS; rx++) {
                                         // clock pin12 (0x10) HIGH + data pin13 a framebuffer es \
                                         // aktiv sor szerint beallitva (<<5), valamint blanking (0xf)
	  uint8_t dx = rx + dispOffset;
      PORTB  =  0x1f | ( !( fb[(dx >> 3) % BLOCKS][screenRow] & (0x80>>(dx & 0x07)) ) <<5);
      PORTB &= ~0x10; //clock pin12 LOW
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

  for (uint16_t i = 0; i < 5000; i++) {
    if (Serial.available()) return;
    delay(1);
  }

  uint16_t bytes = ((EEPROM.read(0) & 0x03) << 8) | (EEPROM.read(1));
  uint16_t epos = 0;
  while (epos < bytes) {
    const byte first = EEPROM.read((epos++) + 2);
    if ((first & 0xC0) == 0xC0) {
      const byte y1 = (first >> 3) & 0x07;
      const byte y2 = first & 0x07;
      const byte second = EEPROM.read((epos++) + 2);
      const byte x1 = (second >> 4) & 0x0F;
      const byte x2 = second & 0x0F;
      for (uint8_t y = y1; y < y2; y++) {
        for (uint8_t x = x1; x < x2; x++) {
          fb[opsOffset + x][y] = EEPROM.read((epos++) + 2);
        }
      }
    } else if ((first & 0xE0) == 0x20) {
      opsOffset = first & 0x1F;
    } else if ((first & 0x40) == 0x40) {
      scrollCounter = 0;
      scrollSpeed = first & 0x3F;
    } else if ((first & 0x02) == 0x02) {
      if (first == 0x02) {
        dispOffset = EEPROM.read((epos++) + 2);
      }
    }
  }
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
          fb[opsOffset + x][y] = Serial.read();
        }
      }
      Serial.write(0x42);
    } else if ((first & 0xE0) == 0x20) {
      opsOffset = first & 0x1F;
      Serial.write(opsOffset);
    } else if ((first & 0x40) == 0x40) {
      scrollCounter = 0;
      scrollSpeed = first & 0x3F;
      Serial.write(scrollSpeed);
    } else if ((first & 0x02) == 0x02) {
      if (first == 0x02) {
        while (!Serial.available());
        dispOffset = Serial.read();
      }
      Serial.write(dispOffset);
    } else if ((first & 0xFC) == 0x80) {
      while (!Serial.available());
      const uint16_t bytes = ((first & 0x03) << 8) | Serial.read();
      EEPROM.write(0, first);
      EEPROM.write(1, bytes);
      Serial.write(bytes);
      for (uint16_t i = 2; i < bytes + 2; i++) {
        while (!Serial.available());
        const uint8_t b = Serial.read();
        EEPROM.write(i, b);
        Serial.write(b);
      }
    }
  }
}
