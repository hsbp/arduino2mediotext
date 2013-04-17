//#include <Mediotext.h>
#include <TimerOne.h>
//#include "linux8x8.h"
#include "CP852.h"
#include <avr/pgmspace.h>
#define SERIAL

void stepRow();
void setRow(byte row);
void setup();
void updateFb(uint8_t row);
void loop();
int run=0;
const char* SCROLLTEXT = 
//23456789012                                                                             123456789012

"            Ret3k Maximum Selecta Dubstep Is Fun! - www.bitlabrecords.com/dubstepisfun http://hspbp.org            ";
//"            http://hspbp.org/ - Hackerspace Budapest - We are the glider, not the gun ***            ";
//        if (o>((SCROLLTEXT_SIZE+(SIZEX/8*2))*8)) {
//416 208
//const int SCROLLTEXT_SIZE=sizeof(SCROLLTEXT)/sizeof(char);
#define SCROLLTEXT_SIZE 114
const int FONT_OFFSET = 0x0000;                         
const prog_uint8_t C64_CHAR[] PROGMEM = { FONTDATA };
byte timer=0;

#define SIZEX 96
#define SIZEY 7

byte fb[SIZEX/8][SIZEY];

int update=1;
int screenRow;
int dataPin=13;
int clockPin=12;
byte rx;
//int x,y,o;
//uint8_t o;
uint16_t o;
//int loopCount=0;
int waitVsync=1;
 
// ATMEL ATMEGA8 & 168 / ARDUINO
//
//                  +-\/-+
//            PC6  1|    |28  PC5 (AI 5)
//      (D 0) PD0  2|    |27  PC4 (AI 4)
//      (D 1) PD1  3|    |26  PC3 (AI 3)
//      (D 2) PD2  4|    |25  PC2 (AI 2)
// PWM+ (D 3) PD3  5|    |24  PC1 (AI 1)
//      (D 4) PD4  6|    |23  PC0 (AI 0)
//            VCC  7|    |22  GND
//            GND  8|    |21  AREF
//            PB6  9|    |20  AVCC
//            PB7 10|    |19  PB5 (D 13)
// PWM+ (D 5) PD5 11|    |18  PB4 (D 12)
// PWM+ (D 6) PD6 12|    |17  PB3 (D 11) PWM
//      (D 7) PD7 13|    |16  PB2 (D 10) PWM
//      (D 8) PB0 14|    |15  PB1 (D 9) PWM
//                  +----+

void stepRow() {
//digitalWrite(13,HIGH);

  updateFb(SIZEY-screenRow-1);
  screenRow--;
  if (screenRow<0) {
	  screenRow=6;
  }


//  screenRow=--screenRow%SIZEY;
  cli();
  PORTB=7;
  for (rx=0; rx<12; rx++) {
    //shiftOut(dataPin, clockPin, MSBFIRST, 255-(fb[rx][screenRow]));
    for (int8_t i=7; i>=0; i--)  {       // clock pin12 (0x10) HIGH + data pin13 a framebuffer es \
                                         // aktiv sor szerint beallitva (<<5), valamint blanking (0xf)
//      PORTB  =  0x1f | ( !( fb[rx][SIZEY-1-screenRow] & (1<<i) ) <<5);
      PORTB  =  0x1f | ( !( fb[rx][screenRow] & (1<<i) ) <<5);
      PORTB &= ~0x10; //clock pin12 LOW
    }
  }
//  PORTB=(SIZEY-1-screenRow);                     //select corresponding row
  PORTB=(screenRow);                     //select corresponding row
  sei();
//digitalWrite(13,LOW);
}

void setup() {
#ifdef SERIAL
  Serial.begin(9600);
  Serial.println("setup() running");
#endif
  for (uint8_t i = 8; i <= 13; i++) {
    pinMode(i, OUTPUT);
  }
  for (rx=0; rx<12; rx++) {
  fb[rx][3]=1;
  }
  PORTB=0xf;
  Timer1.initialize(2800);
  Timer1.attachInterrupt(stepRow);

#ifdef SERIAL
//  Serial.println("setup() done");
#endif
}

void updateFb(uint8_t row) {
  if (row == SIZEY-1) {
    //if ((waitVsync==1 && screenRow==6) || waitVsync==0) {
    //if (screenRow==6) {
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
//    }
  }
}


void loop() {
#ifdef SERIAL
  if (Serial.available()>0) {
    byte b=Serial.read();
    if (b=='q') {
      timer++;
    } else if (b=='a') {
      timer--;
    } else if (b=='w') {
      update=1;
    } else if (b=='s') {
      update=0;
    } else if (b=='e') {
      waitVsync=1;
    } else if (b=='d') {
      waitVsync=0;
    } else if (b=='r') {
      update=0;
    } else if (b=='f') {
      updateFb(SIZEY);
      Serial.println("stepped");
    }
    if (timer<=0) {
      timer=0;
    }
    Timer1.setPeriod(2900+(1<<timer));
    Serial.print("Interrupt timing is now: ");
    Serial.print(1<<timer);
    Serial.println(" ms.");
  }
#endif
}
