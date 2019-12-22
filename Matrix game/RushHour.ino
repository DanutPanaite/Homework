#include "LedControl.h" //  need the library
#include <LiquidCrystal.h>
#include <EEPROM.h>

LiquidCrystal lcd(13, 9, 5, 4, 3, 2);

LedControl lc = LedControl(12, 11, 10, 1); //DIN, CLK, LOAD, No. DRIVER
 
// pin 12 is connected to the MAX7219 pin 1
// pin 11 is connected to the CLK pin 13
// pin 10 is connected to LOAD pin 12
// 1 as we are only using 1 MAX7219
const int pinX = A0;
const int pinY = A1;
const int pinSW = 7;
const int pinBacklight = 6;

int collisionMatrix[8][8] = {
  {0, 0, 0, 0, 0, 0, 0, 0}, 
  {0, 0, 0, 0, 0, 0, 0, 0}, 
  {0, 0, 0, 0, 0, 0, 0, 0}, 
  {0, 0, 0, 0, 0, 0, 0, 0}, 
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}, 
  {0, 0, 0, 0, 0, 0, 0, 0}, 
  {0, 0, 0, 0, 0, 0, 0, 0},
};

//custom characters for the hp and powers to be shown on the lcd

byte heartShape[] = {
  B00000,
  B01010,
  B11111,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000
};

byte shieldShape[] = {
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000
};


int xValue = 0;
int yValue = 0;
int minThreshold = 400;
int maxThreshold = 600;
int swState = HIGH;
int lastSwState = HIGH;

int playerXCoordinate = 2;
int playerYCoordinate = 5;
int botXCoordinate = 8;
int botYCoordinate = 2;
int Score = 0;
int scoreIncrement = 1;

bool joyMoved = false;
bool gameStarted = false;
bool isGameOver = false;
bool shouldCreateBot = true;
bool takenDamage = false;
bool isInvincible = false;
bool didCheckLevel = false;

int carSpeed = 600;
int botSpawnRate = 12.5 * carSpeed;
int currentLevel = 1;
int numberOfLives = 3;
int lastNumberOfLives = 3;
int numberOfShields = 3;
int lastNumberOfShields = 3;
int defaultLevel = 1;
int highScore;
int chosenLevel = 1;

// variables for the menu

int menuLevel = 0;
int menuOption = 0;
int linkPart = 0;
int infoMenuOption = 0;
int secondaryMenu = 0;
int brightness = 128;
int endMenuOption = 0;

bool gameJustOpened = true;
bool inMainMenu = true;
bool isScrolled = false;
bool lockedLevel = false;
bool shouldChangeLevel = false;
bool lockedOnLink = false;
bool lockedBrightness = false;

unsigned long lastRefreshTime = 0;                  //for the cars to update
unsigned long lastSpawnTime = 0;                    //to spawn a new car
unsigned long lastLevelChange = 0;
unsigned long levelTimer = 15000;
unsigned long lastIncrementTime = 0;
unsigned long lastDamageTaken = 0;
unsigned long lastBlink = 0;
unsigned long elapsedTime = 0;
unsigned long endTime = 0;                        
unsigned long levelReset = 0;                     //a variable that i use to "reset" the millis
unsigned long startInvincibility = 0;

void setup()
{
  // the zero refers to the MAX7219 number, it is zero for 1 chip
  lc.shutdown(0, false); // turn off power saving, enables display
  lc.setIntensity(0, 2); // sets brightness (0~15 possible values)
  lc.clearDisplay(0);// clear screen
  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);
  pinMode(pinSW, INPUT_PULLUP);
  lcd.begin(16,2);
  lcd.createChar(0, heartShape);
  lcd.createChar(1, shieldShape);
  EEPROM.get(0, highScore);
  pinMode(pinBacklight, OUTPUT);
}

