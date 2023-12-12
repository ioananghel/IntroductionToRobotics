#include "LedControl.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "custom_chars.h"
const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;

const int pinX = A0;
const int pinY = A1;
const int pinLDR = A3;
const int pinSW = 2;
const int buzzerPin = 3;

const byte rs = 13;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;
const int lcdBacklight = 9;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
byte lcdBrightness = 5;

unsigned long lastChangeX = 0, lastChangeY = 0, lastChangeSW;
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);
byte matrixBrightness = 7.5;
byte xPos = 0;
byte yPos = 0;
byte xLastPos = 0;
byte yLastPos = 0;
int xValue, yValue;

const int lowerThresholdX = 400, lowerThresholdY = 400;
const int upperThresholdX = 650, upperThresholdY = 650;

const int startUpTime = 2000;
int startUpAt = 0;
const int debounceTime = 300, debounceTimeBack = 500;
const int shootDebounceTime = 500;
const int second = 1000;
const int bulletBlinkingTime = 100, bulletSpeed = 200, playerBlinkingTime = 400;
unsigned long lastBulletMove = 0;
unsigned long lastBulletBlink = 0, lastPlayerBlink = 0, lastBulletSound = 0, lastHitSound = 0;
bool bulletState = 0, playerState = 0;

// const byte matrixSize = 8;
bool matrixChanged = true;

bool menuDisplayed = false, waitingForInput = false, finished = false, playDestroySound = false, playShootSound = false, automaticBrightness = false;
const int lcdBrightnessAddress = 3, matrixBrightnessAddress = 3 + sizeof(byte) + 1;
bool inMenu = true, standby = false;
int selected = 0, option = 0;
bool start = 0, uncovered = 0;
int noWalls = 0, initialNoWalls = 0;
unsigned long startTime = 0;

const int menu = 0, play = 1, easy = 10, medium = 11, hard = 12, settings = 2, setLCDBrightness = 20, lcdLow = 200, lcdMed = 201, lcdHigh = 202, setMatrixBrightness = 21, matrixLow = 210, matrixMed = 211, matrixHigh = 212, matrixAuto = 213, about = 3, expandedAboutGameName = 30, expandedAboutCreatorName = 31, expandedAboutGitHub = 32, expandedAboutLinkedin = 33;
const int select = 10;
const int easyTime = 90 * second, mediumTime = 60 * second, hardTime = 30 * second;
int roundTime = 90000;
unsigned long lastUpdateTime = 0;
int menuNo = 3, aboutNo = 3, settingsNo = 1, lcdNo = 2, matrixNo = 3;

const int soundFrequencies = 3;
int currentFrequency = 0;
int bulletSoundFrequencies[] = {800, 1000, 1200};
int bulletSoundDurations[] = {80, 50, 40};
int wallHitSoundFrequencies[] = {600, 400, 200};
int wallHitSoundDurations[] = {40, 50, 60};

struct direction {
    int x, y;
};
direction up = {0, -1};
direction down = {0, 1};
direction left = {-1, 0};
direction right = {1, 0};
direction currentDirection = {0, 0};

// byte matrix[matrixSize][matrixSize] = {
//   {1, 1, 1, 0, 0, 1, 1, 1},
//   {1, 0, 0, 0, 0, 0, 0, 1},
//   {1, 0, 0, 0, 0, 0, 0, 1},
//   {0, 0, 0, 0, 0, 0, 0, 0},
//   {0, 0, 0, 0, 0, 0, 0, 0},
//   {1, 0, 0, 0, 0, 0, 0, 1},
//   {1, 0, 0, 0, 0, 0, 0, 1},
//   {1, 1, 1, 0, 0, 1, 1, 1}  
// };
/// here, i could make these walls that i want to be permanent have another value, so that i can not destroy them!

byte matrix[matrixSize][matrixSize] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}  
};

void updateMatrix();
void printMenu(int subMenu = 0);
void selectInMenu(bool fromJoystick = false);

