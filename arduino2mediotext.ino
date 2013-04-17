#include <TimerOne.h>

int run=0;
byte timer=0;

#define SIZEX 96
#define SIZEY 7

byte fb[SIZEX/8][SIZEY];

int update=1;
int screenRow;
byte rx;
uint16_t o;
 
void stepRow() {

  updateFb(SIZEY-screenRow-1);
  screenRow--;
  if (screenRow<0) {
	  screenRow=6;
  }

  cli();
  PORTB=7;
  for (rx = 0; rx < (SIZEX / 8); rx++) {
    for (int8_t i=7; i>=0; i--)  {       // clock pin12 (0x10) HIGH + data pin13 a framebuffer es \
                                         // aktiv sor szerint beallitva (<<5), valamint blanking (0xf)
      PORTB  =  0x1f | ( !( fb[rx][screenRow] & (1<<i) ) <<5);
      PORTB &= ~0x10; //clock pin12 LOW
    }
  }
  PORTB=(screenRow);                     //select corresponding row
  sei();
}

void setup() {
  for (uint8_t i = 8; i <= 13; i++) {
    pinMode(i, OUTPUT);
  }
  for (rx=0; rx<12; rx++) {
  fb[rx][3]=1;
  }
  PORTB=0xf;
  Timer1.initialize(2800);
  Timer1.attachInterrupt(stepRow);
}

void updateFb(uint8_t row) {
  if (row == SIZEY-1) {
     if (update==1) {
        o++;
        for (byte b = 0; b<SIZEX/8; b++) {
          for (byte i = 0; i<SIZEY; i++) {
            fb[b][i]=(pgm_read_byte_near(C64_CHAR + FONT_OFFSET + i+(SCROLLTEXT[o/8+b]-32)*8)<<o%8)+
            (pgm_read_byte_near(C64_CHAR + FONT_OFFSET + i+(SCROLLTEXT[o/8+b+1]-32)*8)>>(8-o%8));
          }
        }
        if (o>((SCROLLTEXT_SIZE-(SIZEX/8))*8)) {
          o=0;
        }
      }
  }
}


void loop() {
}
