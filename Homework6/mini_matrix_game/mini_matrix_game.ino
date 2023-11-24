#include "LedControl.h" 
const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;


const int pinX = A0;
const int pinY = A1;
const int pinSW = 2;

int lastChangeX = 0, lastChangeY = 0, lastSW;
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);
byte matrixBrightness = 2;
byte xPos = 0;
byte yPos = 0;
byte xLastPos = 0;
byte yLastPos = 0;
int xValue, yValue;

const int lowerThresholdX = 400, lowerThresholdY = 400;
const int upperThresholdX = 650, upperThresholdY = 650;

const int debounceTime = 200;
const int bulletBlinkingTime = 100, playerBlinkingTime = 400;
int lastBulletBlink = 0, lastPlayerBlink = 0; // these are going to be included in the classes, most likely

const byte matrixSize = 8;
bool matrixChanged = true;

struct direction {
    int x, y;
};
direction up = {0, -1};
direction down = {0, 1};
direction left = {-1, 0};
direction right = {1, 0};

byte matrix[matrixSize][matrixSize] = {
  {1, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}  
};

class Player {

}

class Bullet {

}

void setup() {
    Serial.begin(9600);
    lc.shutdown(0, false);
    lc.setIntensity(0, matrixBrightness);
    lc.clearDisplay(0); 
    matrix[xPos][yPos] = 1;

    pinMode(pinX, INPUT);
    pinMode(pinY, INPUT);
    pinMode(pinSW, INPUT_PULLUP);
}

void loop() {
    if(millis() - lastBulletBlink > bulletBlinkingTime) {
        lastBulletBlink = millis();
        //change state of the bullet led -- in class
    }
    if(millis() - lastPlayerBlink > playerBlinkingTime) {
        lastPlayerBlink = millis();
        //change state of the player led -- in class
    }

    readJoystick();
    actOnJoystick();
}

void readJoystick() {
    xValue = analogRead(pinX);
    yValue = analogRead(pinY);
}

void actOnJoystick() {
    if(xValue > upperThresholdX && millis() - lastChangeX > debounceTime) {
        lastChangeX = millis();
        move(up);
    }
    else if(xValue < lowerThresholdX && millis() - lastChangeX > debounceTime) {        
        lastChangeX = millis();
        move(down);
    }
    if(yValue > upperThresholdY && millis() - lastChangeY > debounceTime) {
        lastChangeY = millis();
        move(right);
    }
    else if(yValue < lowerThresholdY && millis() - lastChangeY > debounceTime) {
        lastChangeY = millis();
        move(left);
    }
}

void move(direction dir) {
    lc.clearDisplay(0);

    xPos = (xLastPos + dir.x) % matrixSize;
    yPos = (yLastPos + dir.y) % matrixSize;
    if(xPos < 0) {
        xPos = matrixSize - 1;
    }
    if(yPos < 0) {
        yPos = matrixSize - 1;
    }
    matrix[xPos][yPos] = 1;
    matrix[xLastPos][yLastPos] = 0;

    updateMatrix();
    xLastPos = xPos;
    yLastPos = yPos;
}

void updateMatrix() {
    for (int row = 0; row < matrixSize; row++) {
        for (int col = 0; col < matrixSize; col++) {
            lc.setLed(0, row, col, matrix[row][col]);
        }
    }
}