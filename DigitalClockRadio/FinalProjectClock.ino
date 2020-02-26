#include <LiquidCrystal.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <EEPROM.h>
#include <radio.h>
#include <RDA5807M.h>
#include "pitches.h"

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

#define FIX_BAND RADIO_BAND_FM
#define FIX_VOLUME 15

RDA5807M radio;

const int pinSW = 13;
const int buzzerPin = 10;
const int backLight = 9;

const int potPin = A0;
const int timeMenu = 0;
const int dateMenu = 1;
const int alarmMenu = 2;
const int radioMenu = 3;

const int contrastNumber = 140;

int currentMenu = 0;
int swState = HIGH;
int lastSwState = HIGH;
int potValue = 0;

int alarmTime[2] = {0};
int alarmHour = 0;
int alarmMinute = 0;
int alarmSet = 0;
int melody[] = {
NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};
// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
4, 8, 8, 4, 4, 4, 4, 4
};

int stationFQ[8]= {8800, 8900, 9270, 10020,  10060,   10190,      10670, 10730};
               //Impuls, Zu, Tananana, Virgin, Rock FM, Romantic, Europa, Smart
int currentStation = 0;

unsigned long alarmDelay = 60000;
unsigned long alarmStart = 0;

bool changingAlarm = false;
bool playAlarm = false;
bool changingFQ = false;



tmElements_t tm;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.begin(16,2);
  pinMode(pinSW, INPUT_PULLUP);
  pinMode(backLight, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(potPin, INPUT);
  analogWrite(backLight, contrastNumber);
  EEPROM.get(0, alarmTime);
  alarmHour = alarmTime[0];
  alarmMinute = alarmTime[1];

  radio.init();

  radio.debugEnable();

  radio.setBandFrequency(FIX_BAND, stationFQ[currentStation]);
  radio.setVolume(FIX_VOLUME);
  radio.setMono(false);
  radio.setMute(false);
}

void loop() {
  showMenu();
  checkAlarm(tm);
  char s[12];
  radio.formatFrequency(s, sizeof(s));
  Serial.print("Station:"); 
  Serial.println(s);
  
  Serial.print("Radio:"); 
  radio.debugRadioInfo();
  
  Serial.print("Audio:"); 
  radio.debugAudioInfo();
}

void printDigits(int number) {
  if (number >= 0 && number < 10) {
    lcd.print('0');
  }
  lcd.print(number);
}

void printTime(tmElements_t tm) {

  if (RTC.read(tm)) {
    printDigits(tm.Hour);
    lcd.write(':');
    printDigits(tm.Minute);
    lcd.write(':');
    printDigits(tm.Second);
  }
}

void printDate(tmElements_t tm) {
  if(RTC.read(tm)){
    printDigits(tm.Day);
    lcd.write('/');
    printDigits(tm.Month);
    lcd.write('/');
    lcd.print(tmYearToCalendar(tm.Year));
    lcd.println();
  }
}
void alarm() {
  for (int thisNote = 0; thisNote < 8; thisNote++) {
  // to calculate the note duration, take one second divided by the note type.
  //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
  int noteDuration = 1000 / noteDurations[thisNote];
  tone(buzzerPin, melody[thisNote], noteDuration);
  // to distinguish the notes, set a minimum time between them.
  // the note's duration + 30% seems to work well:
  int pauseBetweenNotes = noteDuration * 1.30;
  delay(pauseBetweenNotes);
  // stop the tone playing:
  swState = digitalRead(pinSW);
  if (swState !=  lastSwState) {             
    if (swState == LOW) {
      playAlarm = false;                      
    }
  }
  lastSwState = swState;
  noTone(8);
  }
}

void checkAlarm(tmElements_t tm) {
  if(millis() - alarmStart > alarmDelay){
    if(alarmHour == tm.Hour && alarmMinute == tm.Minute) {
      playAlarm = true;
      alarmStart = millis();
    }
  }
  while(playAlarm == true) {
    alarm();
  }
}

