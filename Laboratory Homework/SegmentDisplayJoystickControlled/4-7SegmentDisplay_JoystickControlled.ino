const int pinSW = 4;
const int pinA = 12;
const int pinB = 8;
const int pinC = 5;
const int pinD = 3;
const int pinE = 2;
const int pinF = 11;
const int pinG = 6;
const int pinDP = A0;
const int pinD1 = 7;
const int pinD2 = 9;
const int pinD3 = 10;
const int pinD4 = 13;
const int pinX = A1;
const int pinY = A2;

const int segSize = 8;

const int noOfDisplays = 4;
const int noOfDigits = 10;

int xValue = 0;
int yValue = 0;
int minThreshold= 400;
int maxThreshold= 600;
int swState = LOW;
int lastSwState = HIGH;
int shouldCheck = 1;
int numberChosen = 0;
int digit = noOfDisplays - 1;

bool joyMoved = false;

unsigned long lastDebounceTime = 0;

// segments array, similar to before
int segments[segSize] = {
  pinA, pinB, pinC, pinD, pinE, pinF, pinG, pinDP
};
// digits array, to switch between them easily
int digits[noOfDisplays] = {
  pinD1, pinD2, pinD3, pinD4
 };   

int number[4] = {
  0, 0, 0, 0
};
 
byte digitMatrix[noOfDigits][segSize - 1] = {
// a  b  c  d  e  f  g
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1}  // 9
};


void displayNumber(byte digit, byte decimalPoint) {
  for (int i = 0; i < segSize - 1; i++) {
    digitalWrite(segments[i], digitMatrix[digit][i]);
  }

  // write the decimalPoint to DP pin
  digitalWrite(segments[segSize - 1], decimalPoint);
}

// a function to make the number blink when we are choosing the display
void blinkNumber(byte digit) {
  for (int i = 0; i < segSize - 1; i++) {
    digitalWrite(segments[i], 0);
  }
  delay(75);
  for (int i = 0; i < segSize - 1; i++) {
    digitalWrite(segments[i], digitMatrix[digit][i]);
  }
}

// activate the display no. received as param
void showDigit(int num) {
  for (int i = 0; i < noOfDisplays; i++) {
    digitalWrite(digits[i], HIGH);
  }
  digitalWrite(digits[num], LOW);
}
         

void setup() {
  for (int i = 0; i < segSize - 1; i++){
    pinMode(segments[i], OUTPUT);  
  }
  for (int i = 0; i < noOfDisplays; i++){
    pinMode(digits[i], OUTPUT);  
  }
  pinMode(pinSW, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  swState = digitalRead(pinSW);
  Serial.println(digit);
  if(numberChosen == 0){
    //using the function to blink
    showDigit(digit);
    blinkNumber(number[digit]);
    if (xValue < minThreshold && joyMoved == false) {
      if (digit > 0) {
          digit--;
      } else {
          digit = noOfDisplays - 1;
      }
      joyMoved = true;
    }
  
   // On Ox axis, if the value is bigger than a chosen max threshold, then
   // increase by 1 the digit value
    if (xValue > maxThreshold && joyMoved == false) {
      if (digit < noOfDisplays - 1) {
          digit++;
      } else {
          digit = 0;
      }
      joyMoved = true;
      }
    
      if (xValue >= minThreshold && xValue <= maxThreshold) {
        joyMoved = false;
      }
  }

  else{
    digitalWrite(segments[segSize - 1], HIGH);
    if (yValue < minThreshold && joyMoved == false) {
      if (number[digit] > 0) {
          number[digit]--;
      } else {
          number[digit] = 9;
      }
      joyMoved = true;
    }
  
   
    if (yValue > maxThreshold && joyMoved == false) {
      if (number[digit] < 9) {
          number[digit]++;
      } else {
          number[digit] = 0;
    }
    joyMoved = true;
    }
  
    if (yValue >= minThreshold && yValue <= maxThreshold) {
      joyMoved = false;
    }
  }
  //code for the button
  if(millis() - lastDebounceTime > 1000){
    shouldCheck = 1;
  }
  swState = digitalRead(pinSW);
  //verify if the button was pushed
  if(swState != lastSwState && shouldCheck == 1){
      lastDebounceTime = millis();
      if(numberChosen == 1){
        numberChosen = 0;
      }
      else{
        numberChosen = 1;
      }
      shouldCheck = 0;
    }
  lastSwState = swState;
  //display the number
  for (int j = noOfDisplays - 1; j >= 0; j--){
    showDigit(j);
    displayNumber(number[j], HIGH);
    delay(5);
  }
}