void SpawnBot()                         //this function will spawn a bot at the designated coordinates, modifying the collision matrix
{
  lc.setLed(0,botXCoordinate, botYCoordinate, true);
  lc.setLed(0,botXCoordinate - 1, botYCoordinate, true);
  lc.setLed(0,botXCoordinate, botYCoordinate + 1, true);
  lc.setLed(0,botXCoordinate, botYCoordinate - 1, true);
  lc.setLed(0,botXCoordinate + 1, botYCoordinate, true);
  lc.setLed(0,botXCoordinate + 2, botYCoordinate, true);
  lc.setLed(0,botXCoordinate + 2, botYCoordinate + 1, true);
  lc.setLed(0,botXCoordinate + 2, botYCoordinate - 1, true);
  //since i want the bot to fade out, i have to check if i have to modify the collision matrix, otherwise the game will get stuck trying to acces a nonexistent position
  if(7-botXCoordinate <= 7 && 7- botXCoordinate >= 0){
    collisionMatrix[7-botXCoordinate][7-botYCoordinate] = 1;
    collisionMatrix[7-botXCoordinate][7-(botYCoordinate + 1)] = 1;
    collisionMatrix[7-botXCoordinate][7-(botYCoordinate - 1)] = 1;
  }
  if(7-(botXCoordinate - 1) <= 7 && 7- botXCoordinate >= 0)
    collisionMatrix[7-(botXCoordinate - 1)][7-botYCoordinate] = 1;
  if(7-(botXCoordinate + 1) <= 7 && 7 - (botXCoordinate + 1) >= 0)
    collisionMatrix[7-(botXCoordinate + 1)][7-botYCoordinate] = 1;
  if(7-(botXCoordinate + 2) <= 7 && 7 - (botXCoordinate + 2) >= 0){
    collisionMatrix[7-(botXCoordinate + 2)][7-botYCoordinate] = 1;
    collisionMatrix[7-(botXCoordinate + 2)][7-(botYCoordinate + 1)] = 1;
    collisionMatrix[7-(botXCoordinate + 2)][7-(botYCoordinate - 1)] = 1;
  }
  
}

void CreateBot()         //it creates random coordinates for the bot cars
{
  botXCoordinate = 9;
  int randomNumber = random(0,11);
  if(randomNumber % 2 == 0)
    botYCoordinate = 5;
  else
    botYCoordinate = 2;
  SpawnBot();
}

void CreateWalls()      
{
  int row = 0;
  for (int col = 0; col < 8; col++)
  {
    lc.setLed(0, col, row, true); // turns on LED at col, row
    delay(1);
  }
  row = 7;
  for (int col = 0; col < 8; col++)
  {
    lc.setLed(0, col, row, true); // turns on LED at col, row
    delay(1);
  }
}

void SpawnPlayer()
{
  lc.setLed(0,playerXCoordinate, playerYCoordinate, true);
  lc.setLed(0,playerXCoordinate + 1, playerYCoordinate, true);
  lc.setLed(0,playerXCoordinate, playerYCoordinate + 1, true);
  lc.setLed(0,playerXCoordinate, playerYCoordinate - 1, true);
  lc.setLed(0,playerXCoordinate - 1, playerYCoordinate, true);
  lc.setLed(0,playerXCoordinate - 2, playerYCoordinate, true);
  lc.setLed(0,playerXCoordinate - 2, playerYCoordinate + 1, true);
  lc.setLed(0,playerXCoordinate - 2, playerYCoordinate - 1, true);
}

void ClearLastPlayerPosition()              //every time the player changes position i use this function to turn off the led's thus avoiding the matrix blinking all the time
{
  lc.setLed(0,playerXCoordinate, playerYCoordinate, false);
  lc.setLed(0,playerXCoordinate + 1, playerYCoordinate, false);
  lc.setLed(0,playerXCoordinate, playerYCoordinate + 1, false);
  lc.setLed(0,playerXCoordinate, playerYCoordinate - 1, false);
  lc.setLed(0,playerXCoordinate - 1, playerYCoordinate, false);
  lc.setLed(0,playerXCoordinate - 2, playerYCoordinate, false);
  lc.setLed(0,playerXCoordinate - 2, playerYCoordinate + 1, false);
  lc.setLed(0,playerXCoordinate - 2, playerYCoordinate - 1, false);
}

