#include "LedControl.h"
#include <LiquidCrystal.h>
#include "custom_chars.h"
const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;

const int pinX = A0;
const int pinY = A1;
const int pinLDR = A3;
const int pinSW = 2;
const int buzzerPin = 3;

const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

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
const int debounceTime = 300;
const int shootDebounceTime = 500;
const int second = 1000;
const int bulletBlinkingTime = 100, bulletSpeed = 200, playerBlinkingTime = 400;
unsigned long lastBulletMove = 0;
unsigned long lastBulletBlink = 0, lastPlayerBlink = 0, lastBulletSound = 0, lastHitSound = 0;
bool bulletState = 0, playerState = 0;

// const byte matrixSize = 8;
bool matrixChanged = true;

bool menuDisplayed = false, waitingForInput = false, finished = false, playDestroySound = false, playShootSound = false, automaticBrightness = false;
bool inMenu = true;
int selected = 0, option = 0;
bool start = 0, uncovered = 0;
int noWalls = 0;
unsigned long startTime = 0;

int menu = 0, play = 1, settings = 2, about = 3;

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

byte matrix[matrixSize][matrixSize] = {
  {1, 1, 1, 0, 0, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 1, 1, 0, 0, 1, 1, 1}  
};
/// here, i could make these walls that i want to be permanent have another value, so that i can not destroy them!

void updateMatrix();
void printMenu(int subMenu = 0);

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
        }
        
        if(noWalls == 0 && !finished) {
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
    if(xValue < lowerThresholdX && millis() - lastChangeX > debounceTime) {
        lastChangeX = millis();
        selected++;
        if(selected > 3) {
            selected = 0;
        }
        printMenu(selected);
    }
    else if(xValue > upperThresholdX && millis() - lastChangeX > debounceTime) {        
        lastChangeX = millis();
        selected--;
        if(selected < 0) {
            selected = 3;
        }
        printMenu(selected);
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
            lcd.print("Main menu      ");
            lcd.write(byte(7));
            menuDisplayed = true;
            break;
        case play:
            // start = 1;
            // startTime = millis();
            // inMenu = false;
            // these are actually for selecting play
            lcd.setCursor(0, 0);
            lcd.print("> Play ");
            lcd.write(byte(4));
            lcd.print("       ");
            lcd.write(byte(7));
            Serial.println("Play");
            break;
        case settings:
            lcd.setCursor(0, 0);
            lcd.print("> Settings ");
            lcd.write(byte(2));
            lcd.print("   ");
            Serial.println("Settings");
            lcd.write(byte(7));
            waitingForInput = true;
            break;
        case about:
            lcd.setCursor(0, 0);
            lcd.print("> About ");
            lcd.write(byte(5));
            lcd.print("      ");
            lcd.write(byte(7));
            Serial.println("About");
            break;
        default:
            Serial.println("Invalid options");
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

    menuDisplayed = 0;
    uncovered = 0;
    finished = 1;
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