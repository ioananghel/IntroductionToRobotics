int incomingByte = 0;
bool menuDisplayed = false, waitingForInput = false;
int selected = -1, option = -1; // 1 = sensor settings, 2 = reset logger data, 3 = system status, 4 = RGB LED control

int samplingInterval = 10000; // 10 seconds
int ultrasonicThreshold = 100; // 100 cm
int ldrThreshold = 100; // 100 lux

void setup() {
    Serial.begin(9600);
}

void printMenu(int subMenu = -1);

void loop() {
    if(!menuDisplayed) {
        selected = -1;
        printMenu();
    }

    if (Serial.available() > 0) {

        if(waitingForInput) {
            Serial.println("in wainting for input");
            int input = Serial.parseInt();
            switch(option) {
                case 11:
                    if(input >= 1 && input <= 10) {
                        samplingInterval = input * 1000;
                        Serial.println("Sampling interval updated");
                        Serial.println(samplingInterval);
                    }
                    else {
                        Serial.println("Invalid input");
                    }
                    break;
                case 12:
                    if(input >= 1 && input <= 100) {
                        ultrasonicThreshold = input;
                        Serial.println("Ultrasonic threshold updated");
                        Serial.println(ultrasonicThreshold);
                    }
                    else {
                        Serial.println("Invalid input");
                    }
                    break;
                case 13:
                    if(input >= 1 && input <= 100) {
                        ldrThreshold = input;
                        Serial.println("LDR threshold updated");
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
            Serial.println("------------");
            Serial.println(option);

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
                Serial.println(option);
                Serial.println(selected);
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
        default:
            Serial.println("Invalid options");
            Serial.println(subMenu);
            Serial.print("\n");
            break;
    }
}

void resetData() {
    
}