void ClearLastBotPosition()               //same principle as for the player
{
  lc.setLed(0,botXCoordinate, botYCoordinate, false);
  lc.setLed(0,botXCoordinate - 1, botYCoordinate, false);
  lc.setLed(0,botXCoordinate, botYCoordinate + 1, false);
  lc.setLed(0,botXCoordinate, botYCoordinate - 1, false);
  lc.setLed(0,botXCoordinate + 1, botYCoordinate, false);
  lc.setLed(0,botXCoordinate + 2, botYCoordinate, false);
  lc.setLed(0,botXCoordinate + 2, botYCoordinate + 1, false);
  lc.setLed(0,botXCoordinate + 2, botYCoordinate - 1, false);
  //since i want the bot to fade out, i have to check if i have to modify the collision matrix, otherwise the game will get stuck trying to acces a nonexistent position
  if(7-botXCoordinate <= 7 && 7- botXCoordinate >= 0){
    collisionMatrix[7-botXCoordinate][7-botYCoordinate] = 0;
    collisionMatrix[7-botXCoordinate][7-(botYCoordinate + 1)] = 0;
    collisionMatrix[7-botXCoordinate][7-(botYCoordinate - 1)] = 0;
  }
  if(7-(botXCoordinate - 1) <= 7 && 7- botXCoordinate >= 0)
    collisionMatrix[7-(botXCoordinate - 1)][7-botYCoordinate] = 0;
  if(7-(botXCoordinate + 1) <= 7 && 7 - (botXCoordinate + 1) >= 0)
    collisionMatrix[7-(botXCoordinate + 1)][7-botYCoordinate] = 0;
  if(7-(botXCoordinate + 2) <= 7 && 7 - (botXCoordinate + 2) >= 0){
    collisionMatrix[7-(botXCoordinate + 2)][7-botYCoordinate] = 0;
    collisionMatrix[7-(botXCoordinate + 2)][7-(botYCoordinate + 1)] = 0;
    collisionMatrix[7-(botXCoordinate + 2)][7-(botYCoordinate - 1)] = 0;
  }
}

void CheckCollision()               
{
  if(takenDamage == false)
    if(collisionMatrix[7-playerXCoordinate][7-playerYCoordinate] == 1 || collisionMatrix[7-(playerXCoordinate+1)][7-playerYCoordinate] == 1 || collisionMatrix[7-(playerXCoordinate-1)][7-playerYCoordinate] == 1 || collisionMatrix[7-(playerXCoordinate-2)][7-playerYCoordinate] == 1)
      if(numberOfLives > 1){
        numberOfLives -= 1;
        takenDamage = true;
        lastDamageTaken = millis();
      }
      else{
        numberOfLives = 0;
        GameUI();       // i want to show the update game UI ( with no lives)
        endGame();
      }
  if(takenDamage == true){            // the player will blink so he knows he is invincible after taking damage
    ClearLastPlayerPosition();
    if(millis() - lastBlink >= 500){
      SpawnPlayer();
      lastBlink = millis();
    }
  }
  if(millis() - lastDamageTaken >= 2000)
    takenDamage = false;
    
}

void endGame()        //when the game is over i make sure i print to the lcd the important information
{
  isGameOver = true;
  elapsedTime = millis()/1000;
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("GAME OVER");
  endTime = millis();
  while(millis() - endTime < 1000)
    ;
  lcd.clear();
  if(highScore < Score){        // saving the highscore if needed
    EEPROM.put(0, Score);
    lcd.setCursor(0,0);
    lcd.print("You got a ");
    lcd.setCursor(2,1);
    lcd.print("high score!");
  }
  else{
    lcd.setCursor(0,0);
    lcd.print("You were ");
    lcd.print(highScore - Score);
    lcd.setCursor(2,1);
    lcd.print("points behind");
  }
  endTime = millis();
  while(millis() - endTime < 3000)
    ;
  lcd.clear();
}