class Bullet {
    int xPos, yPos;
    int xLastPos, yLastPos;
    direction dir;

    public:
        Bullet(int xPos, int yPos, direction dir) {
            this->xPos = xPos;
            this->yPos = yPos;
            this->dir = dir;

            // Serial.println("Bullet created");
            playShootSound = true;
        }
        Bullet& operator=(const Bullet& other) {
            direction position = other.getPosition();
            this->xPos = position.x;
            this->yPos = position.y;
            position = other.getLastPosition();
            this->xLastPos = position.x;
            this->yLastPos = position.y;
            this->dir = other.getDirection();
            return *this;
        }
        direction getPosition() {
            return {xPos, yPos};
        }
        direction getLastPosition() {
            return {xLastPos, yLastPos};
        }
        direction getDirection() {
            return this->dir;
        }

        bool move() {
            xLastPos = xPos;
            yLastPos = yPos;
            xPos = (xPos + dir.x);
            yPos = (yPos + dir.y);
            if(xPos >= matrixSize) {
                matrix[xPos][yPos] = 0;
                updateMatrix();
                return 0;
            }
            if(yPos >= matrixSize) {
                matrix[xPos][yPos] = 0;
                updateMatrix();
                return 0;
            }
            if(xPos < 0) {
                matrix[xPos][yPos] = 0;
                updateMatrix();
                return 0;
            }
            if(yPos < 0) {
                matrix[xPos][yPos] = 0;
                updateMatrix();
                return 0;
            }

            if(matrix[xPos][yPos] == 1) {
                playDestroySound = true;
                noWalls--;
                // Serial.println(noWalls);
                matrix[xPos][yPos] = 0;
                matrix[xLastPos][yLastPos] = 0;
                updateMatrix();
                return 0;
            }

            updateMatrix();
            return 1;
        }
        void blink(bool state) {
            lc.setLed(0, xPos, yPos, state);
        }
};

class BulletNode {
    Bullet *bullet;
    BulletNode* next;

    public:
        BulletNode(Bullet* bullet) {
            this->bullet = bullet;
            this->next = NULL;
        }
        void setNext(BulletNode* next) {
            this->next = next;
        }
        BulletNode* getNext() {
            return this->next;
        }
        Bullet* getBullet() {
            return this->bullet;
        }
        ~BulletNode() {
            delete this->bullet;
        }
};

class BulletList {
    BulletNode* head;
    BulletNode* tail;

    public:
        BulletList() {
            this->head = NULL;
            this->tail = NULL;
        }
        BulletNode* getHead() {
            return this->head;
        }
        BulletNode* getTail() {
            return this->tail;
        }
        void addNode(BulletNode* node) {
            if(this->head == NULL) {
                this->head = node;
                this->tail = node;
            }
            else {
                this->tail->setNext(node);
                this->tail = node;
            }
        }
        void removeNode(BulletNode* node) {
            if(node == this->head) {
                this->head = this->head->getNext();
                delete node;
            }
            else {
                BulletNode* prev = this->head;
                while(prev->getNext() != node) {
                    prev = prev->getNext();
                }
                prev->setNext(node->getNext());
                delete node;
            }
        }
};

BulletList bullets;

