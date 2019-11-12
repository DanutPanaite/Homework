#include "pitches.h"

const int passiveBuzzerPin = A1;
const int activeBuzzerPin = 11;
const int pushButton = 2;
const int threshold = 10;

int speakerValue = 0;
int buttonState = 0;
int shouldPlaySong = 0;

unsigned long timeBeforeSong = 5000;
unsigned long lastKnock = 0;

int melody[] = {
NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};
// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
4, 8, 8, 4, 4, 4, 4, 4
};

void playSong(){
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    buttonState = digitalRead(pushButton);
    if(buttonState == 1){
      return;
    }
    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(activeBuzzerPin, melody[thisNote], noteDuration);
    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(8);
  }
}
void setup() {
  // put your setup code here, to run once:
  pinMode(passiveBuzzerPin, INPUT);
  pinMode(pushButton, INPUT);
  pinMode(activeBuzzerPin, OUTPUT);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  buttonState = digitalRead(pushButton);
  speakerValue = analogRead(passiveBuzzerPin);
  if(speakerValue > threshold){
    lastKnock = millis();
    shouldPlaySong = 1; // a variable for keeping track if i should play the song or not
  }
  if(shouldPlaySong != 0){
    if(millis() > lastKnock + timeBeforeSong){
      playSong();
      shouldPlaySong = 0;
    }
  }
}
