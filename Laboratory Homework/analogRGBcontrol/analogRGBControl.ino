const int redPin = 9;
const int greenPin = 10;
const int bluePin = 11;
const int redPot = A0;
const int greenPot = A1;
const int bluePot = A2;
int redValue = 0;
int greenValue = 0;
int blueValue = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(redPot, INPUT);
  pinMode(greenPot, INPUT);
  pinMode(bluePot, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  redValue = map(analogRead(redPot), 0, 1023, 0, 255);
  greenValue = map(analogRead(greenPot), 0, 1023, 0,255);
  blueValue = map(analogRead(bluePot), 0, 1023, 0, 255);
  analogWrite(redPin, redValue);
  analogWrite(greenPin, greenValue);
  analogWrite(bluePin, blueValue);
  //to check if the problem is in the program or in the hardware
  Serial.println(redValue);
  Serial.println(greenValue);
  Serial.println(blueValue);
  delay(1000);

  
}
