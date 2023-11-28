#include "LedControl.h" 
#include <time.h>
const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;

const int pinX = A0;
const int pinY = A1;
const int pinLDR = A3;
const int pinSW = 2;
const int buzzerPin = 3;

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

const int debounceTime = 300;
const int shootDebounceTime = 500;
const int second = 1000;
const int bulletBlinkingTime = 100, bulletSpeed = 200, playerBlinkingTime = 400;
unsigned long lastBulletMove = 0;
unsigned long lastBulletBlink = 0, lastPlayerBlink = 0, lastBulletSound = 0, lastHitSound = 0;
bool bulletState = 0, playerState = 0;

const byte matrixSize = 8;
bool matrixChanged = true;

bool menuDisplayed = false, waitingForInput = false, finished = false, playDestroySound = false, playShootSound = false, automaticBrightness = false;
int selected = -1, option = -1;
bool start = 0, uncovered = 0;
int noWalls = 0;
unsigned long startTime = 0;

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
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}  
};

byte trophyMatrix[matrixSize][matrixSize] = {
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 1, 1, 1, 1, 0, 1},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 0, 1, 1, 1, 1, 0, 0}
};

class Player {

};

void updateMatrix();
void printMenu(int subMenu = -1);

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
}

void loop() {

    if(!menuDisplayed && !start) {
        selected = -1;
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

    if (!start && Serial.available() > 0) {
        if(waitingForInput) {
            waitingForInput = false;
            option = -1;
            option = Serial.parseInt();
            switch(option) {
                case 1:
                    automaticBrightness = false;
                    matrixBrightness = 2;
                    lc.setIntensity(0, matrixBrightness);
                    break;
                case 2:
                    automaticBrightness = false;
                    matrixBrightness = 8;
                    lc.setIntensity(0, matrixBrightness);
                    break;
                case 3:
                    automaticBrightness = false;
                    matrixBrightness = 15;
                    lc.setIntensity(0, matrixBrightness);
                    break;
                case 4:
                    automaticBrightness = true;
                case 5:
                    break;
                default:
                    Serial.println("Invalid option");
                    break;
            }
            printMenu();
        }
        else {
            option = -1;
            option = Serial.parseInt();

            if (option != -1) {
                selected = option;
                printMenu(option);
            }
            else {
                Serial.println("Invalid option");
            }
        }
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

void printMenu(int subMenu = -1) {
    switch(subMenu) {
        case -1:
            Serial.println("Main menu:");
            Serial.println("1. Play");
            Serial.println("2. Set Matrix Brightness");
            Serial.print("\n");
            menuDisplayed = true;
            break;
        case 1:
            start = 1;
            startTime = millis();
            break;
        case 2:
            Serial.println("Set Matrix Brightness:");
            Serial.println("1. Low");
            Serial.println("2. Medium");
            Serial.println("3. High");
            Serial.println("4. Auto");
            Serial.println("5. Cancel");
            waitingForInput = true;
            Serial.print("\n");
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