void setup() {
    Serial.begin(9600);
    lcd.begin(16, 2);

    lcd.createChar(0, timerChar);
    lcd.createChar(1, trophyChar);
    lcd.createChar(2, wrenchChar);
    lcd.createChar(3, amazedChar);
    // lcd.createChar(4, explosion1Step);
    // lcd.createChar(5, explosion2Step);
    // lcd.createChar(6, explosion3Step);
    // lcd.createChar(7, fullMatrix);
    lcd.createChar(4, playButton);
    lcd.createChar(5, heartChar);
    lcd.createChar(6, skullChar);
    lcd.createChar(7, upDownArrows);

    matrixBrightness = EEPROM.get(matrixBrightnessAddress, matrixBrightness);
    lcdBrightness = EEPROM.get(lcdBrightnessAddress, lcdBrightness);

    lc.shutdown(0, false);
    lc.setIntensity(0, matrixBrightness);
    lc.clearDisplay(0);
    coverMatrix();

    randomSeed(analogRead(A2));
    randomStartPos();
    matrix[xPos][yPos] = 1;
    generateWalls();
    // updateMatrix();

    pinMode(pinX, INPUT);
    pinMode(pinY, INPUT);
    pinMode(pinSW, INPUT_PULLUP);

    lcd.setCursor(0, 0);
    lcd.print("Hello, player!");
    lcd.setCursor(0, 2);
    lcd.print("Remember:be fast!");
    startUpAt = millis();
}

void loop() {
    if(millis() - startUpAt < startUpTime) {
        return;
    }

    if(standby) {
        if(digitalRead(pinSW) == 1 && millis() - lastChangeSW > debounceTime) {
            start = 0;
            standby = false;
            inMenu = true;
            menuDisplayed = false;
        }
    }

    if(!menuDisplayed && !start) {
        selected = 0;
        printMenu();
    }

    if(automaticBrightness) {
        matrixBrightness = map(analogRead(pinLDR), 0, 1023, 0, 15);
        lc.setIntensity(0, matrixBrightness);
    }

    if(start) {
        if(!uncovered) {
            coverMatrix();
            uncoverMatrix();
            uncovered = 1;
            Serial.println("Game started");
            inGameLCD();
        }

        if(millis() - lastUpdateTime > second) {
            lastUpdateTime = millis();
            lcd.setCursor(1, 1);
            lcd.print("   ");
            lcd.setCursor(1, 1);
            lcd.print((roundTime - (millis() - startTime)) / second);
            Serial.print("Time left: ");
            Serial.println((roundTime - (millis() - startTime)) / second);

            if((roundTime - (millis() - startTime)) / second == 0) {
                standby = true;
                coverMatrix();
                displayAnimation(trophyMatrix);
                resetBoard();
                Serial.print("Congrats, you finished in ");
                Serial.print((millis() - startTime) / second);
                Serial.println(" seconds");
                animateLCD(3);
                Serial.println("Time's up!");
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Time's up!");
                lcd.setCursor(0, 1);
                lcd.print("You got: ");
                lcd.print(initialNoWalls - noWalls);
                lcd.print(" points");
            }
        }
        
        if(noWalls == 0 && !finished) {
            standby = true;
            coverMatrix();
            displayAnimation(trophyMatrix);
            resetBoard();
            Serial.print("Congrats, you finished in ");
            Serial.print((millis() - startTime) / second);
            Serial.println(" seconds");
        }

        blinkLEDs();
        readJoystick();
        actOnJoystick();
        actOnSW();
        bulletsTravel();
    }

    if(playDestroySound) {
        tone(buzzerPin, wallHitSoundFrequencies[currentFrequency], wallHitSoundDurations[currentFrequency]);
        if(millis() - lastHitSound > wallHitSoundDurations[currentFrequency]) {
            lastHitSound = millis();
            noTone(buzzerPin);
            currentFrequency++;
            if(currentFrequency == soundFrequencies) {
                currentFrequency = 0;
                playDestroySound = false;
            }
        }
    }

    if(playShootSound) {
        tone(buzzerPin, bulletSoundFrequencies[currentFrequency], bulletSoundDurations[currentFrequency]);
        if(millis() - lastBulletSound > bulletSoundDurations[currentFrequency]) {
            lastBulletSound = millis();
            noTone(buzzerPin);
            currentFrequency++;
            if(currentFrequency == soundFrequencies) {
                currentFrequency = 0;
                playShootSound = false;
            }
        }
    }
    if (!start && inMenu) {
        readJoystick();
        navigateMenu();
        selectInMenu();
    }
}