void RunGame()            // the function that is running the game
{
  SpawnPlayer();
  CreateWalls();
  if(didCheckLevel == false)
    if(defaultLevel != currentLevel){
      shouldChangeLevel = true;
      currentLevel--;
      didCheckLevel = true;
    }
  else
    didCheckLevel = true;
  if(((millis() - levelReset - lastLevelChange) >= levelTimer && currentLevel < 7) || shouldChangeLevel == true){      // i want the level to change based on seconds survived
    currentLevel++;
    shouldChangeLevel = false;
    lastLevelChange = millis() - levelReset;
    switch (currentLevel) {
      case 0:
      break;
      case 1:
        scoreIncrement = 1;    
        carSpeed = 600;
        botSpawnRate = 12.5 * carSpeed;
        levelTimer = 15000;
        break;
      case 2:    
        carSpeed = 450;
        levelTimer = 15000;
        botSpawnRate = 12.5 * carSpeed;
        scoreIncrement = 2;
        break;
      case 3:    
        carSpeed = 250;
        levelTimer = 30000;
        botSpawnRate = 12.5 * carSpeed;
        scoreIncrement = 3;
        break;
      case 4:    
        carSpeed = 125;
        levelTimer = 60000;
        botSpawnRate = 12.5 * carSpeed;
        scoreIncrement = 4;
        break;
      case 5:
        carSpeed = 80;
        levelTimer = 60000;
        botSpawnRate = 12.5 * carSpeed;
        scoreIncrement = 5;
        break;
      case 6:
        carSpeed = 60;
        botSpawnRate = 12.5 * carSpeed;
        scoreIncrement = 6;
        levelTimer = 20000;
        break;
      case 7:
        carSpeed = 40;
        botSpawnRate = 12.5 * carSpeed;
        scoreIncrement = 7;
        break;
    }
  }
  if( millis() - lastIncrementTime >= 1000){          //incrementing the score every second
    Score += scoreIncrement;
    lastIncrementTime = millis();
  }
  if(millis() - lastRefreshTime >= carSpeed){        //moving the bot cars based on their speed
    ClearLastBotPosition();
    botXCoordinate -= 1;
    lastRefreshTime = millis();
  }
  if(millis() - lastSpawnTime >= botSpawnRate && shouldCreateBot == true){    //checking to see if i need to create another car
    CreateBot();
    shouldCreateBot = false;
    lastSpawnTime = millis();
  }
  SpawnBot();
  swState = digitalRead(pinSW);
  if(isInvincible == false){                //code for the invincibility
    swState = digitalRead(pinSW);
    if (swState !=  lastSwState) {
      if (swState == HIGH && numberOfShields > 0) {
        isInvincible = true;
        numberOfShields--;
        startInvincibility = millis();
      }
    }
    CheckCollision();
  }
  else 
  {
    if(millis() - startInvincibility >= 2000)
      isInvincible = false;
    ClearLastPlayerPosition();
    if(millis() - lastBlink >= 500){
      SpawnPlayer();
      lastBlink = millis();
    }
  }
  lastSwState = swState;
  if(isGameOver == false)
    GameUI();                                   //updating the UI everytime
  lastNumberOfLives = numberOfLives;
  lastNumberOfShields = numberOfShields;
  if(botXCoordinate <= -3)
    shouldCreateBot = true;
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  if (xValue < minThreshold && joyMoved == false) {           //code for moving the player
      if (playerXCoordinate > 2) {
        ClearLastPlayerPosition();
        playerXCoordinate -= 1;
      } else {
        playerXCoordinate = playerXCoordinate;
      }
      joyMoved = true;
    }
  if (xValue > maxThreshold && joyMoved == false) {
      if (playerXCoordinate < 6) {
        ClearLastPlayerPosition();
        playerXCoordinate += 1;
      } else {
        playerXCoordinate = playerXCoordinate;
      }
      joyMoved = true;
  }
  if (yValue < minThreshold && joyMoved == false) {
      if (playerYCoordinate < 3) {
        ClearLastPlayerPosition();
        playerYCoordinate += 3;
      } else {
        playerYCoordinate = playerYCoordinate;
      }
      joyMoved = true;
    }
  if (yValue > maxThreshold && joyMoved == false) {
      if (playerYCoordinate > 3) {
        ClearLastPlayerPosition();
        playerYCoordinate -= 3;
      } else {
        playerYCoordinate = playerYCoordinate;
      }
      joyMoved = true;
  }
  if (xValue >= minThreshold && xValue <= maxThreshold && yValue >= minThreshold && yValue <= maxThreshold) {
      joyMoved = false;
  }
}

