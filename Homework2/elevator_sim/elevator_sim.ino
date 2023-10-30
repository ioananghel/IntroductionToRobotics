const int firstFloorButton = 2;
const int secondFloorButton = 4;
const int thirdFloorButton = 7;
const int buzzerPin = 9;
const int lowVolBuzzerPin = 10;

const int firstFloorLed = 3;
const int secondFloorLed = 5;
const int thirdFloorLed = 6;
const int elevatorLed = 8;

const int closingTime = 1000;
const int openingTime = 1000;
const int movingTime = 2000;
const int waitingTime = 2000;
const int blinkingTime = 300;

volatile bool firstFloorCall = false, secondFloorCall = false, thirdFloorCall = false;
volatile unsigned long firstFloorLastCall = 0, secondFloorLastCall = 0, thirdFloorLastCall = 0;

volatile unsigned long lastDoorOpen = 0;

const int debounceDelay = 500;

volatile int elevatorState = 0;
volatile int elevatorLedState =  1;
// -1 - going down
// 0 - stopped
// 1 - going up
volatile int currentFloor = 1;

void setup() {
  pinMode(firstFloorButton, INPUT_PULLUP);
  pinMode(secondFloorButton, INPUT_PULLUP);
  pinMode(thirdFloorButton, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(lowVolBuzzerPin, OUTPUT);

  pinMode(firstFloorLed, OUTPUT);
  pinMode(secondFloorLed, OUTPUT);
  pinMode(thirdFloorLed, OUTPUT);
  pinMode(elevatorLed, OUTPUT);

  Serial.begin(9600);
}

void loop() {

  firstFloorCall = digitalRead(firstFloorButton);
  Serial.println(firstFloorCall);
  secondFloorCall = digitalRead(secondFloorButton);
  Serial.println(secondFloorCall);
  thirdFloorCall = digitalRead(thirdFloorButton);
  Serial.println(thirdFloorCall);

  digitalWrite(elevatorLed, elevatorLedState);


  if(firstFloorCall) {
    if(millis() - firstFloorLastCall > debounceDelay) {
      Serial.println("First floor call");
      firstFloorLastCall = millis();

      digitalWrite(firstFloorLed, HIGH);

      if(currentFloor == 1) {
        openDoor();
        Serial.println("Door opened");
      }
      else if(elevatorState == 0) {
        elevatorState = 1;

        if(currentFloor == 2) {
          moving(movingTime);
          openDoor();
          currentFloor = 1;
          Serial.println("Moving down");
        }
        else {
          if(currentFloor == 3) {
            moving(movingTime * 2);
            openDoor();
            currentFloor = 1;
            Serial.println("Moving down");
          }
        }
      }

      digitalWrite(firstFloorLed, LOW);
    }
  }

  if(secondFloorCall) {
    if(millis() - secondFloorLastCall > debounceDelay) {
      Serial.println("Second floor call");
      secondFloorLastCall = millis();

      digitalWrite(secondFloorLed, HIGH);

      if(currentFloor == 2) {
        openDoor();
        Serial.println("Door opened");
      }
      else if(elevatorState == 0) {
        elevatorState = 1;

        if(currentFloor == 1) {
          moving(movingTime);
          openDoor();
          currentFloor = 2;
          Serial.println("Moving up");
        }
        else {
          if(currentFloor == 3) {
            moving(movingTime);
            openDoor();
            currentFloor = 2;
            Serial.println("Moving down");
          }
        }
      }

      digitalWrite(secondFloorLed, LOW);
    }
  }

  if(thirdFloorCall) {
    if(millis() - thirdFloorLastCall > debounceDelay) {
      Serial.println("Third floor call");
      thirdFloorLastCall = millis();

      digitalWrite(thirdFloorLed, HIGH);

      if(currentFloor == 3) {
        openDoor();
        Serial.println("Door opened");
      }
      else if(elevatorState == 0) {
        elevatorState = 1;

        if(currentFloor == 1) {
          moving(movingTime * 2);
          openDoor();
          currentFloor = 3;
          Serial.println("Moving up");
        }
        else {
          if(currentFloor == 2) {
            moving(movingTime);
            openDoor();
            currentFloor = 3;
            Serial.println("Moving up");
          }
        }
      }

      digitalWrite(thirdFloorLed, LOW);
    }
  }
}

void openDoor() {
  lastDoorOpen = millis();
  elevatorState = 0;

  tone(buzzerPin, 2000, openingTime / 2);
  createDelay(openingTime / 2);
  tone(buzzerPin, 1000, openingTime / 2);
  createDelay(openingTime / 2);

  createDelay(waitingTime);

  closeDoor();

  noTone(buzzerPin);
}

void closeDoor() {
  tone(buzzerPin, 1000, closingTime / 2);
  createDelay(closingTime / 2);
  tone(buzzerPin, 2000, closingTime / 2);
  createDelay(closingTime / 2);

  noTone(buzzerPin);
}

void moving(int time) {
  tone(lowVolBuzzerPin, 600, time);
  createDelay(time);

  Serial.println("Am ajuns");

  noTone(buzzerPin);
}

void createDelay(int time) {
  unsigned long start = millis();

  while(millis() - start < time) {
    elevatorLedState = ((millis() - start) / blinkingTime) % 2;

    digitalWrite(elevatorLed, elevatorLedState);
  }
}