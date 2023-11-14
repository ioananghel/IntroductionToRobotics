const int latchPin = 11;
const int clockPin = 10;
const int dataPin = 12;
const int startButton = 2;
const int resetButton = 3;
const int lapButton = 13;
int lastStartPressed = 0, lastResetPressed = 0, lastLapPressed = 0;
bool startButtonState = 0, resetButtonState = 0, lapButtonState = 0;
int lapNr = 0;
const int debounceDelay = 1000;
const int resetDebounceDelay = 1000;

const int segD1 = 4;
const int segD2 = 5;
const int segD3 = 6;
const int segD4 = 7;

int displayDigits[] = {
  segD1, segD2, segD3, segD4
};

const byte regSize = 8;
byte registers[regSize];

const int encodingsNumber = 16;
byte encodingArray[encodingsNumber] = {
  // A B C D E F G DP
  B11111100, // 0
  B01100000, // 1
  B11011010, // 2
  B11110010, // 3
  B01100110, // 4
  B10110110, // 5
  B10111110, // 6
  B11100000, // 7
  B11111110, // 8
  B11110110, // 9
  B11101110, // A
  B00111110, // b
  B10011100, // C
  B01111010, // d
  B10011110, // E
  B10001110 // F
};

const int displayCount = 4;
const int second = 1000;
volatile bool counting = 0;
volatile int startTime = 0;
volatile int pausedTime = 0;
bool displayLap = false;

int digits[displayCount] = {
  0, 0, 0, 0
};
int laps[4][displayCount] = {
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};
int displayedLap[displayCount];

void setup() {
    pinMode(latchPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, OUTPUT);

    pinMode(startButton, INPUT_PULLUP);
    pinMode(resetButton, INPUT_PULLUP);
    pinMode(lapButton, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(startButton), start, FALLING);
    attachInterrupt(digitalPinToInterrupt(resetButton), reset, FALLING);

    for (int i = 0; i < displayCount; i++) {
        pinMode(displayDigits[i], OUTPUT);
        digitalWrite(displayDigits[i], LOW);
     }

  Serial.begin(9600);
}

void loop() {
    int lapState = digitalRead(lapButton);

    if(lapState == 1 && millis() - lastLapPressed > debounceDelay) {
        lastResetPressed = millis();
        lastStartPressed = millis();
        lastLapPressed = millis();
        lap();
    }

    if(displayLap) {
        for(int i = 0; i < displayCount; i++) {
            if(i == 2) {
                writeReg(encodingArray[displayedLap[i]] | B00000001);
            }
            else {
                writeReg(encodingArray[displayedLap[i]]);
            }
            activateDisplay(i);
            delay(1);
        }
        activateDisplay(-1);
    }

    // Serial.println(lapState);

    if(counting) {
        int currentTime = millis();
        int timePassed = currentTime - startTime;

        int seconds = timePassed / second;
        int tenths = (timePassed % second) / 100;

        digits[0] = seconds / 100; // first digit Dxx.x
        digits[1] = seconds / 10 % 10; // second digit xDx.x
        digits[2] = seconds % 10; // third digit xxD.x
        digits[3] = tenths; // tenths digit xxx.D


        for(int i = 0; i < displayCount; i++) {
            if(i == 2) {
                writeReg(encodingArray[digits[i]] | B00000001);
            }
            else {
                writeReg(encodingArray[digits[i]]);
            }
            activateDisplay(i);
            delay(1);
        }
        activateDisplay(-1);
    }
    else if(!counting) {
        for(int i = 0; i < displayCount; i++) {
            if(i == 2) {
                writeReg(encodingArray[digits[i]] | B00000001);
            }
            else {
                writeReg(encodingArray[digits[i]]);
            }
            activateDisplay(i);
            delay(1);
        }
        activateDisplay(-1);
    }
}

void writeReg(byte encoding) {
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, encoding);
    digitalWrite(latchPin, HIGH);
}

void activateDisplay(int activeDisplay) {
    for(int i = 0; i < displayCount; i++) {
        digitalWrite(displayDigits[i], i != activeDisplay);
    }
}

void start() {
    Serial.println("START");
    volatile int current = micros() / second;
    if(counting && current - lastStartPressed > debounceDelay) {
        lapNr = 0;
        displayLap = false;
        lastResetPressed = micros() / second;
        lastStartPressed = current;
        counting = !counting;
        pausedTime = current - startTime;
        startTime = current;
        // Serial.println(startTime);
    } /// TREBUIE SA VAD PE CE RAMURA INTRA
    else if(!counting && current - lastStartPressed > debounceDelay) {
        lapNr = 0;
        displayLap = false;
        lastResetPressed = micros() / second;
        // Serial.println(startTime);
        lastStartPressed = current;
        if(startTime == 0) {
            startTime = current;
            Serial.println("PE 1"); /// PENTRU LATER ON, AICI INTRA, FIINDCA START TIME RAMANE PE 0
        }
        else {
            startTime = current - pausedTime; // i want to keep the counter going from the same point
            Serial.println("PE 2");
        }
        counting = !counting;
    }
    else if(current - lastStartPressed > debounceDelay) {
        lapNr = 0;
        displayLap = false;
        lastResetPressed = micros() / second;
        lastStartPressed = current;
        counting = !counting;
        if(counting) {
            startTime = micros() / second;
        }
    }
}

void lap() {
    lastResetPressed = millis();
    lastStartPressed = millis();
    if(counting) {
        int current = millis();
        // if(current - lastLapPressed > debounceDelay) {
        Serial.println("LAP");
        lastLapPressed = current;
        counting = !counting;
        for(int i = 3; i >= 1; i--) {
            for(int j = 0; j < displayCount; j++) {
                laps[i][j] = laps[i - 1][j];
            }
        }
        laps[0][0] = digits[0];
        laps[0][1] = digits[1];
        laps[0][2] = digits[2];
        laps[0][3] = digits[3];

        counting = !counting;
        for(int i = 0; i < 4; i++) { // we can see here that the laps are being properly saved
            Serial.print(laps[0][i]);
        }
        // }
    }
    if(!counting) {
        Serial.println("LAP1");
        //cycling through the saved laps
        int current = millis();
        lapNr = lapNr % 4;
        // if(current - lastLapPressed > debounceDelay) {
            // Serial.println("LAP");
        for(int j = 0; j < displayCount; j++) {
            displayedLap[j] = laps[lapNr][j];
        }
        lapNr++;
        displayLap = true;
        // }
    }
}

void reset() {
    int current = micros() / second;
    lastStartPressed = current;
    if(!counting && current - lastResetPressed > resetDebounceDelay) {
        displayLap = false;
        lapNr = 0;
        Serial.println("RESET");
        lastResetPressed = current;
        startTime = 0;
        pausedTime = 0;
        counting = 0;
        for(int i = 0; i < displayCount; i++) {
            digits[i] = 0;
        }
    }
}