void ShowMenu()
{
  swState = digitalRead(pinSW);
  if(inMainMenu == true){                    //printing a welcoming message 
    if(gameJustOpened == true){
      lcd.setCursor(4,0);
      lcd.print("Welcome");
      lcd.setCursor(3,1);
      lcd.print("Challenger!");
    }
    while(gameJustOpened == true){
      if(millis() > 2000){
        gameJustOpened = false;
        lcd.clear();
      }
    }
    if(menuLevel == 0){              //showing the main menu
      lcd.setCursor(1, 0);
      lcd.print("PLAY");
      lcd.setCursor(1,1);
      lcd.print("OPTIONS");
      lcd.setCursor(10,0);
      lcd.print("HSCORE");
      lcd.setCursor(10,1);
      lcd.print("INFO");
    }
    switch (menuOption) {         //keeping track on which option of the main menu the player is on
        case 0:    
          lcd.setCursor(0,0);
          lcd.print(">");
        break;
        case 1:    
          lcd.setCursor(0,1);
          lcd.print(">");
        break;
        case 2:    
          lcd.setCursor(9,0);
          lcd.print(">");
        break;
        case 3:   
          lcd.setCursor(9,1);
          lcd.print(">");
        break;
      }
    xValue = analogRead(pinX);
    yValue = analogRead(pinY);
    if (xValue < minThreshold && joyMoved == false) {                  ///updating the option based on input
      if (menuOption < 3) {
        menuOption += 1;
      }
      joyMoved = true;
    } 
    if (xValue > maxThreshold && joyMoved == false) {
        if (menuOption > 0) {
          menuOption -= 1;
        } 
        joyMoved = true;
    }
    if (yValue < minThreshold && joyMoved == false) {
        if (menuOption > 1) {
          menuOption -= 2;
        }
        joyMoved = true;
      }
    if (yValue > maxThreshold && joyMoved == false) {
        if (menuOption < 2 ) {
          menuOption += 2;
        }
        joyMoved = true;
    }
    if (xValue >= minThreshold && xValue <= maxThreshold && yValue >= minThreshold && yValue <= maxThreshold) {
        joyMoved = false;
    }
    if(joyMoved == true){
      lcd.clear();
    }
    if (swState !=  lastSwState) {             //pressing the joystick will enter a secondary menu, inMainMenu keeps track of that
      if (swState == HIGH) {
        inMainMenu = false;
        lcd.clear();
      }
    }
  }
  else 
    switch (menuOption) {     //for each option of the main menu, there is a separate action that must be done, so we use a switch and a couple of functions
      case 0:    
        gameStarted = true;
        lastSpawnTime = millis();
        break;
      case 1: 
        OptionsMenu();
        break;
      case 2:
        lcd.setCursor(0,0);
        lcd.print("Hi-Score: ");
        lcd.print(highScore);
        lcd.setCursor(9,1);
        lcd.print(">");
        lcd.setCursor(10,1);
        lcd.print("BACK");
        if (swState !=  lastSwState) {
          if (swState == HIGH) {
            inMainMenu = true;
            lcd.clear();
          }
        }
        break;
      case 3:
        InfoMenu();
        break;
    }
    lastSwState = swState;
}