void blinkLEDs() {
    if(millis() - lastPlayerBlink > playerBlinkingTime) {
        lastPlayerBlink = millis();
        playerState = !playerState;
        lc.setLed(0, xLastPos, yLastPos, playerState);
    }
    if(millis() - lastBulletBlink > bulletBlinkingTime) {
        lastBulletBlink = millis();
        bulletState = !bulletState;
        BulletNode* node = bullets.getHead();
        while(node != NULL) {
            node->getBullet()->blink(bulletState);
            node = node->getNext();
        }
    }
}

void coverMatrix() {
    for (int row = 0; row < matrixSize; row++) {
        for (int col = 0; col < matrixSize; col++) {
            lc.setLed(0, row, col, true);
            delay(25);
        }
    }
}

void uncoverMatrix() {
    for (int row = 0; row < matrixSize; row++) {
        for (int col = 0; col < matrixSize; col++) {
            if(matrix[row][col] == 0) {
                lc.setLed(0, row, col, false);
            }
            delay(25);
        }
    }
}

void readJoystick() {
    xValue = analogRead(pinX);
    yValue = analogRead(pinY);
}

void actOnJoystick() {
    if(xValue > upperThresholdX && millis() - lastChangeX > debounceTime) {
        lastChangeX = millis();
        currentDirection = up;
        move(up);
    }
    else if(xValue < lowerThresholdX && millis() - lastChangeX > debounceTime) {        
        lastChangeX = millis();
        currentDirection = down;
        move(down);
    }
    if(yValue > upperThresholdY && millis() - lastChangeY > debounceTime) {
        lastChangeY = millis();
        currentDirection = right;
        move(right);
    }
    else if(yValue < lowerThresholdY && millis() - lastChangeY > debounceTime) {
        lastChangeY = millis();
        currentDirection = left;
        move(left);
    }
}

void navigateMenu() {
    if(option != 0 && option < 100) {
        menuNo = option == 20 ? settingsNo : aboutNo;
    }
    else if(option == 200 || option == 210) {
        menuNo = option == 200 ? lcdNo : matrixNo;
    }
    else {
        menuNo = 3;
    }
    if(xValue < lowerThresholdX && millis() - lastChangeX > debounceTime) {
        lastChangeX = millis();
        selected++;
        if(selected > menuNo) {
            selected = 0;
        }
        Serial.print("Navigating to: ");
        Serial.println(option + selected);
        printMenu(option + selected);
    }
    else if(xValue > upperThresholdX && millis() - lastChangeX > debounceTime) {        
        lastChangeX = millis();
        selected--;
        if(selected < 0) {
            selected = menuNo;
        }
        Serial.print("Navigating to: ");
        Serial.println(option + selected);
        printMenu(option + selected);
    }

    if(yValue < lowerThresholdY && millis() - lastChangeY > debounceTimeBack) {
        if(option != 0) {
            selected = (option / 10) % 10;
            option /= 100;
            printMenu(option + selected);
        }
    }
    else if(selected != 1 && yValue > upperThresholdY && millis() - lastChangeY > debounceTime) {
        selectInMenu(true);
    }
}

void actOnSW() {
    if(digitalRead(pinSW) == 1 && millis() - lastChangeSW > shootDebounceTime) {
        lastChangeSW = millis();

        Bullet* bullet = new Bullet(xLastPos, yLastPos, currentDirection);
        BulletNode* node = new BulletNode(bullet);
        bullets.addNode(node);
    }
}

void selectInMenu(bool fromJoystick = false) {
    if((digitalRead(pinSW) == 1 || fromJoystick) && millis() - lastChangeSW > debounceTime) {
        Serial.print("Selecting in menu: selected = ");
        Serial.print(selected);
        Serial.print(", option = ");
        Serial.println(option);
        lcd.createChar(4, downwardArrow);
        lastChangeSW = millis();
        if(option == 0 && selected == 0) {
            return;
        }
        // if(option != 0) {
        //     selected = option / 10;
        //     option = 0;
        //     printMenu(option + selected);
        // }
        else if(option == 0 || option / 10 < 10 || option == 200 || option == 210) {
            option = ((option / 10) * 10 + selected) * 10;
            selected = 0;
            printMenu(option + selected);
        }
    }
}

