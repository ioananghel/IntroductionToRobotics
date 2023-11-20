#include <EEPROM.h>

int incomingByte = 0;
bool menuDisplayed = false, waitingForInput = false, printReadings = false, selectColor = false, automaticState = true;
int selected = -1, option = -1; // 1 = sensor settings, 2 = reset logger data, 3 = system status, 4 = RGB LED control
int redPin = 11, greenPin = 10, bluePin = 9;
int userRedPin = A0, userGreenPin = A1, userBluePin = A2;
int redValue = 255, greenValue = 255, blueValue = 0;
int userRedValue = 125, userGreenValue = 125, userBlueValue = 125;

const int trigPin = 5;
const int echoPin = 6;
long duration = 0;
int distance = 0;
int photocellPin = A5;
int photocellValue;

const int nrReadings = 10;
int ultrasonicReadings[nrReadings];
int ldrReadings[nrReadings];

const int firstUltrasonicReading = 0;
const int firstLDRReading = nrReadings * sizeof(int) + 1;

int samplingInterval = 10000; // 10 seconds
int ultrasonicThreshold = 100; // 100 cm
int ldrThreshold = 200; // 200 lux
unsigned long previousRead = 0;

int ultrasonicValue = 0, ldrValue = 0;

void setup() {
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    pinMode(userRedPin, INPUT);
    pinMode(userGreenPin, INPUT);
    pinMode(userBluePin, INPUT);

    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    Serial.begin(9600);
}

void printMenu(int subMenu = -1);

void loop() {
    if(!menuDisplayed) {
        selected = -1;
        printMenu();
    }

    if(millis() - previousRead >= samplingInterval) {
        previousRead = millis();
        readUltrasonic();
        readLDR();
    }

    if(automaticState) {
        analogWrite(redPin, redValue);
        analogWrite(greenPin, greenValue);
        analogWrite(bluePin, blueValue);

        if(ultrasonicValue > ultrasonicThreshold || ldrValue > ldrThreshold) {
            greenValue = 0;
            redValue = 255;
        }
        else {
            greenValue = 255;
            redValue = 0;
        }
    }
    else {
        if(ultrasonicValue > ultrasonicThreshold || ldrValue > ldrThreshold) {
            greenValue = userGreenValue;
            redValue = userRedValue;
            blueValue = userBlueValue;
        }
        else {
            greenValue = 255;
            redValue = 0;
            blueValue = 0;
        }
        analogWrite(redPin, redValue);
        analogWrite(greenPin, greenValue);
        analogWrite(bluePin, blueValue);
    }

    if(selectColor) {
        userRedValue = map(analogRead(userRedPin), 0, 1023, 0, 255);
        userGreenValue = map(analogRead(userGreenPin), 0, 1023, 0, 255);
        userBlueValue = map(analogRead(userBluePin), 0, 1023, 0, 255);

        if(Serial.available() > 0) {
            incomingByte = Serial.read();
            if(incomingByte == 'S') {
                selectColor = false;
                Serial.print("Color saved: RGB = ");
                Serial.print(userRedValue);
                Serial.print(", ");
                Serial.print(userGreenValue);
                Serial.print(", ");
                Serial.println(userBlueValue);
            }
            menuDisplayed = false;
        }
    }

    if(printReadings && millis() - previousRead >= samplingInterval) {
        previousRead = millis();
        clearScreen();

        Serial.println("Input 'Q' to return to main menu");
        Serial.println("Current sensor readings:");
        Serial.print("Ultrasonic: ");
        Serial.println(ultrasonicValue);
        Serial.print("LDR: ");
        Serial.println(ldrValue);
    }

    if (Serial.available() > 0) {

        if(printReadings) {
            incomingByte = Serial.read();
            if(incomingByte == 'Q') {
                printReadings = false;
                menuDisplayed = false;
            }
        }

        if(waitingForInput) {
            // Serial.println("in wainting for input");
            int input = Serial.parseInt();
            switch(option) {
                case 11:
                    if(input >= 1 && input <= 10) {
                        samplingInterval = input * 1000;
                        Serial.println("Sampling interval updated: ");
                        Serial.println(samplingInterval);
                    }
                    else {
                        Serial.println("Invalid input");
                    }
                    break;
                case 12:
                    if(input >= 1 && input <= 100) {
                        ultrasonicThreshold = input;
                        Serial.println("Ultrasonic threshold updated: ");
                        Serial.println(ultrasonicThreshold);
                    }
                    else {
                        Serial.println("Invalid input");
                    }
                    break;
                case 13:
                    if(input >= 1 && input <= 100) {
                        ldrThreshold = input;
                        Serial.println("LDR threshold updated: ");
                        Serial.println(ldrThreshold);
                    }
                    else {
                        Serial.println("Invalid input");
                    }
                    break;
                default:
                    break;
            }

            waitingForInput = false;
            menuDisplayed = false;
        }
        else {
            option = -1;
            option = Serial.parseInt();
            // Serial.println("------------");
            // Serial.println(option);

            if(option != -1 && selected != -1) {
                option = selected * 10 + option; // two digit numbers like D1D2 are going to represent submenus D1.D2
                printMenu(option);
                selected = -1;
            }
            else if (option != -1) {
                selected = option;
                printMenu(option);
            }
            else {
                Serial.println("Invalid option");
                // Serial.println(option);
                // Serial.println(selected);
            }

            delay(30);
        }
    }
}