void OptionsMenu()                                   //this function takes care of the options menu along with modifying the values for the starting level or for the lcd brightness if the player wishes so
{
    xValue = analogRead(pinX);
    yValue = analogRead(pinY);
    if(lockedLevel == true){
      if (xValue < minThreshold && joyMoved == false) {
        if (currentLevel > 0) {
          currentLevel -= 1;
        }
        joyMoved = true;
      } 
      if (xValue > maxThreshold && joyMoved == false) {
        if (currentLevel < 7 ) {
          currentLevel += 1;
        } 
        joyMoved = true;
      }
      if (xValue >= minThreshold && xValue <= maxThreshold ) {
        joyMoved = false;
      }
      if(joyMoved == true){
        lcd.clear();
      }
      if (swState !=  lastSwState) {
        if (swState == HIGH) {
          lockedLevel = false;
          lastSwState = swState;
          lcd.clear();
        }
      }
    }
    chosenLevel = currentLevel;
    if(lockedBrightness == true){
      if (xValue < minThreshold && joyMoved == false) {
        if (brightness > 31) {
          brightness -= 32;
        }
        else 
          brightness = 224;
        joyMoved = true;
      } 
      if (xValue > maxThreshold && joyMoved == false) {
        if (brightness < 224 ) {
          brightness += 32;
        }
        else
          brightness = 0;
        joyMoved = true;
      }
      if (xValue >= minThreshold && xValue <= maxThreshold ) {
        joyMoved = false;
      }
      if(joyMoved == true){
        lcd.clear();
      }
      if (swState !=  lastSwState) {
        if (swState == HIGH) {
          lockedBrightness = false;
          lastSwState = swState;
          lcd.clear();
        }
      }
    }
    if(lockedLevel == false && lockedBrightness == false){
      if (xValue < minThreshold && joyMoved == false) {
        if (secondaryMenu < 2) {
          secondaryMenu += 1;
        }
        joyMoved = true;
      } 
      if (xValue > maxThreshold && joyMoved == false) {
          if (secondaryMenu > 0) {
            secondaryMenu -= 1;
          } 
          joyMoved = true;
      }
      if (xValue >= minThreshold && xValue <= maxThreshold ) {
          joyMoved = false;
      }
      if(joyMoved == true){
        lcd.clear();
      }
    }
  switch(secondaryMenu){
    case 0:
      lcd.setCursor(0,0);
      lcd.print(">");
      isScrolled = false;
      if (swState !=  lastSwState) {
        if (swState == HIGH) {
          lockedLevel = true;
          lcd.clear();
        }
      }
      break;
    case 1:
      lcd.setCursor(0,1);
      lcd.print(">");
      isScrolled = false;
      if (swState !=  lastSwState) {
        if (swState == HIGH) {
          lockedBrightness = true;
          lcd.clear();
        }
      }
      break;
    case 2:
      lcd.setCursor(1,0);
      lcd.print("BRIGHTNESS: ");
      lcd.print(brightness/32);
      lcd.setCursor(1,1);
      lcd.print("BACK");
      lcd.setCursor(0,1);
      lcd.print(">");
      isScrolled = true;
      if (swState !=  lastSwState) {
        if (swState == HIGH) {
          inMainMenu = true;
          lcd.clear();
        }
      }
  }
  if(isScrolled == false){                                    //having 3 options to show, i need to keep track if the lcd is scrolled and print the according options
    lcd.setCursor(1,0);
    lcd.print("STARTLEVEL: ");
    lcd.print(currentLevel);
    lcd.setCursor(1,1);
    lcd.print("BRIGHTNESS: ");
    lcd.print(brightness/32);
  }
}

void InfoMenu()                                             
{
  swState = digitalRead(pinSW);
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  if(lockedOnLink == false){                        //i split the link in three parts to cycle through with the joystick
    if (xValue < minThreshold && joyMoved == false) {
      if (infoMenuOption < 4) {
        infoMenuOption += 1;
      }
      joyMoved = true;
    } 
    if (xValue > maxThreshold && joyMoved == false) {
        if (infoMenuOption > 0) {
          infoMenuOption -= 1;
        } 
        joyMoved = true;
    }
    if(joyMoved == true){
      lcd.clear();  
    }
    if(xValue >= minThreshold && xValue <= maxThreshold)
      joyMoved = false;
  }
  else {
    if (yValue < minThreshold && joyMoved == false) {
        if (linkPart > 0) {
          linkPart -= 1;
        }
        joyMoved = true;
      }
    if (yValue > maxThreshold && joyMoved == false) {
        if (linkPart < 2 ) {
          linkPart += 1;
        }
        joyMoved = true;
    }
    if (yValue >= minThreshold && yValue <= maxThreshold) {
        joyMoved = false;
    }
    if(joyMoved == true){
      lcd.clear();  
    }
    if (swState !=  lastSwState) {
      if (swState == HIGH) {
        lockedOnLink = false;
        lastSwState = swState;
      }
    }
  }
  if(isScrolled == false) {
    lcd.setCursor(1,0);
    lcd.print("Danut Panaite");
    lcd.setCursor(1,1);
    switch(linkPart) {
      case 0:
        lcd.print("https://github.");
        break;
      case 1:
        lcd.print("com/DanutPanait");
        break;
      case 2:
        lcd.print("e/Lab-Homework");
        break;   
    }
  }
  else {
    lcd.setCursor(1,0);
    lcd.print("RushHour");
    lcd.setCursor(1,1);
    lcd.print("@UnibucRobotics");   
  }
  switch(infoMenuOption) {
    case 0:
      lcd.setCursor(0,0);
      lcd.print(">");
      isScrolled = false;
      break;
    case 1:
      lcd.setCursor(0,1);
      lcd.print(">");
      if (swState !=  lastSwState) {
        if (swState == HIGH) {
          lockedOnLink = true;
          lcd.clear();
        }
      }
      isScrolled = false;
      break;
    case 2:
      lcd.setCursor(0,0);
      lcd.print(">");
      isScrolled = true;
      break;
    case 3:
      lcd.setCursor(0,1);
      lcd.print(">");
      isScrolled = true;
      break;
  }
  if(infoMenuOption != 1)
    if (swState !=  lastSwState) {          //pressing the button on any option other than the link, will take you back to the main menu
      if (swState == HIGH) {
        inMainMenu = true;
        lcd.clear();
      }
    }
  lastSwState = swState;
}
void GameUI()                            
{
  lcd.createChar(0, heartShape);
  lcd.createChar(1, shieldShape);
  if(lastNumberOfLives != numberOfLives)
    lcd.clear();
  if(lastNumberOfShields != numberOfShields)
    lcd.clear();
  for(int i=0; i < numberOfLives; i++){
    lcd.home();
    lcd.setCursor(i,0);
    lcd.write(byte(0));
  }
  for(int i=0; i < numberOfShields; i++){
    lcd.home();
    lcd.setCursor(i,1);
    lcd.write(byte(1));
  }
  lcd.setCursor(6,0);
  lcd.print("Level:");
  lcd.print(currentLevel);
  lcd.setCursor(6,1);
  lcd.print("Score:");
  lcd.print(Score);
}

