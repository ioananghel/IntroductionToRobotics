const int latchPin = 11;
const int clockPin = 10;
const int dataPin = 12;
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
bool counting = 1;
int startTime = 0;

int digits[displayCount] = {
  0, 0, 0, 0
};

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  for (int i = 0; i < displayCount; i++) {
    pinMode(displayDigits[i], OUTPUT);
    digitalWrite(displayDigits[i], LOW);
  }

  Serial.begin(9600);
}

void loop() {
    if(counting) {
        int currentTime = millis();
        int timePassed = currentTime - startTime;

        int seconds = timePassed / second;
        int tenths = (timePassed % second) / 100;

        digits[0] = seconds / 100;
        digits[1] = seconds / 10 % 10;
        digits[2] = seconds % 10;
        digits[3] = tenths;

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