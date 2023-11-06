const int pinSW = 2;
const int pinX = A0;
const int pinY = A1;
byte swState = HIGH;
int xValue = 0;
int yValue = 0;

const int pinA = 12;
const int pinB = 10;
const int pinC = 9;
const int pinD = 8;
const int pinE = 7;
const int pinF = 6;
const int pinG = 5;
const int pinDP = 4;
const int segSize = 8;

int segments[segSize] = {
    pinA, pinB, pinC, pinD, pinE, pinF, pinG, pinDP
};

const int lowerThresholdX = 450, lowerThresholdY = 450;
const int upperThresholdX = 570, upperThresholdY = 570;

volatile int debounceDelay = 50;
const int resetDelay = 2000;
const int blinkingTime = 300;
volatile unsigned long lastPressed = 0;
int lastStateChange = 0;
bool toggled = 0;
volatile bool swToggled = 0;
int currentSegment = pinDP;
bool currentSegmentState = 0;

const int noOfMoves = 4; //up, down, left, right

const int up = 0, down = 1, left = 2, right = 3;

volatile bool states[segSize] = {
    0, 0, 0, 0, 0, 0, 0, 0
};

int neighbours[segSize][noOfMoves] = {
    // up, down, left, right
    {-1, pinG, pinF, pinB}, // A
    {pinA, pinG, pinF, -1}, // B
    {pinG, pinD, pinE, pinDP}, // C
    {pinG, -1, pinE, pinC}, // D
    {pinG, pinD, -1, pinC}, // E
    {pinA, pinG, -1, pinB}, // F
    {pinA, pinD, -1, -1}, // G
    {-1, -1, pinC, -1}, // DP
};

void setup() {
    pinMode(pinSW, INPUT_PULLUP);
    pinMode(pinX, INPUT);
    pinMode(pinY, INPUT);

    for(int i = 0; i < segSize; i++) {
        pinMode(segments[i], OUTPUT);
    }

    attachInterrupt(digitalPinToInterrupt(pinSW), onChange, CHANGE);

    Serial.begin(9600);
}

void loop() {

    if(millis() - lastStateChange > blinkingTime) {
        lastStateChange = millis();

        currentSegmentState = !currentSegmentState;
        digitalWrite(currentSegment, currentSegmentState);
    }

    xValue = analogRead(pinX);
    yValue = analogRead(pinY);

    if(xValue > upperThresholdX && !toggled) {
        toggled = 1;
        
        move(up, currentSegment);
    }
    else if(xValue < lowerThresholdX && !toggled) {
        toggled = 1;
        
        move(down, currentSegment);
    }
    if(yValue > upperThresholdY && !toggled) {
        toggled = 1;

        move(right, currentSegment);
    }
    else if(yValue < lowerThresholdY && !toggled) {
        toggled = 1;

        move(left, currentSegment);
    }
    
    if(xValue < upperThresholdX && xValue > lowerThresholdX && yValue < upperThresholdY && yValue > lowerThresholdY) {
        toggled = 0;
        delay(10);
    }
}

void move(int direction, int currSegment) {
    currentSegment = neighbours[pinToIndex(currSegment)][direction] != -1 ? neighbours[pinToIndex(currSegment)][direction] : currSegment;

    if(states[pinToIndex(currSegment)]) {
        digitalWrite(currSegment, HIGH);
    }
    else {
        digitalWrite(currSegment, LOW);
    }
}

void onChange() {

    unsigned long current = micros() / 1000 - lastPressed;

    if(current < debounceDelay) {
        return;
    }

    if(current > resetDelay && swState == LOW) {
        resetDisplay();

        swState = !swState;
        lastPressed = micros() / 1000;

        return;
    }

    if(swState == HIGH) {
        states[pinToIndex(currentSegment)] = !states[pinToIndex(currentSegment)];
    }

    lastPressed = micros() / 1000;
    swState = !swState;
}

void resetDisplay() {
    for(int i = 0; i < segSize; i++) {
        digitalWrite(segments[i], LOW);
        states[i] = 0;
    }

    currentSegment = pinDP;
}

int pinToIndex(int pin) {
    switch (pin)
    {
        case 12:
            return 0;
        case 10:
            return 1;
        case 9:
            return 2;
        case 8:
            return 3;
        case 7:
            return 4;
        case 6:
            return 5;
        case 5:
            return 6;
        case 4:
            return 7;
        
        default:
            return 7;
    }
}