void EndMenu()
{
  lcd.setCursor(0,0);
  lcd.print("SCORE:");
  lcd.print(Score);
  lcd.setCursor(0,1);
  lcd.print("TIME:");
  lcd.print(elapsedTime - (levelReset/1000));
  lcd.setCursor(11,1);
  lcd.print("RETRY");
  lcd.setCursor(11,0);
  lcd.print("MENU");
  lcd.setCursor(10, endMenuOption);
  lcd.print(">");
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  if (xValue < minThreshold && joyMoved == false) {
      if (endMenuOption < 1) {
        endMenuOption += 1;
      }
      joyMoved = true;
  } 
  if (xValue > maxThreshold && joyMoved == false) {
    if (endMenuOption > 0) {
      endMenuOption -= 1;
    } 
    joyMoved = true;
  }
  if(joyMoved == true){
    lcd.clear();  
  }
  if(xValue >= minThreshold && xValue <= maxThreshold)
    joyMoved = false;
  swState = digitalRead(pinSW);
  if (swState !=  lastSwState)        //resetting the game
      if (swState == HIGH){ 
        if(endMenuOption == 1){
          numberOfLives = 3;
          numberOfShields = 3;
          lastLevelChange = 0;
          scoreIncrement = 1;    
          carSpeed = 600;
          botSpawnRate = 12.5 * carSpeed;
          isGameOver = false;
          gameStarted = true;
          didCheckLevel = false;
          ClearLastPlayerPosition();
          ClearLastBotPosition();
          botXCoordinate = 8;
          botYCoordinate = 2;
          playerXCoordinate = 2;
          playerYCoordinate = 5;
          levelReset = millis();
          Score = 0;
          currentLevel = chosenLevel;
          lcd.clear();
        }
        if(endMenuOption == 0) {
          numberOfLives = 3;
          numberOfShields = 3;
          lastLevelChange = 0;
          scoreIncrement = 1; 
          EEPROM.get(0, highScore);
          didCheckLevel = false;
          inMainMenu = true;  
          carSpeed = 600;
          botSpawnRate = 12.5 * carSpeed;
          gameStarted = false;
          isGameOver = false;
          ClearLastPlayerPosition();
          ClearLastBotPosition();
          botXCoordinate = 8;
          botYCoordinate = 2;
          playerXCoordinate = 2;
          playerYCoordinate = 5;
          levelReset = millis();
          Score = 0;
          currentLevel = 1;
          lcd.clear();
      }
  }
  lastSwState = swState;
  
}

void loop()
{
  Serial.println(gameStarted);
  analogWrite(pinBacklight, brightness);      
  if(gameStarted == true){
    if(isGameOver == false)
      RunGame();
    else
      EndMenu();
  }
  else
    ShowMenu();
}