void printMenu(int subMenu = -1) {
    switch(subMenu) {
        case -1:
            Serial.println("Main menu:");
            Serial.println("1. Sensor Settings");
            Serial.println("2. Reset Logger Data");
            Serial.println("3. System Status");
            Serial.println("4. RGB LED Control");
            Serial.print("\n");
            menuDisplayed = true;
            break;
        case 1:
            Serial.println("1. Sensors Sampling Interval");
            Serial.println("2. Ultrasonic Alert Threshold");
            Serial.println("3. LDR Alert Threshold");
            Serial.println("4. Back");
            Serial.print("\n");
            break;
        case 2:
            Serial.println("Are you sure you want to reset the logger data?");
            Serial.println("1. Yes");
            Serial.println("2. No");
            Serial.print("\n");
            break;
        case 3:
            Serial.println("1. Current Sensor Readings");
            Serial.println("2. Current Sensor Settings");
            Serial.println("3. Display Logged Data");
            Serial.println("4. Back");
            Serial.print("\n");
            break;
        case 4:
            Serial.println("1. Manual Color Control");
            Serial.println("2. Toggle Automatic ON/OFF");
            Serial.println("3. Back");
            Serial.print("\n");
            break;
        case 11:
            Serial.println("Enter a new sampling rate between 1 and 10 seconds: ");
            Serial.print("\n");
            waitingForInput = true;
            break;
        case 12:
            Serial.println("Enter a new ultrasonic threshold between 1 and 100 cm: ");
            Serial.print("\n");
            waitingForInput = true;
            break;
        case 13:
            Serial.println("Enter a new LDR threshold between 1 and 100 lux: ");
            Serial.print("\n");
            waitingForInput = true;
            break;
        case 14:
            Serial.println("Returning to main menu");
            Serial.print("\n");
            menuDisplayed = false;
            break;
        case 21:
            Serial.println("Resetting logger data");
            Serial.print("\n");
            resetData();
        case 22:
            menuDisplayed = false;
            break;
        case 31:
            printReadings = true;
            break;
        case 32:
            Serial.println("Current sensor settings:");
            Serial.print("Sampling interval: ");
            Serial.println(samplingInterval);
            Serial.print("Ultrasonic threshold: ");
            Serial.println(ultrasonicThreshold);
            Serial.print("LDR threshold: ");
            Serial.println(ldrThreshold);
            Serial.print("\n");
            menuDisplayed = false;
            break;
        case 33:
            resetData();
            printLoggedData();
            menuDisplayed = false;
            break;
        case 34:
            menuDisplayed = false;
            break;
        case 41:
            Serial.println("Select color for alerting using the potentiometers and then input 'S' to save the color");
            selectColor = true;
            automaticState = false;
            break;
        case 42:
            Serial.print("Setting automatic state to: ");
            Serial.println(automaticState == false ? "ON" : "OFF");
            automaticState = !automaticState;
            menuDisplayed = false;
            break;
        default:
            Serial.println("Invalid options");
            Serial.print("\n");
            break;
    }
}

void resetData() {
    int nullValue = 0;
    for(int i = 0; i < nrReadings * sizeof(int); i += sizeof(int)) {
        EEPROM.put(firstUltrasonicReading + i, nullValue);
        EEPROM.put(firstLDRReading + i, nullValue);
    }
}

void clearScreen() {
    Serial.print("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
}

void printLoggedData() {
    writeLoggedData();
    Serial.println("Ultrasonic \t|\t LDR");
    for(int i = 0; i < nrReadings * sizeof(int); i += sizeof(int)) {
        int distance, ldr;
        distance = EEPROM.get(firstUltrasonicReading + i, distance);
        ldr = EEPROM.get(firstLDRReading + i, ldr);
        // Serial.print("Ultrasonic: ");
        Serial.print("\t");
        Serial.print(distance);
        Serial.print("\t|\t");
        // Serial.print(", LDR: ");
        Serial.println(ldr);
    }
}

void writeLoggedData() {
    int address = firstUltrasonicReading;
    for(int i = 0; i < nrReadings; i++) {
        EEPROM.put(address, ultrasonicReadings[i]);
        address += sizeof(int);
    }
    address = firstLDRReading;
    for(int i = 0; i < nrReadings; i++) {
        EEPROM.put(address, ldrReadings[i]);
        address += sizeof(int);
    }
}

void readUltrasonic() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    ultrasonicValue = duration*0.034/2;
    
    addUltrasonicReading(ultrasonicValue);
}

void readLDR() {
    ldrValue = analogRead(photocellPin);  
    ldrValue = map(ldrValue, 0, 1023, 0, 255);
    
    addLDRReading(ldrValue);
}

void addUltrasonicReading(int reading) {
    for(int i = 0; i < nrReadings - 1; i++) {
        ultrasonicReadings[i] = ultrasonicReadings[i + 1];
    }
    ultrasonicReadings[nrReadings - 1] = reading;
}
void addLDRReading(int reading) {
    for(int i = 0; i < nrReadings - 1; i++) {
        ldrReadings[i] = ldrReadings[i + 1];
    }
    ldrReadings[nrReadings - 1] = reading;
}