void inGameLCD() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write(byte(5));
    lcd.write(byte(5));
    lcd.write(byte(5));
    lcd.setCursor(0, 1);
    startTime = millis();
    lcd.write(byte(0));
    lcd.print((roundTime - (millis() - startTime)) / second);
    lastUpdateTime = millis();
}

void bulletsTravel() {
    if(millis() - lastBulletMove > bulletSpeed) {
        lastBulletMove = millis();
        BulletNode* node = bullets.getHead();
        while(node != NULL) {
            if(node->getBullet()->move() == 0) {
                bullets.removeNode(node);
            }
            node = node->getNext();
        }
    }
}

void move(direction dir) {
    xPos = (xLastPos + dir.x) % matrixSize;
    yPos = (yLastPos + dir.y) % matrixSize;
    if(xPos < 0) {
        xPos = matrixSize - 1;
    }
    if(yPos < 0) {
        yPos = matrixSize - 1;
    }

    if(matrix[xPos][yPos] == 1) {
        return;
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

void randomStartPos() {
    randomSeed(analogRead(A2));

    xPos = random() % matrixSize;
    yPos = random() % matrixSize;

    xLastPos = xPos;
    yLastPos = yPos;
}

void generateWalls() {
    // 8x8 matrix => 64 cells
    // 50% - 75% walls => 32 - 48 walls
    randomSeed(analogRead(A2));

    noWalls = random() % 17 + 32;
    initialNoWalls = noWalls;
    for(int i = 0; i < noWalls; i++) {
        int x = random() % matrixSize;
        int y = random() % matrixSize;

        if(matrix[x][y] == 1) {
            i--;
        }
        matrix[x][y] = 1;
    }
}

void printMenu(int subMenu = 0) {
    lcd.clear();
    switch(subMenu) {
        case menu:
            Serial.println("Main menu:");
            lcd.setCursor(0, 0);
            lcd.print("Rapid Shootout");
            lcd.write(byte(7));
            menuDisplayed = true;
            break;
        case play:
            lcd.createChar(4, playButton);
            lcd.setCursor(0, 0);
            lcd.print("> Play ");
            lcd.write(byte(4));
            lcd.print("       ");
            lcd.write(byte(7));
            Serial.println("Play");
            break;
        case easy:
            lcd.setCursor(0, 0);
            lcd.write(byte(4));
            lcd.print(" Play");
            lcd.print("       ");
            lcd.write(byte(7));
            lcd.setCursor(1, 1);
            lcd.print("Easy         ");
            break;
        case easy * select:
            roundTime = easyTime;
            start = 1;
            startTime = millis();
            inMenu = false;
            break;
        case medium:
            lcd.setCursor(0, 0);
            lcd.write(byte(4));
            lcd.print(" Play");
            lcd.print("       ");
            lcd.write(byte(7));
            lcd.setCursor(1, 1);
            lcd.print("Medium       ");
            break;
        case medium * select:
            roundTime = mediumTime;
            start = 1;
            startTime = millis();
            inMenu = false;
            break;
        case hard:
            lcd.setCursor(0, 0);
            lcd.write(byte(4));
            lcd.print(" Play");
            lcd.print("       ");
            lcd.write(byte(7));
            lcd.setCursor(1, 1);
            lcd.print("Hard         ");
            break;
        case hard * select:
            roundTime = hardTime;
            start = 1;
            startTime = millis();
            inMenu = false;
            break;
        // case 10:
        //     start = 1;
        //     startTime = millis();
        //     inMenu = false;
        //     break;
        case settings:
            lcd.setCursor(0, 0);
            lcd.print("> Settings ");
            lcd.write(byte(2));
            lcd.print("   ");
            Serial.println("Settings");
            lcd.write(byte(7));
            waitingForInput = true;
            break;
        case setLCDBrightness:
            lcd.setCursor(0, 0);
            lcd.write(byte(4));
            lcd.print(" Settings ");
            lcd.write(byte(2));
            lcd.setCursor(1, 1);
            lcd.print("LCD Glow     ");
            lcd.write(byte(7));
            Serial.println("LCD Glow");
            break;
        case lcdLow:
            lcd.setCursor(0, 0);
            lcd.write(byte(4));
            lcd.print(" LCD Glow");
            lcd.setCursor(1, 1);
            lcd.print("Low          ");
            lcd.write(byte(7));
            break;
        case lcdLow * select:
            lcdBrightness = 200;
            EEPROM.put(lcdBrightnessAddress, lcdBrightness);
            setLcdBrightness();
            option = lcdLow;
            selected = lcdLow % 10;
            printMenu(option + selected);
            EEPROM.put(lcdBrightnessAddress, lcdBrightness);
            break;
        case lcdMed:
            lcd.setCursor(0, 0);
            lcd.write(byte(4));
            lcd.print(" LCD Glow");
            lcd.setCursor(1, 1);
            lcd.print("Medium       ");
            lcd.write(byte(7));
            break;
        case lcdMed * select:
            lcdBrightness = 600;
            EEPROM.put(lcdBrightnessAddress, lcdBrightness);
            setLcdBrightness();
            option = lcdLow;
            selected = lcdMed % 10;
            printMenu(option + selected);
            EEPROM.put(lcdBrightnessAddress, lcdBrightness);
            break;
        case lcdHigh:
            lcd.setCursor(0, 0);
            lcd.write(byte(4));
            lcd.print(" LCD Glow");
            lcd.setCursor(1, 1);
            lcd.print("High         ");
            lcd.write(byte(7));
            break;
        case lcdHigh * select:
            lcdBrightness = 1000;
            EEPROM.put(lcdBrightnessAddress, lcdBrightness);
            setLcdBrightness();
            option = lcdLow;
            selected = lcdHigh % 10;
            printMenu(option + selected);
            EEPROM.put(lcdBrightnessAddress, lcdBrightness);
            break;
        case setMatrixBrightness:
            lcd.setCursor(0, 0);
            lcd.write(byte(4));
            lcd.print(" Settings ");
            lcd.write(byte(2));
            lcd.setCursor(1, 1);
            lcd.print("Matrix Glow  ");
            lcd.write(byte(7));
            Serial.println("Matrix Glow");
            break;
        case matrixLow:
            lcd.setCursor(0, 0);
            lcd.write(byte(4));
            lcd.print(" Matrix Glow");
            lcd.setCursor(1, 1);
            lcd.print("Low          ");
            lcd.write(byte(7));
            break;
        case matrixLow * select:
            matrixBrightness = 2;
            EEPROM.put(matrixBrightnessAddress, matrixBrightness);
            lc.setIntensity(0, matrixBrightness);
            option = matrixLow;
            selected = matrixLow % 10;
            printMenu(option + selected);
            EEPROM.put(matrixBrightnessAddress, matrixBrightness);
            break;
        case matrixMed:
            lcd.setCursor(0, 0);
            lcd.write(byte(4));
            lcd.print(" Matrix Glow");
            lcd.setCursor(1, 1);
            lcd.print("Medium       ");
            lcd.write(byte(7));
            break;
        case matrixMed * select:
            matrixBrightness = 7.5;
            EEPROM.put(matrixBrightnessAddress, matrixBrightness);
            lc.setIntensity(0, matrixBrightness);
            option = matrixLow;
            selected = matrixMed % 10;
            printMenu(option + selected);
            EEPROM.put(matrixBrightnessAddress, matrixBrightness);
            break;
        case matrixHigh:
            lcd.setCursor(0, 0);
            lcd.write(byte(4));
            lcd.print(" Matrix Glow");
            lcd.setCursor(1, 1);
            lcd.print("High         ");
            lcd.write(byte(7));
            break;
        case matrixHigh * select:
            matrixBrightness = 15;
            EEPROM.put(matrixBrightnessAddress, matrixBrightness);
            lc.setIntensity(0, matrixBrightness);
            option = matrixLow;
            selected = matrixHigh % 10;
            printMenu(option + selected);
            EEPROM.put(matrixBrightnessAddress, matrixBrightness);
            break;
        case matrixAuto:
            lcd.setCursor(0, 0);
            lcd.write(byte(4));
            lcd.print(" Matrix Glow");
            lcd.setCursor(1, 1);
            lcd.print("Automatic    ");
            lcd.write(byte(7));
            break;
        case matrixAuto * select:
            automaticBrightness = !automaticBrightness;
            option = matrixLow;
            selected = matrixAuto % 10;
            printMenu(option + selected);
            break;
        case about:
            lcd.setCursor(0, 0);
            lcd.print("> About ");
            lcd.write(byte(5));
            lcd.print("      ");
            lcd.write(byte(7));
            Serial.println("About");
            break;
        case expandedAboutGameName:
            lcd.setCursor(0, 0);
            lcd.write(byte(4));
            lcd.print(" About");
            lcd.print("      ");
            // lcd.write(byte(7));
            lcd.setCursor(1, 1);
            lcd.print("Rapid Shootout");
            lcd.write(byte(7));
            break;
        case expandedAboutCreatorName:
            lcd.setCursor(0, 0);
            lcd.write(byte(4));
            lcd.print(" About");
            lcd.print("      ");
            // lcd.write(byte(7));
            lcd.setCursor(1, 1);
            lcd.print("Ioan Anghel   ");
            lcd.write(byte(7));
            break;
        case expandedAboutGitHub:
            lcd.createChar(3, github);
            lcd.setCursor(0, 0);
            lcd.write(byte(4));
            lcd.print(" About");
            lcd.print("      ");
            // lcd.write(byte(7));
            lcd.setCursor(1, 1);
            lcd.write(byte(3));
            lcd.print(" ioananghel  ");
            lcd.write(byte(7));
            break;
        case expandedAboutLinkedin:
            lcd.createChar(3, linkedin);
            lcd.setCursor(0, 0);
            lcd.write(byte(4));
            lcd.print(" About");
            lcd.print("      ");
            // lcd.write(byte(7));
            lcd.setCursor(1, 1);
            lcd.write(byte(3));
            lcd.print(" ioananghel  ");
            lcd.write(byte(7));
            break;
        default:
            Serial.print("Invalid options: ");
            Serial.println(subMenu);
            Serial.print("\n");
            break;
    }
}

void displayAnimation(byte matrix[matrixSize][matrixSize]) {
    for(int row = 0; row < matrixSize; row++) {
        for(int col = 0; col < matrixSize; col++) {
            lc.setLed(0, row, col, matrix[row][col]);
       }
    }
}

void resetBoard() {
    for(int row = 0; row < matrixSize; row++) {
        for(int col = 0; col < matrixSize; col++) {
            matrix[row][col] = 0;
        }
    }

    // menuDisplayed = 0;
    uncovered = 0;
    finished = 1;
    option = 0;
    selected = 0;
    start = 0;
    randomSeed(analogRead(A2));
    randomStartPos();
    matrix[xPos][yPos] = 1;
    generateWalls();
}

void animateLCD(int ownChar) {
    lcd.clear();
    int lcdRows = 2, lcdCols = 16;
    for(int i = 0; i < 2; i++) {
        for(int j = 0; j < lcdCols; j++) {

            lcd.setCursor(j, i);
            lcd.write(byte(ownChar));
            delay(50);
        }
    }
    lcd.clear();
}

void setLcdBrightness() {
    analogWrite(lcdBacklight, lcdBrightness);
}