void showMenu() {
  swState = digitalRead(pinSW);
  switch (currentMenu) {         //keeping track on which option of the main menu the player is on
       case 0:
         lcd.setCursor(0,0);
         lcd.print("TIME: ");
         printTime(tm);
         lcd.setCursor(1,1);
         lcd.print("BACK");
         lcd.setCursor(12,1);
         lcd.print("NEXT");
       break;
       case 1:    
         lcd.setCursor(0,0);
         lcd.print("DATE: ");
         printDate(tm);
         lcd.setCursor(1,1);
         lcd.print("BACK");
         lcd.setCursor(12,1);
         lcd.print("NEXT");
       break;
       case 2:    
         lcd.setCursor(0,0);
         lcd.print("ALARM: ");
         printDigits(alarmHour);
         lcd.print(":");
         printDigits(alarmMinute);
         lcd.setCursor(1,1);
         lcd.print("BACK");
         lcd.setCursor(7,1);
         lcd.print("CNG");
         lcd.setCursor(12,1);
         lcd.print("NEXT");
       break;
       case 3:    
         lcd.setCursor(0,0);
         lcd.print("RADIO: ");
         lcd.print(stationFQ[currentStation]);
         lcd.print(" MHz");
         lcd.setCursor(1,1);
         lcd.print("BACK");
         lcd.setCursor(7,1);
         lcd.print("CNG");
         lcd.setCursor(12,1);
         lcd.print("NEXT");
       break;
     }
    switch (potValue) {         //keeping track on which option of the main menu the player is on
        case 0:
          lcd.setCursor(11,1);
          lcd.print(" ");
          lcd.setCursor(6,1);
          lcd.print(" ");   
          lcd.setCursor(0,1);
          lcd.print(">");
        break;
        case 1:    
          lcd.setCursor(11,1);
          lcd.print(" ");
          lcd.setCursor(0,1);
          lcd.print(" ");
          lcd.setCursor(6,1);
          lcd.print(">");
        break;
        case 2:    
          lcd.setCursor(0,1);
          lcd.print(" ");
          lcd.setCursor(6,1);
          lcd.print(" ");   
          lcd.setCursor(11,1);
          lcd.print(">");
        break;
        case 3:    
          lcd.setCursor(0,1);
          lcd.print(" ");
          lcd.setCursor(6,1);
          lcd.print(" ");   
          lcd.setCursor(11,1);
          lcd.print(">");
        break;
      }
  if(changingAlarm == false && changingFQ == false) {
    potValue = map(analogRead(potPin),0, 1023, 0, 3);
    if((currentMenu == timeMenu || currentMenu == dateMenu) && potValue == 1)
      potValue = 2;
      if (swState !=  lastSwState) {             
        if (swState == LOW) {
          if(potValue == 2) {
            lcd.clear();
            if(currentMenu < 3)
              currentMenu++;
            else
              currentMenu = 0;
          }
          if(potValue == 0) {
            lcd.clear();
            if(currentMenu > 0)
              currentMenu--;
            else
              currentMenu = 3;
          }
          if(potValue == 1) {
            if(currentMenu == alarmMenu){
              changingAlarm = true; 
            }
            if(currentMenu == radioMenu){
              changingFQ = true;
            }
          }
        }
    }
    lastSwState = swState;
    }
    if(changingAlarm) {
      alarmSet = map(analogRead(potPin), 0, 1023, 0, 1440);
      alarmHour = alarmSet/60;
      alarmMinute = alarmSet%60; 
      if (swState !=  lastSwState) {             
        if (swState == LOW) {
          alarmTime[0] = alarmHour;
          alarmTime[1] = alarmMinute;
          EEPROM.put(0, alarmTime);
          changingAlarm = false;
        }
      }
      lastSwState = swState;
    }
    if(changingFQ) {
      currentStation = map(analogRead(potPin), 0, 1023, 0, 8);
      if (swState !=  lastSwState) {             
        if (swState == LOW) {
          radio.setBandFrequency(FIX_BAND, stationFQ[currentStation]);
          changingFQ = false;
        }
    }
    lastSwState = swState;
    }
}
